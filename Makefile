###############################################################################
# make target
# target:
#   all
#   clean / cleanall (+./docs)
#   docs
#   sharedlib
#   clientlib 
#   serverlib

###############################################################################
# Archiver / compiler / linker

AR	 := ar
CC     	 := gcc
CFLAGS   := -Wall -fPIC
LD_FLAGS :=

SRCS  	 := src

###############################################################################
# Shared lib

SHARED_LIB_BIN     := gvshared
SHARED_LIB_OBJS    := 			\
	cond_var.o			\
	lock.o				\
	process_state_map.o		\
	rwlock.o			\
	serializer.o			\
	shared_memory.o			\
	sleep.o				\
	thread_state_map.o		\
	shm_stream_transport.o
SHARED_LIB_OBJS     := $(SHARED_LIB_OBJS:%=src/%)
SHARED_LIB_LIBS     := -lpthread -lvrb
SHARED_LIB_VERSION  := 1.0.1

###############################################################################
# Client lib

CLIENT_LIB_BIN      := gvclient
CLIENT_LIB_OBJS     :=		\
	client_egl_gles.o	\
	client_heap_manager.o	\
	client_janitor.o	\
	client_serializer.o	\
	client_state_tracker.o
CLIENT_LIB_OBJS     := $(CLIENT_LIB_OBJS:%=src/%)
CLIENT_LIB_VERSION  := 1.0.1

###############################################################################
# Server lib

SERVER_LIB_BIN      := gvserver
SERVER_LIB_OBJS     :=		\
	server_egl_gles.o	\
	server_dispatcher.o	\
	server_heap_manager.o	\
	server_janitor.o	\
	server_serializer.o	\
	server_state_tracker.o
SERVER_LIB_OBJS     := $(SERVER_LIB_OBJS:%=src/%)
SERVER_LIB_LIBS     := -ldlmalloc -lEGL -lGLESv2
SERVER_LIB_VERSION  := 1.0.1

###############################################################################
# Conditionals and rules

#
# All
#

all: sharedlib clientlib serverlib

#
# Phony
#

.PHONY: clean cleanall docs

#
# Shared lib
#

sharedlib: $(SHARED_LIB_OBJS)
	$(CC) -shared -Wl,-soname,lib$(SHARED_LIB_BIN).so.$(firstword $(subst ., ,$(SHARED_LIB_VERSION))) -o lib$(SHARED_LIB_BIN).so.$(SHARED_LIB_VERSION) $(SHARED_LIB_OBJS) $(SHARED_LIB_LIBS)

#
# Client lib
#

clientlib: sharedlib $(CLIENT_LIB_OBJS)
	$(CC) -shared -Wl,-soname,lib$(CLIENT_LIB_BIN).so.$(firstword $(subst ., ,$(CLIENT_LIB_VERSION))) -o lib$(CLIENT_LIB_BIN).so.$(CLIENT_LIB_VERSION) $(CLIENT_LIB_OBJS) $(CLIENT_LIB_LIBS)

#
# Server lib
#

serverlib: sharedlib $(SERVER_LIB_OBJS)
	$(CC) -shared -Wl,-soname,lib$(SERVER_LIB_BIN).so.$(firstword $(subst ., ,$(SERVER_LIB_VERSION))) -o lib$(SERVER_LIB_BIN).so.$(SERVER_LIB_VERSION) $(SERVER_LIB_OBJS) $(SERVER_LIB_LIBS)

#
# Compilation
#

%.o: %.c
	$(CC) -Iinclude $(CFLAGS) -c $< -o $@

#
# Cleansing
#

clean:
	rm -f $(SHARED_LIB_OBJS) *$(SHARED_LIB_BIN)*
	rm -f $(CLIENT_LIB_OBJS) *$(CLIENT_LIB_BIN)*
	rm -f $(SERVER_LIB_OBJS) *$(SERVER_LIB_BIN)*

cleanall: clean
	rm -R docs

#
# Docs
#

docs:
	doxygen default.doxygen