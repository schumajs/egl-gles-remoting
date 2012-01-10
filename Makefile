###############################################################################
# make target [CONCEPT=1-1]
# target:
#   all
#   clean / cleanall (+./docs)
#   docs
#   sharedlib
#   clientlib 
#   serverlib
#   dashboard
#   democlient

###############################################################################
# Archiver / compiler / linker

AR	 := ar
CC     	 := gcc
CFLAGS 	 := -Wall

VER_MAJ  := 1
VER_MIN  := 0
VER_REV  := 1

###############################################################################
# Shared lib

SH_TAR := sharedlib
SH_BIN := gvshared

SH_C0_OBJS  := dispatcher.0.o lock.0.o shared_memory.0.o sleep.0.o transport.0.o
SH_C1_OBJS  :=

###############################################################################
# Client lib

CL_TAR := clientlib
CL_BIN := gvclient

CL_C0_OBJS  := client/dispatcher.0.o client/egl_gles.0.o client/memory_manager.0.o client/server.0.o
CL_C1_OBJS  :=

###############################################################################
# Server lib

SL_TAR := serverlib
SL_BIN := gvserver

SL_C0_OBJS  := server/dispatcher.0.o server/egl_gles.0.o server/memory_manager.0.o server/server.0.o
SL_C1_OBJS  :=

###############################################################################
# Conditionals and rules

ifndef CONCEPT
	CONCEPT := 0
endif

ifeq ($(CONCEPT), 0)
	CL_OBJS := $(CL_C0_OBJS:%=src/%)
	SL_OBJS := $(SL_C0_OBJS:%=src/%)
        SH_OBJS := $(SH_C0_OBJS:%=src/%)
else ifeq ($(CONCEPT), 1)
	CL_OBJS := $(CL_C1_OBJS:%=src/%)
	SL_OBJS := $(SL_C1_OBJS:%=src/%)
	SH_OBJS := $(SH_C1_OBJS:%=src/%)
endif

CL_BIN_LIB := lib$(CL_BIN)
CL_BIN_MAJ := $(CL_BIN_LIB).so.$(VER_MAJ)
CL_BIN_REV := $(CL_BIN_MAJ).$(VER_MIN).$(VER_REV)
SL_BIN_LIB := lib$(SL_BIN)
SL_BIN_MAJ := $(SL_BIN_LIB).so.$(VER_MAJ)
SL_BIN_REV := $(SL_BIN_MAJ).$(VER_MIN).$(VER_REV)
SH_BIN_LIB := lib$(SH_BIN)
SH_BIN_MAJ := $(SH_BIN_LIB).so.$(VER_MAJ)
SH_BIN_REV := $(SH_BIN_MAJ).$(VER_MIN).$(VER_REV)

all: $(SH_TAR) $(CL_TAR) $(SL_TAR) dashboard democlient

$(CL_TAR) $(SL_TAR) $(SH_TAR): CFLAGS += -fPIC

$(SH_TAR): $(SH_OBJS)
	$(CC) -shared -Wl,-soname,$(SH_BIN_MAJ) -o $(SH_BIN_REV) $(SH_OBJS) -lpthread -lvrb
	ln -sf $(SH_BIN_REV) $(SH_BIN_MAJ)
	ln -sf $(SH_BIN_REV) $(SH_BIN_LIB)

$(CL_TAR): $(SH_TAR) $(CL_OBJS)
	$(CC) -shared -Wl,-rpath,.,-soname,$(CL_BIN_MAJ) -o $(CL_BIN_REV) $(CL_OBJS) $(SH_BIN_LIB)
	ln -sf $(CL_BIN_REV) $(CL_BIN_MAJ)
	ln -sf $(CL_BIN_REV) $(CL_BIN_LIB)

$(SL_TAR): $(SH_TAR) $(SL_OBJS)
	$(CC) -shared -Wl,-rpath,.,-soname,$(SL_BIN_MAJ) -o $(SL_BIN_REV) $(SL_OBJS) $(SH_BIN_LIB) -ldlmalloc -lEGL -lGLESv2
	ln -sf $(SL_BIN_REV) $(SL_BIN_MAJ)
	ln -sf $(SL_BIN_REV) $(SL_BIN_LIB)

dashboard: $(SH_TAR) $(SL_TAR)
	$(CC) -Iinclude -Wl,-rpath,. -o gvdb src/dashboard.c $(SH_BIN_LIB) $(SL_BIN_LIB) -lncurses

democlient: $(CL_TAR)
	$(CC) -Iinclude -Wl,-rpath,. -o gvdc src/democlient.c $(SH_BIN_LIB) $(CL_BIN_LIB)

%.o: %.c
	$(CC) -Iinclude $(CFLAGS) -c $< -o $@

.PHONY: clean cleanall docs

clean:
	rm -f $(SH_OBJS) $(SH_BIN_LIB) $(SH_BIN_MAJ) $(SH_BIN_REV) 
	rm -f $(CL_OBJS) $(CL_BIN_LIB) $(CL_BIN_MAJ) $(CL_BIN_REV) 
	rm -f $(SL_OBJS) $(SL_BIN_LIB) $(SL_BIN_MAJ) $(SL_BIN_REV) 
	rm -f src/dashboard.o gvdb
	rm -f src/gvdc.o gvdc

cleanall: clean
	rm -R docs

docs:
	doxygen default.doxygen