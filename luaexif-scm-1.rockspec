package = "luaexif"
version = "scm-1"
source = {
   url = "https://github.com/minoki/luaexif"
}
description = {
   summary = "Lua binding for libexif",
   detailed = [[
This is a Lua binding for libexif (http://libexif.sourceforge.net/).]],
   homepage = "https://github.com/minoki/luaexif",
   license = "MIT"
}
dependencies = {
   "lua >= 5.1, < 5.4"
}
external_dependencies = {
   LIBEXIF = {
      header = "libexif/exif-data.h"
   }
}
build = {
   type = "builtin",
   modules = {
      exif = {
         sources = "lexif.c",
         libraries = {"exif"},
         incdirs = {"$(LIBEXIF_INCDIR)"},
         libdirs = {"$(LIBEXIF_LIBDIR)"}
      }
   }
}
