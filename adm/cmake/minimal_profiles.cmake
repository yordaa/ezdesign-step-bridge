# Minimal build profiles for OCCT
# Each profile defines a set of modules optimized for specific use cases

# Profile: step-export-minimal
# Purpose: STEP file import/export functionality (shape-based only, no CAF)
# Modules: FoundationClasses, ModelingData, ModelingAlgorithms, DataExchange
# Note: Excludes ApplicationFramework (CAF) for minimal size
# API: STEPControl_Reader/Writer (shape-based), no STEPCAFControl_Reader/Writer
set (MINIMAL_PROFILE_step-export-minimal_MODULES
  FoundationClasses
  ModelingData
  ModelingAlgorithms
  DataExchange
)

# Profile: step-export
# Purpose: STEP file import/export functionality with full CAF support
# Modules: FoundationClasses, ModelingData, ModelingAlgorithms, ApplicationFramework, DataExchange
# Note: ApplicationFramework is required for document-based STEP operations
# API: Both STEPControl_Reader/Writer and STEPCAFControl_Reader/Writer available
set (MINIMAL_PROFILE_step-export_MODULES
  FoundationClasses
  ModelingData
  ModelingAlgorithms
  ApplicationFramework
  DataExchange
)

# Profile: geometry-only
# Purpose: Core geometry and topology operations without data exchange or visualization
# Modules: FoundationClasses, ModelingData, ModelingAlgorithms
set (MINIMAL_PROFILE_geometry-only_MODULES
  FoundationClasses
  ModelingData
  ModelingAlgorithms
)

# Profile: data-exchange
# Purpose: All data exchange formats (STEP, IGES, STL, VRML, OBJ, PLY, glTF, etc.)
# Modules: FoundationClasses, ModelingData, ModelingAlgorithms, ApplicationFramework, DataExchange
# Note: ApplicationFramework is required as DataExchange depends on it
set (MINIMAL_PROFILE_data-exchange_MODULES
  FoundationClasses
  ModelingData
  ModelingAlgorithms
  ApplicationFramework
  DataExchange
)

# Function to resolve toolkit dependencies for a list of modules
# This ensures all required toolkits are included transitively
function (RESOLVE_MINIMAL_MODULE_DEPS MODULE_LIST RESULT_TOOLKITS)
  set (RESOLVED_TOOLKITS)
  
  # First, collect toolkits from specified modules
  foreach (MODULE ${MODULE_LIST})
    if (DEFINED ${MODULE}_TOOLKITS)
      list (APPEND RESOLVED_TOOLKITS ${${MODULE}_TOOLKITS})
    else()
      message (WARNING "Module ${MODULE} not found in OCCT_MODULES")
    endif()
  endforeach()
  
  # Remove duplicates
  list (REMOVE_DUPLICATES RESOLVED_TOOLKITS)
  
  # Resolve transitive dependencies
  set (FINAL_TOOLKITS)
  set (PROCESSED_TOOLKITS)
  
  # Use existing dependency resolution
  foreach (TOOLKIT ${RESOLVED_TOOLKITS})
    if (NOT ${TOOLKIT} IN_LIST PROCESSED_TOOLKITS)
      list (APPEND PROCESSED_TOOLKITS ${TOOLKIT})
      EXCTRACT_TOOLKIT_FULL_DEPS ("src" ${TOOLKIT} TOOLKIT_DEPS _)
      list (APPEND FINAL_TOOLKITS ${TOOLKIT})
      list (APPEND FINAL_TOOLKITS ${TOOLKIT_DEPS})
    endif()
  endforeach()
  
  list (REMOVE_DUPLICATES FINAL_TOOLKITS)
  set (${RESULT_TOOLKITS} ${FINAL_TOOLKITS} PARENT_SCOPE)
endfunction()

# Function to get modules for a profile
function (GET_MINIMAL_PROFILE_MODULES PROFILE_NAME RESULT_MODULES)
  if ("${PROFILE_NAME}" STREQUAL "custom")
    # Custom profile uses BUILD_MINIMAL_CUSTOM_MODULES
    if (DEFINED BUILD_MINIMAL_CUSTOM_MODULES)
      separate_arguments (BUILD_MINIMAL_CUSTOM_MODULES)
      set (${RESULT_MODULES} ${BUILD_MINIMAL_CUSTOM_MODULES} PARENT_SCOPE)
    else()
      message (FATAL_ERROR "BUILD_MINIMAL_PROFILE is set to 'custom' but BUILD_MINIMAL_CUSTOM_MODULES is not defined")
    endif()
  elseif (DEFINED MINIMAL_PROFILE_${PROFILE_NAME}_MODULES)
    set (${RESULT_MODULES} ${MINIMAL_PROFILE_${PROFILE_NAME}_MODULES} PARENT_SCOPE)
  else()
    message (FATAL_ERROR "Unknown minimal build profile: ${PROFILE_NAME}. Available profiles: step-export-minimal, step-export, geometry-only, data-exchange, custom")
  endif()
endfunction()

# Function to validate profile modules exist
function (VALIDATE_MINIMAL_PROFILE_MODULES MODULE_LIST)
  foreach (MODULE ${MODULE_LIST})
    list (FIND OCCT_MODULES ${MODULE} MODULE_FOUND)
    if (${MODULE_FOUND} EQUAL -1)
      message (FATAL_ERROR "Module ${MODULE} specified in minimal profile does not exist in OCCT")
    endif()
  endforeach()
endfunction()

