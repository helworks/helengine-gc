# City Games Tilt Trial Design

## Goal

Add a new game-facing scene generation path for a Monkey Ball style prototype instead of extending the physics validation catalog. The first playable slice should be a small authentic tilt course built from primitive boxes, reachable from a new `Games` menu category and also directly launchable for testing.

## Why This Belongs Outside Physics

The requested experience is no longer a physics validation scene. It introduces game-facing concerns:

- top-level menu categorization
- player-oriented naming and presentation
- course reset rules
- explicit control bindings
- a dedicated camera contract
- future win/fail flow and progression hooks

Keeping this inside `PhysicsSceneFactory` would mix validation content with game content and make future expansion harder. The new prototype should live under a `GameSceneFactory` and `GameSceneCatalog`, with physics used as an implementation dependency rather than as the feature boundary.

## Scope

This design covers one first playable game scene:

- internal neutral scene id for a tilt prototype
- menu label in the new `Games` section
- one tiny course built from boxes
- one dynamic player sphere
- authentic stage tilt through runtime rotation of the course root
- soft reset when the sphere falls below a configured height
- one dedicated follow camera with split movement and orbit bindings

This design does not cover:

- scoring
- timers
- checkpoints
- multi-stage progression
- HUD beyond whatever shared overlay remains required
- static-mesh gameplay collision
- polished branding decisions beyond a temporary menu label

## Scene Shape

The first course should stay intentionally small so the control model can be validated without content noise.

### Authored Entities

The generated scene should include:

- `StageRoot`
- `StartPad`
- `Ramp`
- `GoalPad`
- `CatchFloor`
- `PlayerSphere`
- `PlayerSpawn`
- `GoalMarker`
- `GameCamera`
- shared UI or menu-return entities only if the runtime path requires them

### Course Layout

The recommended layout is:

- one flat start pad near the spawn point
- one gentle ramp connected directly to the start pad
- one flat goal pad at the end of the ramp
- one wide catch floor below the course so missed landings are visible before reset

The stage pieces should all be primitive box-based gameplay geometry. No static-mesh collision should be involved in the first pass.

## Runtime Behavior

### Stage Tilt

The course root should rotate at runtime and drive ball motion through gravity and contact, not through injected ball locomotion forces. This preserves the requested Monkey Ball style feel.

The controller should:

- read tilt input each frame
- maintain current pitch and roll state
- clamp both axes to a medium range around `12-15` degrees
- apply the resolved rotation to the gameplay stage root consistently

Yaw rotation is out of scope for this first pass. The stage should tilt only on two axes.

### Ball Reset

The prototype should use a soft reset:

- detect when the sphere center falls below a configured world height
- restore the sphere to the spawn marker
- clear linear and angular velocity
- preserve current stage tilt after reset for faster iteration

This keeps the reset path small and makes control testing faster.

### Camera

The prototype should not reuse the existing static-mesh showcase camera input contract unchanged. A dedicated game camera component should:

- follow the player sphere by position
- orbit around the sphere with manual player control
- keep orbit input separate from stage tilt input

Keyboard bindings:

- `WASD` tilts the stage
- arrow keys orbit the camera

Gamepad bindings:

- left stick tilts the stage
- right stick orbits the camera

This separation avoids control conflicts and preserves the game-facing feel.

## Factory And Catalog Structure

### New Game Catalog

Add a dedicated game-scene catalog for game-owned scene ids. This keeps playable game content distinct from physics validation content and gives a clean home for future game prototypes.

### New Game Factory

Add a dedicated game-scene factory responsible for:

- generated authored scenes for game content
- playable runtime scene assembly
- camera and controller attachment for game scenes
- direct-test scene id normalization if the existing startup path requires it

The first scene in this factory should be the tilt prototype. Internal ids should stay neutral even if the menu label is more evocative.

## Menu Integration

The main menu should gain a third top-level category:

- `Games`

That category should contain the first playable tilt prototype as its initial entry. The temporary user-facing label should be evocative but generic. Recommended direction:

- internal id: neutral, such as `tilt_trial`
- menu label: `Tilt Trial` or another generic ball-rolling label

Avoid direct homage naming in code or menu labels for now.

## Direct Launch And Testing

The scene must support both:

- launch from the main menu through the new `Games` category
- direct startup for fast iteration in Windows builds

The direct launch path should use the same generated content and runtime components as the menu entry. There should be no separate debug-only variant unless a later testing need proves that necessary.

## Components

The first pass should introduce focused game-facing components rather than overloading physics showcase components.

### `DemoTiltStageComponent`

Responsibilities:

- read tilt input
- maintain pitch and roll state
- clamp tilt to configured limits
- apply rotation to the stage root

### `DemoTiltBallResetComponent`

Responsibilities:

- track the target sphere
- compare its height to the reset threshold
- restore spawn transform
- clear sphere velocities

### Dedicated Follow Camera Component

Responsibilities:

- resolve and track the target sphere
- manage orbit yaw and pitch
- apply the camera pose
- use arrow keys and right stick instead of the physics showcase camera bindings

These responsibilities should stay separated so the course, reset logic, and camera can evolve independently.

## Error Handling

The new runtime components should preserve the existing codebase style:

- require valid serialized target references
- throw when required runtime entities are missing
- throw when required configuration is absent
- avoid silent fallbacks or best-effort behavior

This is especially important for generated scene content because invalid ids or missing markers should fail loudly during testing.

## Testing Strategy

Smallest useful validation for this slice:

- source-level tests that verify the new game catalog and factory expose the tilt scene
- source-level tests that verify the new menu path includes the `Games` category and the first tilt entry
- one runtime smoke launch in Windows that boots directly into the tilt scene
- confirm no fatal startup exception and confirm the expected scene id is loaded

The first pass does not require deep gameplay automation. Manual verification is appropriate because the primary risk is scene assembly and runtime wiring, not complex simulation correctness.

## Naming Recommendation

Recommended naming split:

- code-facing ids remain generic and descriptive
- user-facing menu label can be more expressive

Example direction:

- internal scene id: `games/tilt_trial.helen`
- menu label: `Tilt Trial`

This keeps code stable while leaving room to rename the presentation later.

## Risks

- Rotating a gameplay stage may expose limitations in how current runtime transforms propagate to the active physics representation.
- Reusing existing camera code too aggressively would create input conflicts with the tilt controller.
- Placing game content inside physics catalogs would make future menu and progression work messy.

The design addresses these risks by introducing a game-owned factory boundary, a dedicated tilt controller, and a dedicated camera input contract.

## Success Criteria

The slice is successful when:

- the main menu shows a `Games` category
- selecting the tilt prototype loads the new scene
- the course is built from primitive boxes, not static meshes
- `WASD` or left stick tilts the stage
- arrows or right stick orbit the camera
- the sphere moves due to gravity on the tilted stage
- falling below the reset threshold respawns the sphere at the start
- the same scene can be booted directly for iteration
