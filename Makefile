
CC = gcc
CFLAGS  = -g -W -Wall
# you'll have to edit these to reflect where your "teem-build" is
IPATH = -I/Users/gk/teem-build/include/
LPATH = -L/Users/gk/teem-build/bin

ifeq ($(OS), WINNT)
  SHLIB_EXT = dll
else
  UNAME_S := $(shell uname -s)
  ifeq ($(UNAME_S),Linux)
    SHLIB_EXT = so
    CFLAGS += -fPIC
    LD = ld -shared
  endif
  ifeq ($(UNAME_S),Darwin)
    SHLIB_EXT = dylib
    CFLAGS += -dynamic
    LD = libtool -dynamic
  endif
endif

SRCS = about.c flotsam.c util.c fold.c
HDRS = rva.h
OBJS = $(SRCS:.c=.o)
LIBNAME = rva
DEMO = reva
LIB = lib$(LIBNAME).$(SHLIB_EXT)

all: $(LIB) $(DEMO) $(DLDEMO)

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) $(IPATH) -c $< -o $@

$(LIB): $(OBJS)
	$(LD) -o $(LIB) $(OBJS) $(LPATH)  -lteem -lm

$(DEMO): $(LIB) $(DEMO).c
	$(CC) $(CFLAGS) $(IPATH) $(DEMO).c -o $(DEMO) $(LPATH) -L. -l$(LIBNAME) -lteem -lm

clean:
	rm -rf *.o $(LIB) $(DEMO) $(DLDEMO) $(DEMO).dSYM
