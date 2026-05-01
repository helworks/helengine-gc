# Helengine GameCube Host

This repository contains the native GameCube host scaffold for Helengine.

## Current milestone

- Docker-only build using devkitPro GameCube tooling
- Native `.dol` output for direct loading in Dolphin
- First boot check with a solid green screen

## Build

```bash
docker build -t helengine-gc .
docker run --rm -v "$PWD":/workspace -w /workspace helengine-gc make
```

The build emits `build/helengine_gc.dol`.

## Generated core seam

The native build reserves `HELENGINE_CORE_CPP_ROOT` for later `cs2.cpp` integration, but the first milestone does not compile generated core output yet.

## Boot check

Load `build/helengine_gc.dol` in Dolphin. The expected result for this milestone is a solid green frame with no immediate crash or reset loop.
