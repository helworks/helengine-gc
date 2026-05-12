#!/usr/bin/env bash
set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
readonly CITY_COOKED_ROOT="/mnt/c/dev/helprojs/city/output/windows/cooked"
readonly STAGE_ROOT="${REPO_ROOT}/tmp/city-cube-test-content/cooked"

mkdir -p \
    "${STAGE_ROOT}/scenes/rendering" \
    "${STAGE_ROOT}/engine/models" \
    "${STAGE_ROOT}/engine/materials" \
    "${STAGE_ROOT}/shaders"

cp "${CITY_COOKED_ROOT}/scenes/rendering/cube_test.hasset" "${STAGE_ROOT}/scenes/rendering/cube_test.hasset"
cp "${CITY_COOKED_ROOT}/engine/models/cube.hasset" "${STAGE_ROOT}/engine/models/cube.hasset"
cp "${CITY_COOKED_ROOT}/engine/materials/standard.hasset" "${STAGE_ROOT}/engine/materials/standard.hasset"
cp "${CITY_COOKED_ROOT}/shaders/ForwardStandardShader.dx11.hasset" "${STAGE_ROOT}/shaders/ForwardStandardShader.dx11.hasset"

printf 'Staged GameCube cube-test content under %s\n' "${STAGE_ROOT}"
