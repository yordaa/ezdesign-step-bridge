// Created on: 2025
// Created by: EzDesign to STEP Converter
// Copyright (c) 2025 Yang Song. All rights reserved.
//
// This file is proprietary and confidential. Unauthorized copying, modification,
// distribution, or use of this file, via any medium, is strictly prohibited.
// See LICENSE file in this directory for terms and conditions.

#include "EzDesignJsonReader.hxx"

#include <Standard_Failure.hxx>
#include <Standard_ErrorHandler.hxx>
#include <fstream>
#include <sstream>

// Include nlohmann/json
#ifdef nlohmann_json_INCLUDE_DIR
  #include <nlohmann/json.hpp>
#else
  #include <nlohmann/json.hpp>
#endif

using json = nlohmann::json;

//=======================================================================
// function : EzDesignJsonReader
// purpose  : Constructor
//=======================================================================
EzDesignJsonReader::EzDesignJsonReader()
: myIsDone(Standard_False)
{
}

//=======================================================================
// function : ~EzDesignJsonReader
// purpose  : Destructor
//=======================================================================
EzDesignJsonReader::~EzDesignJsonReader()
{
}

//=======================================================================
// function : ReadFile
// purpose  : Read JSON file and parse topology/geometry data
//=======================================================================
Standard_Boolean EzDesignJsonReader::ReadFile(const TCollection_AsciiString& theFileName)
{
  myIsDone = Standard_False;
  myErrors.clear();
  myVertices.clear();
  myEdges.clear();
  myHalfEdges.clear();
  myLoops.clear();
  myFaces.clear();
  myShells.clear();

  try {
    OCC_CATCH_SIGNALS

    // Read JSON file
    std::ifstream file(theFileName.ToCString());
    if (!file.is_open()) {
      addError("Cannot open file: " + std::string(theFileName.ToCString()));
      return Standard_False;
    }

    json jsonData;
    file >> jsonData;
    file.close();

    // Extract data from "subd" entry if present (data.db.subd structure)
    json dataToParse = jsonData;
    if (jsonData.contains("data") && jsonData["data"].is_object() &&
        jsonData["data"].contains("db") && jsonData["data"]["db"].is_object() &&
        jsonData["data"]["db"].contains("subd") && jsonData["data"]["db"]["subd"].is_object()) {
      dataToParse = jsonData["data"]["db"]["subd"];
    }

    // Parse JSON
    if (!parseJson(dataToParse)) {
      return Standard_False;
    }

    // Validate parsed data
    if (!Validate()) {
      return Standard_False;
    }

    myIsDone = Standard_True;
    return Standard_True;
  }
  catch (const json::parse_error& e) {
    std::ostringstream oss;
    oss << "JSON parse error at byte " << e.byte << ": " << e.what();
    addError(oss.str());
    return Standard_False;
  }
  catch (const Standard_Failure& e) {
    addError(std::string("OCCT exception: ") + e.GetMessageString());
    return Standard_False;
  }
  catch (const std::exception& e) {
    addError(std::string("Standard exception: ") + e.what());
    return Standard_False;
  }
  catch (...) {
    addError("Unknown exception occurred");
    return Standard_False;
  }
}

//=======================================================================
// function : parseJson
// purpose  : Parse JSON object into topology/geometry structures
//=======================================================================
Standard_Boolean EzDesignJsonReader::parseJson(const json& theJson)
{
  if (!theJson.is_object()) {
    addError("JSON root must be an object");
    return Standard_False;
  }

  // Iterate through all objects in JSON
  for (auto& [key, value] : theJson.items()) {
    if (!value.is_object()) {
      continue;
    }

    int id = 0;
    try {
      id = std::stoi(key);
    }
    catch (...) {
      addError("Invalid ID format: " + key);
      continue;
    }

    // Check type field
    if (!value.contains("type") || !value["type"].is_string()) {
      addError("Object " + key + " missing 'type' field");
      continue;
    }

    std::string type = value["type"].get<std::string>();

    // Parse based on type
    if (type == "Vertex") {
      if (!parseVertex(value, id)) {
        addError("Failed to parse vertex " + key);
      }
    }
    else if (type == "Edge") {
      if (!parseEdge(value, id)) {
        addError("Failed to parse edge " + key);
      }
    }
    else if (type == "HalfEdge") {
      if (!parseHalfEdge(value, id)) {
        addError("Failed to parse half-edge " + key);
      }
    }
    else if (type == "Loop") {
      if (!parseLoop(value, id)) {
        addError("Failed to parse loop " + key);
      }
    }
    else if (type == "Face") {
      if (!parseFace(value, id)) {
        addError("Failed to parse face " + key);
      }
    }
    else if (type == "Shell") {
      if (!parseShell(value, id)) {
        addError("Failed to parse shell " + key);
      }
    }
    else if (type == "Body") {
      if (!parseBody(value, id)) {
        addError("Failed to parse body " + key);
      }
    }
  }

  return Standard_True;
}

//=======================================================================
// function : parseVertex
// purpose  : Parse vertex from JSON
//=======================================================================
Standard_Boolean EzDesignJsonReader::parseVertex(const json& theJson, int theId)
{
  EzVertex vertex;
  vertex.id = theId;

  // Parse position
  if (theJson.contains("position") && theJson["position"].is_array() && theJson["position"].size() == 3) {
    vertex.position[0] = theJson["position"][0].get<double>();
    vertex.position[1] = theJson["position"][1].get<double>();
    vertex.position[2] = theJson["position"][2].get<double>();
  }
  else {
    addError("Vertex " + std::to_string(theId) + " missing or invalid 'position' field");
    return Standard_False;
  }

  // Parse half_edge (optional)
  if (theJson.contains("half_edge")) {
    vertex.half_edge_id = theJson["half_edge"].get<int>();
  }
  else {
    vertex.half_edge_id = 0;
  }

  myVertices[theId] = vertex;
  return Standard_True;
}

//=======================================================================
// function : parseEdge
// purpose  : Parse edge from JSON
//=======================================================================
Standard_Boolean EzDesignJsonReader::parseEdge(const json& theJson, int theId)
{
  EzEdge edge;
  edge.id = theId;

  // Parse half_edge
  if (theJson.contains("half_edge")) {
    edge.half_edge_id = theJson["half_edge"].get<int>();
  }
  else {
    addError("Edge " + std::to_string(theId) + " missing 'half_edge' field");
    return Standard_False;
  }

  myEdges[theId] = edge;
  return Standard_True;
}

//=======================================================================
// function : parseHalfEdge
// purpose  : Parse half-edge from JSON
//=======================================================================
Standard_Boolean EzDesignJsonReader::parseHalfEdge(const json& theJson, int theId)
{
  EzHalfEdge halfEdge;
  halfEdge.id = theId;

  // Parse required fields
  if (theJson.contains("edge")) {
    halfEdge.edge_id = theJson["edge"].get<int>();
  }
  else {
    addError("HalfEdge " + std::to_string(theId) + " missing 'edge' field");
    return Standard_False;
  }

  if (theJson.contains("vertex")) {
    halfEdge.vertex_id = theJson["vertex"].get<int>();
  }
  else {
    addError("HalfEdge " + std::to_string(theId) + " missing 'vertex' field");
    return Standard_False;
  }

  if (theJson.contains("loop")) {
    halfEdge.loop_id = theJson["loop"].get<int>();
  }
  else {
    halfEdge.loop_id = 0;
  }

  if (theJson.contains("next")) {
    halfEdge.next_id = theJson["next"].get<int>();
  }
  else {
    addError("HalfEdge " + std::to_string(theId) + " missing 'next' field");
    return Standard_False;
  }

  if (theJson.contains("previous")) {
    halfEdge.previous_id = theJson["previous"].get<int>();
  }
  else {
    halfEdge.previous_id = 0;
  }

  if (theJson.contains("opposite")) {
    halfEdge.opposite_id = theJson["opposite"].get<int>();
  }
  else {
    halfEdge.opposite_id = 0;
  }

  // Parse curve_data (optional)
  if (theJson.contains("curve_data")) {
    if (!parseCurveData(theJson["curve_data"], halfEdge.curve_data)) {
      addError("HalfEdge " + std::to_string(theId) + " has invalid 'curve_data'");
      // Don't fail - curve_data is optional
    }
  }

  myHalfEdges[theId] = halfEdge;
  return Standard_True;
}

//=======================================================================
// function : parseLoop
// purpose  : Parse loop from JSON
//=======================================================================
Standard_Boolean EzDesignJsonReader::parseLoop(const json& theJson, int theId)
{
  EzLoop loop;
  loop.id = theId;

  if (theJson.contains("face")) {
    loop.face_id = theJson["face"].get<int>();
  }
  else {
    addError("Loop " + std::to_string(theId) + " missing 'face' field");
    return Standard_False;
  }

  if (theJson.contains("half_edge")) {
    loop.half_edge_id = theJson["half_edge"].get<int>();
  }
  else {
    addError("Loop " + std::to_string(theId) + " missing 'half_edge' field");
    return Standard_False;
  }

  myLoops[theId] = loop;
  return Standard_True;
}

//=======================================================================
// function : parseFace
// purpose  : Parse face from JSON
//=======================================================================
Standard_Boolean EzDesignJsonReader::parseFace(const json& theJson, int theId)
{
  EzFace face;
  face.id = theId;

  if (theJson.contains("shell")) {
    face.shell_id = theJson["shell"].get<int>();
  }
  else {
    addError("Face " + std::to_string(theId) + " missing 'shell' field");
    return Standard_False;
  }

  // Parse loops
  if (theJson.contains("loops") && theJson["loops"].is_array()) {
    for (const auto& loopId : theJson["loops"]) {
      face.loop_ids.push_back(loopId.get<int>());
    }
  }
  else {
    addError("Face " + std::to_string(theId) + " missing or invalid 'loops' field");
    return Standard_False;
  }

  // Parse surface_data
  if (theJson.contains("surface_data")) {
    if (!parseSurfaceData(theJson["surface_data"], face.surface_data)) {
      addError("Face " + std::to_string(theId) + " has invalid 'surface_data'");
      return Standard_False;
    }
  }
  else {
    addError("Face " + std::to_string(theId) + " missing 'surface_data' field");
    return Standard_False;
  }

  // Parse is_surface_normal_same
  if (theJson.contains("is_surface_normal_same")) {
    face.is_surface_normal_same = theJson["is_surface_normal_same"].get<int>() != 0;
  }
  else {
    face.is_surface_normal_same = true;  // Default
  }

  // Note: is_normal_outward is ignored per design decision

  myFaces[theId] = face;
  return Standard_True;
}

//=======================================================================
// function : parseShell
// purpose  : Parse shell from JSON
//=======================================================================
Standard_Boolean EzDesignJsonReader::parseShell(const json& theJson, int theId)
{
  EzShell shell;
  shell.id = theId;

  if (theJson.contains("body")) {
    shell.body_id = theJson["body"].get<int>();
  }
  else {
    addError("Shell " + std::to_string(theId) + " missing 'body' field");
    return Standard_False;
  }

  // Parse faces
  if (theJson.contains("faces") && theJson["faces"].is_array()) {
    for (const auto& faceId : theJson["faces"]) {
      shell.face_ids.push_back(faceId.get<int>());
    }
  }
  else {
    addError("Shell " + std::to_string(theId) + " missing or invalid 'faces' field");
    return Standard_False;
  }

  myShells[theId] = shell;
  return Standard_True;
}

//=======================================================================
// function : parseBody
// purpose  : Parse body from JSON
//=======================================================================
Standard_Boolean EzDesignJsonReader::parseBody(const json& theJson, int theId)
{
  myBody.id = theId;

  // Parse shells
  if (theJson.contains("shells") && theJson["shells"].is_array()) {
    for (const auto& shellId : theJson["shells"]) {
      myBody.shell_ids.push_back(shellId.get<int>());
    }
  }
  else {
    addError("Body " + std::to_string(theId) + " missing or invalid 'shells' field");
    return Standard_False;
  }

  return Standard_True;
}

//=======================================================================
// function : parseCurveData
// purpose  : Parse curve data from JSON
//=======================================================================
Standard_Boolean EzDesignJsonReader::parseCurveData(const json& theJson, EzCurveData& theCurveData)
{
  // Parse control_points
  if (theJson.contains("control_points")) {
    if (!parseControlPoints(theJson["control_points"], theCurveData.control_points)) {
      return Standard_False;
    }
  }
  else {
    return Standard_False;
  }

  // Parse basis
  if (theJson.contains("basis")) {
    if (!parseBasis(theJson["basis"], theCurveData.basis)) {
      return Standard_False;
    }
  }
  else {
    return Standard_False;
  }

  return Standard_True;
}

//=======================================================================
// function : parseSurfaceData
// purpose  : Parse surface data from JSON
//=======================================================================
Standard_Boolean EzDesignJsonReader::parseSurfaceData(const json& theJson, EzSurfaceData& theSurfaceData)
{
  // Parse control_points
  if (theJson.contains("control_points")) {
    if (!parseControlPoints(theJson["control_points"], theSurfaceData.control_points)) {
      return Standard_False;
    }
  }
  else {
    return Standard_False;
  }

  // Parse u_basis
  if (theJson.contains("u_basis")) {
    if (!parseBasis(theJson["u_basis"], theSurfaceData.u_basis)) {
      return Standard_False;
    }
  }
  else {
    return Standard_False;
  }

  // Parse v_basis
  if (theJson.contains("v_basis")) {
    if (!parseBasis(theJson["v_basis"], theSurfaceData.v_basis)) {
      return Standard_False;
    }
  }
  else {
    return Standard_False;
  }

  // Note: is_normal_outward and trimming_loop are ignored per design decision

  return Standard_True;
}

//=======================================================================
// function : parseControlPoints
// purpose  : Parse control points from JSON
//=======================================================================
Standard_Boolean EzDesignJsonReader::parseControlPoints(const json& theJson, EzControlPoints& theControlPoints)
{
  // Parse data array
  if (theJson.contains("data") && theJson["data"].is_array()) {
    for (const auto& val : theJson["data"]) {
      theControlPoints.data.push_back(val.get<double>());
    }
  }
  else {
    return Standard_False;
  }

  // Parse dimension
  if (theJson.contains("dimension")) {
    theControlPoints.dimension = theJson["dimension"].get<int>();
  }
  else {
    return Standard_False;
  }

  // Parse number_u_points
  if (theJson.contains("number_u_points")) {
    theControlPoints.number_u_points = theJson["number_u_points"].get<int>();
  }
  else {
    return Standard_False;
  }

  // Parse number_v_points
  if (theJson.contains("number_v_points")) {
    theControlPoints.number_v_points = theJson["number_v_points"].get<int>();
  }
  else {
    return Standard_False;
  }

  // Parse is_rational
  if (theJson.contains("is_rational")) {
    theControlPoints.is_rational = theJson["is_rational"].get<bool>();
  }
  else {
    theControlPoints.is_rational = false;  // Default
  }

  return Standard_True;
}

//=======================================================================
// function : parseBasis
// purpose  : Parse basis from JSON
//=======================================================================
Standard_Boolean EzDesignJsonReader::parseBasis(const json& theJson, EzBasis& theBasis)
{
  // Parse degree
  if (theJson.contains("degree")) {
    theBasis.degree = theJson["degree"].get<int>();
  }
  else {
    return Standard_False;
  }

  // Parse knot_vector
  if (theJson.contains("knot_vector") && theJson["knot_vector"].is_array()) {
    for (const auto& val : theJson["knot_vector"]) {
      theBasis.knot_vector.push_back(val.get<double>());
    }
  }
  else {
    return Standard_False;
  }

  // Parse bounds
  if (theJson.contains("bounds")) {
    if (theJson["bounds"].contains("minimum")) {
      theBasis.bounds.minimum = theJson["bounds"]["minimum"].get<double>();
    }
    if (theJson["bounds"].contains("maximum")) {
      theBasis.bounds.maximum = theJson["bounds"]["maximum"].get<double>();
    }
  }

  return Standard_True;
}

//=======================================================================
// function : GetBody
// purpose  : Get parsed body
//=======================================================================
const EzBody& EzDesignJsonReader::GetBody() const
{
  return myBody;
}

//=======================================================================
// function : GetVertex
// purpose  : Get vertex by ID
//=======================================================================
const EzVertex& EzDesignJsonReader::GetVertex(int theId) const
{
  static EzVertex empty;
  auto it = myVertices.find(theId);
  if (it != myVertices.end()) {
    return it->second;
  }
  return empty;
}

//=======================================================================
// function : GetEdge
// purpose  : Get edge by ID
//=======================================================================
const EzEdge& EzDesignJsonReader::GetEdge(int theId) const
{
  static EzEdge empty;
  auto it = myEdges.find(theId);
  if (it != myEdges.end()) {
    return it->second;
  }
  return empty;
}

//=======================================================================
// function : GetHalfEdge
// purpose  : Get half-edge by ID
//=======================================================================
const EzHalfEdge& EzDesignJsonReader::GetHalfEdge(int theId) const
{
  static EzHalfEdge empty;
  auto it = myHalfEdges.find(theId);
  if (it != myHalfEdges.end()) {
    return it->second;
  }
  return empty;
}

//=======================================================================
// function : GetLoop
// purpose  : Get loop by ID
//=======================================================================
const EzLoop& EzDesignJsonReader::GetLoop(int theId) const
{
  static EzLoop empty;
  auto it = myLoops.find(theId);
  if (it != myLoops.end()) {
    return it->second;
  }
  return empty;
}

//=======================================================================
// function : GetFace
// purpose  : Get face by ID
//=======================================================================
const EzFace& EzDesignJsonReader::GetFace(int theId) const
{
  static EzFace empty;
  auto it = myFaces.find(theId);
  if (it != myFaces.end()) {
    return it->second;
  }
  return empty;
}

//=======================================================================
// function : GetShell
// purpose  : Get shell by ID
//=======================================================================
const EzShell& EzDesignJsonReader::GetShell(int theId) const
{
  static EzShell empty;
  auto it = myShells.find(theId);
  if (it != myShells.end()) {
    return it->second;
  }
  return empty;
}

//=======================================================================
// function : Validate
// purpose  : Validate parsed data
//=======================================================================
Standard_Boolean EzDesignJsonReader::Validate() const
{
  return validateTopology();
}

//=======================================================================
// function : validateTopology
// purpose  : Validate topology structure
//=======================================================================
Standard_Boolean EzDesignJsonReader::validateTopology() const
{
  // Create mutable copy for error collection
  std::vector<std::string> errors;

  // Validate that all referenced IDs exist
  // Check half-edge chains are closed
  // Check edge-half-edge relationships
  // This is a simplified validation - can be expanded

  // Check that all half-edges have valid next pointers
  for (const auto& [id, halfEdge] : myHalfEdges) {
    if (halfEdge.next_id != 0) {
      if (myHalfEdges.find(halfEdge.next_id) == myHalfEdges.end()) {
        errors.push_back("HalfEdge " + std::to_string(id) + " references non-existent next half-edge " + std::to_string(halfEdge.next_id));
      }
    }
  }

  // Check that all loops reference valid half-edges
  for (const auto& [id, loop] : myLoops) {
    if (myHalfEdges.find(loop.half_edge_id) == myHalfEdges.end()) {
      errors.push_back("Loop " + std::to_string(id) + " references non-existent half-edge " + std::to_string(loop.half_edge_id));
    }
  }

  // Check that all faces reference valid loops
  for (const auto& [id, face] : myFaces) {
    for (int loopId : face.loop_ids) {
      if (myLoops.find(loopId) == myLoops.end()) {
        errors.push_back("Face " + std::to_string(id) + " references non-existent loop " + std::to_string(loopId));
      }
    }
  }

  // Check that all shells reference valid faces
  for (const auto& [id, shell] : myShells) {
    for (int faceId : shell.face_ids) {
      if (myFaces.find(faceId) == myFaces.end()) {
        errors.push_back("Shell " + std::to_string(id) + " references non-existent face " + std::to_string(faceId));
      }
    }
  }

  // Check that body references valid shells
  for (int shellId : myBody.shell_ids) {
    if (myShells.find(shellId) == myShells.end()) {
      errors.push_back("Body references non-existent shell " + std::to_string(shellId));
    }
  }

  // Copy errors to member (need const_cast for const method)
  const_cast<EzDesignJsonReader*>(this)->myErrors.insert(
    const_cast<EzDesignJsonReader*>(this)->myErrors.end(),
    errors.begin(), errors.end());

  return errors.empty() && myErrors.empty();
}

//=======================================================================
// function : GetErrors
// purpose  : Get validation errors
//=======================================================================
const std::vector<std::string>& EzDesignJsonReader::GetErrors() const
{
  return myErrors;
}

//=======================================================================
// function : IsDone
// purpose  : Check if reading was successful
//=======================================================================
Standard_Boolean EzDesignJsonReader::IsDone() const
{
  return myIsDone;
}

//=======================================================================
// function : addError
// purpose  : Add validation error
//=======================================================================
void EzDesignJsonReader::addError(const std::string& theError)
{
  myErrors.push_back(theError);
}

