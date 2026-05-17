# GameCube Platform-Cooked Textures Design

## Goal

Move GameCube texture and font-atlas format conversion out of the runtime and into the editor-owned cook pipeline, using the same per-platform asset-settings model already used by DS. The first supported GameCube texture format will be GX `RGB5A3`.

Success for this slice means:

- the editor exposes GameCube per-platform texture settings alongside the existing DS/PS2/PSP/Windows settings
- GameCube builds cook textures into a GameCube-owned payload format instead of generic RGBA32 `TextureAsset` bytes
- GameCube font atlases are also cooked into that same platform-owned texture format
- the GameCube runtime no longer converts menu textures or font atlases from RGBA32 to GX format during scene load

## Problem

The current GameCube runtime still performs texture transcoding on the CPU:

- `GameCubeRenderManager2D::BuildTextureFromRaw(TextureAsset*)`
- `GameCubeRuntimeTexture::LoadFromRaw(TextureAsset*)`
- `GameCubeRuntimeTexture::EncodeRgba32ToRgb5A3(TextureAsset*)`

The same conversion path is hit indirectly by font loads because `FontAssetBinarySerializer` deserializes the atlas as a `TextureAsset` and immediately calls `BuildTextureFromRaw(...)`.

This means the editor is still cooking generic texture payloads for GC, while the runtime pays the cost of:

- reading RGBA32 or generic texture payloads
- converting them into GX texture memory layout
- allocating and filling a second buffer before first draw

That is the wrong ownership boundary. DS already established the correct pattern: expose per-platform texture settings in the editor, cook the texture to the target platform’s representation there, and keep the runtime focused on binding/using the result.

## Design

### Shared editor and asset model

The shared texture import pipeline remains the single source of truth for per-platform texture settings. GameCube will use the same `TextureAssetProcessorSettings` surface the editor already exposes for other platforms:

- `MaxResolution`
- `ColorFormat`
- `AlphaPrecision`

GameCube adds one new texture color format to the shared enum set:

- `GxRgb5A3`

For this initial slice:

- GameCube textures may select only `GxRgb5A3`
- GameCube alpha precision remains configurable through the existing `AlphaPrecision` field
- the cooked payload must preserve enough metadata for runtime validation, but not require runtime color conversion

This is intentionally editor-owned and generic so other consoles can add their own native cooked texture formats later.

### Runtime asset contract

GameCube should stop loading menu and material textures as generic `TextureAsset` RGBA payloads when the data was already cooked for GC.

The new runtime contract is:

- cooked GameCube texture assets deserialize as a GameCube-owned cooked texture asset type, not a generic raw RGBA texture contract
- `GameCubeRenderManager2D` builds a runtime texture directly from the cooked GC payload
- no `EncodeRgba32ToRgb5A3(...)` call is performed for cooked GC assets

The same contract applies to font atlases:

- font cooking emits a GameCube-owned cooked atlas payload
- font deserialization uses that cooked atlas payload directly
- menu text startup should no longer pay RGBA-to-GX conversion cost on the console

### Builder and platform metadata

GameCube platform metadata should expose the new editor-facing format choice just like DS exposes its texture settings. The GameCube platform definition should:

- advertise the GameCube platform texture setting surface in the editor
- default GameCube textures to `GxRgb5A3`
- keep `MaxResolution` and `AlphaPrecision` available as normal per-platform settings

The GameCube build graph should stay fully editor-owned:

- editor cooks the chosen GameCube texture format
- builder packages the cooked payload
- runtime only loads and binds the cooked payload

### Backward compatibility

This slice should preserve existing behavior for non-GameCube platforms. Only GameCube gains the new native cooked format.

For GameCube:

- previously cooked assets may still exist in caches as generic RGBA32 payloads
- asset identity must change when `ColorFormat` changes to `GxRgb5A3`
- fresh editor-owned GameCube builds must regenerate affected textures and fonts automatically

## Approaches Considered

### 1. Recommended: shared editor-owned native format

Add `GxRgb5A3` to the shared texture pipeline, cook it in the editor, and consume the cooked payload directly on GameCube.

Why this is the right choice:

- matches the DS pattern already established
- removes CPU work from GC runtime
- keeps asset ownership in the editor where it belongs
- gives other consoles a reusable pattern for native texture formats

### 2. GameCube-only runtime optimization

Keep the generic cooked payload and just optimize `EncodeRgba32ToRgb5A3(...)` or cache converted textures in memory.

Why not:

- keeps the wrong ownership boundary
- still pays runtime conversion cost at first load
- does not help font atlases cleanly
- does not create a reusable platform-cooked texture pipeline

### 3. GameCube-only builder sidecar hack

Inject a GC-specific extra processing pass only in `helengine-gc` without extending the shared editor texture contract.

Why not:

- duplicates editor pipeline logic in the platform repo
- diverges from DS/PS2/Windows builder patterns
- makes future consoles repeat the same special-case work

## Implementation Shape

The implementation should be split into these areas:

1. Shared asset schema and processor
   - add `GxRgb5A3` to shared texture format definitions
   - make `TextureAssetProcessor` able to emit GX-ready cooked bytes
   - ensure cache identity changes when GC texture settings change

2. Editor settings and serialization
   - expose `GxRgb5A3` in the editor’s per-platform texture settings for `gamecube`
   - persist and round-trip the new format in texture import sidecars

3. GameCube runtime asset loading
   - introduce a GameCube-owned cooked texture payload path
   - update material texture resolution to use the cooked GC path
   - update font deserialization to use the cooked GC atlas path

4. Cleanup of runtime conversion hot path
   - keep RGBA conversion only as a fallback/debug path if still needed
   - remove cooked-path reliance on `EncodeRgba32ToRgb5A3(...)`

## Verification

We should verify at four levels:

1. Shared editor tests
   - GameCube texture settings round-trip with `GxRgb5A3`
   - texture processor emits GC-native payload shape
   - asset cache identity changes when GameCube texture format changes

2. Font cooking tests
   - GameCube font atlas cooking emits the GC-native texture representation
   - deserialization reads it without falling back to RGBA conversion

3. Builder/export tests
   - editor-owned `--build gamecube` output contains freshly cooked GC texture/font assets
   - the menu logo payload is materially smaller than the old RGBA32-derived version when `MaxResolution = 256`

4. Runtime verification in Dolphin
   - main menu still renders correctly
   - `ContentManager` logs still show the expected assets
   - first-load timing improves relative to the current `647 ms` menu boot measurement

## Non-Goals

This slice does not include:

- adding multiple GameCube texture formats beyond `GxRgb5A3`
- texture compression formats
- mipmaps
- a generalized runtime texture transcoder for every platform
- changing the main menu layout or scene content itself
