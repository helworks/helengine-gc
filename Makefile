DEVKITPRO ?= /opt/devkitpro
DEVKITPPC ?= $(DEVKITPRO)/devkitPPC
LIBOGC ?= $(DEVKITPRO)/libogc
HELENGINE_CORE_CPP_ROOT ?=

include $(DEVKITPPC)/gamecube_rules

BUILD_DIR := build
TARGET_ELF := $(BUILD_DIR)/helengine_gc.elf
TARGET_DOL := $(BUILD_DIR)/helengine_gc.dol
SOURCE_DIR := src
LIBOGC_GAMECUBE_LIB_DIR := $(LIBOGC)/lib/cube
SOURCES := \
	$(SOURCE_DIR)/main.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeApplication.cpp
GENERATED_BRIDGE_SOURCES :=
GENERATED_CORE_SOURCE :=

CXX := $(DEVKITPPC)/bin/powerpc-eabi-g++
ELF2DOL := $(DEVKITPRO)/tools/bin/elf2dol

CPPFLAGS := \
	-I$(SOURCE_DIR) \
	-I$(LIBOGC)/include \
	-DGEKKO \
	-DHW_DOL=1 \
	-D__gamecube__=1

ifeq ($(strip $(HELENGINE_CORE_CPP_ROOT)),)
CPPFLAGS += -DHELENGINE_GAMECUBE_HAS_GENERATED_CORE=0
else
GENERATED_CORE_SOURCE := $(HELENGINE_CORE_CPP_ROOT)/helengine_core_unity.cpp
ifeq ($(wildcard $(HELENGINE_CORE_CPP_ROOT)/helcpp_config.hpp),)
$(error HELENGINE_CORE_CPP_ROOT does not contain helcpp_config.hpp)
endif
ifeq ($(wildcard $(GENERATED_CORE_SOURCE)),)
$(error HELENGINE_CORE_CPP_ROOT does not contain helengine_core_unity.cpp)
endif
GENERATED_BRIDGE_SOURCES := \
	$(SOURCE_DIR)/platform/gamecube/GameCubeInputManager.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeKeyboard.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeMouse.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeRenderManager2D.cpp \
	$(SOURCE_DIR)/platform/gamecube/GameCubeRenderManager3D.cpp
CPPFLAGS += -DHELENGINE_GAMECUBE_HAS_GENERATED_CORE=1 -I$(HELENGINE_CORE_CPP_ROOT)
endif

ALL_SOURCE_SOURCES := $(SOURCES) $(GENERATED_BRIDGE_SOURCES)
OBJECTS := $(patsubst $(SOURCE_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(ALL_SOURCE_SOURCES))

ifneq ($(strip $(GENERATED_CORE_SOURCE)),)
OBJECTS += $(BUILD_DIR)/generated/helengine_core_unity.o
endif

CXXFLAGS := \
	-std=gnu++17 \
	-O2 \
	-Wall \
	-Wextra \
	$(MACHDEP) \
	-ffunction-sections \
	-fdata-sections

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

$(BUILD_DIR)/generated/helengine_core_unity.o: $(GENERATED_CORE_SOURCE)
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -rf $(BUILD_DIR)
