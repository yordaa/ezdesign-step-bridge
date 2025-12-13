# C API Documentation

## Overview

The `ezd2step` C API provides a programmatic interface for converting EZDesign JSON files (`.ezd`) to STEP format. This API is designed for integration into other applications and languages.

## Header File

Include the header file in your C or C++ code:

```c
#include "ezd_to_step.h"
```

## Function Signature

```c
int ezd_to_step(const char* input_path, const char* output_path, 
                const struct ezd_to_step_options* options);
```

### Parameters

- **`input_path`**: Path to input `.ezd` file (must not be NULL)
- **`output_path`**: Path to output `.step` file (must not be NULL)
- **`options`**: Optional options structure (NULL = use defaults)

### Return Values

- **`0`**: Success
- **`1`**: Invalid arguments (NULL input_path or output_path)
- **`2`**: File I/O error (input file not found, output directory not writable)
- **`3`**: JSON parsing error
- **`4`**: Geometry conversion error
- **`5`**: STEP export error

## Options Structure

```c
struct ezd_to_step_options {
  int verbose;           // 0 = quiet, 1 = progress, 2 = debug
  const char* log_file;  // Optional log file path (NULL = no log file)
};
```

### Fields

- **`verbose`**: Controls output verbosity
  - `0`: Quiet mode (no output)
  - `1`: Progress messages (default)
  - `2`: Debug messages (includes detailed conversion information)

- **`log_file`**: Optional path to log file
  - `NULL`: Output to stdout/stderr (default)
  - Non-NULL: Write all messages to the specified file

## Usage Examples

### Example 1: Basic Usage (Default Options)

```c
#include "ezd_to_step.h"
#include <stdio.h>

int main() {
  int result = ezd_to_step("input.ezd", "output.step", NULL);
  if (result != 0) {
    fprintf(stderr, "Conversion failed with exit code: %d\n", result);
    return result;
  }
  printf("Conversion successful!\n");
  return 0;
}
```

### Example 2: Custom Options (Debug Mode with Log File)

```c
#include "ezd_to_step.h"
#include <stdio.h>

int main() {
  struct ezd_to_step_options options = {
    .verbose = 2,              // Debug mode
    .log_file = "conversion.log"
  };
  
  int result = ezd_to_step("input.ezd", "output.step", &options);
  if (result != 0) {
    fprintf(stderr, "Conversion failed with exit code: %d\n", result);
    return result;
  }
  printf("Conversion successful! Check conversion.log for details.\n");
  return 0;
}
```

### Example 3: Quiet Mode

```c
#include "ezd_to_step.h"

int main() {
  struct ezd_to_step_options quiet_options = {
    .verbose = 0,  // Quiet mode
    .log_file = NULL
  };
  
  int result = ezd_to_step("input.ezd", "output.step", &quiet_options);
  return result;  // Return exit code directly
}
```

## Linking

### macOS

```bash
gcc -o myapp myapp.c -lezd2step -L/path/to/lib -I/path/to/include
```

### Windows

```cmd
cl /I"C:\path\to\include" myapp.c /link /LIBPATH:"C:\path\to\lib" ezd2step.lib
```

### Linux

```bash
gcc -o myapp myapp.c -lezd2step -L/path/to/lib -I/path/to/include
```

## Error Handling

The function returns exit codes that match the CLI tool. Check the return value and handle errors appropriately:

```c
int result = ezd_to_step("input.ezd", "output.step", NULL);
switch (result) {
  case 0:
    printf("Success!\n");
    break;
  case 1:
    fprintf(stderr, "Invalid arguments\n");
    break;
  case 2:
    fprintf(stderr, "File I/O error\n");
    break;
  case 3:
    fprintf(stderr, "JSON parsing error\n");
    break;
  case 4:
    fprintf(stderr, "Geometry conversion error\n");
    break;
  case 5:
    fprintf(stderr, "STEP export error\n");
    break;
  default:
    fprintf(stderr, "Unknown error\n");
    break;
}
```

## Thread Safety

The C API is **not thread-safe**. Each call should be made from a single thread, or external synchronization should be used if calling from multiple threads.

## Memory Management

The C API manages all memory internally. No manual memory management is required. The `log_file` string is copied internally, so the caller's string can be freed or go out of scope after the function call.

## Notes

- The function performs the same validation and conversion steps as the CLI tool
- Generated STEP files are verified by reading them back with OCCT
- Error messages are written to stderr (or log file if specified)
- Progress messages are written to stdout (or log file if specified)

