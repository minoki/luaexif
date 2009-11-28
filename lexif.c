/*
** Copyright (C) 2009 ARATA Mizuki
** See file COPYRIGHT for more information
*/

#include <libexif/exif-data.h>
#include <libexif/exif-content.h>
#include <libexif/exif-tag.h>
#include <libexif/exif-entry.h>

#include <lua.h>
#include <lauxlib.h>

#if defined(__STDC__) && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
/* C99: no need of '#define inline ...' */
#elif defined(__GNUC__)
#define inline __inline__
#elif defined(_MSC_VER)
#define inline __inline
#else
#define inline
#endif

static void push_unique (lua_State *L, void *ptr, const char *tname, void (*ref)(void *)) {
  if (!ptr) {
    lua_pushnil(L);
  } else {
    lua_pushlightuserdata(L, ptr);
    lua_rawget(L, LUA_ENVIRONINDEX);
    if (lua_isnil(L, -1)) {
      void **p;
      lua_pop(L, 1);
      p = (void **)lua_newuserdata(L, sizeof(void *));
      *p = NULL;
      luaL_getmetatable(L, tname);
      lua_setmetatable(L, -2);
      *p = ptr;
      ref(ptr);
      lua_pushlightuserdata(L, ptr);
      lua_pushvalue(L, -2);
      lua_rawset(L, LUA_ENVIRONINDEX);
    }
  }
}

#define DataType(ctype,name) \
  static const char tname_##name[] = "exif:" #ctype; \
  static inline ctype *to##name##p(lua_State *L) { \
    return (ctype*)luaL_checkudata(L, 1, tname_##name); \
  } \
  static inline ctype check##name(lua_State *L) { \
    return *to##name##p(L); \
  } \
  static inline void push##name(lua_State *L, ctype p) { \
    push_unique(L, p, tname_##name, (void (*)(void *))exif_##name##_ref); \
  } \
  static int name##_gc (lua_State *L) { \
    ctype *p = to##name##p(L); \
    if (*p != NULL) { \
      exif_##name##_unref(*p); \
      *p = NULL; \
    } \
    return 0; \
  }
DataType(ExifData *, data)
DataType(ExifContent *, content)
DataType(ExifEntry *, entry)
DataType(ExifMnoteData *, mnote_data)
#undef DataType

static int Lnew (lua_State *L) { /** new() */
  pushdata(L, exif_data_new());
  return 1;
}

static int Lloadfile (lua_State *L) { /** loadfile(path) */
  const char *path = luaL_checkstring(L, 1);
  pushdata(L, exif_data_new_from_file(path));
  return 1;
}

static int Lloadbuffer (lua_State *L) { /** loadbuffer(buf) */
  size_t length;
  const char *buffer = luaL_checklstring(L, 1, &length);
  pushdata(L, exif_data_new_from_data((const unsigned char *)buffer, length));
  return 1;
}

static int Dfix (lua_State *L) { /** data:fix() */
  ExifData *data = checkdata(L);
  exif_data_fix(data);
  return 0;
}

static int Dloadbuffer (lua_State *L) { /** data:loadbuffer(buf) */
  ExifData *data = checkdata(L);
  size_t length;
  const char *buffer = luaL_checklstring(L, 2, &length);

  exif_data_load_data(data, (const unsigned char *)buffer, length);

  return 0;
}

static int Dcontent (lua_State *L) { /** data:content(ifd) */
  ExifData *data = checkdata(L);
  static const char *const ifdnames[] = {
    "0",
    "1",
    "EXIF",
    "GPS",
    "Interoperability",
    NULL
  };
  static const ExifIfd ifds[] = {
    EXIF_IFD_0,
    EXIF_IFD_1,
    EXIF_IFD_EXIF,
    EXIF_IFD_GPS,
    EXIF_IFD_INTEROPERABILITY
  };
  int i = luaL_checkoption(L, 2, NULL, ifdnames);
  pushcontent(L, data->ifd[ifds[i]]);
  return 1;
}

static void contentfunc (ExifContent *content, void *userdata) {
  lua_State *L = (lua_State *)userdata;
  int n = lua_objlen(L, -1);
  pushcontent(L, content);
  lua_rawseti(L, -2, n+1);
}

static int Dcontents (lua_State *L) { /** data:contents() */
  ExifData *data = checkdata(L);
  lua_newtable(L);
  exif_data_foreach_content(data, contentfunc, (void *)L);
  return 1;
}

static int Dgetmnotedata (lua_State *L) { /** data.mnotedata */
  ExifData *data = checkdata(L);
  pushmnote_data(L, exif_data_get_mnote_data(data));
  return 1;
}

static int Cfix (lua_State *L) { /** content:fix() */
  ExifContent *content = checkcontent(L);
  exif_content_fix(content);
  return 0;
}

static int Centry (lua_State *L) { /** content:entry(tag) */
  ExifContent *content = checkcontent(L);
  ExifTag tag = exif_tag_from_name(luaL_checkstring(L, 2));
  pushentry(L, exif_content_get_entry(content, tag));
  return 1;
}

static int Cgetifd (lua_State *L) { /** content.ifd */
  ExifContent *content = checkcontent(L);
  ExifIfd ifd = exif_content_get_ifd(content);
  lua_pushstring(L, exif_ifd_get_name(ifd));
  return 1;
}

static void entryfunc (ExifEntry *entry, void *userdata) {
  lua_State *L = (lua_State *)userdata;
  int n = lua_objlen(L, -1);
  pushentry(L, entry);
  lua_rawseti(L, -2, n+1);
}

static int Centries (lua_State *L) { /** content:entries() */
  ExifContent *content = checkcontent(L);
  lua_newtable(L);
  exif_content_foreach_entry(content, entryfunc, (void *)L);
  return 1;
}

static int Cgetparent (lua_State *L) { /** content.parent */
  ExifContent *content = checkcontent(L);
  pushdata(L, content->parent);
  return 1;
}

static int Efix (lua_State *L) { /** entry:fix() */
  ExifEntry *entry = checkentry(L);
  exif_entry_fix(entry);
  return 0;
}

static int Egettag (lua_State *L) { /** entry.tag */
  ExifEntry *entry = checkentry(L);
  ExifIfd ifd = exif_entry_get_ifd(entry);
  lua_pushstring(L, exif_tag_get_name_in_ifd(entry->tag, ifd));
  return 1;
}

static int Egetvalue (lua_State *L) { /** entry.value */
  ExifEntry *entry = checkentry(L);
  char buffer[1024];
  exif_entry_get_value(entry, buffer, sizeof(buffer)/sizeof(char));
  lua_pushstring(L, buffer);
  return 1;
}

static int Egetcomponents (lua_State *L) { /** entry.components */
  ExifEntry *entry = checkentry(L);
  lua_pushnumber(L, entry->components);
  return 1;
}

static int Egetformat (lua_State *L) { /** entry.format */
  ExifEntry *entry = checkentry(L);
  lua_pushstring(L, exif_format_get_name(entry->format));
  return 1;
}

static int Egetrawdata (lua_State *L) { /** entry.rawdata */
  ExifEntry *entry = checkentry(L);
  lua_pushlstring(L, (const char *)entry->data, entry->size);
  return 1;
}

static int Egetparent (lua_State *L) { /** entry.parent */
  ExifEntry *entry = checkentry(L);
  pushcontent(L, entry->parent);
  return 1;
}

static int M_len (lua_State *L) { /** #mnotedata */
  ExifMnoteData *md = checkmnote_data(L);
  lua_pushnumber(L, exif_mnote_data_count(md));
  return 1;
}

static int M_index (lua_State *L) { /** mnotedata[n] */
  if (lua_isnumber(L, 2)) {
    ExifMnoteData *md = checkmnote_data(L);
    lua_Number n = lua_tonumber(L, 2);
    if (n < 1 || exif_mnote_data_count(md) < n) {
      lua_pushnil(L);
    } else {
      unsigned int n_ = (unsigned int)(n-1);
      lua_createtable(L, 0, 5);
      lua_pushnumber(L, exif_mnote_data_get_id(md, n_));
      lua_setfield(L, -2, "tagid");
      lua_pushstring(L, exif_mnote_data_get_name(md, n_));
      lua_setfield(L, -2, "tag");
      lua_pushstring(L, exif_mnote_data_get_title(md, n_));
      lua_setfield(L, -2, "title");
      lua_pushstring(L, exif_mnote_data_get_description(md, n_));
      lua_setfield(L, -2, "description");
      {
        char buffer[1024];
        lua_pushstring(L, exif_mnote_data_get_value(md, n_, buffer, sizeof(buffer)/sizeof(char)));
        lua_setfield(L, -2, "value");
      }
    }
    return 1;
  } else {
    lua_getmetatable(L, 1);
    lua_pushvalue(L, 2);
    lua_gettable(L, -2);
    return 1;
  }
}


static const luaL_Reg funcs[] = {
  {"new", Lnew},
  {"loadfile", Lloadfile},
  {"loadbuffer", Lloadbuffer},
  {NULL, NULL}
};

static const luaL_Reg datapropget[] = {
  {"mnotedata", Dgetmnotedata},
  {NULL, NULL}
};

static const luaL_Reg datafuncs[] = {
  {"__gc", data_gc},
  {"fix", Dfix},
  {"loadbuffer", Dloadbuffer},
  {"content", Dcontent},
  {"contents", Dcontents},
  {NULL, NULL}
};

static const luaL_Reg contentfuncs[] = {
  {"__gc", content_gc},
  {"fix", Cfix},
  {"entry", Centry},
  {"entries", Centries},
  {NULL, NULL}
};

static const luaL_Reg contentpropget[] = {
  {"ifd", Cgetifd},
  {"parent", Cgetparent},
  {NULL, NULL}
};

static const luaL_Reg entryfuncs[] = {
  {"__gc", entry_gc},
  {"__tostring", Egetvalue},
  {"fix", Efix},
  {NULL, NULL}
};

static const luaL_Reg entrypropget[] = {
  {"tag", Egettag},
  {"value", Egetvalue},
  {"components", Egetcomponents},
  {"format", Egetformat},
  {"rawdata", Egetrawdata},
  {"parent", Egetparent},
  {NULL, NULL}
};

static const luaL_Reg mnotedatafuncs[] = {
  {"__gc", mnote_data_gc},
  {"__len", M_len},
  {"__index", M_index},
  {NULL, NULL}
};

static int prop_index (lua_State *L) {
  lua_pushvalue(L, 2);
  lua_gettable(L, lua_upvalueindex(2)); /* getter[key] */
  if (lua_isfunction(L, -1)) {
    lua_pushvalue(L, 1);
    lua_call(L, 1, 1);
    return 1;
  }
  lua_pushvalue(L, 2);
  lua_gettable(L, lua_upvalueindex(1)); /* metatable[key] */
  return 1;
}

static void setupprop (lua_State *L, const luaL_Reg *get) {
  lua_pushvalue(L, -1); /* metatable */
  lua_newtable(L);
  luaL_register(L, NULL, get);
  lua_pushcclosure(L, prop_index, 2);
  lua_setfield(L, -2, "__index");
}

LUALIB_API int luaopen_exif (lua_State *L) {
  lua_newtable(L);
  lua_createtable(L, 0, 1);
  lua_pushliteral(L, "v");
  lua_setfield(L, -2, "__mode");
  lua_setmetatable(L, -2);
  lua_replace(L, LUA_ENVIRONINDEX);

  luaL_newmetatable(L, tname_data);
  setupprop(L, datapropget);
  luaL_register(L, NULL, datafuncs);

  luaL_newmetatable(L, tname_content);
  setupprop(L, contentpropget);
  luaL_register(L, NULL, contentfuncs);

  luaL_newmetatable(L, tname_entry);
  setupprop(L, entrypropget);
  luaL_register(L, NULL, entryfuncs);

  luaL_newmetatable(L, tname_mnote_data);
  luaL_register(L, NULL, mnotedatafuncs);

  luaL_register(L, "exif", funcs);
  return 1;
}
