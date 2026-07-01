#!/usr/bin/env bash
set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
readonly CITY_COOKED_ROOT="/mnt/c/dev/helprojs/output/windows/cooked"
readonly STAGE_ROOT="${REPO_ROOT}/tmp/city-packaged-gcm-root/files/cooked"
readonly MENU_LOGO_TEXTURE_HASH="3df8b122ea9a854acc1138581a579c38858b774c8b72e02117d059539873897a"

mkdir -p \
    "${STAGE_ROOT}/scenes" \
    "${STAGE_ROOT}/scenes/rendering" \
    "${STAGE_ROOT}/scenes/physics" \
    "${STAGE_ROOT}/fonts" \
    "${STAGE_ROOT}/imported" \
    "${STAGE_ROOT}/engine" \
    "${STAGE_ROOT}/materials"

cp "${CITY_COOKED_ROOT}/scenes/DemoDiscMainMenu.hasset" "${STAGE_ROOT}/scenes/DemoDiscMainMenu.hasset"
cp "${CITY_COOKED_ROOT}/scenes/rendering/"*.hasset "${STAGE_ROOT}/scenes/rendering/"
cp "${CITY_COOKED_ROOT}/scenes/physics/"*.hasset "${STAGE_ROOT}/scenes/physics/"
cp "${CITY_COOKED_ROOT}/fonts/DemoDiscBody.hefont" "${STAGE_ROOT}/fonts/DemoDiscBody.hefont"
cp "${CITY_COOKED_ROOT}/fonts/default.hefont" "${STAGE_ROOT}/fonts/default.hefont"
cp "${CITY_COOKED_ROOT}/imported/${MENU_LOGO_TEXTURE_HASH}" "${STAGE_ROOT}/imported/${MENU_LOGO_TEXTURE_HASH}"
cp -R "${CITY_COOKED_ROOT}/engine/." "${STAGE_ROOT}/engine/"
cp -R "${CITY_COOKED_ROOT}/materials/." "${STAGE_ROOT}/materials/"
cp -R "${CITY_COOKED_ROOT}/imported/." "${STAGE_ROOT}/imported/"

printf 'Staged GameCube demo disc main menu content under %s\n' "${STAGE_ROOT}"
