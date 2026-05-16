# GameCube Main Menu 2D Rendering Design

## Goal

Render the authored city demo-disc main menu scene on GameCube as visible 2D GPU output inside the packaged `.gcm` boot path.

This slice does not include menu navigation, focus movement, activation, pointer input, or scene transitions. The success bar is strictly that the existing authored menu scene boots and draws visibly on screen through the GameCube runtime path.

## Current State

The current GameCube packaged-disc path can:

- boot a retail-style `.gcm`
- mount packaged assets from `dvd:/`
- load authored city scenes
- render stable 3D content through GX

The current GameCube 2D path cannot render anything useful yet:

- [src/platform/gamecube/GameCubeRenderManager2D.hpp](C:/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeRenderManager2D.hpp) exists only as a minimal generated-core bridge
- [src/platform/gamecube/GameCubeRenderManager2D.cpp](C:/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeRenderManager2D.cpp) accepts sprite/text/rounded-rect draw calls but drops them on the floor
- [src/platform/gamecube/GameCubeApplication.cpp](C:/dev/helworks/helengine-gc/src/platform/gamecube/GameCubeApplication.cpp) already constructs `GameCubeRenderManager2D` and passes it into `Core::Initialize(...)`

On the authored-content side, the menu scene already exists in the city project:

- `SceneId = "Scenes/DemoDiscMainMenu.helen"` in [DemoDiscMainMenuSceneFactory.cs](C:/dev/helprojs/city/assets/codebase/menu.tools/DemoDiscMainMenuSceneFactory.cs)
- the authored scene uses `CameraComponent`, `MenuComponent`, `ViewportComponent`, `ReferenceCanvasFitComponent`, `ClipRectComponent`, text entities, rounded-rect panel surfaces, and decorative overlay sprites

So the missing piece is not menu content authoring. The missing piece is the GameCube 2D GPU renderer for the generated drawables that scene already produces.

## Non-Goals

This design does not include:

- menu navigation or selection changes
- input-driven focus updates
- button activation or scene transitions
- generalized 2D effects beyond what the authored main menu immediately requires
- replacing the existing 3D renderer
- inventing a GC-specific menu scene separate from the city-authored one

## Recommended Approach

Implement the minimum GX-backed 2D rendering path needed by the existing authored city demo-disc main menu scene, then boot directly into that scene for validation.

This is the right approach because:

- it keeps the success bar narrow and observable
- it uses the same packaged scene/content/runtime path that already works for 3D scenes
- it avoids a fake "2D probe" branch that would have to be thrown away
- it proves the real authored menu scene can render without coupling this pass to input/navigation work

## Alternatives Considered

### 1. Temporary GC-only 2D probe

Create a hardcoded triangle/quad/text probe scene just for 2D bring-up.

Why not:

- it duplicates work
- it does not prove the authored menu scene path
- it delays the actual integration seam, which is generated menu drawables

### 2. Full menu rendering plus interaction in one pass

Bring up rendering and navigation at the same time.

Why not:

- too many variables at once
- harder to isolate failures between drawing, layout, fonts, clipping, and input
- slower iteration than needed for the current checkpoint

## Functional Design

### Startup Scene

The packaged GameCube boot flow should start directly in the authored city demo-disc main menu scene:

- startup scene id: `Scenes/DemoDiscMainMenu.helen`
- packaged scene asset: the cooked asset generated from that scene id

This keeps validation immediate and removes menu-catalog indirection while the renderer is coming online.

### 2D Drawable Support Scope

The renderer should support only the drawable kinds required by the authored main menu scene:

1. Sprites
2. Text
3. Rounded rectangles

The design assumes the menu scene is composed from those generated drawables. If additional drawable types appear during scene bring-up, they should be surfaced explicitly as new boundaries rather than guessed over.

### Rendering Path

The GameCube 2D path should remain GPU-driven through GX.

That means:

- generated/core code produces 2D drawables through the existing `RenderManager2D` contract
- `GameCubeRenderManager2D` records or forwards those drawables into a native 2D frame path
- GX performs the visible rasterization

This must not degrade into a CPU raster fallback.

### Render Ordering

The first pass should preserve the authored menu's visible stacking using the generated draw order that already exists in the scene/runtime pipeline.

We do not need to introduce a new z-buffered 2D architecture. The main requirement is that panel surfaces, overlays, and text appear in the same order the generated scene submits them.

### Camera / Canvas Handling

The authored menu scene already contains:

- a menu camera
- canvas/profile sizing
- viewport fitting components

So the GC implementation should not replace layout or invent a separate projection model unless the generated 2D drawables require one. The preferred path is:

- let the shared runtime continue producing final 2D drawable positions/sizes
- consume those results in the GC renderer

## Likely Implementation Units

### `GameCubeRenderManager2D`

Role:

- stop being a no-op bridge
- capture sprite/text/rounded-rect draw requests for the current frame
- own any GC-native runtime-texture hookup needed by 2D sprite/text rendering

Boundary:

- no menu logic
- no layout logic
- no scene generation logic

### New GameCube 2D Raster/Submission Layer

Role:

- convert captured 2D drawables into GX draw calls
- handle textured quads for sprites/text glyphs
- handle flat-color geometry for rounded-rect/menu-surface visuals
- preserve stable draw ordering

This should be a separate native rendering unit instead of burying all 2D raster logic inside `GameCubeApplication`.

### Menu Content Staging / Builder Flow

Role:

- stage the authored menu scene and its required font/overlay assets into the packaged-disc content root
- update the startup-scene manifest to point at the authored menu scene

This should reuse the existing packaged scene/material/font staging pattern, not invent a special-case manual copy path.

## Asset Expectations

The menu scene will likely require at least:

- cooked scene asset for `Scenes/DemoDiscMainMenu.helen`
- cooked font asset(s), including `default.hefont` or the menu theme's configured body font
- decorative overlay sprite texture/material assets referenced by the scene/theme

The design expectation is:

- missing menu assets should fail loudly with the same style of explicit packaged-path diagnostics used during the earlier 3D bring-up
- no silent "best effort" substitutes

## Error Handling

Failures should remain explicit and traceable.

Examples:

- missing staged menu scene asset
- missing font asset
- missing overlay sprite texture
- unsupported 2D drawable kind
- invalid runtime texture decode/build for 2D assets

The GC path should report the failing scene/component/drawable stage clearly rather than swallowing the problem and rendering nothing.

## Testing Strategy

### Source / Builder Contract Tests

Add or update tests that guard:

- packaged startup scene id switches to `Scenes/DemoDiscMainMenu.helen`
- required menu asset staging contract is present
- GC 2D runtime source contract is no longer a no-op for the supported drawable set

### Native Build Verification

Rebuild the packaged-disc GameCube target and repackage the `.gcm`.

Required verification:

- packaged native build succeeds
- packaged image reports `ISO/GC`
- Dolphin boots the menu scene directly

### Visual Verification

Success criteria for this pass:

- visible 2D menu scene appears on screen
- background, panel shapes, text, and overlay visuals are visibly present
- rendering is stable enough to inspect

Interaction is intentionally out of scope for this checkpoint.

## Risks

### Font Rendering Complexity

Text is likely the highest-risk 2D feature because it depends on font assets, glyph atlas usage, and correct textured-quad submission.

Mitigation:

- treat text as a first-class boundary in diagnostics
- verify font assets are staged before chasing renderer bugs

### Rounded Rectangle Fidelity

Rounded rects may need either:

- a simple geometry approximation
- or a temporary flat-quad fallback if the authored menu can remain legible without exact rounded corners

The design preference is to preserve authored visuals where practical, but not to overbuild a generalized vector UI system for this checkpoint.

### Hidden Drawable Dependencies

The generated menu path may emit a drawable type or state combination not yet seen in the current authored menu scene.

Mitigation:

- implement the minimum supported set
- fail loudly on unsupported types
- extend only when the real scene proves it necessary

## Acceptance Criteria

This design is complete when all of the following are true:

1. The packaged GameCube build boots directly into `Scenes/DemoDiscMainMenu.helen`.
2. The authored menu scene renders visibly through GX using the GameCube 2D path.
3. The visible menu includes the core authored 2D content, not just a blank clear color.
4. No CPU-side raster fallback is introduced.
5. Navigation/input is not required for this checkpoint.
