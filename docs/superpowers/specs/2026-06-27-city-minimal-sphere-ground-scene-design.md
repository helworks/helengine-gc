# City Minimal Sphere Ground Scene Design

## Summary

Add one new generated playable physics scene that contains only:

- one controllable `PlayerSphere` reusing the existing static-mesh showcase sphere behavior
- one static ground cube
- one third-person `DemoFollowCameraComponent` following the sphere
- the existing playable showcase UI/light-toggle pipeline

The new scene must exist alongside `test_scene_static_mesh_showcase` and must not replace or modify the existing showcase layout beyond shared helper reuse that directly serves this feature.

## Goals

- Provide a minimal BEPU validation scene with the fewest moving parts possible.
- Reuse the already working runtime path for:
  - cooked scene generation
  - packaged scene loading
  - controllable player sphere behavior
  - third-person follow camera targeting by serialized scene entity id
- Support both direct boot testing and later main-menu integration using the same generated scene flow as the existing playable physics showcases.

## Non-Goals

- No new movement system for the sphere.
- No new camera system.
- No replacement of `test_scene_static_mesh_showcase`.
- No broad refactor of the physics showcase generation pipeline unless a tiny extraction is needed to avoid obvious duplication.

## Scene Shape

The new scene will be a sibling playable physics showcase scene, tentatively identified as `test_scene_static_mesh_minimal`.

Its authored scenario subtree will contain:

- `Ground`
  - one static cube mesh
  - positioned as a simple flat floor
  - uses the standard physics demo ground material
- `PlayerSphere`
  - reuses the same controllable sphere setup already used by the current static-mesh showcase
  - remains the followed target for the camera

Its root entities will continue to include:

- one camera entity using `DemoFollowCameraComponent`
- the generated playable-scene UI root
- the optional desktop instruction overlay in the live authoring path
- the scenario root

## Integration Points

### PhysicsSceneCatalog

Add one new scene id constant and include it anywhere the playable generated physics scenes are enumerated.

Expected areas:

- scene id declaration
- `GetSceneIds()` or equivalent generated-scene enumeration
- any playable-scene recognition logic used by the factory

### PhysicsSceneFactory

Add one new scene generator dedicated to the minimal layout.

Expected work:

- create one new authored scene method for the minimal scenario
- create one new playable-scene branch in `CreatePlayablePhysicsShowcaseSceneDefinition(...)`
- normalize the short scene id in `NormalizePlayablePhysicsShowcaseSceneId(...)`
- include the new scene in any playable-scene predicate used by generation

The camera path must reuse the already fixed follow-camera serialization flow, including the post-live-id-assignment rebinding step so the serialized target id matches the persisted `PlayerSphere`.

### Scene Generation Outputs

The new scene must be emitted by the same generation commands already used for the physics scenes so it appears in cooked outputs without requiring any hand-authored scene file maintenance.

## Runtime Behavior

- The player controls the same sphere behavior already used in the static-mesh showcase.
- The camera behaves as the same third-person follow camera already introduced for that showcase.
- The only physical environment geometry is the ground cube.

This keeps the runtime repro path minimal while still exercising:

- BEPU static collision against ground
- dynamic controllable sphere behavior
- follow-camera target resolution
- packaged scene loading

## Testing Strategy

Keep validation narrow and targeted.

### Source-Level Coverage

Add one source-level regression test asserting that the new generated scene path:

- defines the new scene id branch
- includes a `Ground`
- includes a `PlayerSphere`
- uses `DemoFollowCameraComponent`

### Asset/Build Verification

- Regenerate the generated physics scenes.
- Build the Windows output with the new scene as startup for direct verification.
- Launch the Windows build and verify:
  - the startup scene is the new minimal scene
  - the process stays alive
  - logs do not contain the old follow-camera target-resolution fatal

## Risks

### Scene Id Drift

Adding a new playable scene touches multiple factory switch points. Missing one branch would cause generation or direct-load failures.

### Regressing Follow-Camera Target Binding

If the new scene bypasses the existing rebinding path, the camera could serialize a stale target id and recreate the runtime failure that was just fixed.

### Unnecessary Refactor Scope

Trying to over-generalize the existing showcase generator would increase risk. The implementation should prefer a small sibling scene addition unless a trivial helper extraction clearly reduces duplication without changing behavior.

## Recommended Implementation Shape

Implement this as a separate sibling generated scene with maximum reuse of existing sphere and follow-camera setup, and minimum refactoring beyond what is necessary to keep the new scene concise and correct.
