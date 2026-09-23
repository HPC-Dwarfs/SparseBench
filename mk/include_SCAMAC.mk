# Copyright (C) NHR@FAU, University Erlangen-Nuremberg.
# All rights reserved. This file is part of SparseBench.
# Use of this source code is governed by a MIT-style
# license that can be found in the LICENSE file.

# ScaMaC matrix generation.
#
# ScaMaC (Scalable Matrix Collection, https://bitbucket.org/essex/matrixcollection,
# modified BSD license) generates application matrices of scalable size from
# parameter strings, e.g.  -m scamac:Anderson,Lx=100,Ly=100,Lz=100,ranpot=2.5
#
# It is bundled in ext/scamac and always built as a static library with the
# same toolchain as SparseBench (see the $(SCAMAC_LIB) rule in the Makefile).
SCAMAC_DIR = ./ext/scamac
SCAMAC_LIB = $(SCAMAC_DIR)/build/$(TOOLCHAIN)/libscamac.a
SCAMAC_SRC = $(wildcard $(SCAMAC_DIR)/src/*.[ch] $(SCAMAC_DIR)/includes/*.[ch])

INCLUDES += -I$(SCAMAC_DIR)/includes
LIBS     += $(SCAMAC_LIB) -lm
