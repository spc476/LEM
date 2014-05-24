
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#define abs_index(L,i) ((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : lua_gettop(L) + (i) + 1)

/***********************************************************************/

#define ZIP_MAGIC_EOCD	0x06054B50
#define ZIP_MAGIC_CFILE	0x02014B50
#define ZIP_MAGIC_FILE  0x04034B50

typedef enum
{
  ZIPE_CPU_NONE,
  ZIPE_CPU_ALPHA,
  ZIPE_CPU_ARM,
  ZIPE_CPU_AVR32,
  ZIPE_CPU_ETRAX,
  ZIPE_CPU_C6X,
  ZIPE_CPU_H8,
  ZIPE_CPU_HPPA,
  ZIPE_CPU_IBM390,
  ZIPE_CPU_IBMZ,
  ZIPE_CPU_ITANIUM,
  ZIPE_CPU_68000,
  ZIPE_CPU_68010,
  ZIPE_CPU_68020,
  ZIPE_CPU_68030,
  ZIPE_CPU_68040,
  ZIPE_CPU_M32R,
  ZIPE_CPU_MICROBLAZE,
  ZIPE_CPU_MIPS,
  ZIPE_CPU_PPC,
  ZIPE_CPU_SH3EB,
  ZIPE_CPU_SH3EI,
  ZIPE_CPU_SPARC,
  ZIPE_CPU_SPARC64,
  ZIPE_CPU_VAX,
  ZIPE_CPU_i386,
  ZIPE_CPU_i486,
  ZIPE_CPU_x86,
  ZIPE_CPU_x86_64,
} zipe_cpu__e;

typedef enum
{
  ZIPE_OS_NONE,
  ZIPE_OS_AIX,
  ZIPE_OS_BSD,
  ZIPE_OS_FREEBSD,
  ZIPE_OS_LINUX,
  ZIPE_OS_MACOSX,
  ZIPE_OS_MINGW,
  ZIPE_OS_SOLARIS
} zipe_os__e;
  
typedef struct
{
  uint32_t magic;	/* 0x06054b50 */
  uint16_t disknum;
  uint16_t diskstart;
  uint16_t entries;
  uint16_t totalentries;
  uint32_t size;
  uint32_t offset;
  uint16_t commentlen;
  uint8_t  data[];
} __attribute__((packed)) zip_eocd__s;

typedef struct
{
  uint32_t magic;	/* 0x02014b50 */
  uint16_t byversion;
  uint16_t forversion;
  uint16_t flags;
  uint16_t compression;
  uint16_t modtime;
  uint16_t moddate;
  uint32_t crc;
  uint32_t csize;
  uint32_t usize;
  uint16_t namelen;
  uint16_t extralen;
  uint16_t commentlen;
  uint16_t diskstart;
  uint16_t iattr;
  uint32_t eattr;
  uint32_t offset;
  uint8_t  data[];
} __attribute__((packed)) zip_dir__s;

typedef struct
{
  uint32_t magic;	/* 0x04034b50 */
  uint16_t byversion;
  uint16_t flags;
  uint16_t compression;
  uint16_t modtime;
  uint16_t moddate;
  uint32_t crc;
  uint32_t csize;
  uint32_t usize;
  uint16_t namelen;
  uint16_t extralen;
  uint8_t  data[];
} __attribute__((packed)) zip_file__s;

#define ZIPIA_TEXT	(1 << 0)
#define ZIP_EXT_LUA	0x4C45

typedef struct
{
  uint16_t id;		/* 0x4C45 */
  uint16_t size;
  uint16_t luavmin;	/* Lua minimum version support */
  uint16_t luavmax;	/* Lua maximum version support */
  uint16_t os;
  uint16_t cpu;
  uint16_t version;	/* Version */
} __attribute__((packed)) zip_lua_ext__s;

typedef struct
{
  uint16_t modtime;
  uint16_t moddate;
} modtime__s;

/***********************************************************************/

static uint16_t zwlua_toversion(lua_State *L,int idx)
{
  const char    *version;
  char          *p;
  unsigned long  mj;
  unsigned long  mn;
  
  if (lua_isnil(L,idx))
    return 0;
  else
  {
    version = luaL_checkstring(L,idx);
    mj      = strtoul(version,&p,10); p++;
    mn      = strtoul(p,NULL,10);
  
    return (uint16_t)((((uint8_t)mj) << 8) | ((uint8_t)mn));
  }
}

/***********************************************************************/

static uint16_t zwlua_toos(lua_State *L,int idx)
{
  const char *os = luaL_checkstring(L,idx);
  
  if (strcmp(os,"Linux") == 0)
    return ZIPE_OS_LINUX;
  else if (strcmp(os,"SunOS") == 0)
    return ZIPE_OS_SOLARIS;
  else if (strcmp(os,"none") == 0)
    return ZIPE_OS_NONE;
  else
    return luaL_error(L,"bad operating system");
}

/***********************************************************************/

static uint16_t zwlua_tocpu(lua_State *L,int idx)
{
  const char *cpu = luaL_checkstring(L,idx);
  
  if (strcmp(cpu,"sparcv9") == 0)
    return ZIPE_CPU_SPARC64;
  else if (strcmp(cpu,"x86") == 0)
    return ZIPE_CPU_x86;
  else if (strcmp(cpu,"none") == 0)
    return ZIPE_CPU_NONE;
  else
    return luaL_error(L,"bad CPU");
}

/***********************************************************************/

static void zwlua_toluaext(lua_State *L,int idx,zip_lua_ext__s *ext)
{
  idx       = abs_index(L,idx);
  ext->id   = ZIP_EXT_LUA;
  ext->size = sizeof(zip_lua_ext__s) - (sizeof(uint16_t) * 2);
  
  lua_getfield(L,idx,"luamin");
  ext->luavmin = zwlua_toversion(L,-1);
  lua_getfield(L,idx,"luamax");
  ext->luavmax = zwlua_toversion(L,-1);
  lua_getfield(L,idx,"version");
  ext->version = zwlua_toversion(L,-1);
  lua_getfield(L,idx,"os");
  ext->os = zwlua_toos(L,-1);
  lua_getfield(L,idx,"cpu");
  ext->cpu = zwlua_tocpu(L,-1);
  lua_pop(L,5);
}

/***********************************************************************/

static modtime__s zwlua_tomodtime(lua_State *L,int idx)
{
  modtime__s mod;
  time_t     mtime;
  struct tm  stm;
  
  mtime = lua_tonumber(L,idx);
  stm   = *gmtime(&mtime);
  
  mod.modtime = (stm.tm_hour << 11)
              | (stm.tm_min  <<  5)
              | (stm.tm_sec  <<  0)
              ;
  mod.moddate = (((stm.tm_year + 1900) - 1980) << 9)
              |  ((stm.tm_mon  +    1)         << 5)
              |  ((stm.tm_mday       )         << 0)
              ;
  return mod;
}

/***********************************************************************/

static void zwlua_tofile(lua_State *L,int idx,zip_file__s *file)
{
  modtime__s mod;
  
  idx = abs_index(L,idx);
  
  file->magic       = ZIP_MAGIC_FILE;
  file->byversion   = 20;	/* MS-DOS, ZIP version 2.0 */
  file->compression = 8;	/* deflate */
  
  lua_getfield(L,idx,"modtime");
  mod = zwlua_tomodtime(L,-1);
  file->modtime = mod.modtime;
  file->moddate = mod.moddate;
  lua_getfield(L,idx,"crc");
  file->crc = lua_tonumber(L,-1);
  lua_getfield(L,idx,"csize");
  file->csize = lua_tointeger(L,-1);
  lua_getfield(L,idx,"usize");
  file->usize = lua_tointeger(L,-1);
  lua_getfield(L,idx,"module");
  file->namelen  = lua_objlen(L,-1);
  file->extralen = sizeof(zip_lua_ext__s);
  file->flags    = 0;
  lua_pop(L,5);
}

/***********************************************************************/

static int zipwlua_file(lua_State *L)
{
  FILE           **pfp;
  long             pos;
  zip_file__s     *file;
  size_t           filelen;
  zip_lua_ext__s   ext;
  const char      *name;
  size_t           namelen;
  const char      *data;
  size_t           datasize;
  uint8_t         *p;
  
  pfp = luaL_checkudata(L,1,LUA_FILEHANDLE);
  pos = ftell(*pfp);
  luaL_checktype(L,2,LUA_TTABLE);
  zwlua_toluaext(L,2,&ext);
  lua_getfield(L,2,"module");
  name = luaL_checklstring(L,-1,&namelen);
  
  filelen = sizeof(zip_file__s) + namelen + sizeof(zip_lua_ext__s);
  file    = malloc(filelen);
  
  if (file == NULL)
  {
    lua_pushnil(L);
    lua_pushinteger(L,ENOMEM);
    return 2;
  }
  
  zwlua_tofile(L,2,file);
  p = file->data;
  memcpy(p,name,namelen); 
  p += namelen;
  memcpy(p,&ext,sizeof(ext));
  fwrite(file,filelen,1,*pfp);
  lua_getfield(L,2,"zip");
  data = lua_tolstring(L,-1,&datasize);
  fwrite(data,1,datasize,*pfp);
  
  free(file);
  
  lua_pushnumber(L,pos);
  lua_pushinteger(L,0);
  return 2;
}

/***********************************************************************/

static int zipwlua_dir(lua_State *L)
{
  FILE           **pfp;
  long             pos;
  zip_dir__s      *dir;
  zip_file__s     *file;
  zip_lua_ext__s   ext;
  size_t           dirlen;
  const char      *name;
  size_t           namelen;
  uint8_t         *p;
  
  pfp = luaL_checkudata(L,1,LUA_FILEHANDLE);
  pos = ftell(*pfp);
  luaL_checktype(L,2,LUA_TTABLE);
  zwlua_toluaext(L,2,&ext);
  lua_getfield(L,2,"module");
  name   = luaL_checklstring(L,-1,&namelen);
  dirlen = sizeof(zip_dir__s) + namelen + sizeof(zip_lua_ext__s);
  dir    = malloc(dirlen);
  file   = malloc(sizeof(zip_file__s));
  
  if ((file == NULL) || (dir == NULL))
  {
    free(file);
    free(dir);
    lua_pushnil(L);
    lua_pushinteger(L,ENOMEM);
    return 2;
  }
  
  zwlua_tofile(L,2,file);
  
  dir->magic       = ZIP_MAGIC_CFILE;
  dir->byversion   = file->byversion;
  dir->forversion  = file->byversion;
  dir->flags       = file->flags;
  dir->compression = file->compression;
  dir->modtime     = file->modtime;
  dir->moddate     = file->moddate;
  dir->crc         = file->crc;
  dir->csize       = file->csize;
  dir->usize       = file->usize;
  dir->namelen     = file->namelen;
  dir->extralen    = file->extralen;
  dir->commentlen  = 0;
  dir->diskstart   = 0;
  dir->iattr       = ext.cpu == ZIPE_CPU_NONE ? ZIPIA_TEXT : 0;
  dir->eattr       = 0;
  
  lua_getfield(L,2,"offset");
  dir->offset = lua_tonumber(L,-1);
  
  p = dir->data;
  memcpy(p,name,namelen);
  p += namelen;
  memcpy(p,&ext,sizeof(ext));
  fwrite(dir,dirlen,1,*pfp);
  
  free(file);
  free(dir);
  
  lua_pushnumber(L,pos);
  lua_pushinteger(L,0);
  return 2;
}

/***********************************************************************/

static int zipwlua_eocd(lua_State *L)
{
  zip_eocd__s  *eocd;
  FILE        **pfp;
  
  eocd = malloc(sizeof(zip_eocd__s));
  if (eocd == NULL)
  {
    lua_pushinteger(L,ENOMEM);
    return 1;
  }
  
  pfp                = luaL_checkudata(L,1,LUA_FILEHANDLE);
  eocd->magic        = ZIP_MAGIC_EOCD;
  eocd->disknum      = 0;
  eocd->diskstart    = 0;
  eocd->entries      = lua_tointeger(L,2);
  eocd->totalentries = lua_tointeger(L,2);
  eocd->size         = lua_tointeger(L,3);
  eocd->offset       = lua_tointeger(L,4);
  eocd->commentlen   = 0;
  
  fwrite(eocd,sizeof(zip_eocd__s),1,*pfp);
  lua_pushinteger(L,0);
  return 1;
}

/***********************************************************************/

static const luaL_Reg m_ziplua[] =
{
  { "file" 	, zipwlua_file	} ,
  { "dir"	, zipwlua_dir	} ,
  { "eocd"	, zipwlua_eocd	} ,
  { NULL	, NULL		}
};

int luaopen_zipw(lua_State *L)
{
  luaL_register(L,"zipw",m_ziplua);
  return 1;
}
