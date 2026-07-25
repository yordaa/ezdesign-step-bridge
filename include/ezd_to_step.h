//=======================================================================
// File: ezd_to_step.h
// Purpose: C API for EZDesign to STEP conversion
// Copyright (c) 2025 Yang Song. All rights reserved.
//=======================================================================

#ifndef _ezd_to_step_HeaderFile
#define _ezd_to_step_HeaderFile

#ifdef __cplusplus
extern "C" {
#endif

//! Options structure for ezd_to_step conversion
struct ezd_to_step_options {
  //! Verbose level: 0 = quiet, 1 = progress messages, 2 = debug messages
  int verbose;
  
  //! Optional log file path (NULL = no log file)
  //! If set, error messages and progress will be written to this file
  const char* log_file;
};

//! Convert EZDesign JSON file (.ezd) to STEP file format
//!
//! @param input_path  Path to input .ezd file (must not be NULL)
//! @param output_path Path to output .step file (must not be NULL)
//! @param options     Optional options structure (NULL = use defaults)
//!                    Default options: verbose=1, log_file=NULL
//!
//! @return Exit code:
//!   - 0: Success
//!   - 1: Invalid arguments (NULL input_path or output_path)
//!   - 2: File I/O error (input file not found, output directory not writable)
//!   - 3: JSON parsing error
//!   - 4: Geometry conversion error
//!   - 5: STEP export error
//!
//! Example usage:
//!   struct ezd_to_step_options opts = {1, NULL};  // verbose=1, no log file
//!   int result = ezd_to_step("input.ezd", "output.step", &opts);
//!   if (result != 0) {
//!     // Handle error
//!   }
int ezd_to_step(const char* input_path, const char* output_path, 
                const struct ezd_to_step_options* options);

#ifdef __cplusplus
}
#endif

#endif /* _ezd_to_step_HeaderFile */

