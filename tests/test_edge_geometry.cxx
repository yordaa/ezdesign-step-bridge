// Created on: 2026
// Created by: EzDesign to STEP Converter
// Copyright (c) 2025 Yang Song. All rights reserved.
//
// This file is proprietary and confidential. Unauthorized copying, modification,
// distribution, or use of this file, via any medium, is strictly prohibited.
// See LICENSE file in this directory for terms and conditions.

#include "EzDesignJsonReader.hxx"
#include "EzDesignToOCCTConverter.hxx"

#include <BRepCheck_Analyzer.hxx>
#include <BRep_Tool.hxx>
#include <ShapeAnalysis_Edge.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopTools_MapOfShape.hxx>

#include <iostream>

int main(int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <input.ezd>" << std::endl;
    return 1;
  }

  EzDesignJsonReader aReader;
  if (!aReader.ReadFile(argv[1])) {
    std::cerr << "ERROR: failed to read EzDesign input" << std::endl;
    return 1;
  }

  EzDesignToOCCTConverter aConverter(aReader);
  const TopoDS_Shape aShape = aConverter.ConvertBody(aReader.GetBody());
  if (aShape.IsNull()) {
    std::cerr << "ERROR: conversion failed" << std::endl;
    return 1;
  }

  constexpr Standard_Real anExportTolerance = 1.0e-5;
  TopTools_MapOfShape aSeenEdges;
  ShapeAnalysis_Edge anEdgeAnalyzer;
  int anEdgeCount = 0;
  for (TopExp_Explorer anExplorer(aShape, TopAbs_EDGE); anExplorer.More(); anExplorer.Next()) {
    if (!aSeenEdges.Add(anExplorer.Current())) {
      continue;
    }

    ++anEdgeCount;
    const TopoDS_Edge anEdge = TopoDS::Edge(anExplorer.Current());
    Standard_Real aFirst, aLast;
    if (BRep_Tool::Curve(anEdge, aFirst, aLast).IsNull()) {
      std::cerr << "ERROR: edge " << anEdgeCount << " has no 3D curve" << std::endl;
      return 1;
    }

    Standard_Real aMaxDeviation = 0.0;
    anEdgeAnalyzer.CheckSameParameter(anEdge, aMaxDeviation, 101);
    if (aMaxDeviation > anExportTolerance) {
      std::cerr << "ERROR: edge " << anEdgeCount
                << " 3D/pcurve deviation is " << aMaxDeviation << std::endl;
      return 1;
    }
  }

  if (anEdgeCount == 0
      || !BRepCheck_Analyzer(aShape, Standard_True, Standard_False, Standard_True).IsValid()) {
    std::cerr << "ERROR: converted shape is not exact-valid" << std::endl;
    return 1;
  }

  std::cout << "Validated " << anEdgeCount << " unique edge representations" << std::endl;
  return 0;
}
