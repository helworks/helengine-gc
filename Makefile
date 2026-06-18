DEVKITPRO ?= /opt/devkitpro
DEVKITPPC ?= $(DEVKITPRO)/devkitPPC
LIBOGC ?= $(DEVKITPRO)/libogc
HELENGINE_CORE_CPP_ROOT ?=
HELENGINE_GAMECUBE_BATCH_VERIFY_FRAME_LIMIT ?= 0
HELENGINE_GAMECUBE_MINIMAL_SAMPLE ?= 0

include $(DEVKITPPC)/gamecube_rules

BUILD_DIR := build
TARGET_ELF := $(BUILD_DIR)/helengine_gc.elf
TARGET_DOL := $(BUILD_DIR)/helengine_gc.dol
SOURCE_DIR := src
THIRD_PARTY_DIR := third_party
APPLOADER_SOURCE_ROOT := $(THIRD_PARTY_DIR)/cubeboot-tools
PACKAGED_DISC_BUILD_DIR := $(BUILD_DIR)/packaged-disc
PACKAGED_DISC_APPLOADER_ELF := $(PACKAGED_DISC_BUILD_DIR)/apploader.elf
PACKAGED_DISC_APPLOADER_RAW_IMAGE := $(PACKAGED_DISC_BUILD_DIR)/apploader-code.img
PACKAGED_DISC_APPLOADER_IMAGE := $(PACKAGED_DISC_BUILD_DIR)/apploader.img
PACKAGED_DISC_CUBEBOOT_APPLOADER_ELF := $(PACKAGED_DISC_BUILD_DIR)/cubeboot-apploader.elf
PACKAGED_DISC_CUBEBOOT_APPLOADER_IMAGE := $(PACKAGED_DISC_BUILD_DIR)/cubeboot-apploader.img
PACKAGED_DISC_APPLOADER_LINKER_SCRIPT := $(APPLOADER_SOURCE_ROOT)/ppc/apploader/apploader_ldscript.txt
PACKAGED_DISC_GENERIC_BOOT_IMAGE := $(PACKAGED_DISC_BUILD_DIR)/gbi.hdr
PACKAGED_DISC_MKAPPLOADER := $(PACKAGED_DISC_BUILD_DIR)/mkapploader
PACKAGED_DISC_MKGBI := $(PACKAGED_DISC_BUILD_DIR)/mkgbi
PACKAGED_DISC_MKAPPLOADER_OBJECT := $(PACKAGED_DISC_BUILD_DIR)/mkapploader.o
PACKAGED_DISC_MKGBI_OBJECT := $(PACKAGED_DISC_BUILD_DIR)/mkgbi.o
PACKAGED_DISC_MKGBI_LIB_OBJECT := $(PACKAGED_DISC_BUILD_DIR)/mkgbi-lib.o
LIBOGC_GAMECUBE_LIB_DIR := $(LIBOGC)/lib/cube
GENERATED_CONFIG := $(HELENGINE_CORE_CPP_ROOT)/helcpp_config.hpp
ifeq ($(strip $(HELENGINE_GAMECUBE_MINIMAL_SAMPLE)),1)
SOURCES := \
	$(SOURCE_DIR)/platform/gamecube/GameCubeMinimalSampleMain.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeApplication.cpp
else
SOURCES := \
	$(SOURCE_DIR)/main.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeApplication.cpp
endif
GENERATED_BRIDGE_SOURCES :=
GENERATED_CORE_SOURCE :=
GENERATED_CORE_TRANSLATION_UNIT :=

CXX := $(DEVKITPPC)/bin/powerpc-eabi-g++
CC := $(DEVKITPPC)/bin/powerpc-eabi-gcc
HOSTCC := gcc
ELF2DOL := $(DEVKITPRO)/tools/bin/elf2dol
OBJCOPY := $(DEVKITPPC)/bin/powerpc-eabi-objcopy
READELF := $(DEVKITPPC)/bin/powerpc-eabi-readelf

CPPFLAGS := \
	-I$(SOURCE_DIR) \
	-I$(LIBOGC)/include \
	-DGEKKO \
	-DHW_DOL=1 \
	-D__gamecube__=1 \
	-DHELENGINE_GAMECUBE_BATCH_VERIFY_FRAME_LIMIT=$(HELENGINE_GAMECUBE_BATCH_VERIFY_FRAME_LIMIT)

ifeq ($(strip $(HELENGINE_GAMECUBE_MINIMAL_SAMPLE)),1)
CPPFLAGS += -DHELENGINE_GAMECUBE_MINIMAL_SAMPLE=1
else
CPPFLAGS += -DHELENGINE_GAMECUBE_MINIMAL_SAMPLE=0
endif

ifeq ($(strip $(HELENGINE_GAMECUBE_BOOT_MODE)),packaged-disc)
CPPFLAGS += -DHELENGINE_GAMECUBE_PACKAGED_DISC_BOOT=1
else
CPPFLAGS += -DHELENGINE_GAMECUBE_PACKAGED_DISC_BOOT=0
endif

ifeq ($(strip $(HELENGINE_CORE_CPP_ROOT)),)
CPPFLAGS += -DHELENGINE_GAMECUBE_HAS_GENERATED_CORE=0
CPPFLAGS += -DHELENGINE_GAMECUBE_HAS_PHYSICS3D_RUNTIME_REGISTRATION=0
else
ifneq ($(wildcard $(HELENGINE_CORE_CPP_ROOT)/helengine_core_amalgamated.cpp),)
GENERATED_CORE_TRANSLATION_UNIT := helengine_core_amalgamated.cpp
else ifneq ($(wildcard $(HELENGINE_CORE_CPP_ROOT)/helengine_core_unity.cpp),)
GENERATED_CORE_TRANSLATION_UNIT := helengine_core_unity.cpp
else
$(error HELENGINE_CORE_CPP_ROOT does not contain helengine_core_amalgamated.cpp or helengine_core_unity.cpp)
endif
GENERATED_CORE_SOURCE := $(HELENGINE_CORE_CPP_ROOT)/$(GENERATED_CORE_TRANSLATION_UNIT)
ifeq ($(wildcard $(GENERATED_CONFIG)),)
$(error HELENGINE_CORE_CPP_ROOT does not contain helcpp_config.hpp)
endif
ifeq ($(shell tr -d '\r' < $(GENERATED_CONFIG) 2>/dev/null | grep -Ec '^#define HE_CPP_COMPILER_GCC 1$$'),0)
$(error HELENGINE_CORE_CPP_ROOT helcpp_config.hpp must define HE_CPP_COMPILER_GCC 1)
endif
ifeq ($(shell tr -d '\r' < $(GENERATED_CONFIG) 2>/dev/null | grep -Ec '^#define HE_CPP_PLATFORM_(GAMECUBE|RETROPPC) 1$$'),0)
$(error HELENGINE_CORE_CPP_ROOT helcpp_config.hpp must define HE_CPP_PLATFORM_GAMECUBE 1 or HE_CPP_PLATFORM_RETROPPC 1)
endif
ifeq ($(shell tr -d '\r' < $(GENERATED_CONFIG) 2>/dev/null | grep -Ec '^#define HE_CPP_PLATFORM_IS_LITTLE_ENDIAN 0$$'),0)
$(error HELENGINE_CORE_CPP_ROOT helcpp_config.hpp must define HE_CPP_PLATFORM_IS_LITTLE_ENDIAN 0)
endif
ifeq ($(shell tr -d '\r' < $(GENERATED_CONFIG) 2>/dev/null | grep -Ec '^#define HE_CPP_PLATFORM_IS_WINDOWS_HOST 0$$'),0)
$(error HELENGINE_CORE_CPP_ROOT helcpp_config.hpp must define HE_CPP_PLATFORM_IS_WINDOWS_HOST 0)
endif
GENERATED_BRIDGE_SOURCES := \
	$(SOURCE_DIR)/platform/gamecube/GameCubeInputManager.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeRenderManager2D.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeRenderManager3D.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeCubeTestSceneInstaller.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeDiscFileSystem.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeSceneBootstrap.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeFramePlan.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeSceneRenderBridge.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeMeshCache.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeRuntimeMaterial.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeRuntimeTexture.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeRasterRenderer.cpp
CPPFLAGS += -DHELENGINE_GAMECUBE_HAS_GENERATED_CORE=1 -I$(HELENGINE_CORE_CPP_ROOT)
ifneq ($(wildcard $(HELENGINE_CORE_CPP_ROOT)/Physics3DRuntimeComponentRegistration.hpp),)
CPPFLAGS += -DHELENGINE_GAMECUBE_HAS_PHYSICS3D_RUNTIME_REGISTRATION=1
else
CPPFLAGS += -DHELENGINE_GAMECUBE_HAS_PHYSICS3D_RUNTIME_REGISTRATION=0
endif
endif

ALL_SOURCE_SOURCES := $(SOURCES) $(GENERATED_BRIDGE_SOURCES)
OBJECTS := $(patsubst $(SOURCE_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(ALL_SOURCE_SOURCES))

ifneq ($(strip $(GENERATED_CORE_SOURCE)),)
OBJECTS += $(BUILD_DIR)/generated/$(GENERATED_CORE_TRANSLATION_UNIT:.cpp=.o)
endif

CXXFLAGS := \
	-std=gnu++20 \
	-O2 \
	-Wall \
	-Wextra \
	$(MACHDEP) \
	-ffunction-sections \
	-fdata-sections

APPLOADER_CFLAGS := \
	-std=gnu11 \
	-O2 \
	-Wall \
	-Wextra \
	$(MACHDEP) \
	-ffreestanding \
	-fno-builtin \
	-I$(APPLOADER_SOURCE_ROOT)/ppc/include \
	-I$(APPLOADER_SOURCE_ROOT)/include

APPLOADER_OBJECTS := \
	$(PACKAGED_DISC_BUILD_DIR)/apploader.o \
	$(PACKAGED_DISC_BUILD_DIR)/lib.o \
	$(PACKAGED_DISC_BUILD_DIR)/misc.o

CUBEBOOT_APPLOADER_OBJECTS := \
	$(PACKAGED_DISC_BUILD_DIR)/cubeboot-apploader.o \
	$(PACKAGED_DISC_BUILD_DIR)/lib.o \
	$(PACKAGED_DISC_BUILD_DIR)/misc.o

MKGBI_CFLAGS := \
	-std=gnu11 \
	-O2 \
	-Wall \
	-Wextra \
	-I$(APPLOADER_SOURCE_ROOT)/include

LDFLAGS := \
	$(MACHDEP) \
	-L$(LIBOGC_GAMECUBE_LIB_DIR) \
	-L$(LIBOGC)/lib \
	-Wl,-Map,$(BUILD_DIR)/helengine_gc.map \
	-Wl,--gc-sections

LDLIBS := \
	-logc \
	-ldb \
	-lm

.PHONY: all clean packaged-disc-assets

all: $(TARGET_DOL)

packaged-disc-assets: $(PACKAGED_DISC_APPLOADER_IMAGE)
packaged-disc-assets: $(PACKAGED_DISC_GENERIC_BOOT_IMAGE)

$(TARGET_DOL): $(TARGET_ELF)
	$(ELF2DOL) $< $@

$(TARGET_ELF): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CXX) $(OBJECTS) $(LDFLAGS) $(LDLIBS) -o $@

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/generated/$(GENERATED_CORE_TRANSLATION_UNIT:.cpp=.o): $(GENERATED_CORE_SOURCE)
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(PACKAGED_DISC_APPLOADER_RAW_IMAGE): $(PACKAGED_DISC_APPLOADER_ELF)
	@mkdir -p $(dir $@)
	$(OBJCOPY) --strip-all -O binary $< $@

$(PACKAGED_DISC_APPLOADER_IMAGE): $(PACKAGED_DISC_MKAPPLOADER) $(PACKAGED_DISC_APPLOADER_RAW_IMAGE)
	@mkdir -p $(dir $@)
	ENTRY_POINT=`$(READELF) -h $(PACKAGED_DISC_APPLOADER_ELF) | awk '/Entry point address:/ { print $$4 }'`; \
	$(PACKAGED_DISC_MKAPPLOADER) $(PACKAGED_DISC_APPLOADER_RAW_IMAGE) $@ $$ENTRY_POINT

$(PACKAGED_DISC_APPLOADER_ELF): $(APPLOADER_OBJECTS) $(PACKAGED_DISC_APPLOADER_LINKER_SCRIPT)
	@mkdir -p $(dir $@)
	$(CC) -nostartfiles -nodefaultlibs -Wl,-Ttext=0x81200000 -Wl,-e,al_start -Wl,-T,$(PACKAGED_DISC_APPLOADER_LINKER_SCRIPT) $(APPLOADER_OBJECTS) -o $@

$(PACKAGED_DISC_CUBEBOOT_APPLOADER_IMAGE): $(PACKAGED_DISC_CUBEBOOT_APPLOADER_ELF)
	@mkdir -p $(dir $@)
	$(OBJCOPY) --strip-all -O binary $< $@

$(PACKAGED_DISC_CUBEBOOT_APPLOADER_ELF): $(CUBEBOOT_APPLOADER_OBJECTS) $(PACKAGED_DISC_APPLOADER_LINKER_SCRIPT)
	@mkdir -p $(dir $@)
	$(CC) -nostartfiles -nodefaultlibs -Wl,-Ttext=0x81200000 -Wl,-e,al_start -Wl,-T,$(PACKAGED_DISC_APPLOADER_LINKER_SCRIPT) $(CUBEBOOT_APPLOADER_OBJECTS) -o $@

$(PACKAGED_DISC_BUILD_DIR)/apploader.o: $(APPLOADER_SOURCE_ROOT)/ppc/apploader/retail_apploader.c $(APPLOADER_SOURCE_ROOT)/ppc/include/system.h $(APPLOADER_SOURCE_ROOT)/include/gcm.h $(APPLOADER_SOURCE_ROOT)/include/dol.h
	@mkdir -p $(dir $@)
	$(CC) $(APPLOADER_CFLAGS) -c $< -o $@

$(PACKAGED_DISC_BUILD_DIR)/cubeboot-apploader.o: $(APPLOADER_SOURCE_ROOT)/ppc/apploader/apploader.c $(APPLOADER_SOURCE_ROOT)/ppc/include/system.h $(APPLOADER_SOURCE_ROOT)/include/gcm.h $(APPLOADER_SOURCE_ROOT)/include/dol.h
	@mkdir -p $(dir $@)
	$(CC) $(APPLOADER_CFLAGS) -c $< -o $@

$(PACKAGED_DISC_BUILD_DIR)/lib.o: $(APPLOADER_SOURCE_ROOT)/ppc/common/lib.c $(APPLOADER_SOURCE_ROOT)/ppc/include/system.h $(APPLOADER_SOURCE_ROOT)/include/gcm.h
	@mkdir -p $(dir $@)
	$(CC) $(APPLOADER_CFLAGS) -c $< -o $@

$(PACKAGED_DISC_BUILD_DIR)/misc.o: $(APPLOADER_SOURCE_ROOT)/ppc/common/misc.S $(APPLOADER_SOURCE_ROOT)/ppc/include/system.h
	@mkdir -p $(dir $@)
	$(CC) $(MACHDEP) -c $< -o $@

$(PACKAGED_DISC_GENERIC_BOOT_IMAGE): $(PACKAGED_DISC_MKGBI) $(PACKAGED_DISC_CUBEBOOT_APPLOADER_IMAGE) $(APPLOADER_SOURCE_ROOT)/icons/opening.bnr
	@mkdir -p $(dir $@)
	$(PACKAGED_DISC_MKGBI) -a $(PACKAGED_DISC_CUBEBOOT_APPLOADER_IMAGE) -b $(APPLOADER_SOURCE_ROOT)/icons/opening.bnr -o $@

$(PACKAGED_DISC_MKAPPLOADER): $(PACKAGED_DISC_MKAPPLOADER_OBJECT)
	@mkdir -p $(dir $@)
	$(HOSTCC) $(PACKAGED_DISC_MKAPPLOADER_OBJECT) -o $@

$(PACKAGED_DISC_MKGBI): $(PACKAGED_DISC_MKGBI_OBJECT) $(PACKAGED_DISC_MKGBI_LIB_OBJECT)
	@mkdir -p $(dir $@)
	$(HOSTCC) $(PACKAGED_DISC_MKGBI_OBJECT) $(PACKAGED_DISC_MKGBI_LIB_OBJECT) -o $@

$(PACKAGED_DISC_MKAPPLOADER_OBJECT): $(APPLOADER_SOURCE_ROOT)/mkapploader/mkapploader.c
	@mkdir -p $(dir $@)
	$(HOSTCC) $(MKGBI_CFLAGS) -c $< -o $@

$(PACKAGED_DISC_MKGBI_OBJECT): $(APPLOADER_SOURCE_ROOT)/mkgbi/mkgbi.c $(APPLOADER_SOURCE_ROOT)/include/lib.h $(APPLOADER_SOURCE_ROOT)/include/gcm.h
	@mkdir -p $(dir $@)
	$(HOSTCC) $(MKGBI_CFLAGS) -c $< -o $@

$(PACKAGED_DISC_MKGBI_LIB_OBJECT): $(APPLOADER_SOURCE_ROOT)/common/lib.c $(APPLOADER_SOURCE_ROOT)/include/lib.h $(APPLOADER_SOURCE_ROOT)/include/gcm.h
	@mkdir -p $(dir $@)
	$(HOSTCC) $(MKGBI_CFLAGS) -c $< -o $@

clean:
	@rm -rf $(BUILD_DIR)
