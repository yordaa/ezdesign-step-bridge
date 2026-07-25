// Created on: 2026
// Created by: EzDesign to STEP Converter
// Copyright (c) 2025 Yang Song. All rights reserved.
//
// This file is proprietary and confidential. Unauthorized copying, modification,
// distribution, or use of this file, via any medium, is strictly prohibited.
// See LICENSE file in this directory for terms and conditions.

#include "EzDesignJsonReader.hxx"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace
{
bool writeFile(const std::string& thePath, const std::string& theContents)
{
  std::ofstream aFile(thePath);
  if (!aFile.is_open()) {
    std::cerr << "Failed to create fixture: " << thePath << std::endl;
    return false;
  }

  aFile << theContents;
  return true;
}

void printErrors(const EzDesignJsonReader& theReader)
{
  for (const std::string& anError : theReader.GetErrors()) {
    std::cerr << "  " << anError << std::endl;
  }
}

bool expect(Standard_Boolean theCondition, const std::string& theMessage)
{
  if (!theCondition) {
    std::cerr << "FAIL: " << theMessage << std::endl;
    return false;
  }

  return true;
}

bool testEntitiesSchemaParsesReferenceIds()
{
  const std::string aPath =
    (std::filesystem::temp_directory_path() / "ezd2step-reader-entities-schema.ezd").string();
  const std::string aJson = R"JSON(
{
  "metadata": {
    "name": "EzDesign Document",
    "version": "0.1.7"
  },
  "data": {
    "db": {
      "body_ids": [1],
      "entities": {
        "1": {
          "type": "SubdivisionBody",
          "id": 1,
          "data": {
            "shell_ids": [2]
          }
        },
        "2": {
          "type": "Shell",
          "id": 2,
          "data": {
            "body_id": 1,
            "face_ids": [3]
          }
        },
        "3": {
          "type": "SubdivisionFace",
          "id": 3,
          "data": {
            "shell_id": 2,
            "loop_ids": [4],
            "is_surface_normal_same": 1
          }
        },
        "4": {
          "type": "Loop",
          "id": 4,
          "data": {
            "owner_id": 3,
            "owner_kind": "Face",
            "half_edge_id": 5
          }
        },
        "5": {
          "type": "SubdivisionHalfEdge",
          "id": 5,
          "data": {
            "edge_id": 6,
            "vertex_id": 7,
            "loop_id": 4,
            "next_id": 5,
            "previous_id": 5,
            "opposite_id": 0
          }
        },
        "6": {
          "type": "SubdivisionEdge",
          "id": 6,
          "data": {
            "half_edge_id": 5
          }
        },
        "7": {
          "type": "SubdivisionVertex",
          "id": 7,
          "data": {
            "position": [1.0, 2.0, 3.0],
            "half_edge_id": 5
          }
        }
      }
    }
  }
}
)JSON";

  if (!writeFile(aPath, aJson)) {
    return false;
  }

  EzDesignJsonReader aReader;
  const Standard_Boolean isRead = aReader.ReadFile(aPath.c_str());
  std::remove(aPath.c_str());

  if (!isRead) {
    std::cerr << "Reader errors for entities schema:" << std::endl;
    printErrors(aReader);
    return expect(Standard_False, "data.db.entities schema should parse successfully");
  }

  bool isOk = true;
  isOk &= expect(aReader.IsDone(), "reader should be marked done");
  isOk &= expect(aReader.GetBody().id == 1, "body id should come from entity id");
  isOk &= expect(aReader.GetBody().shell_ids.size() == 1 && aReader.GetBody().shell_ids[0] == 2,
                 "body shell_ids should be parsed");
  isOk &= expect(aReader.GetShell(2).body_id == 1, "shell body_id should be parsed");
  isOk &= expect(aReader.GetShell(2).face_ids.size() == 1 && aReader.GetShell(2).face_ids[0] == 3,
                 "shell face_ids should be parsed");
  isOk &= expect(aReader.GetFace(3).shell_id == 2, "face shell_id should be parsed");
  isOk &= expect(aReader.GetFace(3).loop_ids.size() == 1 && aReader.GetFace(3).loop_ids[0] == 4,
                 "face loop_ids should be parsed");
  isOk &= expect(aReader.GetLoop(4).face_id == 3, "loop face_id should be parsed");
  isOk &= expect(aReader.GetLoop(4).half_edge_id == 5, "loop half_edge_id should be parsed");
  isOk &= expect(aReader.GetHalfEdge(5).edge_id == 6, "half-edge edge_id should be parsed");
  isOk &= expect(aReader.GetHalfEdge(5).vertex_id == 7, "half-edge vertex_id should be parsed");
  isOk &= expect(aReader.GetHalfEdge(5).loop_id == 4, "half-edge loop_id should be parsed");
  isOk &= expect(aReader.GetHalfEdge(5).next_id == 5, "half-edge next_id should be parsed");
  isOk &= expect(aReader.GetHalfEdge(5).previous_id == 5, "half-edge previous_id should be parsed");
  isOk &= expect(aReader.GetEdge(6).half_edge_id == 5, "edge half_edge_id should be parsed");
  isOk &= expect(aReader.GetVertex(7).half_edge_id == 5, "vertex half_edge_id should be parsed");

  return isOk;
}

bool testLegacyBodiesWrapperIsRejected()
{
  const std::string aPath =
    (std::filesystem::temp_directory_path() / "ezd2step-reader-legacy-bodies.ezd").string();
  const std::string aJson = R"JSON(
{
  "metadata": {
    "name": "EzDesign Document",
    "version": "0.1.7"
  },
  "data": {
    "db": {
      "bodies": {}
    }
  }
}
)JSON";

  if (!writeFile(aPath, aJson)) {
    return false;
  }

  EzDesignJsonReader aReader;
  const Standard_Boolean isRead = aReader.ReadFile(aPath.c_str());
  std::remove(aPath.c_str());

  if (isRead) {
    return expect(Standard_False, "legacy data.db.bodies wrapper should be rejected");
  }

  bool foundEntitiesError = false;
  for (const std::string& anError : aReader.GetErrors()) {
    foundEntitiesError = foundEntitiesError || anError.find("data.db.entities") != std::string::npos;
  }

  return expect(foundEntitiesError, "rejection should mention missing data.db.entities");
}
}

int main()
{
  bool isOk = true;
  isOk &= testEntitiesSchemaParsesReferenceIds();
  isOk &= testLegacyBodiesWrapperIsRejected();

  if (!isOk) {
    return 1;
  }

  std::cout << "All EzDesignJsonReader entities-schema tests passed" << std::endl;
  return 0;
}
