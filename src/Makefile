#*************************************************************************
# Copyright (c) 2002 The University of Chicago, as Operator of Argonne
# National Laboratory.
# Copyright (c) 2002 The Regents of the University of California, as
# Operator of Los Alamos National Laboratory.
# This file is distributed subject to a Software License Agreement found
# in the file LICENSE that is included with this distribution. 
#*************************************************************************
#
# Makefile,v 1.16 2012/08/15 19:56:48 jba Exp
#
TOP = ..
include $(TOP)/configure/CONFIG

CMPLR = STRICT

HOST_OPT=

INC = chandata.h

#USR_CFLAGS += -I$(EPICS_EXTENSIONS_INCLUDE)
USR_CFLAGS += -DACCESS_SECURITY -D_NO_PROTO -DUSE_EZCA

LIBSRCS := \
	hash3.c cafunc.c caevent.c caQueue.c caScan.c caWF.c \
	caDb.c caRead.c 

LIBRARY_HOST := Lca


ifdef BASE_3_15
Lca_LIBS = dbCore ca Com
PROD_LIBS = Lca dbCore ca Com
else
Lca_LIBS = dbStaticHost ca Com
PROD_LIBS = Lca dbStaticHost ca Com
endif

dbStaticHost_DIR = $(EPICS_BASE_LIB)
ezca_DIR = $(INSTALL_LIB)
Lca_DIR = . 
ca_DIR = $(EPICS_BASE_LIB)
Com_DIR = $(EPICS_BASE_LIB)

caGet1_SRCS = caGet1.c
caPut1_SRCS = caPut1.c
caInfo1_SRCS = caInfo1.c
caInfo_SRCS = caInfo.c

PROD_HOST_WIN32 = caGet1 caPut1 caInfo1
PROD_HOST_DEFAULT = caGet1 caPut1 caInfo1 caInfo

hash3_CFLAGS += -O
caevent_CFLAGS += -O
caQueue_CFLAGS += -O
caScan_CFLAGS += -O
caWF_CFLAGS += -O
caDb_CFLAGS += -O
caRead_CFLAGS += -O
caGet1_CFLAGS += -O
caPut1_CFLAGS += -O
caInfo1_CFLAGS += -O
caInfo_CFLAGS += -O

include $(TOP)/configure/RULES

