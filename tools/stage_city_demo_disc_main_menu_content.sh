#!/usr/bin/env bash
set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
readonly CITY_COOKED_ROOT="/mnt/c/dev/helprojs/output/windows/cooked"
readonly STAGE_ROOT="${REPO_ROOT}/tmp/city-packaged-gcm-root/files/cooked"
readonly MENU_LOGO_TEXTURE_HASH="3df8b122ea9a854acc1138581a579c38858b774c8b72e02117d059539873897a"

mkdir -p \
    "${STAGE_ROOT}/scenes" \
    "${STAGE_ROOT}/fonts" \
    "${STAGE_ROOT}/imported"

cp "${CITY_COOKED_ROOT}/scenes/DemoDiscMainMenu.hasset" "${STAGE_ROOT}/scenes/DemoDiscMainMenu.hasset"
cp "${CITY_COOKED_ROOT}/fonts/DemoDiscBody.hefont" "${STAGE_ROOT}/fonts/DemoDiscBody.hefont"
cp "${CITY_COOKED_ROOT}/fonts/default.hefont" "${STAGE_ROOT}/fonts/default.hefont"
cp "${CITY_COOKED_ROOT}/imported/${MENU_LOGO_TEXTURE_HASH}" "${STAGE_ROOT}/imported/${MENU_LOGO_TEXTURE_HASH}"

printf 'Staged GameCube demo disc main menu content under %s\n' "${STAGE_ROOT}"
