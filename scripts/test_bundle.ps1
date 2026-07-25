param(
  [Parameter(Mandatory = $true)]
  [string]$BundleArchive,

  [string]$TestDirectory = (Join-Path $env:TEMP "ezd2step_bundle_test"),

  [string]$SampleEzd = ""
)

$ErrorActionPreference = "Stop"
$testsPassed = 0
$testsFailed = 0

function Write-TestResult {
  param(
    [string]$Name,
    [bool]$Passed,
    [string]$Details = ""
  )

  if ($Passed) {
    Write-Host "[PASS] $Name" -ForegroundColor Green
    $script:testsPassed++
  } else {
    Write-Host "[FAIL] $Name" -ForegroundColor Red
    if ($Details) {
      Write-Host "  $Details"
    }
    $script:testsFailed++
  }
}

Write-Host "=========================================="
Write-Host "ezd2step Bundle Smoke Tests"
Write-Host "=========================================="
Write-Host "Bundle archive: $BundleArchive"
Write-Host "Test directory: $TestDirectory"
if ($SampleEzd) {
  Write-Host "Sample EZD: $SampleEzd"
}
Write-Host ""

if (!(Test-Path $BundleArchive)) {
  throw "Bundle archive not found: $BundleArchive"
}

Remove-Item -Recurse -Force $TestDirectory -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Path $TestDirectory | Out-Null

Write-Host "Extracting bundle..."
Expand-Archive -Path $BundleArchive -DestinationPath $TestDirectory -Force

$bundleName = [System.IO.Path]::GetFileNameWithoutExtension($BundleArchive)
if ($bundleName.EndsWith(".tar")) {
  $bundleName = [System.IO.Path]::GetFileNameWithoutExtension($bundleName)
}
$bundleDir = Join-Path $TestDirectory $bundleName
if (!(Test-Path $bundleDir)) {
  throw "Bundle directory not found after extraction: $bundleDir"
}

$exe = Join-Path $bundleDir "ezd2step.exe"
Write-TestResult "Executable exists" (Test-Path $exe)
if (!(Test-Path $exe)) {
  exit 1
}

$versionOutput = & $exe --version 2>&1
Write-TestResult "--version flag works" (($LASTEXITCODE -eq 0) -and ($versionOutput -match "ezd2step version")) "Exit code: $LASTEXITCODE; Output: $versionOutput"

$helpOutput = & $exe --help 2>&1
Write-TestResult "--help flag works" (($LASTEXITCODE -eq 0) -and ($helpOutput -match "Usage:")) "Exit code: $LASTEXITCODE"

& $exe *> $null
Write-TestResult "Invalid arguments return exit code 1" ($LASTEXITCODE -eq 1) "Exit code: $LASTEXITCODE"

& $exe "nonexistent.ezd" "output.step" *> $null
Write-TestResult "Missing input file returns exit code 2" ($LASTEXITCODE -eq 2) "Exit code: $LASTEXITCODE"

$requiredDlls = @("TKDESTEP", "TKXSBase", "TKDE", "TKBRep", "TKernel")
$missingDlls = @()
foreach ($dll in $requiredDlls) {
  if (!(Get-ChildItem -Path $bundleDir -Filter "$dll*.dll" -ErrorAction SilentlyContinue)) {
    $missingDlls += $dll
  }
}
Write-TestResult "Required OCCT DLLs present" ($missingDlls.Count -eq 0) "Missing: $($missingDlls -join ', ')"

if ($SampleEzd) {
  if (!(Test-Path $SampleEzd)) {
    Write-TestResult "Sample EZD exists" $false "Missing sample: $SampleEzd"
  } else {
    $sampleCopy = Join-Path $bundleDir "sample.ezd"
    $sampleStep = Join-Path $bundleDir "sample.step"
    Copy-Item $SampleEzd $sampleCopy -Force
    & $exe $sampleCopy $sampleStep
    Write-TestResult "Sample conversion creates STEP output" (($LASTEXITCODE -eq 0) -and (Test-Path $sampleStep) -and ((Get-Item $sampleStep).Length -gt 0)) "Exit code: $LASTEXITCODE"
  }
} else {
  Write-TestResult "Sample conversion skipped (no sample provided)" $true
}

Write-TestResult "README.txt exists" (Test-Path (Join-Path $bundleDir "README.txt"))
Write-TestResult "LICENSE.txt exists" (Test-Path (Join-Path $bundleDir "LICENSE.txt"))

Write-Host ""
Write-Host "=========================================="
Write-Host "Test Summary"
Write-Host "=========================================="
Write-Host "Passed: $testsPassed" -ForegroundColor Green
if ($testsFailed -gt 0) {
  Write-Host "Failed: $testsFailed" -ForegroundColor Red
  exit 1
}

Write-Host "Failed: 0" -ForegroundColor Green
Write-Host "All tests passed!" -ForegroundColor Green
