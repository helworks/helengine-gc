#!/usr/bin/env bash
set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
readonly CITY_COOKED_ROOT="/mnt/c/dev/helprojs/output/windows/cooked"
readonly STAGE_ROOT="${REPO_ROOT}/tmp/city-packaged-gcm-root/files/cooked"

mkdir -p \
    "${STAGE_ROOT}/scenes/rendering" \
    "${STAGE_ROOT}/engine/models" \
    "${STAGE_ROOT}/fonts" \
    "${STAGE_ROOT}/materials/rendering/colored_cube_grid"

cp "${CITY_COOKED_ROOT}/scenes/rendering/colored_cube_grid.hasset" "${STAGE_ROOT}/scenes/rendering/colored_cube_grid.hasset"
cp "${CITY_COOKED_ROOT}/engine/models/cube.hasset" "${STAGE_ROOT}/engine/models/cube.hasset"
cp "${CITY_COOKED_ROOT}/fonts/default.hefont" "${STAGE_ROOT}/fonts/default.hefont"
cp "${CITY_COOKED_ROOT}/materials/rendering/colored_cube_grid/"Cube*.hasset "${STAGE_ROOT}/materials/rendering/colored_cube_grid/"

printf 'Staged GameCube colored cube grid content under %s\n' "${STAGE_ROOT}"
