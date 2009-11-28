LUA= /usr/local
LUAINC= $(LUA)/include
LUALIB= $(LUA)/lib
LUABIN= $(LUA)/bin

LIBEXIF= /opt/local
LIBEXIFINC= $(LIBEXIF)/include
LIBEXIFLIB= $(LIBEXIF)/lib

CC= gcc
CFLAGS= -ansi $(INCS) $(WARN) -O2 $G
WARN= -pedantic -Wall
INCS= -I$(LUAINC) -I$(LIBEXIFINC)
LIBS= -L$(LIBEXIFLIB) -lexif

T= exif.so
OBJS= lexif.o

all: $T

$T: $(OBJS)
	$(CC) -o $@ -bundle -undefined dynamic_lookup $(OBJS) $(LIBS)

clean:
	rm -f $(OBJS) $T

