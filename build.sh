#!/usr/bin/env bash
set -euo pipefail

# Legacy positional interface retained for existing release workflows.
# $4 (old Boost download mode), $9 and $10 are retained but no longer needed.
if (( $# < 5 )); then
  echo "Usage: $0 PATCH CONFIG WITH_TESTS LEGACY_MODE TARGET [ARCHIVE|test] [CXX] [JOBS] [BOOST_INCLUDE] [BOOST_LIB] [BOOST_ROOT]" >&2
  exit 2
fi
source_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
build_dir="${PROTEI_BUILD_DIR:-$source_dir/build}"
args=(-S "$source_dir" -B "$build_dir" "-DPATCH_VERSION=$1" "-DCMAKE_BUILD_TYPE=$2" "-DWITH_TESTS=$3")
if [[ -n "${7:-}" ]]; then args+=("-DCMAKE_CXX_COMPILER=$7"); fi
if [[ -n "${11:-}" ]]; then args+=("-DBoost_ROOT=${11}"); fi
cmake "${args[@]}"
cmake --build "$build_dir" --config "$2" --target "$5" protei_cov --parallel "${8:-2}"
if [[ -n "${6:-}" && "$6" != test ]]; then
  cd "$build_dir"
  cmake -E tar cf "$6.zip" --format=zip -- "$5" base.json
fi
