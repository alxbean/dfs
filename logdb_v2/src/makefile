 .SUFFIXES: .c .o

SPX_SRC_PATH = ../spx/
SPX_HEADER_PATH = ../spx/include/
MSGPACK_SRC_PATH = ./messagepack/
MSGPACK_HEADER_PATH = ./messagepack/include/
SKIPLIST_SRC_PATH = ./skiplist/
SKIPLIST_HEADER_PATH = ./skiplist/include/
UTILS_SRC_PATH = ./utils/
UTILS_HEADER_PATH = ./utils/
TARGET = logdb_server
TARGET_PATH = /usr/local/bin
HEADER_TARGET_PATH = /usr/local/include/

CC = gcc
CFLAG = -pipe -o0  -W -Wall -Werror -g \
 		-Wpointer-arith -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wunused-value 
INC_PATH = -I/usr/local/include \
			-I$(SPX_HEADER_PATH) \
			-I$(MSGPACK_HEADER_PATH) \
			-I$(SKIPLIST_HEADER_PATH) \
			-I$(UTILS_HEADER_PATH)
LIB_PATH = -L/usr/local/lib -lev -lpthread
DEFS := 

SYS := $(strip $(shell uname -s | tr '[:upper:]' '[:lower:]'))
ifeq ($(SYS), linux)
	DEFS += -DSpxLinux
else
	ifeq ($(SYS),unix)
		DEFS += -DSpxUnix
	else
		ifeq ($(SYS),darwin)
			DEFS += -DSpxMac
		endif
	endif
endif

BITS := $(findstring 64,$(shell uname -m ))
ifeq (64, $(BITS))
	DEFS += -DSpx64
else
	ifeq (32, $(BITS))
		DEFS += -DSpx32
	else
		DEFS += -DSpx32
	endif
endif

SRC_FILE = $(wildcard *.c) \
			$(wildcard $(SPX_SRC_PATH)/*.c) \
			$(wildcard $(MSGPACK_SRC_PATH)/*.c) \
			$(wildcard $(SKIPLIST_SRC_PATH)/*.c) \
			$(wildcard $(UTILS_SRC_PATH)/*.c)

HEADER_FILE = $(wildcard *.h) \
			$(wildcard $(SPX_HEADER_PATH)/*.h) \
			$(wildcard $(MSGPACK_HEADER_PATH)/*.h) \
			$(wildcard $(SKIPLIST_HEADER_PATH)/*.h) \
			$(wildcard $(UTILS_HEADER_PATH)/*.h)

SHARED_OBJS = $(patsubst %.c,%.o,$(SRC_FILE) )

all:$(TARGET)

%.o: %.c
	$(CC) $(CFLAG) -c $< -o $@ $(LIB_PATH) $(INC_PATH) $(DEFS)

$(TARGET) : $(SHARED_OBJS)
	$(CC) $(CFLAG) $(SHARED_OBJS) $(LIB_PATH) $(INC_PATH) -o $(TARGET) $(DEFS)

install:
	cp -f $(TARGET) $(TARGET_PATH)
	cp -f $(HEADER_FILE) $(HEADER_TARGET_PATH)
clean:
	rm -f $(SHARED_OBJS) $(TARGET)
uninstall:
	cd $(TARGET_PATH)
	rm -f $(TARGET)
	cd $(HEADER_TARGET_PATH)
	rm -f $(HEADER_FILE)
