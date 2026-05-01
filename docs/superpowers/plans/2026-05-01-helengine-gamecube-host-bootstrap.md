# Helengine GameCube Host Bootstrap Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the first Docker-only GameCube host that compiles to `build/helengine_gc.dol` and boots in Dolphin with a solid green screen.

**Architecture:** The GameCube project stays intentionally small: one devkitPro-based Docker image, one `Makefile`, one tiny `main.cpp`, and one `GameCubeBootHost` class that owns VIDEO and GX initialization plus the infinite present loop. The build exposes a reserved generated-core root variable now, but phase 1 does not compile generated code or add fake handoff artifacts.

**Tech Stack:** Docker, devkitPro, devkitPPC, libogc, VIDEO, GX, C++17, GNU Make.

---

## File Structure

- `Dockerfile`
  Responsible for provisioning the Linux-based devkitPro GameCube build environment and dropping the container into `/workspace`.
- `Makefile`
  Responsible for compiling the native sources, linking the GameCube executable, converting it to a `.dol`, and exposing the future generated-core root variable.
- `README.md`
  Responsible for documenting the Docker build flow, the output artifact, and the Dolphin verification steps.
- `src/main.cpp`
  Responsible only for constructing `GameCubeBootHost`, calling `Run()`, and returning the exit code.
- `src/platform/gamecube/GameCubeBootHost.hpp`
  Responsible for declaring the thin GameCube bootstrap boundary and its native state.
- `src/platform/gamecube/GameCubeBootHost.cpp`
  Responsible for VIDEO setup, framebuffer allocation, GX configuration, and the infinite green-frame present loop.

### Task 1: Docker build environment

**Files:**
- Create: `Dockerfile`
- Create: `README.md`

- [ ] **Step 1: Create the Docker image definition**

```dockerfile
# syntax=docker/dockerfile:1

FROM devkitpro/devkitppc:latest

RUN dkp-pacman -Syu --noconfirm \
    && dkp-pacman -S --noconfirm gamecube-dev

WORKDIR /workspace
CMD ["/bin/bash"]
```

- [ ] **Step 2: Build the image**

Run:

```bash
rtk docker build -t helengine-gc .
```

Expected:

```text
The build completes successfully and produces a local image tagged helengine-gc.
```

- [ ] **Step 3: Create the initial README focused on the first milestone**

````markdown
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

## Boot check

Load `build/helengine_gc.dol` in Dolphin. The expected result for this milestone is a solid green frame.
````

- [ ] **Step 4: Commit the Docker/bootstrap documentation**

Run:

```bash
rtk git -c safe.directory=/mnt/c/dev/helworks/helengine-gc add Dockerfile README.md
rtk git -c safe.directory=/mnt/c/dev/helworks/helengine-gc commit -m "Add GameCube Docker build scaffold"
```

Expected:

```text
A commit is created with the Docker image and first-pass README.
```

### Task 2: Native build script and entrypoint seam

**Files:**
- Create: `Makefile`
- Create: `src/main.cpp`

- [ ] **Step 1: Create the GameCube build script with a reserved generated-core variable**

```makefile
DEVKITPRO ?= /opt/devkitpro
DEVKITPPC ?= $(DEVKITPRO)/devkitPPC
LIBOGC ?= $(DEVKITPRO)/libogc
HELENGINE_CORE_CPP_ROOT ?=

BUILD_DIR := build
TARGET_ELF := $(BUILD_DIR)/helengine_gc.elf
TARGET_DOL := $(BUILD_DIR)/helengine_gc.dol
SOURCE_DIR := src
SOURCES := \
	$(SOURCE_DIR)/main.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeBootHost.cpp
OBJECTS := $(patsubst $(SOURCE_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))

CXX := powerpc-eabi-g++
ELF2DOL := elf2dol

CPPFLAGS := \
	-I$(SOURCE_DIR) \
	-I$(LIBOGC)/include \
	-DGEKKO \
	-DHW_RVL=0

ifeq ($(strip $(HELENGINE_CORE_CPP_ROOT)),)
CPPFLAGS += -DHELENGINE_GAMECUBE_HAS_GENERATED_CORE=0
else
CPPFLAGS += -DHELENGINE_GAMECUBE_HAS_GENERATED_CORE=1 -I$(HELENGINE_CORE_CPP_ROOT)
endif

CXXFLAGS := \
	-std=gnu++17 \
	-O2 \
	-Wall \
	-Wextra \
	-ffunction-sections \
	-fdata-sections

LDFLAGS := \
	-specs=$(LIBOGC)/lib/cube.specs \
	-Wl,-Map,$(BUILD_DIR)/helengine_gc.map \
	-Wl,--gc-sections

LDLIBS := \
	-logc \
	-ldb \
	-lm

.PHONY: all clean

all: $(TARGET_DOL)

$(TARGET_DOL): $(TARGET_ELF)
	$(ELF2DOL) $< $@

$(TARGET_ELF): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CXX) $(OBJECTS) $(LDFLAGS) $(LDLIBS) -o $@

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -rf $(BUILD_DIR)
```

- [ ] **Step 2: Create the tiny native entrypoint**

```cpp
#include "platform/gamecube/GameCubeBootHost.hpp"

int main() {
    helengine::gamecube::GameCubeBootHost host;
    return host.Run();
}
```

- [ ] **Step 3: Build the project before the host implementation exists**

Run:

```bash
rtk docker run --rm -v "$PWD":/workspace -w /workspace helengine-gc make
```

Expected:

```text
FAIL because src/platform/gamecube/GameCubeBootHost.hpp and .cpp do not exist yet.
```

- [ ] **Step 4: Commit the build-script seam**

Run:

```bash
rtk git -c safe.directory=/mnt/c/dev/helworks/helengine-gc add Makefile src/main.cpp
rtk git -c safe.directory=/mnt/c/dev/helworks/helengine-gc commit -m "Add GameCube native build entrypoint"
```

Expected:

```text
A commit is created with the Makefile and thin launcher.
```

### Task 3: GameCube video bootstrap and green-frame host

**Files:**
- Create: `src/platform/gamecube/GameCubeBootHost.hpp`
- Create: `src/platform/gamecube/GameCubeBootHost.cpp`

- [ ] **Step 1: Create the host declaration with a small native state surface**

```cpp
#pragma once

#include <gccore.h>

namespace helengine::gamecube {
    /// Owns the first GameCube native video bootstrap and boot-frame presentation loop.
    class GameCubeBootHost {
    public:
        /// Creates the GameCube boot host with no initialized native state.
        GameCubeBootHost();

        /// Initializes the native video path and presents the first boot frame until shutdown.
        int Run();

    private:
        /// Initializes the VI display state and allocates the first framebuffer.
        bool InitializeVideo();

        /// Initializes GX for a simple clear-and-present loop.
        bool InitializeGraphics();

        /// Presents one solid green frame to the active framebuffer.
        void PresentFrame();

        /// Stores the preferred video mode selected for the current console or emulator.
        GXRModeObj* RenderMode;

        /// Stores the allocated external framebuffer used for display output.
        void* FrameBuffer;

        /// Stores the GX command FIFO allocation used by the first renderer bootstrap.
        void* FifoBuffer;
    };
}
```

- [ ] **Step 2: Create the host implementation with VIDEO and GX initialization**

```cpp
#include "platform/gamecube/GameCubeBootHost.hpp"

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <malloc.h>

namespace helengine::gamecube {
    namespace {
        constexpr std::size_t DefaultFifoSize = 256 * 1024;
    }

    /// Creates the GameCube boot host with no initialized native state.
    GameCubeBootHost::GameCubeBootHost()
        : RenderMode(nullptr)
        , FrameBuffer(nullptr)
        , FifoBuffer(nullptr) {
    }

    /// Initializes the native video path and presents the first boot frame until shutdown.
    int GameCubeBootHost::Run() {
        if (!InitializeVideo()) {
            return 1;
        }

        if (!InitializeGraphics()) {
            return 1;
        }

        while (true) {
            PresentFrame();
        }

        return 0;
    }

    /// Initializes the VI display state and allocates the first framebuffer.
    bool GameCubeBootHost::InitializeVideo() {
        VIDEO_Init();

        RenderMode = VIDEO_GetPreferredMode(nullptr);

        if (RenderMode == nullptr) {
            return false;
        }

        FrameBuffer = MEM_K0_TO_K1(SYS_AllocateFramebuffer(RenderMode));

        if (FrameBuffer == nullptr) {
            return false;
        }

        VIDEO_Configure(RenderMode);
        VIDEO_SetNextFramebuffer(FrameBuffer);
        VIDEO_SetBlack(FALSE);
        VIDEO_Flush();
        VIDEO_WaitVSync();

        if (RenderMode->viTVMode & VI_NON_INTERLACE) {
            VIDEO_WaitVSync();
        }

        return true;
    }

    /// Initializes GX for a simple clear-and-present loop.
    bool GameCubeBootHost::InitializeGraphics() {
        FifoBuffer = memalign(32, DefaultFifoSize);

        if (FifoBuffer == nullptr) {
            return false;
        }

        std::memset(FifoBuffer, 0, DefaultFifoSize);
        GX_Init(FifoBuffer, DefaultFifoSize);

        const f32 yScale = GX_GetYScaleFactor(RenderMode->efbHeight, RenderMode->xfbHeight);
        const u16 xfbHeight = GX_SetDispCopyYScale(yScale);

        GX_SetScissor(0, 0, RenderMode->fbWidth, RenderMode->efbHeight);
        GX_SetDispCopySrc(0, 0, RenderMode->fbWidth, RenderMode->efbHeight);
        GX_SetDispCopyDst(RenderMode->fbWidth, xfbHeight);
        GX_SetCopyFilter(RenderMode->aa, RenderMode->sample_pattern, GX_TRUE, RenderMode->vfilter);
        GX_SetFieldMode(RenderMode->field_rendering, ((RenderMode->viHeight == (RenderMode->xfbHeight * 2)) ? GX_ENABLE : GX_DISABLE));
        GX_SetCullMode(GX_CULL_NONE);
        GX_SetDispCopyGamma(GX_GM_1_0);
        GX_SetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
        GX_SetColorUpdate(GX_TRUE);

        return true;
    }

    /// Presents one solid green frame to the active framebuffer.
    void GameCubeBootHost::PresentFrame() {
        GX_SetCopyClear(GXColor { 0x00, 0xFF, 0x00, 0xFF }, 0x00FFFFFF);
        GX_CopyDisp(FrameBuffer, GX_TRUE);
        GX_DrawDone();
        VIDEO_SetNextFramebuffer(FrameBuffer);
        VIDEO_Flush();
        VIDEO_WaitVSync();
    }
}
```

- [ ] **Step 3: Build the GameCube `.dol`**

Run:

```bash
rtk docker run --rm -v "$PWD":/workspace -w /workspace helengine-gc make
```

Expected:

```text
The build succeeds and emits build/helengine_gc.elf and build/helengine_gc.dol.
```

- [ ] **Step 4: Verify the produced artifacts exist**

Run:

```bash
rtk ls -la build
```

Expected:

```text
The build directory contains helengine_gc.elf, helengine_gc.dol, object files, and helengine_gc.map.
```

- [ ] **Step 5: Commit the native GameCube host**

Run:

```bash
rtk git -c safe.directory=/mnt/c/dev/helworks/helengine-gc add src/platform/gamecube/GameCubeBootHost.hpp src/platform/gamecube/GameCubeBootHost.cpp
rtk git -c safe.directory=/mnt/c/dev/helworks/helengine-gc commit -m "Add GameCube green-screen bootstrap host"
```

Expected:

```text
A commit is created with the VIDEO and GX bootstrap implementation.
```

### Task 4: Final documentation and verification pass

**Files:**
- Modify: `README.md`

- [ ] **Step 1: Expand the README with the exact mounted-container build command**

````markdown
## Build

```bash
docker build -t helengine-gc .
docker run --rm -v "$PWD":/workspace -w /workspace helengine-gc make
```

The build emits `build/helengine_gc.dol`.
````

- [ ] **Step 2: Add the future generated-core seam note without pretending it is active**

````markdown
## Generated core seam

The native build reserves `HELENGINE_CORE_CPP_ROOT` for later `cs2.cpp` integration, but the first milestone does not compile generated core output yet.
````

- [ ] **Step 3: Add the Dolphin verification instructions**

````markdown
## Boot check

Load `build/helengine_gc.dol` in Dolphin. The expected result for this milestone is a solid green frame with no immediate crash or reset loop.
````

- [ ] **Step 4: Re-run the full build after the documentation matches reality**

Run:

```bash
rtk docker build -t helengine-gc .
rtk docker run --rm -v "$PWD":/workspace -w /workspace helengine-gc make clean
rtk docker run --rm -v "$PWD":/workspace -w /workspace helengine-gc make
```

Expected:

```text
The image rebuilds successfully, the clean step removes build output, and the final make step recreates build/helengine_gc.dol.
```

- [ ] **Step 5: Commit the documentation and verification pass**

Run:

```bash
rtk git -c safe.directory=/mnt/c/dev/helworks/helengine-gc add README.md
rtk git -c safe.directory=/mnt/c/dev/helworks/helengine-gc commit -m "Document GameCube Dolphin bootstrap flow"
```

Expected:

```text
A commit is created with the final README updates.
```
