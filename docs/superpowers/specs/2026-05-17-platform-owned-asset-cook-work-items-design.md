# Platform-Owned Asset Cook Work Items Design

## Summary

The editor-owned build graph should remain the single source of truth for what assets a platform build contains and what each platform builder must do with them. Platform builders should not rediscover textures, fonts, materials, meshes, audio, or any other source assets by crawling staged scenes or guessing from cooked payloads. Instead, the editor should emit a generic manifest section containing explicit platform-owned asset cook work items. Builders then execute those work items to produce final platform-native runtime artifacts.

This design replaces the current wrong boundary where GameCube-specific texture conversion was pushed into shared editor texture processing or builder-side rewrite logic. The editor should expose per-platform settings and dependency resolution. Builders should own only the final platform-native transformation.

The first implementation slice uses this generic system for GameCube textures and font atlas textures, but the manifest contract is generic from day one and must be suitable for every asset kind and future platform.

## Problem Statement

The current GameCube texture path is wrong in two ways:

1. GameCube-specific texture conversion was attempted in shared editor texture processing. That makes platform-native byte layout part of the shared cook path instead of the GameCube builder contract.
2. Builder-side post-hoc rewriting of already cooked generic artifacts is also the wrong boundary. It forces builders to patch staged outputs after the editor has already decided on the wrong payload shape.

This produces three structural problems:

- stale or misleading cook outputs are hard to diagnose because the editor-owned build graph and the platform builder do not share a clean asset ownership contract
- platform builders are pushed toward rediscovering dependencies by crawling scenes, materials, and fonts instead of receiving explicit work from the editor
- scaling this pattern to many platforms would duplicate reference discovery and asset routing logic across builders

The desired model is:

- editor knows the target platform, processor settings, and dependency graph
- editor emits explicit platform cook work items
- builder executes those work items and stages the outputs
- runtime loads only final platform-owned payloads

## Goals

- Keep the editor as the authority on asset inclusion, platform settings, and dependency discovery.
- Add a generic build-manifest contract for builder-owned asset cook work items.
- Make builders consume explicit work items instead of crawling asset references.
- Support every asset kind through one generic work-item model, even if each platform implements kinds incrementally.
- Ensure runtime assets on GameCube can be fully platform-native without runtime CPU conversion.
- Keep `CookedArtifacts` semantically clean as already finalized artifacts ready to stage.

## Non-Goals

- Do not introduce GameCube-specific fields into shared build-manifest types.
- Do not make builders parse scene graphs or material graphs to discover dependencies for implemented asset kinds.
- Do not make runtime decide how to reinterpret generic editor-cooked blobs into platform-native formats.
- Do not broaden the first implementation slice beyond what is needed to prove the generic contract, except where shared type design must anticipate future asset kinds.

## Recommended Approach

### Option 1: Generic Platform Cook Work Item Manifest Section

Recommended.

Add a new generic manifest collection, tentatively `PlatformCookWorkItems`, that the editor populates during build-graph execution. Each entry declares exactly one builder-owned platform cook task, including source identity, resolved platform settings, output path, and output asset kind.

Why this is recommended:

- keeps discovery centralized in the editor
- keeps execution centralized in builders
- preserves clean separation between source assets and final platform-owned runtime artifacts
- scales to many platforms without builder-side dependency rediscovery

### Option 2: Extend `PlatformBuildArtifact` To Mean Both Source Work And Final Output

Not recommended.

This overloads one model with two conflicting meanings:

- artifact already finalized and stageable
- source asset that still requires builder transformation

That ambiguity would spread through staging, diagnostics, and packaging.

### Option 3: Put Builder-Owned Work Only On Loose Assets And Infer Everything Else

Not recommended.

That still pushes builders or scene packagers toward graph crawling for scene-referenced textures, fonts, materials, and similar assets. It breaks the rule that the editor should already know what the builder must do.

## Architecture

### 1. Shared Manifest Contract

Introduce a new shared build-manifest type in the base platform layer, such as `PlatformCookWorkItem`.

Each work item should be generic and include at minimum:

- stable work item id
- source asset id
- source asset path
- source asset kind
- target platform id
- target runtime artifact kind
- output relative path
- output logical artifact id
- source content hash
- settings hash
- serialized resolved platform settings payload
- optional metadata key-value entries for platform-neutral extensions

The manifest should gain a new collection:

- `PlatformCookWorkItem[] PlatformCookWorkItems`

`CookedArtifacts` remains separate and continues to mean finalized artifacts ready for staging or packaging.

### 2. Editor Responsibilities

The editor-owned build graph should:

- discover all referenced source assets for scenes and loose assets
- resolve the active platform definition and platform-specific processor settings
- decide which asset kinds are editor-cooked directly and which are builder-owned for the target platform
- emit explicit `PlatformCookWorkItems` for builder-owned transforms
- emit `CookedArtifacts` only for outputs already finalized by the editor

This should be driven by platform capability metadata, not by hardcoded GameCube checks in scene packaging.

For example, GameCube can declare that:

- imported textures are builder-owned for final native texture payload generation
- font atlas textures are builder-owned for final native texture payload generation

The editor then emits those work items directly, instead of attempting to serialize final GameCube bytes itself.

### 3. Platform Capability Model

Each platform definition should expose enough metadata for the editor to know whether a given asset category is:

- editor-cooked
- builder-cooked
- unsupported

This can be modeled through a generic capability table keyed by asset category and output kind. The capability contract should support all future asset kinds, including:

- texture
- font-atlas-texture
- material
- mesh
- audio
- animation
- shader package
- imported binary payload
- future platform-owned cooked scene adjuncts

The capability model should be platform-neutral. GameCube is only the first consumer.

### 4. Builder Responsibilities

Builders should receive the manifest and execute only the work items they support.

For each supported work item, the builder should:

- load the declared source asset from the source path or staged source location provided by the editor-owned build graph
- deserialize the resolved platform settings payload
- perform the platform-native transform
- write the final runtime artifact to the declared output relative path
- report diagnostics against the originating source asset id and work item id

Builders should fail loudly on unsupported work-item kinds that were emitted for them.

Builders should not discover additional work by scanning scenes or arbitrary cooked blobs for implemented asset kinds.

### 5. Runtime Responsibilities

Runtime should load only final platform-owned payloads.

For the GameCube first slice, that means:

- menu/imported textures load from builder-produced GameCube-native texture assets
- font atlas textures load from builder-produced GameCube-native texture assets
- no runtime RGB5A3 transcode for those cooked paths

Fallback runtime transcode paths may remain temporarily for legacy non-platform-owned assets, but they should not be the active path for editor-owned GameCube builds once this system is in place.

## GameCube First Slice

The first implementation slice should prove the generic system with:

- imported textures
- font atlas textures

The editor emits GameCube cook work items for these categories. The GameCube builder consumes them and produces final GameCube-owned runtime texture payloads. The GameCube runtime then loads those payloads directly.

This slice is enough to validate:

- generic work-item manifest plumbing
- editor-owned dependency discovery and setting resolution
- builder-owned native cooking
- runtime direct-load path
- measurable reduction in menu startup time
- correction of the current logo-color bug at the proper ownership boundary

## Data Flow

1. User requests `--build gamecube` in the editor-owned build graph.
2. Editor packages scenes and loose assets.
3. Editor resolves target platform capabilities and per-platform processor settings.
4. Editor emits:
   - generic cooked artifacts ready to stage
   - explicit platform cook work items for builder-owned asset kinds
5. GameCube builder executes those work items and writes final platform-owned runtime artifacts.
6. GameCube builder stages both editor-finalized cooked artifacts and builder-produced platform-owned artifacts.
7. Native GameCube runtime loads only the final staged payloads.

## Diagnostics

Diagnostics must become more explicit than the current stale/cooked confusion.

The system should log or report:

- emitted platform cook work item count by asset kind
- builder execution start and completion per work item kind
- source asset id and output relative path for failures
- settings hash used for the cook
- runtime build stamp and content load path, as already added on GameCube

This will make stale or misrouted assets much easier to identify.

## Migration Strategy

1. Add shared manifest support for `PlatformCookWorkItems`.
2. Add platform capability metadata that tells the editor which asset kinds are builder-owned.
3. Teach the editor build graph to emit work items generically.
4. Implement GameCube builder execution for texture and font-atlas-texture work items.
5. Switch GameCube runtime/editor-owned build path to consume those outputs directly.
6. Remove the incorrect shared-editor GameCube final texture conversion path.
7. Expand other platforms and asset kinds incrementally onto the same contract.

## Risks

### Manifest Churn

Adding a new manifest section will require coordinated updates to tests and builders. This is acceptable because the current contract is missing a necessary ownership boundary.

### Partial Adoption

If builders only partially adopt the work-item system, the editor may still need to emit a mixed manifest of editor-finalized and builder-owned assets. That is acceptable as long as the boundary is explicit and each artifact belongs to exactly one side.

### Duplicate Outputs

If the editor emits both a generic cooked artifact and a builder cook work item for the same logical asset, staging ambiguity will result. The design must enforce single ownership for each final output path.

## Testing Strategy

The first implementation should add:

- shared manifest tests for `PlatformCookWorkItems`
- editor build-graph tests proving emitted work items for platform-owned assets
- GameCube builder tests proving texture/font work-item execution produces expected outputs
- GameCube runtime source-contract tests proving direct platform-owned load path
- end-to-end editor-owned GameCube build verification using the menu logo and font atlas

## Success Criteria

The first slice is successful when:

- the editor-owned GameCube build emits explicit generic work items for the menu logo texture and menu font atlas texture
- the GameCube builder consumes those work items without crawling scene/material/font references
- the produced runtime artifacts are GameCube-owned payloads
- the menu logo colors are correct on a fresh stamped build
- menu first-draw timing improves relative to runtime texture conversion
- the shared editor no longer performs final GameCube-specific texture byte conversion in its generic texture processor path
