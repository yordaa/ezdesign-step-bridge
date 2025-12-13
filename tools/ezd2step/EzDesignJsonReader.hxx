// Created on: 2025
// Created by: EzDesign to STEP Converter
// Copyright (c) 2025 Yang Song. All rights reserved.
//
// This file is proprietary and confidential. Unauthorized copying, modification,
// distribution, or use of this file, via any medium, is strictly prohibited.
// See LICENSE file in this directory for terms and conditions.

#ifndef _EzDesignJsonReader_HeaderFile
#define _EzDesignJsonReader_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>
#include <TCollection_AsciiString.hxx>
#include "EzDesignTypes.hxx"
#include <vector>
#include <map>
#include <string>

// Include nlohmann/json - it's header-only so safe to include in header
// The include path is set by CMakeLists.txt
#include <nlohmann/json.hpp>

//! JSON reader for ezdesign format
//! Parses JSON file and populates C++ data structures
class EzDesignJsonReader
{
public:
  DEFINE_STANDARD_ALLOC

  //! Constructor
  Standard_EXPORT EzDesignJsonReader();

  //! Destructor
  Standard_EXPORT ~EzDesignJsonReader();

  //! Read JSON file and parse topology/geometry data
  Standard_EXPORT Standard_Boolean ReadFile(const TCollection_AsciiString& theFileName);

  //! Get parsed body
  Standard_EXPORT const EzBody& GetBody() const;

  //! Get vertex by ID
  Standard_EXPORT const EzVertex& GetVertex(int theId) const;

  //! Get edge by ID
  Standard_EXPORT const EzEdge& GetEdge(int theId) const;

  //! Get half-edge by ID
  Standard_EXPORT const EzHalfEdge& GetHalfEdge(int theId) const;

  //! Get loop by ID
  Standard_EXPORT const EzLoop& GetLoop(int theId) const;

  //! Get face by ID
  Standard_EXPORT const EzFace& GetFace(int theId) const;

  //! Get shell by ID
  Standard_EXPORT const EzShell& GetShell(int theId) const;

  //! Validate parsed data
  Standard_EXPORT Standard_Boolean Validate() const;

  //! Get validation errors
  Standard_EXPORT const std::vector<std::string>& GetErrors() const;

  //! Check if reading was successful
  Standard_EXPORT Standard_Boolean IsDone() const;

private:
  //! Parse JSON object into topology/geometry structures
  Standard_Boolean parseJson(const nlohmann::json& theJson);

  //! Parse vertex from JSON
  Standard_Boolean parseVertex(const nlohmann::json& theJson, int theId);

  //! Parse edge from JSON
  Standard_Boolean parseEdge(const nlohmann::json& theJson, int theId);

  //! Parse half-edge from JSON
  Standard_Boolean parseHalfEdge(const nlohmann::json& theJson, int theId);

  //! Parse loop from JSON
  Standard_Boolean parseLoop(const nlohmann::json& theJson, int theId);

  //! Parse face from JSON
  Standard_Boolean parseFace(const nlohmann::json& theJson, int theId);

  //! Parse shell from JSON
  Standard_Boolean parseShell(const nlohmann::json& theJson, int theId);

  //! Parse body from JSON
  Standard_Boolean parseBody(const nlohmann::json& theJson, int theId);

  //! Parse curve data from JSON
  Standard_Boolean parseCurveData(const nlohmann::json& theJson, EzCurveData& theCurveData);

  //! Parse surface data from JSON
  Standard_Boolean parseSurfaceData(const nlohmann::json& theJson, EzSurfaceData& theSurfaceData);

  //! Parse control points from JSON
  Standard_Boolean parseControlPoints(const nlohmann::json& theJson, EzControlPoints& theControlPoints);

  //! Parse basis from JSON
  Standard_Boolean parseBasis(const nlohmann::json& theJson, EzBasis& theBasis);

  //! Add validation error
  void addError(const std::string& theError);

  //! Validate topology structure
  Standard_Boolean validateTopology() const;

private:
  EzBody myBody;
  std::map<int, EzVertex> myVertices;
  std::map<int, EzEdge> myEdges;
  std::map<int, EzHalfEdge> myHalfEdges;
  std::map<int, EzLoop> myLoops;
  std::map<int, EzFace> myFaces;
  std::map<int, EzShell> myShells;
  std::vector<std::string> myErrors;
  Standard_Boolean myIsDone;
};

#endif // _EzDesignJsonReader_HeaderFile

