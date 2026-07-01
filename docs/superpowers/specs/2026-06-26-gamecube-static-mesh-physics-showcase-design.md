# GameCube Static-Mesh Physics Showcase Design

## Goal

Add one authored packaged physics showcase scene that validates BEPU static-mesh collision on GameCube, make it reachable from the existing demo-disc main menu, and keep a direct packaged boot path for fast iteration.

## Scope

Included:
- author one new physics showcase scene in the `city` project for static-mesh walking and collision validation
- expose that scene through the existing `Physics Scenes` demo-disc menu panel
- keep normal packaged boot on `DemoDiscMainMenu`
- add an explicit GameCube packaged startup override so the same authored packaged scene can be loaded directly for smoke testing
- ensure the scene exercises cooked static-mesh collider data rather than box-collider stand-ins

Excluded:
- moving static meshes
- native C++-only test scenes that bypass packaged content
- replacing the existing main menu flow
- adding new generalized runtime tooling beyond what is needed to select a packaged startup scene

## Current Context

The GameCube packaged boot flow in `helengine-gc` already:
- mounts the packaged disc
- reads the generated runtime scene manifest
- creates a runtime scene catalog from packaged entries
- loads a packaged startup scene id through `SceneManager`

The `city` project already:
- authors `DemoDiscMainMenu`
- exposes a `Physics Scenes` panel in the demo-disc menu
- generates several physics validation scenes through `PhysicsSceneFactory`

The existing `MeshGroundStability` scene does not validate the new BEPU static-mesh cook path because it is composed from regular cube mesh entities rather than authored static-mesh collider data.

## Requirements

### Functional Requirements

1. The demo-disc packaged build must still boot to `DemoDiscMainMenu` by default.
2. The menu must expose one new physics entry for the static-mesh showcase scene.
3. The new scene must be loadable through the menu and through a direct packaged startup override.
4. Both entry paths must load the same authored packaged scene id.
5. The scene must serialize a real `StaticMeshCollider3DComponent` path so the BEPU cook/runtime integration is exercised end to end.

### Validation Requirements

1. The player must be able to spawn into the scene and move around the test layout.
2. The scene must contain at least:
   - one flat starting area
   - a few wall boundaries
   - at least one sloped section
   - a visible spawn marker or clearly authored start area
3. Direct boot and menu boot must produce equivalent runtime behavior aside from initial entry flow.

## Recommended Design

### Scene Ownership

The showcase scene should be authored in the `city` project alongside the existing physics showcase content. `helengine-gc` should remain responsible only for packaged boot selection and runtime launch behavior.

This keeps scene content, menu entries, and cooked asset generation in the same authored content pipeline already used by the demo disc.

### Scene Shape

Add one new physics validation scene dedicated to static-mesh walking. The layout should be intentionally simple and deterministic:
- a broad flat floor near spawn
- several low walls to define boundaries and collision edges
- one or two ramps/slopes
- enough open space to roll or walk a sphere/controller around without immediate clutter

The important requirement is not visual complexity. The scene exists to validate cooked static-mesh collision under normal packaged loading.

### Collider Authoring

The new scene must use `StaticMeshCollider3DComponent` authoring data for the walkable environment. The visual mesh and the collider source may come from the same generated geometry or from a paired mesh asset, but the runtime collision path must resolve through the BEPU static-mesh cooked payload.

Box and sphere colliders remain valid for any dynamic test objects or player body if needed, but the environment being walked on and collided against must not be represented solely by primitive colliders.

### Main Menu Integration

Add one new curated physics scene entry to the existing `Physics Scenes` menu catalog in `city`. The menu should continue using the existing generic `LoadScene` action path with no GameCube-specific menu logic.

This keeps the menu contract unchanged:
- the menu owns scene discovery
- the runtime scene catalog resolves the authored scene id
- GameCube only needs the packaged scene to exist in the generated manifest

### Direct Boot Integration

Add one packaged startup scene override in `helengine-gc`. Default behavior remains unchanged and continues to load `DemoDiscMainMenu`.

When the override is explicitly enabled for testing, packaged boot should load the static-mesh physics showcase scene id instead of the menu scene id. The override should operate at the packaged startup scene selection layer, not by building a separate native bootstrap scene.

This preserves one source of truth for scene content while supporting faster smoke iteration.

## Approaches Considered

### Approach A: One Authored Packaged Scene Plus Packaged Startup Override

Use one authored scene in `city`, add it to the physics menu, and allow `helengine-gc` to override the packaged startup scene id for direct boot.

Pros:
- one real scene under test
- menu path and direct path share the same cooked asset
- validates the actual packaged BEPU static-mesh flow

Cons:
- requires coordinated changes across `city` and `helengine-gc`

Recommendation:
- use this approach

### Approach B: One Authored Scene Plus Startup Selection Changes in Project Build Config

Use one authored scene and switch the project-selected GameCube startup scene when direct testing is needed.

Pros:
- fewer runtime changes in `helengine-gc`

Cons:
- slower iteration
- awkward for frequent smoke testing
- encourages build-config churn for a temporary testing need

### Approach C: Menu Scene Plus Native C++ Direct-Test Scene

Use the authored packaged scene for the menu, but keep direct testing on a separate native runtime installer scene in `helengine-gc`.

Pros:
- potentially faster to stand up initially

Cons:
- direct path does not validate the packaged authored asset
- creates two different test surfaces that can drift
- weakens confidence in the BEPU static-mesh cook path

## Implementation Boundaries

### Expected `city` Changes

- extend the physics scene catalog with one new static-mesh showcase scene id
- author the new scene through the existing physics generation pipeline
- add one physics menu entry for the new scene
- ensure the scene is included in cooked packaged output used by the GameCube disc build

### Expected `helengine-gc` Changes

- add a direct packaged startup override mechanism
- keep `DemoDiscMainMenu` as the default packaged startup scene
- keep runtime scene loading generic and driven by the packaged manifest
- avoid GameCube-only authored scene aliases unless required for compatibility with existing menu scene ids

## Risks

- the authored scene generator in `city` may not yet expose a convenient helper for static-mesh collider serialization, which could require authoring-tool updates before the scene itself can be expressed cleanly
- the packaged manifest may need small alignment work if the new scene id naming does not match menu expectations
- direct-boot override handling could accidentally diverge from default packaged boot if it is implemented below the packaged startup selection layer

## Success Criteria

1. A packaged GameCube build boots to the main menu by default.
2. The main menu `Physics Scenes` panel contains the new static-mesh showcase entry.
3. Selecting that menu item loads the new scene successfully.
4. A direct packaged test boot can load that same scene without changing authored content.
5. The player can move around the simple test layout and collide with static-mesh-backed floors, walls, and slopes.

## Validation Plan

Use the smallest validation that proves the feature:
- regenerate or cook the relevant `city` authored content
- stage packaged content containing the menu scene and the new physics scene
- verify default packaged boot reaches the main menu
- verify menu navigation loads the new scene
- verify direct packaged startup override boots straight into the new scene

## Open Decisions Resolved

- The feature will use packaged authored content rather than a native-only GameCube test scene.
- The feature will support both main-menu launch and direct test boot.
- The direct test path will target the same packaged scene id used by the menu entry.
