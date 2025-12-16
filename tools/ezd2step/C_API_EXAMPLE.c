//=======================================================================
// File: C_API_EXAMPLE.c
// Purpose: Example usage of ezd_to_step C API
// Copyright (c) 2025 Yang Song. All rights reserved.
//=======================================================================

#include "ezd_to_step.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <input.ezd> <output.step>\n", argv[0]);
    return 1;
  }

  const char* input_path = argv[1];
  const char* output_path = argv[2];

  // Example 1: Use default options (verbose=1, no log file)
  printf("Converting with default options...\n");
  int result = ezd_to_step(input_path, output_path, NULL);
  if (result != 0) {
    fprintf(stderr, "Conversion failed with exit code: %d\n", result);
    return result;
  }
  printf("Conversion successful!\n");

  // Example 2: Use custom options (verbose=2, log to file)
  /*
  struct ezd_to_step_options options = {
    .verbose = 2,  // Debug mode
    .log_file = "conversion.log"
  };
  
  printf("Converting with custom options (debug mode, logging to file)...\n");
  int result = ezd_to_step(input_path, output_path, &options);
  if (result != 0) {
    fprintf(stderr, "Conversion failed with exit code: %d\n", result);
    return result;
  }
  printf("Conversion successful! Check conversion.log for details.\n");
  */

  // Example 3: Quiet mode (verbose=0)
  /*
  struct ezd_to_step_options quiet_options = {
    .verbose = 0,  // Quiet mode
    .log_file = NULL
  };
  
  int result = ezd_to_step(input_path, output_path, &quiet_options);
  if (result != 0) {
    fprintf(stderr, "Conversion failed with exit code: %d\n", result);
    return result;
  }
  */

  return 0;
}

