LUA= /usr/local
LUAINC= $(LUA)/include
LUALIB= $(LUA)/lib
LUABIN= $(LUA)/bin

LIBEXIF= /usr/local
LIBEXIFINC= $(LIBEXIF)/include
LIBEXIFLIB= $(LIBEXIF)/lib

CC= gcc
CFLAGS= -ansi $(INCS) $(WARN) -O2 $G
# On Linux, set LDFLAGS=-shared
LDFLAGS= -bundle -undefined dynamic_lookup
WARN= -pedantic -Wall
INCS= -I$(LUAINC) -I$(LIBEXIFINC)
LIBS= -L$(LIBEXIFLIB) -lexif

T= exif.so
OBJS= lexif.o

all: $T

$T: $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(OBJS) $(LIBS)

clean:
	rm -f $(OBJS) $T

