#!/usr/bin/env bash
set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
readonly CITY_COOKED_ROOT="/mnt/c/dev/helprojs/output/windows/cooked"
readonly STAGE_ROOT="${REPO_ROOT}/tmp/city-packaged-gcm-root/files/cooked"

readonly TEXTURE_HASHES=(
    "0143f809c636f1cd20053a4c3cfc191b7fb5e510eae696a567970db4d0abdf7e"
    "d3a8cdec6238c63ed57434abed80877ab5d5aabbad851a76e18dfc261d855485"
    "4c5eb874277e3374705e558af7a9b4603a0e224da4dc92d0815a2a2ebdb06bb4"
    "fc9ee8227ea629e6974e3f32924d7e845d8d84bb03c2ebd8e117f1fec186af8d"
    "753096d32f70cfc9a1769144e683d19f973e0f24a206e55ea5bb5b6484456ba3"
    "a137289998782a48e50bef2c5755c70b420b69ac7f4d31ccad94d08353321e80"
    "077764b9996a65108df578775fd4312dfc8de9d9397ae8cf896066ad08981626"
    "5a9ed77790c0bc12f53b2f053c946538907d4d79c23bf5888cba00ea445ad486"
    "3158011fcbbff95cea13b5fd8390a7b3692248405a02f9b79189f57a9751d48f"
    "1ec0f27f988f0c5392b4e1210f3bb3a14061482b2d945dfebc8c87ec24ebda19"
    "dd88638e125560144ef71c1618652192b2292e162eb124b23064ac39fb276b71"
    "adfd390752d23c78fd066541ef827db2bc5fc09938dba36a5f7f8b85ddd963e9"
    "9098e36e79bfce6249e5ce23fb995174c8cebb98520cc6817f311f02f6764d56"
    "fd6cd7cde4625b6a48ab2e4ed43842b290c720656f831f572cc2a3b5392deed2"
    "3728cc4e9f8b82e5fff2693cb2400ccb51ee9e98f7ec8477b76d5223fb1d94e8"
    "0d59b3f08ab55a3b70958d52b2f95d18eb6cf6616d7c1196fbfac091228283e0"
)

mkdir -p \
    "${STAGE_ROOT}/scenes/rendering" \
    "${STAGE_ROOT}/engine/models" \
    "${STAGE_ROOT}/fonts" \
    "${STAGE_ROOT}/imported" \
    "${STAGE_ROOT}/materials/rendering/textured_cube_grid"

cp "${CITY_COOKED_ROOT}/scenes/rendering/textured_cube_grid.hasset" "${STAGE_ROOT}/scenes/rendering/textured_cube_grid.hasset"
cp "${CITY_COOKED_ROOT}/engine/models/cube.hasset" "${STAGE_ROOT}/engine/models/cube.hasset"
cp "${CITY_COOKED_ROOT}/fonts/default.hefont" "${STAGE_ROOT}/fonts/default.hefont"
cp "${CITY_COOKED_ROOT}/materials/rendering/textured_cube_grid/"Cube*.hasset "${STAGE_ROOT}/materials/rendering/textured_cube_grid/"

for texture_hash in "${TEXTURE_HASHES[@]}"; do
    cp "${CITY_COOKED_ROOT}/imported/${texture_hash}" "${STAGE_ROOT}/imported/${texture_hash}"
done

printf 'Staged GameCube textured cube grid content under %s\n' "${STAGE_ROOT}"
