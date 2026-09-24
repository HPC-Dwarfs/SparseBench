# Copyright (C) NHR@FAU, University Erlangen-Nuremberg.
# All rights reserved. This file is part of SparseBench.
# Use of this source code is governed by a MIT-style
# license that can be found in the LICENSE file.

# Builds the ScaMaC library as a static archive (libscamac.a).
# Invoked from the SparseBench Makefile; see mk/include_SCAMAC.mk.

#CONFIGURE BUILD SYSTEM
TARGET          = $(BUILD_DIR)/libscamac.a
BUILD_DIR       = ./build/$(TOOLCHAIN)
SRC_DIR         = ./src
INC_DIR         = ./includes
MAKE_DIR        = ../../mk
Q              ?= @

#DO NOT EDIT BELOW
include ../../config.mk
include $(MAKE_DIR)/include_$(TOOLCHAIN).mk

AR       ?= ar
ARFLAGS   = rcs
# Match the CMake default build type (Release), which defines NDEBUG.
# Only the toolchain's CC/CFLAGS are reused; SparseBench's DEFINES/OPTIONS
# (-DCRS, -DPRECISION, ...) are not meant for this library.
CPPFLAGS := -DNDEBUG -I$(SRC_DIR) -I$(INC_DIR)

VPATH     = $(SRC_DIR)
OBJ       = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o,$(wildcard $(SRC_DIR)/*.c))
SRC       = $(wildcard $(SRC_DIR)/*.h $(SRC_DIR)/*.c)

$(TARGET): $(OBJ)
	$(info ===>  ARCHIVE  $(TARGET))
	$(Q)rm -f $@
	$(Q)$(AR) $(ARFLAGS) $@ $(OBJ)

$(BUILD_DIR)/%.o:  %.c $(MAKE_DIR)/include_$(TOOLCHAIN).mk | $(BUILD_DIR)
	$(info ===>  COMPILE  $@)
	$(Q)$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@
	$(Q)$(CC) $(CPPFLAGS) -MT $@ -MM  $< > $(BUILD_DIR)/$*.d


.PHONY: clean distclean format

clean:
	$(info ===>  CLEAN)
	@rm -rf $(BUILD_DIR)

distclean:
	$(info ===>  DIST CLEAN)
	@rm -rf build

format:
	@for src in $(SRC) ; do \
		echo "Formatting $$src" ; \
		clang-format -i $$src ; \
	done
	@echo "Done"

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

-include $(OBJ:.o=.d)
