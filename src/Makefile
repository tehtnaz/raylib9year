#**************************************************************************************************
#
#   raylib makefile for Desktop platforms, Raspberry Pi, Android and HTML5
#
#   Copyright (c) 2013-2022 Ramon Santamaria (@raysan5)
#
#   This software is provided "as-is", without any express or implied warranty. In no event
#   will the authors be held liable for any damages arising from the use of this software.
#
#   Permission is granted to anyone to use this software for any purpose, including commercial
#   applications, and to alter it and redistribute it freely, subject to the following restrictions:
#
#     1. The origin of this software must not be misrepresented; you must not claim that you
#     wrote the original software. If you use this software in a product, an acknowledgment
#     in the product documentation would be appreciated but is not required.
#
#     2. Altered source versions must be plainly marked as such, and must not be misrepresented
#     as being the original software.
#
#     3. This notice may not be removed or altered from any source distribution.
#
#**************************************************************************************************

.PHONY: all clean

# Define required environment variables
#------------------------------------------------------------------------------------------------
# Define target platform: PLATFORM_DESKTOP, PLATFORM_WEB
PLATFORM              ?= PLATFORM_DESKTOP
# ENABLE OR DISABLE MINGW COMPILE (LINUX ONLY)
MINGW                 ?= DISABLE

# Define project variables
PROJECT_NAME          ?= raylib_game
PROJECT_VERSION       ?= 0.1
PROJECT_BUILD_PATH    ?= ./../build/

# RAYLIB_PATH           ?= ../../raylib

# Library type compilation: STATIC (.a) or SHARED (.so/.dll)
RAYLIB_LIBTYPE        ?= STATIC

# Build mode for project: DEBUG or RELEASE
BUILD_MODE            ?= DEBUG

# Use Wayland display server protocol on Linux desktop (by default it uses X11 windowing system)
# NOTE: This variable is only used for PLATFORM_OS: LINUX
USE_WAYLAND_DISPLAY   ?= FALSE

# PLATFORM_WEB: Default properties
BUILD_WEB_ASYNCIFY    ?= FALSE
BUILD_WEB_SHELL       ?= minshell.html
BUILD_WEB_HEAP_SIZE   ?= 134217728
BUILD_WEB_RESOURCES   ?= TRUE
BUILD_WEB_RESOURCES_PATH  ?= ./../res

DEFAULT_INCLUDE_PATH  ?= ./../include

# Locations of raylib.h and libraylib.a/libraylib.so
RAYLIB_INCLUDE_PATH   ?= $(DEFAULT_INCLUDE_PATH)/raylib-5.0

# Determine PLATFORM_OS in case PLATFORM_DESKTOP selected
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    # No uname.exe on MinGW!, but OS=Windows_NT on Windows!
    # ifeq ($(UNAME),Msys) -> Windows
    ifeq ($(OS),Windows_NT)
        PLATFORM_OS = WINDOWS
        RAYLIB_LIB_PATH ?= ./../lib/raylib-5.0_win64_mingw-w64
    else
        PLATFORM_OS = LINUX
        RAYLIB_LIB_PATH ?= ./../lib/raylib-5.0_linux_amd64
        ifeq ($(MINGW),ENABLE)
            RAYLIB_LIB_PATH = ./../lib/raylib-5.0_win64_mingw-w64
        endif
    endif
endif

ifeq ($(PLATFORM),PLATFORM_WEB)
    RAYLIB_LIB_PATH ?= ./../lib/raylib-5.0_webassembly
endif


ifeq ($(PLATFORM),PLATFORM_WEB)
    # Emscripten required variables
    EMSDK_PATH         ?= ~/emsdk
    EMSCRIPTEN_PATH    ?= $(EMSDK_PATH)/upstream/emscripten
    EMSCRIPTEN_INCLUDE_PATH = $(EMSCRIPTEN_PATH)/cache/sysroot/include
    CLANG_PATH          = $(EMSDK_PATH)/upstream/bin
    PYTHON_PATH         = $(EMSDK_PATH)/python/3.9.2-1_64bit
    NODE_PATH           = $(EMSDK_PATH)/node/14.15.5_64bit/bin
    export PATH         = $(EMSDK_PATH);$(EMSCRIPTEN_PATH);$(CLANG_PATH);$(NODE_PATH);$(PYTHON_PATH):$$(PATH)
endif

$(info $(EMSCRIPTEN_PATH))

# Define default C compiler: CC
#------------------------------------------------------------------------------------------------
CC = gcc

ifeq ($(PLATFORM),PLATFORM_WEB)
    # HTML5 emscripten compiler
    # WARNING: To compile to HTML5, code must be redesigned
    # to use emscripten.h and emscripten_set_main_loop()
    CC = emcc
endif

ifeq ($(MINGW), ENABLE)
    CC = x86_64-w64-mingw32-gcc
endif

# Define default make program: MAKE
#------------------------------------------------------------------------------------------------
MAKE ?= make

ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),WINDOWS)
        MAKE = mingw64-make
    endif
endif

# Define compiler flags: CFLAGS
#------------------------------------------------------------------------------------------------
#  -O1                  defines optimization level
#  -g                   include debug information on compilation
#  -s                   strip unnecessary data from build
#  -Wall                turns on most, but not all, compiler warnings
#  -std=c99             defines C language mode (standard C from 1999 revision)
#  -std=gnu99           defines C language mode (GNU C from 1999 revision)
#  -Wno-missing-braces  ignore invalid warning (GCC bug 53119)
#  -Wno-unused-value    ignore unused return values of some functions (i.e. fread())
#  -D_DEFAULT_SOURCE    use with -std=c99 on Linux and PLATFORM_WEB, required for timespec
CFLAGS = -std=c99 -Wall -Wextra -Wmissing-braces -Wunused-result -Wno-sign-compare -D_DEFAULT_SOURCE

ifeq ($(BUILD_MODE), A)
    CFLAGS += -fno-exceptions -fno-web -fno-var-tracking-assignments -fno-var-tracking -fno-unwind-tables -fno-tree-scev-cprop  -fno-tree-reassoc -fno-tree-phiprop -fno-tree-loop-optimize -fno-tree-loop-ivcanon -fno-tree-loop-im -fno-tree-loop-if-convert -fno-tree-forwprop -fno-tree-cselim -fno-trapping-math -fthreadsafe-statics -fno-strict-volatile-bitfields -fno-stdarg-opt -fno-ssa-backprop  -fno-split-ivs-in-unroller -fno-signed-zeros -fno-shrink-wrap-separate -fno-short-enums -fno-schedule-fusion -fno-sched-stalled-insns-dep -fno-sched-spec-insn-heuristic -fno-sched-spec -fno-sched-rank-heuristic -fno-sched-last-insn-heuristic -fno-sched-interblock  -fno-sched-group-heuristic -fno-sched-dep-count-heuristic -fno-sched-critical-path-heuristic -fno-rtti -fno-rename-registers -fno-reg-struct-return -fno-printf-return-value -fno-prefetch-loop-arrays -fno-plt  -fno-peephole -fno-math-errno -fno-lifetime-dse -fno-jump-tables -fno-ivopts -fno-ira-share-spill-slots -fno-ira-share-save-slots -fno-ira-hoist-pressure -fno-inline-atomics -fno-inline -fno-gcse-lm -fno-function-cse -fno-fp-int-builtin-inexact -fno-early-inlining -fno-dse -fno-delete-null-pointer-checks -fno-dce -fno-auto-inc-dec -fno-asynchronous-unwind-tables -fno-aggressive-loop-optimizations
endif

ifeq ($(BUILD_MODE),DEBUG)
    CFLAGS += -g -D_DEBUG
else
    ifeq ($(PLATFORM),PLATFORM_WEB)
        ifeq ($(BUILD_WEB_ASYNCIFY),TRUE)
            CFLAGS += -O3
        else
            CFLAGS += -Os
        endif
    else
        CFLAGS += -s -O2
    endif
endif

# Additional flags for compiler (if desired)
#CFLAGS += -Wextra -Wmissing-prototypes -Wstrict-prototypes

# Define include paths for required headers: INCLUDE_PATHS
# NOTE: Some external/extras libraries could be required (stb, physac, easings...)
#------------------------------------------------------------------------------------------------
INCLUDE_PATHS := -I. -I $(RAYLIB_INCLUDE_PATH) -I $(DEFAULT_INCLUDE_PATH)/physac -I ./dataHandling -I ./animation -I ./triggers -I ./displayText -I ./levelObjects

ifeq ($(PLATFORM),PLATFORM_WEB)
    INCLUDE_PATHS += -I $(EMSCRIPTEN_INCLUDE_PATH)
endif

$(info $(INCLUDE_PATHS))

# Define library paths containing required libs: LDFLAGS
#------------------------------------------------------------------------------------------------
LDFLAGS = -L. -L $(RAYLIB_LIB_PATH)
# ifeq ($(PLATFORM),PLATFORM_DESKTOP)
#     ifeq ($(PLATFORM_OS),WINDOWS)
#         # NOTE: The resource .rc file contains windows executable icon and properties
#         LDFLAGS += $(RAYLIB_PATH)/src/raylib.rc.data
#         # -Wl,--subsystem,windows hides the console window
#         ifeq ($(BUILD_MODE), RELEASE)
#             LDFLAGS += -Wl,--subsystem,windows
#         endif
#     endif
#     ifeq ($(PLATFORM_OS),LINUX)
        
#     endif
# endif

ifeq ($(PLATFORM),PLATFORM_WEB)
    # -Os                        # size optimization
    # -O2                        # optimization level 2, if used, also set --memory-init-file 0
    # -s USE_GLFW=3              # Use glfw3 library (context/input management)
    # -s ALLOW_MEMORY_GROWTH=1   # to allow memory resizing -> WARNING: Audio buffers could FAIL!
    # -s TOTAL_MEMORY=16777216   # to specify heap memory size (default = 16MB) (67108864 = 64MB)
    # -s USE_PTHREADS=1          # multithreading support
    # -s WASM=0                  # disable Web Assembly, emitted by default
    # -s ASYNCIFY                # lets synchronous C/C++ code interact with asynchronous JS
    # -s FORCE_FILESYSTEM=1      # force filesystem to load/save files data
    # -s ASSERTIONS=1            # enable runtime checks for common memory allocation errors (-O1 and above turn it off)
    # --profiling                # include information for code profiling
    # --memory-init-file 0       # to avoid an external memory initialization code file (.mem)
    # --preload-file resources   # specify a resources folder for data compilation
    # --source-map-base          # allow debugging in browser with source map
    LDFLAGS += -s USE_GLFW=3 -s TOTAL_MEMORY=$(BUILD_WEB_HEAP_SIZE) -s FORCE_FILESYSTEM=1
    
    # Build using asyncify
    ifeq ($(BUILD_WEB_ASYNCIFY),TRUE)
        LDFLAGS += -s ASYNCIFY
    endif
    
    # Add resources building if required
    ifeq ($(BUILD_WEB_RESOURCES),TRUE)
        LDFLAGS += --preload-file $(BUILD_WEB_RESOURCES_PATH)
    endif
    
    # Add debug mode flags if required
    ifeq ($(BUILD_MODE),DEBUG)
        LDFLAGS += -s ASSERTIONS=1 --profiling
    endif

    # Define a custom shell .html and output extension
    LDFLAGS += --shell-file $(BUILD_WEB_SHELL)
    EXT = .html
endif

# Define libraries required on linking: LDLIBS
# NOTE: To link libraries (lib<name>.so or lib<name>.a), use -l<name>
#------------------------------------------------------------------------------------------------
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),WINDOWS)
        # Libraries for Windows desktop compilation
        # NOTE: WinMM library required to set high-res timer resolution
        LDLIBS = -lraylib -lopengl32 -lgdi32 -lwinmm
        # Required for physac examples
        LDLIBS += -static -lpthread
        EXT = .exe
    endif
    ifeq ($(PLATFORM_OS),LINUX)
        # Libraries for Debian GNU/Linux desktop compiling
        # NOTE: Required packages: libegl1-mesa-dev
        LDLIBS = -lraylib -lGL -lm -lpthread -ldl -lrt

        # On X11 requires also below libraries
        LDLIBS += -lX11
        # NOTE: It seems additional libraries are not required any more, latest GLFW just dlopen them
        #LDLIBS += -lXrandr -lXinerama -lXi -lXxf86vm -lXcursor

        # On Wayland windowing system, additional libraries requires
        ifeq ($(USE_WAYLAND_DISPLAY),TRUE)
            LDLIBS += -lwayland-client -lwayland-cursor -lwayland-egl -lxkbcommon
        endif
    endif
    ifeq ($(MINGW),ENABLE)
        # Libraries for Windows desktop compilation
        # NOTE: WinMM library required to set high-res timer resolution
        LDLIBS = -lraylib -lopengl32 -lgdi32 -lwinmm
        # Required for physac examples
        LDLIBS += -static -lpthread

        LDFLAGS += -fstack-protector 
        LDLIBS += -lssp -lmingw32
        EXT = .exe
    endif
endif

ifeq ($(PLATFORM),PLATFORM_WEB)
    # Libraries for web (HTML5) compiling
    # LDLIBS = $(RAYLIB_LIB_PATH)/libraylib.a
    LDLIBS = -lraylib
endif

# Define source code object files required
#------------------------------------------------------------------------------------------------
PROJECT_SOURCE_FILES ?= \
    raylib_game.c \
    dataHandling/dataHandling.c \
    animation/animation.c \
    animation/animatedColliders.c \
    triggers/triggers.c \
    displayText/displayText.c \
    levelObjects/levelObjects.c

# Define all object files from source files
# OBJS = $(patsubst %.c, %.o, $(PROJECT_SOURCE_FILES))


# Define processes to execute
#------------------------------------------------------------------------------------------------

# $(info $(CC) -o $(PROJECT_BUILD_PATH)$(PROJECT_NAME)-$(PROJECT_VERSION)$(EXT) $(PROJECT_SOURCE_FILES) $(CFLAGS) $(INCLUDE_PATHS) $(LDFLAGS) $(LDLIBS) -D$(PLATFORM))

# Default target entry
# NOTE: We call this Makefile target or Makefile.Android target
all:
	$(MAKE) $(PROJECT_NAME)

# Project target defined by PROJECT_NAME
$(PROJECT_NAME): $(OBJS)
	$(CC) -o $(PROJECT_BUILD_PATH)$(PROJECT_NAME)-$(PROJECT_VERSION)$(EXT) $(PROJECT_SOURCE_FILES) $(CFLAGS) $(INCLUDE_PATHS) $(LDFLAGS) $(LDLIBS) -D$(PLATFORM)

# Clean everything
clean:
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),WINDOWS)
		del *.o *.exe /s
    endif
    ifeq ($(PLATFORM_OS),LINUX)
		find . -type f -executable -delete
		rm -fv *.o
    endif
endif
ifeq ($(PLATFORM),PLATFORM_WEB)
	del *.o *.html *.js
endif
	@echo Cleaning done

