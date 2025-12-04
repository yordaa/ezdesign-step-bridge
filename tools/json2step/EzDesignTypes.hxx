// Created on: 2025
// Created by: OCCT json2step tool
// Copyright (c) 2025 OPEN CASCADE SAS
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.
//
// Alternatively, this file may be used under the terms of Open CASCADE
// commercial license or contractual agreement.

#ifndef _EzDesignTypes_HeaderFile
#define _EzDesignTypes_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <vector>
#include <map>

//! Data structures matching ezdesign JSON format

//! Control points for B-spline geometry
struct EzControlPoints
{
  std::vector<double> data;  // Flattened array
  int dimension;  // 2 or 3
  int number_u_points;  // For surfaces: U direction, for curves: always 1
  int number_v_points;  // For surfaces: V direction, for curves: number of control points
  bool is_rational;
};

//! B-spline basis information
struct EzBasis
{
  int degree;
  std::vector<double> knot_vector;
  struct {
    double minimum;
    double maximum;
  } bounds;
};

//! 2D B-spline curve data (parametric on face surface)
struct EzCurveData
{
  EzControlPoints control_points;
  EzBasis basis;
};

//! 3D B-spline surface data
struct EzSurfaceData
{
  EzControlPoints control_points;
  EzBasis u_basis;
  EzBasis v_basis;
  // Note: is_normal_outward and trimming_loop fields in JSON are ignored
  // Face boundaries come from Loop topology elements, not trimming_loop
};

//! Vertex topology element
struct EzVertex
{
  int id;
  double position[3];
  int half_edge_id;
};

//! Edge topology element
struct EzEdge
{
  int id;
  int half_edge_id;  // One of the two half-edges
};

//! HalfEdge topology element
struct EzHalfEdge
{
  int id;
  int edge_id;
  int vertex_id;  // End vertex
  int loop_id;
  int next_id;
  int previous_id;
  int opposite_id;
  EzCurveData curve_data;  // 2D B-spline (optional, may be empty)
};

//! Loop topology element
struct EzLoop
{
  int id;
  int face_id;
  int half_edge_id;  // Starting half-edge
};

//! Face topology element
struct EzFace
{
  int id;
  int shell_id;
  std::vector<int> loop_ids;
  EzSurfaceData surface_data;  // 3D B-spline surface
  bool is_surface_normal_same;  // If true: surface normal == face outward normal
                                 // If false: surface normal opposite to face outward normal
                                 // Affects UV domain orientation: true=CCW in UV, false=CW in UV (but CCW for face)
  // Note: is_normal_outward field in JSON is ignored - not used in conversion
};

//! Shell topology element
struct EzShell
{
  int id;
  int body_id;
  std::vector<int> face_ids;
};

//! Body topology element
struct EzBody
{
  int id;
  std::vector<int> shell_ids;
};

#endif // _EzDesignTypes_HeaderFile

