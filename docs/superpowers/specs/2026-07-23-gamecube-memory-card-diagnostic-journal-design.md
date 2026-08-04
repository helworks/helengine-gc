# GameCube Memory-Card Diagnostic Journal

## Goal

Provide persistent startup diagnostics for the GameCube runtime through the standard GameCube memory-card interface. The same GCM must work with a physical card on retail hardware and with Nintendont's default emulated memory card. The implementation must not require a patched Nintendont loader, a host filesystem, or a second runtime build.

## Scope

The journal records infrequent startup checkpoints only. It is a diagnostic facility for the GameCube runtime, not a general logging framework and not a replacement for normal game-save support.

## Build setting

The GameCube builder exposes an `enable-memory-card-diagnostic-journal` Boolean build setting. Its default is `false`. The resolved build option travels with the GameCube native-build request and becomes the `HELENGINE_GAMECUBE_MEMORY_CARD_DIAGNOSTIC_JOURNAL` native preprocessor value. When disabled, no CARD subsystem calls, diagnostic save file, or journal write code is present in the GCM. Enabling it produces the same retail-compatible GCM format with the journal compiled in; it does not select a Nintendont-only runtime path.

## Storage

The runtime initializes the libogc CARD subsystem using the disc game and company identifiers, probes and mounts Slot A, and opens or creates one small game-owned card file named `HELDBG`.

The file is one card sector in size. It contains a versioned header followed by a fixed-capacity circular sequence of fixed-size records. Each record stores a monotonically increasing sequence number, a stage identifier, a result code, and reserved bytes. Fixed records avoid dynamic allocation and make the dump easy to decode from a Nintendont `.raw` card image or a physical card export.

## Recording behavior

The host writes a record at the boundaries needed to separate the current ambiguity:

1. Card journal availability and mount result.
2. Nintendont transport selection and transport initialization completion.
3. First input capture before and after the Nintendont pad-stub invocation.
4. First core update before and after input early-update.
5. First draw before and after scene-operation commit.

Each checkpoint overwrites the current circular-slot record synchronously, then advances the sequence. No record is written in every frame.

## Failure behavior

Card absence, unformatted media, mount failure, insufficient free space, and write failures must not prevent game startup. The journal reports its availability internally and becomes a no-op after an unrecoverable card error. The implementation must not create a fallback host-file logger because a packaged disc has no valid host filesystem and the result must remain retail-compatible.

## Retrieval

With Nintendont's default individual memory-card emulation, the card image is created in the storage root's `saves` directory as `<GameID>.raw`. The `HELDBG` game-save entry can be extracted with Dolphin's Memory Card Manager and decoded using a repository diagnostic reader. A physical GameCube card can be exported through standard memory-card tooling and decoded by the same reader.

## Validation

Automated source and unit tests will establish the journal record layout, circular overwrite order, unavailable-card no-op behavior, and the presence of the input/update/draw checkpoints. The GameCube package build must compile the native journal code and produce a verified GCM. Hardware validation consists of booting that one GCM under stock Nintendont, copying the resulting `.raw` image, and decoding its final journal record.
