# GameCube Nintendont-compatible Disc Driver

## Goal

Provide one packaged GameCube executable that reads its disc image on both a physical GameCube and Nintendont without changing the loader or using a platform-specific distribution.

## Evidence

The Nintendont log records `Patch:[DI] applied 80 times` but does not record `Patch:[__DVDInterruptHandler]`. Its source identifies that handler replacement as required for every game read. Current libogc uses a private completion handler whose generated instruction sequence does not match that patch point, so synchronous reads wait indefinitely after their DI register addresses have been redirected.

## Design

`GameCubeDiscReader` becomes the sole owner of packaged-disc access. Its initialization registers a dedicated PI/DI interrupt handler and its reads submit bounded, aligned DI requests. Completion is recorded by that handler rather than by libogc's unpatched private DVD handler.

The handler is entered through a small PowerPC assembly bridge. On real hardware, the bridge yields the physical DI register base and transfers to the C++ completion routine. Its first instructions deliberately have the `lis r3, 0xCC00` / `addi r3, r3, 0x6000` shape and neighbouring instruction values that Nintendont uses to locate its supported DVD interrupt hook. Under Nintendont, the loader replaces the `lis` with its existing interrupt bridge; execution then continues through the same C++ completion routine with Nintendont's virtual DI register base. This preserves a single source and GCM.

`GameCubeSceneBootstrap` initializes this reader before its first header verification. It no longer relies on libogc's synchronous DVD interface or performs `DVD_Mount`, which would reset the drive.

## Error handling

The reader validates buffer alignment, byte-range overflow, request size, and single-request ownership before submission. It acknowledges DI interrupts and reports transfer failure through `false`. The existing direct-frame watchdog remains available for an unexpected no-interrupt stall.

## Verification

Source tests assert that the reader uses its owned DI interrupt bridge, that the bridge contains the Nintendont-recognized register pattern, and that packaged bootstrap initializes the reader. A diagnostic GCM will then be built and copied to `I:\games\helen\game.gcm`; its Nintendont log must include `Patch:[__DVDInterruptHandler]` and its screen must advance beyond `A005`.
