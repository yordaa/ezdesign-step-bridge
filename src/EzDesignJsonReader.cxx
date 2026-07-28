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
  myErrors.clear();
  myVertices.clear();
  myEdges.clear();
  myHalfEdges.clear();
  myLoops.clear();
  myFaces.clear();
  myShells.clear();
  myBody = EzBody();

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

    // Check document version (must be >= 0.1.0)
    if (jsonData.contains("metadata") && jsonData["metadata"].is_object() &&
        jsonData["metadata"].contains("version") && jsonData["metadata"]["version"].is_string()) {
      std::string versionStr = jsonData["metadata"]["version"].get<std::string>();
      // Parse version string (format: major.minor.patch or major.minor)
      // Extract major and minor version numbers
      size_t firstDot = versionStr.find('.');
      if (firstDot != std::string::npos) {
        int major = 0, minor = 0;
        try {
          major = std::stoi(versionStr.substr(0, firstDot));
          size_t secondDot = versionStr.find('.', firstDot + 1);
          if (secondDot != std::string::npos) {
            minor = std::stoi(versionStr.substr(firstDot + 1, secondDot - firstDot - 1));
          }
          else {
            minor = std::stoi(versionStr.substr(firstDot + 1));
          }
          // Reject if version < 0.1.0
          if (major == 0 && minor < 1) {
            addError("Document version " + versionStr + " is too old. Minimum required version is 0.1.0");
            return Standard_False;
          }
        }
        catch (...) {
          // If version parsing fails, assume it's invalid and reject
          addError("Invalid document version format: " + versionStr + ". Minimum required version is 0.1.0");
          return Standard_False;
        }
      }
      else {
        // Version string doesn't contain dots, assume invalid
        addError("Invalid document version format: " + versionStr + ". Minimum required version is 0.1.0");
        return Standard_False;
      }
    }

    // Extract data from "entities" entry (data.db.entities structure)
    json dataToParse = jsonData;
    if (jsonData.contains("data") && jsonData["data"].is_object() &&
        jsonData["data"].contains("db") && jsonData["data"]["db"].is_object() &&
        jsonData["data"]["db"].contains("entities") && jsonData["data"]["db"]["entities"].is_object()) {
      dataToParse = jsonData["data"]["db"]["entities"];
    }
    else {
      addError("Invalid document structure: missing 'data.db.entities' field");
      return Standard_False;
    }

    // Parse JSON
    if (!parseJson(dataToParse)) {
      return Standard_False;
    }

    // Validate parsed data
    if (!validateTopology()) {
      return Standard_False;
    }

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

    // Parse based on type (handle both base and subdivision entity types)
    if (type == "Vertex" || type == "SubdivisionVertex") {
      if (!parseVertex(value, id)) {
        addError("Failed to parse vertex " + key);
      }
    }
    else if (type == "Edge" || type == "SubdivisionEdge") {
      if (!parseEdge(value, id)) {
        addError("Failed to parse edge " + key);
      }
    }
    else if (type == "HalfEdge" || type == "SubdivisionHalfEdge") {
      if (!parseHalfEdge(value, id)) {
        addError("Failed to parse half-edge " + key);
      }
    }
    else if (type == "Loop") {
      if (!parseLoop(value, id)) {
        addError("Failed to parse loop " + key);
      }
    }
    else if (type == "Face" || type == "SubdivisionFace") {
      if (!parseFace(value, id)) {
        addError("Failed to parse face " + key);
      }
    }
    else if (type == "Shell") {
      if (!parseShell(value, id)) {
        addError("Failed to parse shell " + key);
      }
    }
    else if (type == "Body" || type == "SubdivisionBody") {
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

  // Check for data wrapper
  if (!theJson.contains("data") || !theJson["data"].is_object()) {
    addError("Vertex " + std::to_string(theId) + " missing 'data' field");
    return Standard_False;
  }
  const json& data = theJson["data"];

  // Parse position
  if (data.contains("position") && data["position"].is_array() && data["position"].size() == 3) {
    vertex.position[0] = data["position"][0].get<double>();
    vertex.position[1] = data["position"][1].get<double>();
    vertex.position[2] = data["position"][2].get<double>();
  }
  else {
    addError("Vertex " + std::to_string(theId) + " missing or invalid 'data.position' field");
    return Standard_False;
  }

  // Parse half_edge_id (optional)
  if (data.contains("half_edge_id")) {
    vertex.half_edge_id = data["half_edge_id"].get<int>();
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

  // Check for data wrapper
  if (!theJson.contains("data") || !theJson["data"].is_object()) {
    addError("Edge " + std::to_string(theId) + " missing 'data' field");
    return Standard_False;
  }
  const json& data = theJson["data"];

  // Parse half_edge_id
  if (data.contains("half_edge_id")) {
    edge.half_edge_id = data["half_edge_id"].get<int>();
  }
  else {
    addError("Edge " + std::to_string(theId) + " missing 'data.half_edge_id' field");
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

  // Check for data wrapper
  if (!theJson.contains("data") || !theJson["data"].is_object()) {
    addError("HalfEdge " + std::to_string(theId) + " missing 'data' field");
    return Standard_False;
  }
  const json& data = theJson["data"];

  // Parse required fields
  if (data.contains("edge_id")) {
    halfEdge.edge_id = data["edge_id"].get<int>();
  }
  else {
    addError("HalfEdge " + std::to_string(theId) + " missing 'data.edge_id' field");
    return Standard_False;
  }

  if (data.contains("vertex_id")) {
    halfEdge.vertex_id = data["vertex_id"].get<int>();
  }
  else {
    addError("HalfEdge " + std::to_string(theId) + " missing 'data.vertex_id' field");
    return Standard_False;
  }

  if (data.contains("loop_id")) {
    halfEdge.loop_id = data["loop_id"].get<int>();
  }
  else {
    halfEdge.loop_id = 0;
  }

  if (data.contains("next_id")) {
    halfEdge.next_id = data["next_id"].get<int>();
  }
  else {
    addError("HalfEdge " + std::to_string(theId) + " missing 'data.next_id' field");
    return Standard_False;
  }

  if (data.contains("previous_id")) {
    halfEdge.previous_id = data["previous_id"].get<int>();
  }
  else {
    halfEdge.previous_id = 0;
  }

  if (data.contains("opposite_id")) {
    halfEdge.opposite_id = data["opposite_id"].get<int>();
  }
  else {
    halfEdge.opposite_id = 0;
  }

  // Parse curve_data (optional)
  if (data.contains("curve_data")) {
    if (!parseCurveData(data["curve_data"], halfEdge.curve_data)) {
      addError("HalfEdge " + std::to_string(theId) + " has invalid 'data.curve_data'");
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

  // Check for data wrapper
  if (!theJson.contains("data") || !theJson["data"].is_object()) {
    addError("Loop " + std::to_string(theId) + " missing 'data' field");
    return Standard_False;
  }
  const json& data = theJson["data"];

  if (data.contains("face_id")) {
    loop.face_id = data["face_id"].get<int>();
  }
  else if (data.contains("owner_id") && data.contains("owner_kind") &&
           data["owner_kind"].is_string() && data["owner_kind"].get<std::string>() == "Face") {
    loop.face_id = data["owner_id"].get<int>();
  }
  else {
    addError("Loop " + std::to_string(theId) + " missing 'data.face_id' or Face owner_id field");
    return Standard_False;
  }

  if (data.contains("half_edge_id")) {
    loop.half_edge_id = data["half_edge_id"].get<int>();
  }
  else {
    addError("Loop " + std::to_string(theId) + " missing 'data.half_edge_id' field");
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

  // Check for data wrapper
  if (!theJson.contains("data") || !theJson["data"].is_object()) {
    addError("Face " + std::to_string(theId) + " missing 'data' field");
    return Standard_False;
  }
  const json& data = theJson["data"];

  // Check if this is a subdivision face (no surface_data)
  std::string type = theJson.contains("type") ? theJson["type"].get<std::string>() : "";
  bool isSubdivisionFace = (type == "SubdivisionFace");

  if (data.contains("shell_id")) {
    face.shell_id = data["shell_id"].get<int>();
  }
  else {
    addError("Face " + std::to_string(theId) + " missing 'data.shell_id' field");
    return Standard_False;
  }

  // Parse loop_ids
  if (data.contains("loop_ids") && data["loop_ids"].is_array()) {
    for (const auto& loopId : data["loop_ids"]) {
      face.loop_ids.push_back(loopId.get<int>());
    }
  }
  else {
    addError("Face " + std::to_string(theId) + " missing or invalid 'data.loop_ids' field");
    return Standard_False;
  }

  // Parse surface_data (required for regular faces, optional for subdivision faces)
  if (data.contains("surface_data")) {
    if (!parseSurfaceData(data["surface_data"], face.surface_data)) {
      addError("Face " + std::to_string(theId) + " has invalid 'data.surface_data'");
      return Standard_False;
    }
  }
  else if (!isSubdivisionFace) {
    // surface_data is required for regular faces
    addError("Face " + std::to_string(theId) + " missing 'data.surface_data' field");
    return Standard_False;
  }
  // For subdivision faces, surface_data is optional (they may not have explicit surface geometry)

  // Parse is_surface_normal_same
  if (data.contains("is_surface_normal_same")) {
    face.is_surface_normal_same = data["is_surface_normal_same"].get<int>() != 0;
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

  // Check for data wrapper
  if (!theJson.contains("data") || !theJson["data"].is_object()) {
    addError("Shell " + std::to_string(theId) + " missing 'data' field");
    return Standard_False;
  }
  const json& data = theJson["data"];

  if (data.contains("body_id")) {
    shell.body_id = data["body_id"].get<int>();
  }
  else {
    addError("Shell " + std::to_string(theId) + " missing 'data.body_id' field");
    return Standard_False;
  }

  // Parse face_ids
  if (data.contains("face_ids") && data["face_ids"].is_array()) {
    for (const auto& faceId : data["face_ids"]) {
      shell.face_ids.push_back(faceId.get<int>());
    }
  }
  else {
    addError("Shell " + std::to_string(theId) + " missing or invalid 'data.face_ids' field");
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

  // Check for data wrapper
  if (!theJson.contains("data") || !theJson["data"].is_object()) {
    addError("Body " + std::to_string(theId) + " missing 'data' field");
    return Standard_False;
  }
  const json& data = theJson["data"];

  // Parse shell_ids
  if (data.contains("shell_ids") && data["shell_ids"].is_array()) {
    for (const auto& shellId : data["shell_ids"]) {
      myBody.shell_ids.push_back(shellId.get<int>());
    }
  }
  else {
    addError("Body " + std::to_string(theId) + " missing or invalid 'data.shell_ids' field");
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
//=======================================================================
// function : validateTopology
// purpose  : Validate topology structure
//=======================================================================
Standard_Boolean EzDesignJsonReader::validateTopology()
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

  myErrors.insert(myErrors.end(), errors.begin(), errors.end());

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
// function : addError
// purpose  : Add validation error
//=======================================================================
void EzDesignJsonReader::addError(const std::string& theError)
{
  myErrors.push_back(theError);
}
