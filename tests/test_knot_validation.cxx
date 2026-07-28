#include "EzDesignJsonReader.hxx"
#include "EzDesignToOCCTConverter.hxx"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

int main(int argc, char* argv[])
{
  if (argc != 2) return 1;
  std::ifstream input(argv[1]);
  nlohmann::json data = nlohmann::json::parse(input);
  const std::filesystem::path path = std::filesystem::temp_directory_path() / "ezd2step-invalid-knots.ezd";

  EzDesignJsonReader reader;
  EzDesignToOCCTConverter converter(reader);
  if (!reader.ReadFile(argv[1]) || converter.ConvertBody(reader.GetBody()).IsNull() || !converter.GetErrors().empty()) return 1;

  for (auto& [id, entity] : data["data"]["db"]["entities"].items()) {
    if (entity["type"] == "HalfEdge" && entity["data"].contains("curve_data")) {
      entity["data"]["curve_data"]["basis"]["knot_vector"].push_back(1);
      break;
    }
  }
  std::ofstream(path) << data;
  EzDesignJsonReader invalidReader;
  EzDesignToOCCTConverter invalidConverter(invalidReader);
  const bool rejected = invalidReader.ReadFile(path.string().c_str())
    && (invalidConverter.ConvertBody(invalidReader.GetBody()).IsNull() || !invalidConverter.GetErrors().empty());
  std::filesystem::remove(path);
  return rejected ? 0 : 1;
}
