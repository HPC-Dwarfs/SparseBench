# Copyright (C) NHR@FAU, University Erlangen-Nuremberg.
# All rights reserved. This file is part of SparseBench.
# Use of this source code is governed by a MIT-style
# license that can be found in the LICENSE file.

#CONFIGURE BUILD SYSTEM
TARGET	   = sparseBench-$(MTX_FMT)-$(TOOLCHAIN)
BUILD_DIR  = ./build/$(MTX_FMT)-$(TOOLCHAIN)
SRC_DIR    = ./src
CUDA_DIR   = ./src/cuda
MAKE_DIR   = ./mk
Q         ?= @

# The stamp-file rule below precedes the link rule; pin the default goal so a
# bare `make` always builds the application.
.DEFAULT_GOAL := $(TARGET)

#DO NOT EDIT BELOW
ifeq (,$(wildcard config.mk))
$(info )
$(info ====================================================================)
$(info config.mk does not exist!)
$(info Creating config.mk from ./mk/config-default.mk)
$(info Please adapt config.mk to your needs and run make again.)
$(info ====================================================================)
$(info )
$(shell cp ./mk/config-default.mk config.mk)
$(error Stopping after creating config.mk - please review and run make again)
endif
include config.mk
include $(MAKE_DIR)/include_$(TOOLCHAIN).mk
INCLUDES  += -I$(SRC_DIR) -I$(BUILD_DIR)

# Derived feature options. A config.mk copied from an older release predates
# the overlap switches entirely (the variables are not even assigned there), so
# supply the defaults and derived defines here. This keeps ENABLE_OVERLAP=true
# effective instead of silently falling back to the blocking exchange.
# (Duplicate -D flags with identical values are harmless.)
ENABLE_OVERLAP ?= true
ifeq ($(strip $(ENABLE_OVERLAP)),true)
ifeq (,$(findstring -DENABLE_OVERLAP,$(DEFINES)))
DEFINES += -DENABLE_OVERLAP
endif
endif
OVERLAP_NUDGE_CHUNKS ?= 8
ifeq (,$(findstring -DOVERLAP_NUDGE_CHUNKS,$(OPTIONS)))
OPTIONS += -DOVERLAP_NUDGE_CHUNKS=$(OVERLAP_NUDGE_CHUNKS)
endif

VPATH     = $(SRC_DIR)
ASM       = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.s,$(wildcard $(SRC_DIR)/*.c))
OBJ       = $(filter-out $(BUILD_DIR)/matrix-%, $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o,$(wildcard $(SRC_DIR)/*.c)))
SRC       = $(wildcard $(SRC_DIR)/*.h $(SRC_DIR)/*.c $(CUDA_DIR)/*.h $(CUDA_DIR)/*.cu)
CPPFLAGS := $(CPPFLAGS) $(DEFINES) $(OPTIONS) $(INCLUDES)

# GPU kernel objects (when TOOLCHAIN=NVCC or HIP)
CUDA_SRC  = $(wildcard $(CUDA_DIR)/*.cu)
CUDA_OBJ  = $(patsubst $(CUDA_DIR)/%.cu, $(BUILD_DIR)/cuda_%.o, $(CUDA_SRC))
ifneq (,$(filter $(TOOLCHAIN),NVCC HIP))
  CPPFLAGS += -D_GPU
  ALL_OBJ   = $(OBJ) $(BUILD_DIR)/matrix-$(MTX_FMT).o $(CUDA_OBJ)
else
  ALL_OBJ   = $(OBJ) $(BUILD_DIR)/matrix-$(MTX_FMT).o
endif

# Rebuild all objects when the effective build flags change even if neither
# config.mk nor the toolchain include changed (e.g. FLOAT_TYPE=SP,
# ENABLE_MPI=false or USE_COMPLEX_ELEMENTS=true passed on the command line
# still land in the same build directory). The always-run recipe below keeps
# the stamp file's mtime unchanged while the flags stay the same, so builds
# with identical flags are not re-triggered.
BUILD_FLAGS := '$(CC) $(NVCC) $(CPPFLAGS) $(CFLAGS) $(NVCCFLAGS) $(LFLAGS)'
FLAGS_STAMP := $(BUILD_DIR)/.build-flags

$(FLAGS_STAMP): FORCE
	@mkdir -p $(@D)
	@printf '%s\n' $(BUILD_FLAGS) > $@.tmp
	@if cmp -s $@.tmp $@ 2>/dev/null; then rm -f $@.tmp; else mv -f $@.tmp $@; fi

FORCE:

c := ,
clist = $(subst $(eval) ,$c,$(strip $1))

define CLANGD_TEMPLATE
CompileFlags:
  Add: [$(call clist,$(CPPFLAGS)), $(call clist,$(CFLAGS)), -xc]
  Compiler: clang
endef

# $(BUILD_DIR) is order-only: the flags stamp creates/removes a temp file in
# it on every run, which would otherwise make the directory look 'newer' and
# cause a pointless relink on alternating invocations.
${TARGET}: .clangd $(ALL_OBJ) | $(BUILD_DIR)
	$(info ===>  LINKING  $(TARGET))
	$(Q)${LD} ${LFLAGS} -o $(TARGET) $(ALL_OBJ) $(LIBS)

$(BUILD_DIR)/%.o:  %.c $(MAKE_DIR)/include_$(TOOLCHAIN).mk config.mk $(FLAGS_STAMP)
	$(info ===>  COMPILE  $@)
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@
	$(Q)$(CC) $(CPPFLAGS) -MT $(@:.d=.o) -MM  $< > $(BUILD_DIR)/$*.d

$(BUILD_DIR)/cuda_%.o: $(CUDA_DIR)/%.cu $(MAKE_DIR)/include_$(TOOLCHAIN).mk config.mk $(FLAGS_STAMP)
	$(info ===>  COMPILE CUDA  $@)
	$(NVCC) -c $(NVCCFLAGS) $(DEFINES) $(OPTIONS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/%.s:  %.c
	$(info ===>  GENERATE ASM  $@)
	$(CC) -S $(CPPFLAGS) $(CFLAGS) $< -o $@

.PHONY: clean distclean info asm format FORCE

clean:
	$(info ===>  CLEAN)
	@rm -rf $(BUILD_DIR)

distclean:
	$(info ===>  DIST CLEAN)
	@rm -rf build
	@rm -f sparseBench-*
	@rm -f compile_commands.json
	@rm -f tags .clangd out*

info:
	$(info $(CFLAGS))
	$(Q)$(CC) $(VERSION)

asm:  $(BUILD_DIR) $(ASM)

format:
	@for src in $(SRC) ; do \
		echo "Formatting $$src" ; \
		clang-format -i $$src ; \
	done
	@echo "Done"

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

.clangd:
	$(file > .clangd,$(CLANGD_TEMPLATE))

-include $(OBJ:.o=.d)
