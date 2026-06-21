# Engine-Owned Platform Font Atlas Generation Design

## Goal

Move platform-specific font atlas generation out of `helengine-gc` and into the shared engine/editor pipeline.

After this change:

- authored font sources remain canonical
- per-platform font metrics and atlas textures are generated on demand by the editor/build pipeline
- generated platform font artifacts are cached in the project cache
- platform builders consume only generated font and texture assets
- platform builders never import raw font files and never generate font atlases

## Problem

The current `font-atlas-texture` builder contract allows `helengine-gc` to receive raw font-oriented inputs and recover atlas pixels itself. In practice this means the GameCube builder still references `GdiFontImporter` and `System.Drawing` so it can import `.ttf` or serialized fonts that still own source atlas payloads.

That is the wrong ownership boundary.

If a platform can change font rasterization settings such as pixel size, padding, glyph set, or atlas budget, then the platform-specific font artifact is not just a texture-format variation. The glyph metrics, UVs, and packed atlas layout all change. That means the shared engine/editor side must own per-platform font variant generation, while builders should only cook the resulting texture payloads into platform-native runtime texture formats.

## Desired Ownership Split

### Editor / engine side

- resolve platform-specific font generation settings
- generate one platform-specific cached font artifact when needed
- generate one platform-specific cached source atlas texture artifact when needed
- write build manifests that reference only generated assets
- invalidate cached variants when any generation input changes

### Builder side

- accept generated texture assets only
- cook generated texture assets into platform-native runtime texture artifacts
- stage generated font artifacts and cooked textures into build output
- never inspect raw font files
- never import fonts
- never generate font atlases

## Target Contract

The special builder-owned `font-atlas-texture` raw-font path will be removed.

The new flow is:

1. The authored project stores one canonical source font file and its import settings.
2. When a build targets a platform, the editor resolves the platform-specific font generation settings for that platform.
3. The editor generates:
   - one cached platform font asset containing platform-specific metrics and atlas UVs
   - one cached platform source atlas texture asset containing RGBA atlas pixels
4. The build manifest points the platform builder at the generated atlas texture asset through the normal builder-owned `texture` cook path.
5. The generated platform font asset references the cooked atlas texture runtime path that the builder will stage.

This keeps the builder contract simple:

- builders cook textures
- editor/build graph generates fonts

## Cache Model

Platform-specific font variants will be generated on demand and stored in project cache, not in authored source directories and not only in temporary build output.

The cache must be keyed by the full set of inputs that affect font variant correctness:

- source font file bytes
- glyph set / charset rules
- rasterization settings such as pixel size, padding, spacing, and atlas budget
- platform texture settings that affect the atlas contract
- target platform identifier

The cache contents should be treated as disposable derived artifacts.

## Invalidation Rules

The cached platform font variant must be regenerated when any of the following changes:

- the source font file contents
- the logical glyph set included in the font
- font generation settings such as size, spacing, padding, or atlas sizing constraints
- platform texture settings that affect the generated source atlas contract
- the target platform identifier

If none of those inputs changed, repeated builds should reuse the cached platform font artifacts across sessions.

## Migration Strategy

### Editor changes

- teach the editor-side font pipeline to generate platform-specific cached font assets and cached atlas texture assets on demand
- update scene/build packaging so platform manifests reference generated font assets plus generated atlas texture assets
- remove editor emission of builder work items that require raw font recovery in the builder

### Builder changes

- remove raw font import fallback from `GameCubePlatformCookWorkItemExecutor`
- remove any builder code path that accepts `.ttf` or serialized font sources as atlas-generation inputs
- keep only generic generated texture cooking

### Runtime/build artifact changes

- staged font assets continue to point at external cooked atlas textures
- generated atlas texture output paths remain canonical runtime-relative paths

## Non-Goals

- moving platform-native texture cooking out of builders
- changing the GameCube runtime font loading contract
- persisting every platform font variant into authored source control
- introducing compatibility shims for legacy raw-font builder behavior

## Risks

### Risk: cache keys miss a generation input

If the invalidation key omits a rasterization or texture-setting input, stale platform font variants could be reused incorrectly.

Mitigation:

- define the full variant key explicitly in one editor-side type
- add tests that prove changes to size, glyph set, and texture settings invalidate the cache

### Risk: platform font generation and builder texture cooking diverge

If editor-generated font assets reference paths inconsistent with builder cooked texture outputs, runtime font loads will break.

Mitigation:

- keep generated atlas texture paths on the same canonical relative-path rules already used by builder texture outputs
- add packaging tests that assert generated font assets point at the staged cooked atlas path

### Risk: partial migration leaves hidden raw-font dependencies

If any builder still accepts raw font files after the new editor cache path lands, platform-specific code will remain in the wrong layer.

Mitigation:

- add source-contract tests that ensure builders no longer reference raw font importer paths
- remove raw-font fallback code rather than leaving dormant branches

## Acceptance Criteria

- `helengine-gc` no longer references `GdiFontImporter` or raw font import paths
- `helengine-gc` no longer generates font atlases from `.ttf` or source `.hefont` inputs
- the editor/build pipeline generates per-platform cached font variants on demand
- builders receive generated atlas texture assets through the normal `texture` cook path
- a change in platform font generation settings regenerates the cached platform variant
- repeated builds for unchanged inputs reuse project-cache font variants across sessions
