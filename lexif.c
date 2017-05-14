/*
** Copyright (C) 2009-2010,2014,2017 ARATA Mizuki
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

#if LUA_VERSION_NUM == 501
static inline size_t lua_rawlen(lua_State *L, int idx) {
  return lua_objlen(L, idx);
}
static inline void luaL_setfuncs(lua_State *L, const luaL_Reg *l, int nup) {
  luaI_openlib(L, NULL, l, nup);
}
#define luaL_newlibtable(L,l) \
  lua_createtable(L, 0, sizeof(l)/sizeof((l)[0]) - 1)
static inline int lua_isinteger(lua_State *L, int idx) {
  return lua_isnumber(L, idx);
}
#endif

/*
  Most of the library functions has the 'pointer -> object' table
  (the table with lightuserdata keys and the corresponding userdata values)
  at the upvalueindex 1.
 */
static void push_unique (lua_State *L, void *ptr, const char *tname, void (*ref)(void *)) {
  if (!ptr) {
    lua_pushnil(L);
  } else {
    lua_pushlightuserdata(L, ptr);
    lua_rawget(L, lua_upvalueindex(1));
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
      lua_rawset(L, lua_upvalueindex(1));
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

static const char tname_rational[] = "exif:rational";
static void pushrational (lua_State *L, lua_Integer numerator, lua_Integer denominator) {
  lua_createtable(L, 0, 2);
  lua_pushinteger(L, numerator);
  lua_setfield(L, -2, "numerator");
  lua_pushinteger(L, denominator);
  lua_setfield(L, -2, "denominator");
  if (denominator == 1) {
    lua_pushinteger(L, numerator);
  } else {
    lua_pushnumber(L, (lua_Number)numerator/(lua_Number)denominator);
  }
  lua_setfield(L, -2, "value");
  luaL_getmetatable(L, tname_rational);
  lua_setmetatable(L, -2);
}


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

static int Difd (lua_State *L) { /** data:ifd(ifd) */
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
  int n = lua_rawlen(L, -1);
  pushcontent(L, content);
  lua_rawseti(L, -2, n+1);
}

static int Difds (lua_State *L) { /** data:ifds() */
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
  int n = lua_rawlen(L, -1);
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
  lua_pushinteger(L, entry->components);
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

static void entry_getdata_aux (lua_State *L, ExifEntry *entry, unsigned int n) {
  ExifFormat format = entry->format;
  size_t formatsize = (size_t)exif_format_get_size(format);
  const unsigned char *ptr = entry->data+formatsize*n;
  ExifByteOrder order = exif_data_get_byte_order(entry->parent->parent);

  switch (format) {
  case EXIF_FORMAT_BYTE: lua_pushinteger(L, (ExifByte)*ptr); break;
  case EXIF_FORMAT_SBYTE: lua_pushinteger(L, (ExifSByte)*ptr); break;
  case EXIF_FORMAT_SHORT: lua_pushinteger(L, exif_get_short(ptr, order)); break;
  case EXIF_FORMAT_SSHORT: lua_pushinteger(L, exif_get_sshort(ptr, order)); break;
  case EXIF_FORMAT_LONG: lua_pushinteger(L, exif_get_long(ptr, order)); break;
  case EXIF_FORMAT_SLONG: lua_pushinteger(L, exif_get_slong(ptr, order)); break;
  case EXIF_FORMAT_RATIONAL: {
      ExifRational rat = exif_get_rational(ptr, order);
      pushrational(L, rat.numerator, rat.denominator);
      break;
    }
  case EXIF_FORMAT_SRATIONAL: {
      ExifSRational rat = exif_get_srational(ptr, order);
      pushrational(L, rat.numerator, rat.denominator);
      break;
    }
  case EXIF_FORMAT_ASCII: lua_pushinteger(L, *ptr); break;
  default:
    lua_pushnil(L);
    break;
  }
}

static int Egetdata (lua_State *L) { /** entry.data */
  entry_getdata_aux(L, checkentry(L), 0);
  return 1;
}

static int E_index (lua_State *L) { /** entry[n] */
  if (lua_isinteger(L, 2)) {
    ExifEntry *entry = checkentry(L);
    lua_Integer n = lua_tointeger(L, 2);
    if (n < 1 || entry->components < n) {
      lua_pushnil(L);
    } else {
      entry_getdata_aux(L, entry, (unsigned int)(n-1));
    }
    return 1;
  } else {
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
}

static int M_len (lua_State *L) { /** #mnotedata */
  ExifMnoteData *md = checkmnote_data(L);
  lua_pushinteger(L, exif_mnote_data_count(md));
  return 1;
}

static int M_index (lua_State *L) { /** mnotedata[n] */
  if (lua_isinteger(L, 2)) {
    ExifMnoteData *md = checkmnote_data(L);
    lua_Integer n = lua_tointeger(L, 2);
    if (n < 1 || exif_mnote_data_count(md) < n) {
      lua_pushnil(L);
    } else {
      unsigned int n_ = (unsigned int)(n-1);
      lua_createtable(L, 0, 5);
      lua_pushinteger(L, exif_mnote_data_get_id(md, n_));
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

static int Rtostring (lua_State *L) { /** tostring(r) */
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_getfield(L, 1, "numerator");
  lua_pushliteral(L, "/");
  lua_getfield(L, 1, "denominator");
  lua_concat(L, 3);
  return 1;
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
  {"ifd", Difd},
  {"ifds", Difds},
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

static const luaL_Reg entrymm[] = {
  {"__gc", entry_gc},
  {"__tostring", Egetvalue},
  {"__len", Egetcomponents},
  /* {"__index", E_index}, -- registered in luaopen_exif */
  {NULL, NULL}
};

static const luaL_Reg entryfuncs[] = {
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
  {"data", Egetdata},
  {NULL, NULL}
};

static const luaL_Reg mnotedatafuncs[] = {
  {"__gc", mnote_data_gc},
  {"__len", M_len},
  {"__index", M_index},
  {NULL, NULL}
};

static const luaL_Reg rationalmm[] = {
  {"__tostring", Rtostring},
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
  lua_pushvalue(L, -1);     /* metatable */
  lua_newtable(L);          /* getters table */
  lua_pushvalue(L, -4);     /* pointer -> object table */
  luaL_setfuncs(L, get, 1); /* set getters table */
  lua_pushcclosure(L, prop_index, 2); /* upvalues: metatable, getters */
  lua_setfield(L, -2, "__index");
}

LUALIB_API int luaopen_exif (lua_State *L) {
  lua_newtable(L); /* pointer -> object table */
  lua_createtable(L, 0, 1);
  lua_pushliteral(L, "v");
  lua_setfield(L, -2, "__mode");
  lua_setmetatable(L, -2);

  luaL_newmetatable(L, tname_data);
  setupprop(L, datapropget); /* set __index */
  lua_pushvalue(L, -2); /* pointer -> object table */
  luaL_setfuncs(L, datafuncs, 1);
  lua_pop(L, 1);

  luaL_newmetatable(L, tname_content);
  setupprop(L, contentpropget); /* set __index */
  lua_pushvalue(L, -2); /* pointer -> object table */
  luaL_setfuncs(L, contentfuncs, 1);
  lua_pop(L, 1);

  luaL_newmetatable(L, tname_entry);
  lua_pushvalue(L, -2); /* pointer -> object table */
  luaL_setfuncs(L, entrymm, 1);
  lua_newtable(L);      /* table of functions */
  lua_pushvalue(L, -3); /* pointer -> object table */
  luaL_setfuncs(L, entryfuncs, 1);
  lua_newtable(L);      /* table of property getters */
  lua_pushvalue(L, -4); /* pointer -> object table */
  luaL_setfuncs(L, entrypropget, 1);
  lua_pushcclosure(L, E_index, 2);
  lua_setfield(L, -2, "__index");
  lua_pop(L, 1);

  luaL_newmetatable(L, tname_mnote_data);
  lua_pushvalue(L, -2); /* pointer -> object table */
  luaL_setfuncs(L, mnotedatafuncs, 1);
  lua_pop(L, 1);

  luaL_newmetatable(L, tname_rational);
  luaL_setfuncs(L, rationalmm, 0);
  lua_pop(L, 1);

  luaL_newlibtable(L, funcs);
  lua_pushvalue(L, -2); /* pointer -> object table */
  luaL_setfuncs(L, funcs, 1);
  return 1;
}
