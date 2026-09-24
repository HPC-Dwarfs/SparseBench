#!/bin/bash

# Compile all format combinations
# Combinations: SCS/CRS/CCRS × true/false (complex) × SP/DP (float)
#               × U/ULL (uint)
# Total: 3 × 2 × 2 × 2 = 24 builds

set -e  # Exit on error
# Check if toolchain argument is provided
if [ -z "$1" ]; then
  echo "Error: Toolchain must be passed as the first argument possible (GCC, CLANG, ICX, NVCC, HIP)"
  echo "Usage: $0 <TOOLCHAIN> [ENABLE_MPI=true|false]"
  exit 1
fi
TOOLCHAIN="${1}"  # Fixed toolchain
# Optional second argument overrides ENABLE_MPI; without it the value from
# config.mk is used (unchanged behavior).
MPI_OVERRIDE=()
if [ -n "$2" ]; then
  MPI_OVERRIDE=("ENABLE_MPI=$2")
fi
BUILD_DIR="./builds"
LOG_DIR="./compile_logs"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")

# Create directories
mkdir -p "$BUILD_DIR"
mkdir -p "$LOG_DIR"

# Compute the total number of combinations up front for progress messages
MTX_FORMATS=("SCS" "CRS" "CCRS")
COMPLEX_OPTIONS=("true" "false")
FLOAT_TYPES=("SP" "DP")
UINT_TYPES=("U" "ULL")
TOTAL_BUILDS=$(( ${#MTX_FORMATS[@]} * ${#COMPLEX_OPTIONS[@]} * ${#FLOAT_TYPES[@]} * ${#UINT_TYPES[@]} ))

echo "======================================"
echo "Compiling $TOTAL_BUILDS Format Combinations"
echo "Toolchain: $TOOLCHAIN"
echo "Timestamp: $TIMESTAMP"
echo "======================================"

# Initial distclean to remove all previous builds
echo "Running initial distclean..."
make distclean > /dev/null 2>&1 || true


BUILD_COUNT=0
SUCCESS_COUNT=0
FAIL_COUNT=0

# Track passed and failed builds for summary
PASSED_BUILDS=()
FAILED_BUILDS=()

# Iterate through all combinations
for mtx in "${MTX_FORMATS[@]}"; do
  for complex in "${COMPLEX_OPTIONS[@]}"; do
    for float_type in "${FLOAT_TYPES[@]}"; do
      for uint_type in "${UINT_TYPES[@]}"; do

        BUILD_COUNT=$((BUILD_COUNT + 1))
        BUILD_NAME="${mtx}_complex-${complex}_${float_type}_${uint_type}"
        LOG_FILE="${LOG_DIR}/${TIMESTAMP}_${BUILD_NAME}.log"
        DEST_DIR="${BUILD_DIR}/${BUILD_NAME}"

        echo ""
        echo "[${BUILD_COUNT}/${TOTAL_BUILDS}] Building: $BUILD_NAME"
        echo "  MTX_FMT=$mtx  USE_COMPLEX_ELEMENTS=$complex  FLOAT=$float_type  UINT=$uint_type"

        # Clean before each build to avoid stale artifacts
        make distclean > /dev/null 2>&1 || true

        # Run the build, capturing stdout+stderr to log
        if make -j MTX_FMT="$mtx" \
                USE_COMPLEX_ELEMENTS="$complex" \
                FLOAT_TYPE="$float_type" \
                UINT_TYPE="$uint_type" \
                TOOLCHAIN="$TOOLCHAIN" \
                "${MPI_OVERRIDE[@]}" \
                ENABLE_NVTX="false" \
                ENABLE_SECTIMER="false" \
                >> "$LOG_FILE" 2>&1; then

          echo "  ✓ PASSED  → log: $LOG_FILE"
          SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
          PASSED_BUILDS+=("$BUILD_NAME")

          # Copy binaries/artifacts into a named subdirectory
          mkdir -p "$DEST_DIR"
          # Adjust the glob below to match your actual output binary name(s)
          cp -r ./build/. "$DEST_DIR"/ 2>/dev/null || \
          find . -maxdepth 1 -type f -executable -exec cp {} "$DEST_DIR"/ \; 2>/dev/null || true
          # The linked binary (TARGET = sparseBench-<fmt>-<complex|real>-<toolchain>)
          # lands at the repo root, not in ./build, and the next iteration's distclean
          # removes it.
          complex_tag="real"; [ "$complex" = "true" ] && complex_tag="complex"
          mv ./sparseBench-"${mtx}"-"${complex_tag}"-"${TOOLCHAIN}" "$DEST_DIR"/ 2>/dev/null || true

        else
          EXIT_CODE=$?
          echo "  ✗ FAILED  (exit $EXIT_CODE) → log: $LOG_FILE"
          FAIL_COUNT=$((FAIL_COUNT + 1))
          FAILED_BUILDS+=("$BUILD_NAME")
        fi

      done
    done
  done
done

echo ""
echo "======================================"
echo "Compilation Summary"
echo "======================================"
echo "Total Builds:    $BUILD_COUNT"
echo "Successful:      $SUCCESS_COUNT"
echo "Failed:          $FAIL_COUNT"
echo "Build Directory: $BUILD_DIR"
echo "Log Directory:   $LOG_DIR"
echo ""

if [ ${#PASSED_BUILDS[@]} -gt 0 ]; then
  echo "✓ Passed builds (${SUCCESS_COUNT}):"
  for b in "${PASSED_BUILDS[@]}"; do
    echo "    • $b"
  done
fi

if [ ${#FAILED_BUILDS[@]} -gt 0 ]; then
  echo ""
  echo "✗ Failed builds (${FAIL_COUNT}):"
  for b in "${FAILED_BUILDS[@]}"; do
    echo "    • $b  →  ${LOG_DIR}/${TIMESTAMP}_${b}.log"
  done
fi

echo "======================================"

if [ $FAIL_COUNT -eq 0 ]; then
  echo "All builds completed successfully!"
  exit 0
else
  echo "Some builds failed. Check logs in $LOG_DIR"
  exit 1
fi