// Created on: 2025
// Created by: EzDesign to STEP Converter
// Copyright (c) 2025 Yang Song. All rights reserved.
//
// This file is proprietary and confidential. Unauthorized copying, modification,
// distribution, or use of this file, via any medium, is strictly prohibited.
// See LICENSE file in this directory for terms and conditions.

#ifndef _EzDesignJsonReader_HeaderFile
#define _EzDesignJsonReader_HeaderFile

#include "EzDesignTypes.hxx"
#include <vector>
#include <map>
#include <string>
#include <filesystem>

// Include nlohmann/json - it's header-only so safe to include in header
// The include path is set by CMakeLists.txt
#include <nlohmann/json.hpp>

//! JSON reader for ezdesign format
//! Parses JSON file and populates C++ data structures
class EzDesignJsonReader
{
public:
  //! Constructor
  EzDesignJsonReader();

  //! Destructor
  ~EzDesignJsonReader();

  //! Read JSON file and parse topology/geometry data
  bool ReadFile(const std::filesystem::path& theFileName);

  //! Get parsed body
  const EzBody& GetBody() const;

  //! Get vertex by ID
  const EzVertex& GetVertex(int theId) const;

  //! Get edge by ID
  const EzEdge& GetEdge(int theId) const;

  //! Get half-edge by ID
  const EzHalfEdge& GetHalfEdge(int theId) const;

  //! Get loop by ID
  const EzLoop& GetLoop(int theId) const;

  //! Get face by ID
  const EzFace& GetFace(int theId) const;

  //! Get shell by ID
  const EzShell& GetShell(int theId) const;

  //! Get validation errors
  const std::vector<std::string>& GetErrors() const;

private:
  //! Parse JSON object into topology/geometry structures
  bool parseJson(const nlohmann::json& theJson);

  //! Parse vertex from JSON
  bool parseVertex(const nlohmann::json& theJson, int theId);

  //! Parse edge from JSON
  bool parseEdge(const nlohmann::json& theJson, int theId);

  //! Parse half-edge from JSON
  bool parseHalfEdge(const nlohmann::json& theJson, int theId);

  //! Parse loop from JSON
  bool parseLoop(const nlohmann::json& theJson, int theId);

  //! Parse face from JSON
  bool parseFace(const nlohmann::json& theJson, int theId);

  //! Parse shell from JSON
  bool parseShell(const nlohmann::json& theJson, int theId);

  //! Parse body from JSON
  bool parseBody(const nlohmann::json& theJson, int theId);

  //! Parse curve data from JSON
  bool parseCurveData(const nlohmann::json& theJson, EzCurveData& theCurveData);

  //! Parse surface data from JSON
  bool parseSurfaceData(const nlohmann::json& theJson, EzSurfaceData& theSurfaceData);

  //! Parse control points from JSON
  bool parseControlPoints(const nlohmann::json& theJson, EzControlPoints& theControlPoints);

  //! Parse basis from JSON
  bool parseBasis(const nlohmann::json& theJson, EzBasis& theBasis);

  //! Add validation error
  void addError(const std::string& theError);

  //! Validate topology structure
  bool validateTopology();

private:
  EzBody myBody;
  std::map<int, EzVertex> myVertices;
  std::map<int, EzEdge> myEdges;
  std::map<int, EzHalfEdge> myHalfEdges;
  std::map<int, EzLoop> myLoops;
  std::map<int, EzFace> myFaces;
  std::map<int, EzShell> myShells;
  std::vector<std::string> myErrors;
};

#endif // _EzDesignJsonReader_HeaderFile
