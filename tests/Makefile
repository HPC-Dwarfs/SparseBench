# # # DL 2025.04.04
# # # Collection of unit tests

# Mirror the main Makefile: bootstrap a config on fresh clones instead of
# failing at the include below.
ifeq (,$(wildcard ../config.mk))
$(info )
$(info ====================================================================)
$(info config.mk does not exist!)
$(info Creating config.mk from ../mk/config-default.mk)
$(info Please adapt config.mk to your needs and run make again.)
$(info ====================================================================)
$(info )
$(shell cp ../mk/config-default.mk ../config.mk)
$(error Stopping after creating config.mk - please review and run make again)
endif
include ../config.mk
include ../mk/include_$(TOOLCHAIN).mk

BUILD_DIR   = ../build
TC_DIR      = $(BUILD_DIR)/$(MTX_FMT)-$(TOOLCHAIN)
CUDA_DIR    = ../src/cuda
# Test objects go into a config-specific directory so switching MTX_FMT or
# TOOLCHAIN cannot link stale objects compiled with different -D flags.
TEST_BUILD  = $(BUILD_DIR)/tests-$(MTX_FMT)-$(TOOLCHAIN)

# List of test modules
MOD1=matrix
MOD2=solver

# Link against all SparseBench object files (main.o excluded)
LINKS = $(shell ls $(TC_DIR)/*.o 2>/dev/null | grep -v main.o)

# Target executables
TARGET     = runTests
HALO_TARGET = haloCheck

# Collect objects from all test modules
# Module 1 (matrix) test objects: SELL-C-sigma conversion tests are SCS only
ifeq ($(MTX_FMT),SCS)
MOD1_OBJECTS=$(TEST_BUILD)/$(MOD1)/convertSCS.o $(TEST_BUILD)/$(MOD1)/matrixTests.o
else
MOD1_OBJECTS=
endif
MOD2_OBJECTS=$(TEST_BUILD)/$(MOD2)/spmvSCS.o $(TEST_BUILD)/$(MOD2)/spmmvSCS.o $(TEST_BUILD)/$(MOD2)/solverTestsSPMV.o $(TEST_BUILD)/$(MOD2)/solverTestsSPMMV.o $(TEST_BUILD)/$(MOD2)/solverTestsSPMVSplit.o
OBJECTS := $(TEST_BUILD)/runTests.o $(shell echo $(MOD1_OBJECTS) $(MOD2_OBJECTS) | tr ' ' '\n' | sort -u | tr '\n' ' ')

# Always leave debugging flag on; apply the same DEFINES/OPTIONS as the main
# build (ARRAY_ALIGNMENT and friends must match or allocations differ).
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

CPPFLAGS_TEST = $(DEFINES) $(OPTIONS) -I../src
CFLAGS = -g -O0 $(OPENMP)

# Rebuild all test objects when the effective flags change even if config.mk
# itself was not touched (command line overrides like FLOAT_TYPE=SP land in
# the same object directory otherwise). The recipe keeps the stamp mtime
# unchanged while the flags stay the same.
TEST_BUILD_FLAGS := '$(CC) $(CPPFLAGS_TEST) $(CFLAGS)'
TEST_FLAGS_STAMP := $(TEST_BUILD)/.build-flags

# The stamp rule precedes 'all'; keep the default goal pointing at runTests.
.DEFAULT_GOAL := all

$(TEST_FLAGS_STAMP): FORCE
	@mkdir -p $(@D)
	@printf '%s\n' $(TEST_BUILD_FLAGS) > $@.tmp
	@if cmp -s $@.tmp $@ 2>/dev/null; then rm -f $@.tmp; else mv -f $@.tmp $@; fi

FORCE:

.PHONY: all clean prebuild halo format FORCE

all: $(TARGET)

prebuild:
	@make -sC ../

clean:
	@make -sC ../ clean
	$(info ===>  CLEAN TESTS)
	@rm -rf $(TEST_BUILD) $(TARGET) $(HALO_TARGET)

# Link the object files to create the executable
$(TARGET): prebuild $(OBJECTS) $(LINKS)
	$(LD) $(LFLAGS) -o $@ $(OBJECTS) $(LINKS) $(LIBS)

# Pattern rule for all test objects (mirrors the tests/ directory layout under
# $(TEST_BUILD)). Depending on the build configuration files guarantees a
# rebuild when e.g. MTX_FMT-relevant flags change without changing TOOLCHAIN.
$(TEST_BUILD)/%.o: %.c ../config.mk ../mk/include_$(TOOLCHAIN).mk $(TEST_FLAGS_STAMP)
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS_TEST) $(CFLAGS) -c -o $@ $<

# MPI halo exchange regression test (separate binary, run it with mpirun)
halo: prebuild $(HALO_TARGET)

$(HALO_TARGET): prebuild $(TEST_BUILD)/halo/haloCheck.o $(LINKS)
	$(LD) $(LFLAGS) -o $@ $(TEST_BUILD)/halo/haloCheck.o $(LINKS) $(LIBS)

format:
	@make -sC ../ format
	@echo "Formatting test files..."
	@find . -type f \( -name '*.c' -o -name '*.h' \) -print0 | xargs -0 -r clang-format -i
	@echo "Done"
