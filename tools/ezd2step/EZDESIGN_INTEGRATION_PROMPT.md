# EZDesign Integration Prompt for AI Coder

## Context

The `ezd2step` converter has been fully implemented, tested, and published to GitHub Releases. Your task is to integrate this tool into the EZDesign application to enable STEP file export functionality.

## Current State

### ✅ Completed
- **ezd2step CLI tool**: Fully functional command-line converter
- **C API library**: `libezd2step.dylib` (macOS) / `ezd2step.dll` (Windows)
- **Distribution bundles**: Published on GitHub Releases
  - Bundle: `ezd2step-1.0.0-macos-arm64.tar.gz` (13MB, self-contained)
  - Source package: Available for LGPL compliance
- **Documentation**: Complete API docs and examples
- **Testing**: Validated with sample models

### 📋 Integration Tasks

## Task 1: Add STEP Export Menu Item/Button

**Location**: EZDesign UI (likely in File menu or Export dialog)

**Requirements**:
- Add "Export to STEP" option in the export menu
- Trigger conversion when user selects this option
- Show file save dialog for `.step` output file

**Implementation Options**:

**Option A: Use CLI Tool (Simplest)**
```typescript
// Pseudo-code example
async function exportToSTEP(inputEzdPath: string, outputStepPath: string): Promise<number> {
  const ezd2stepPath = getEzd2StepExecutablePath(); // Bundle path or system PATH
  const result = await exec(`${ezd2stepPath} "${inputEzdPath}" "${outputStepPath}"`);
  return result.exitCode; // 0 = success, 1-5 = error codes
}
```

**Option B: Use C API (More Control)**
```typescript
// If using Node.js with native bindings or FFI
const libezd2step = require('ffi-napi').Library('./libezd2step', {
  'ezd_to_step': ['int', ['string', 'string', 'pointer']]
});

function exportToSTEP(inputEzdPath: string, outputStepPath: string): number {
  return libezd2step.ezd_to_step(inputEzdPath, outputStepPath, null);
}
```

## Task 2: Error Handling Integration

**Requirements**:
- Map exit codes to user-friendly error messages
- Show error dialogs with actionable messages
- Log errors for debugging

**Exit Code Mapping**:
```typescript
const ERROR_MESSAGES = {
  0: "Export successful",
  1: "Invalid arguments. Please check file paths.",
  2: "File I/O error. Cannot read input file or write output file.",
  3: "JSON parsing error. The EZDesign file may be corrupted.",
  4: "Geometry conversion error. Some geometry data is invalid.",
  5: "STEP export error. Failed to generate STEP file."
};

function handleExportError(exitCode: number): void {
  const message = ERROR_MESSAGES[exitCode] || `Unknown error (code: ${exitCode})`;
  showErrorDialog("STEP Export Failed", message);
}
```

## Task 3: Bundle Distribution Strategy

**Decision Required**: How to distribute `ezd2step` with EZDesign?

**Option A: Bundle with Application**
- Include `ezd2step` bundle in EZDesign app bundle
- Extract on first use or at install time
- Pros: No download needed, works offline
- Cons: Larger app size (~13MB)

**Option B: Download on Demand**
- Download bundle from GitHub Releases when needed
- Cache locally after first download
- Pros: Smaller initial app size
- Cons: Requires internet connection, download time

**Option C: System Installation**
- Require users to install `ezd2step` separately
- Check for installation in PATH
- Pros: No app size impact
- Cons: Extra installation step for users

**Recommended**: Option A (bundle with app) for best user experience.

## Task 4: Progress Feedback

**Requirements**:
- Show progress indicator during conversion
- Display status messages (optional, based on verbose level)
- Handle long-running conversions gracefully

**Implementation**:
```typescript
async function exportToSTEPWithProgress(
  inputEzdPath: string, 
  outputStepPath: string,
  onProgress?: (message: string) => void
): Promise<number> {
  showProgressDialog("Exporting to STEP...");
  
  try {
    // If using CLI, parse stdout for progress messages
    const process = spawn(ezd2stepPath, [inputEzdPath, outputStepPath]);
    
    process.stdout.on('data', (data) => {
      const message = data.toString();
      if (onProgress) onProgress(message);
    });
    
    const exitCode = await new Promise<number>((resolve) => {
      process.on('close', (code) => resolve(code || 0));
    });
    
    return exitCode;
  } finally {
    hideProgressDialog();
  }
}
```

## Task 5: File Format Validation

**Requirements**:
- Validate `.ezd` file before attempting conversion
- Show helpful error if file format is invalid
- Check file extension and basic structure

**Implementation**:
```typescript
function validateEzdFile(filePath: string): { valid: boolean; error?: string } {
  // Check file extension
  if (!filePath.endsWith('.ezd')) {
    return { valid: false, error: "File must have .ezd extension" };
  }
  
  // Check file exists and is readable
  if (!fs.existsSync(filePath)) {
    return { valid: false, error: "File does not exist" };
  }
  
  // Optional: Check file is valid JSON
  try {
    const content = fs.readFileSync(filePath, 'utf8');
    JSON.parse(content); // Basic JSON validation
    return { valid: true };
  } catch (e) {
    return { valid: false, error: "File is not valid JSON" };
  }
}
```

## Task 6: Testing Integration

**Requirements**:
- Test with various EZDesign models (simple to complex)
- Test error cases (invalid files, missing permissions)
- Test on target platforms (macOS, Windows)

**Test Cases**:
1. ✅ Export simple single-face model
2. ✅ Export multi-face model with shared edges
3. ✅ Export complex subdivision surface model
4. ❌ Test with corrupted `.ezd` file (should show error)
5. ❌ Test with read-only output directory (should show error)
6. ❌ Test with missing `ezd2step` executable (should show helpful error)

## Task 7: Documentation Updates

**Requirements**:
- Update EZDesign user manual with STEP export instructions
- Add troubleshooting section for common errors
- Document system requirements

**Content to Add**:
- "How to Export to STEP" section
- "STEP Export Requirements" (ezd2step bundle)
- "Troubleshooting STEP Export" (common errors and solutions)

## Implementation Checklist

- [ ] **Step 1**: Decide on integration method (CLI vs C API)
- [ ] **Step 2**: Add UI element (menu item/button) for STEP export
- [ ] **Step 3**: Implement file save dialog for `.step` output
- [ ] **Step 4**: Implement conversion function with error handling
- [ ] **Step 5**: Add progress feedback UI
- [ ] **Step 6**: Integrate bundle distribution (bundle with app or download)
- [ ] **Step 7**: Add file validation before conversion
- [ ] **Step 8**: Test with various EZDesign models
- [ ] **Step 9**: Update user documentation
- [ ] **Step 10**: Test on all target platforms

## Key Files and Resources

### ezd2step Resources
- **GitHub Release**: https://github.com/yordaa/ezdesign-step-bridge/releases
- **C API Header**: `tools/ezd2step/ezd_to_step.h`
- **C API Docs**: `tools/ezd2step/C_API_README.md`
- **Distribution Guide**: `tools/ezd2step/DISTRIBUTION_README.md`
- **Integration Guide**: `INTEGRATION_PROMPT.md` (in repo root)

### API Reference

**C API Function**:
```c
int ezd_to_step(const char* input_path, const char* output_path, 
                const struct ezd_to_step_options* options);
```

**Options Structure**:
```c
struct ezd_to_step_options {
  int verbose;           // 0 = quiet, 1 = progress, 2 = debug
  const char* log_file; // NULL = no log file, or path to log file
};
```

**Exit Codes**:
- `0`: Success
- `1`: Invalid arguments
- `2`: File I/O error
- `3`: JSON parsing error
- `4`: Geometry conversion error
- `5`: STEP export error

## Example Integration Code

### TypeScript/JavaScript Example (Node.js)

```typescript
import { exec } from 'child_process';
import { promisify } from 'util';
import * as path from 'path';
import * as fs from 'fs';

const execAsync = promisify(exec);

class StepExporter {
  private ezd2stepPath: string;
  
  constructor() {
    // Determine ezd2step executable path
    // Option 1: Bundle with app
    this.ezd2stepPath = path.join(__dirname, '../bundles/ezd2step-1.0.0-macos-arm64/ezd2step');
    
    // Option 2: System PATH
    // this.ezd2stepPath = 'ezd2step';
  }
  
  async exportToSTEP(inputEzdPath: string, outputStepPath: string): Promise<{
    success: boolean;
    exitCode: number;
    error?: string;
  }> {
    // Validate input file
    const validation = this.validateInputFile(inputEzdPath);
    if (!validation.valid) {
      return { success: false, exitCode: 1, error: validation.error };
    }
    
    // Check ezd2step exists
    if (!fs.existsSync(this.ezd2stepPath)) {
      return { 
        success: false, 
        exitCode: 1, 
        error: `ezd2step not found at ${this.ezd2stepPath}` 
      };
    }
    
    try {
      // Execute conversion
      const { stdout, stderr } = await execAsync(
        `"${this.ezd2stepPath}" "${inputEzdPath}" "${outputStepPath}"`
      );
      
      // Check if output file was created
      if (!fs.existsSync(outputStepPath)) {
        return { success: false, exitCode: 5, error: "STEP file was not created" };
      }
      
      return { success: true, exitCode: 0 };
      
    } catch (error: any) {
      const exitCode = error.code || 5;
      return { 
        success: false, 
        exitCode, 
        error: this.getErrorMessage(exitCode) 
      };
    }
  }
  
  private validateInputFile(filePath: string): { valid: boolean; error?: string } {
    if (!filePath.endsWith('.ezd')) {
      return { valid: false, error: "Input file must have .ezd extension" };
    }
    if (!fs.existsSync(filePath)) {
      return { valid: false, error: "Input file does not exist" };
    }
    return { valid: true };
  }
  
  private getErrorMessage(exitCode: number): string {
    const messages: Record<number, string> = {
      1: "Invalid arguments",
      2: "File I/O error. Check file permissions and paths.",
      3: "JSON parsing error. The EZDesign file may be corrupted.",
      4: "Geometry conversion error. Some geometry data is invalid.",
      5: "STEP export error. Failed to generate STEP file."
    };
    return messages[exitCode] || `Unknown error (code: ${exitCode})`;
  }
}

// Usage
const exporter = new StepExporter();
const result = await exporter.exportToSTEP('model.ezd', 'model.step');
if (result.success) {
  console.log('Export successful!');
} else {
  console.error(`Export failed: ${result.error}`);
}
```

### Python Example

```python
import subprocess
import os
import json
from pathlib import Path

class StepExporter:
    def __init__(self):
        # Determine ezd2step executable path
        self.ezd2step_path = Path(__file__).parent / "bundles" / "ezd2step-1.0.0-macos-arm64" / "ezd2step"
    
    def export_to_step(self, input_ezd_path: str, output_step_path: str) -> dict:
        """Export EZDesign file to STEP format.
        
        Returns:
            dict: {'success': bool, 'exit_code': int, 'error': str or None}
        """
        # Validate input
        if not input_ezd_path.endswith('.ezd'):
            return {'success': False, 'exit_code': 1, 'error': 'Input must be .ezd file'}
        
        if not os.path.exists(input_ezd_path):
            return {'success': False, 'exit_code': 2, 'error': 'Input file does not exist'}
        
        # Check ezd2step exists
        if not self.ezd2step_path.exists():
            return {'success': False, 'exit_code': 1, 'error': f'ezd2step not found at {self.ezd2step_path}'}
        
        try:
            # Execute conversion
            result = subprocess.run(
                [str(self.ezd2step_path), input_ezd_path, output_step_path],
                capture_output=True,
                text=True,
                check=False
            )
            
            if result.returncode == 0:
                if not os.path.exists(output_step_path):
                    return {'success': False, 'exit_code': 5, 'error': 'STEP file was not created'}
                return {'success': True, 'exit_code': 0, 'error': None}
            else:
                error_msg = self._get_error_message(result.returncode)
                return {'success': False, 'exit_code': result.returncode, 'error': error_msg}
                
        except Exception as e:
            return {'success': False, 'exit_code': 5, 'error': f'Exception: {str(e)}'}
    
    def _get_error_message(self, exit_code: int) -> str:
        messages = {
            1: "Invalid arguments",
            2: "File I/O error. Check file permissions and paths.",
            3: "JSON parsing error. The EZDesign file may be corrupted.",
            4: "Geometry conversion error. Some geometry data is invalid.",
            5: "STEP export error. Failed to generate STEP file."
        }
        return messages.get(exit_code, f"Unknown error (code: {exit_code})")

# Usage
exporter = StepExporter()
result = exporter.export_to_step('model.ezd', 'model.step')
if result['success']:
    print('Export successful!')
else:
    print(f"Export failed: {result['error']}")
```

## Questions to Answer During Implementation

1. **Where should STEP export appear in the UI?**
   - File menu → Export → STEP?
   - Export dialog with format selector?
   - Toolbar button?

2. **How should errors be displayed?**
   - Modal error dialog?
   - Toast notification?
   - Status bar message?

3. **Should conversion be synchronous or asynchronous?**
   - Block UI during conversion?
   - Run in background thread/process?

4. **What happens if ezd2step is missing?**
   - Show download prompt?
   - Disable export option?
   - Provide installation instructions?

5. **Should we support batch export?**
   - Export multiple files at once?
   - Progress for each file?

## Success Criteria

✅ User can export EZDesign models to STEP format from the UI
✅ Clear error messages for all failure cases
✅ Progress feedback during conversion
✅ Works on macOS and Windows
✅ Documentation updated
✅ Tested with various model complexities

## Next Steps

1. Review this prompt and understand the requirements
2. Examine the EZDesign codebase structure
3. Identify where to add STEP export functionality
4. Implement integration following the checklist
5. Test thoroughly with real EZDesign models
6. Update documentation

---

**Ready to Start**: All ezd2step components are production-ready. Focus on integration into EZDesign UI and workflow.

