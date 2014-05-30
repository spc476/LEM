/***************************************************************************
*
* Copyright 2014 by Sean Conner.
*
* This library is free software; you can redistribute it and/or modify it
* under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation; either version 3 of the License, or (at your
* option) any later version.
*
* This library is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
* License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library; if not, see <http://www.gnu.org/licenses/>.
*
* Comments, questions and criticisms can be sent to: sean@conman.org
*
* ----------------------------------------------------------------------
*
* The ZIP file writer.
*
*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "lem.h"

#if LUA_VERSION_NUM == 501
#  define DEF_LUA_MAJOR	5
#  define DEF_LUA_MINOR 1
#elif LUA_VERSION_NUM == 502
#  define DEF_LUA_MAJOR 5
#  define DEF_LUA_MINOR 2
#endif

#define abs_index(L,i) ((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : lua_gettop(L) + (i) + 1)

/***********************************************************************/

static bool zwlib_fwrite(
        lua_State  *L,
        const void *data,
        size_t      size,
        size_t      nmemb,
        FILE       *fp
)
{
  if (nmemb > 0)
  {
    if (fwrite(data,size,nmemb,fp) != nmemb)
    {
      lua_pushnil(L);
      lua_pushinteger(L,errno);
      return false;
    }
  }
  return true;
}

/***********************************************************************/

static uint16_t zwlua_tolanguage(lua_State *L,int idx,uint16_t def)
{
  if (lua_isstring(L,idx))
  {
    const char *lang = luaL_checkstring(L,idx);
    
    if (strcmp(lang,"Lua") == 0)
      return ZIPE_LANG_LUA;
    else if (strcmp(lang,"LuaJIT") == 0)
      return ZIPE_LANG_LUAJIT;
    else if (strcmp(lang,"BASIC") == 0)
      return ZIPE_LANG_BASIC;
    else
      return ZIPE_LANG_UNKNOWN;
  }
  else if (lua_isnumber(L,idx))
    return lua_tointeger(L,idx);
  else
    return def;
}

/***********************************************************************/

static uint16_t zwlua_toversion(lua_State *L,int idx,int major,int minor)
{
  if (lua_isstring(L,idx))
  {
    char          *p;
    const char    *version = luaL_checkstring(L,idx);
    unsigned long  mj      = strtoul(version,&p,10); p++;
    unsigned long  mn      = strtoul(p,NULL,10);
  
    return (uint16_t)((((uint8_t)mj) << 8) | ((uint8_t)mn));
  }
  else if (lua_isnumber(L,idx))
    return lua_tointeger(L,idx);
  else
    return (uint16_t)((((uint8_t)major) << 8) | ((uint8_t)minor));
}

/***********************************************************************/

static uint16_t zwlua_toos(lua_State *L,int idx)
{
  if (lua_isstring(L,idx))
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
  else if (lua_isnumber(L,idx))
    return lua_tointeger(L,idx);
  else
    return ZIPE_OS_NONE;
}

/***********************************************************************/

static uint16_t zwlua_tocpu(lua_State *L,int idx,uint16_t os)
{
  if (lua_isstring(L,idx))
  {
    const char *cpu = luaL_checkstring(L,idx);
  
    if (strcmp(cpu,"sparcv9") == 0)
      return ZIPE_CPU_SPARC64;
    else if (strcmp(cpu,"x86") == 0)
      return ZIPE_CPU_x86;
    else if (strcmp(cpu,"none") == 0)
      return ZIPE_CPU_NONE;
    else if (strcmp(cpu,"_LEM") == 0) 
    {
      if (os == ZIPE_OS_NONE)
        return ZIPE_META_LEM;
      else
        return luaL_error(L,"bad CPU");
    }
    else
      return luaL_error(L,"bad CPU");
  }
  else if (lua_isnumber(L,idx))
    return lua_tointeger(L,idx);
  else
    return ZIPE_CPU_NONE;
}

/***********************************************************************/

static uint16_t zwlua_tolicense(lua_State *L,int idx)
{
  if (lua_isstring(L,idx))
  {
    const char *lic = luaL_checkstring(L,idx);
  
    if (strcmp(lic,"LGPL3+") == 0)
      return ZIPE_LIC_LGPL3;
    else if (strcmp(lic,"MIT") == 0)
      return ZIPE_LIC_MIT;
    else if (strcmp(lic,"MIT/X11") == 0)
      return ZIPE_LIC_MIT;
    else if (strcmp(lic,"none") == 0)
      return ZIPE_LIC_NONE;
    else
      return ZIPE_LIC_UNKNOWN;
  }
  else if (lua_isnumber(L,idx))
    return lua_tointeger(L,idx);
  else
    return ZIPE_LIC_NONE;
}

/***********************************************************************/

static size_t zwlua_toextra(lua_State *L,int idx,zip_lua_ext__s *ext)
{
  if (lua_isnil(L,idx))
    return 0;
  else
  {
    idx       = abs_index(L,idx);
    ext->id   = ZIP_EXT_LUA;
    ext->size = sizeof(zip_lua_ext__s) - (sizeof(uint16_t) * 2);
    
    lua_getfield(L,idx,"language");
    ext->lang = zwlua_tolanguage(L,-1,ZIPE_LANG_LUA);
    lua_getfield(L,idx,"lvmin");
    ext->lvmin = zwlua_toversion(L,-1,DEF_LUA_MAJOR,DEF_LUA_MINOR);
    lua_getfield(L,idx,"lvmax");
    ext->lvmax = zwlua_toversion(L,-1,DEF_LUA_MAJOR,DEF_LUA_MINOR);
    lua_getfield(L,idx,"version");
    ext->version = zwlua_toversion(L,-1,0,0);
    lua_getfield(L,idx,"os");
    ext->os = zwlua_toos(L,-1);
    lua_getfield(L,idx,"cpu");
    ext->cpu = zwlua_tocpu(L,-1,ext->os);
    lua_getfield(L,idx,"license");
    ext->license = zwlua_tolicense(L,-1);  
    lua_pop(L,6);
    return sizeof(zip_lua_ext__s);
  }
}

/***********************************************************************/

static size_t zwlua_tostring(lua_State *L,int idx,const char **pstr)
{
  if (lua_isnil(L,idx))
    return 0;
  else
  {
    size_t size;
    *pstr = lua_tolstring(L,idx,&size);
    return size;
  }
}

/***********************************************************************/

static void zwlua_tomodtime(
        lua_State *L,
        int idx,
        uint16_t *modtime,
        uint16_t *moddate
)
{
  time_t    mtime;
  struct tm stm;
  
  if (lua_isnil(L,idx))
    mtime = time(NULL);
  else
    mtime = lua_tonumber(L,idx);
  
  stm = *gmtime(&mtime);
  
  *modtime = (stm.tm_hour << 11)
           | (stm.tm_min  <<  5)
           | (stm.tm_sec  >>  1) /* 2 second increment */
           ;
  *moddate = (((stm.tm_year + 1900) - 1980) << 9)
           |  ((stm.tm_mon  +    1)         << 5)
           |  ((stm.tm_mday       )         << 0)
           ;
}

/***********************************************************************
*
*	err = zipw.data(lem,{
*		crc   = crc,		-- MANDATORY
*		csize = compresssize,	-- MANDATORY
*		usize = normalsize	-- MANDATORY
*	})
*
*	lem - file open for binary writing
*
* This function writes the data descriptor record to the LEM file.  This
* MUST be written if zipw.file() was called with a crc, csize and usize
* of 0 (or not present).  This MUST be written immediately after the 
* compressed data has been written.
*
* If it was written, return 0.  Otherwise, return an error.
*
**************************************************************************/

static int zipwlua_data(lua_State *L)
{
  FILE        **pfp;
  zip_data__s   data;
  
  pfp = luaL_checkudata(L,1,LUA_FILEHANDLE);
  luaL_checktype(L,2,LUA_TTABLE);
  
  lua_getfield(L,2,"crc");
  data.crc = luaL_checknumber(L,-1);
  lua_getfield(L,2,"csize");
  data.csize = luaL_checknumber(L,-1);
  lua_getfield(L,2,"usize");
  data.usize = luaL_checknumber(L,-1);
  
  /* FIXME: adjust byte order on big endian systems */

  errno = 0;
  fwrite(&data,sizeof(zip_data__s),1,*pfp);
  lua_pushinteger(L,errno);
  return 1;
}

/***********************************************************************
*
* offset,err = zipw.file(lem, {
*		modtime = modtime,	-- os.time()
*		crc     = crc,		-- [1]
*		csize   = compresssize,	-- [1]
*		usize   = normalsize,	-- [1]
*		name    = name,		-- MANDATORY
*		data    = true,		-- false (used to mark zip_data__s usage)
*		utf8    = true,		-- false (filename in UTF-8)
*		extra   = {
*				luamin = ver,	-- _G._VERSION
*				luamax = ver,	-- _G__VERSION
*				version = ver,	-- '0.0'
*				os      = os,	-- 'none'
*				cpu     = cpu,	-- 'none'
*				license = lic,	-- 'unknown'
*			  },
*	})
*
*	lem - file open for binary writing
*
* This function writes a local file header to the LEM file.  This can appear
* anywhere in the LEM file, but MUST NOT be the last thing written.  This
* does NOT write the compressed data to the file, but any compressed data
* MUST be written after this call, and such data MUST appear immediately
* after this header.  
*
* This returns the offset of the local file header in the file.
*
* [1]	The crc, csize and usize fields are 0, or are not present, then
*	a call to zipw.data() MUST happen after the compressed data is
*	written to the file.  Also, the data flag must be set to true.
*
**************************************************************************/

static int zipwlua_file(lua_State *L)
{
  FILE           **pfp;
  long             pos;
  zip_file__s      file;
  zip_lua_ext__s   ext;
  const char      *name;
  
  pfp = luaL_checkudata(L,1,LUA_FILEHANDLE);
  pos = ftell(*pfp);
  luaL_checktype(L,2,LUA_TTABLE);
  
  file.magic       = ZIP_MAGIC_FILE;
  file.byversion   = 20;	/* MS-DOS, ZIP version 2 */
  file.compression =  8;	/* deflate */
  file.flags       =  0;
  
  lua_getfield(L,2,"modtime");
  zwlua_tomodtime(L,-1,&file.modtime,&file.moddate);
  lua_getfield(L,2,"crc");
  file.crc = luaL_optnumber(L,-1,0);
  lua_getfield(L,2,"csize");
  file.csize = luaL_optnumber(L,-1,0);
  lua_getfield(L,2,"usize");
  file.usize = luaL_optnumber(L,-1,0);
  lua_getfield(L,2,"data");
  file.flags |= lua_toboolean(L,-1) ? ZIPF_DATA : 0;
  lua_getfield(L,2,"utf8");
  file.flags |= lua_toboolean(L,-1) ? ZIPF_UTF8 : 0;
  lua_getfield(L,2,"name");
  file.namelen = zwlua_tostring(L,-1,&name);
  lua_getfield(L,2,"extra");
  file.extralen = zwlua_toextra(L,-1,&ext);
  
  /* FIXME: adjust byte order on big endian systems */
  
  if (!zwlib_fwrite(L,&file,sizeof(file),1,*pfp))
    return 2;
  if (!zwlib_fwrite(L,name,1,file.namelen,*pfp))
    return 2;
  if (!zwlib_fwrite(L,&ext,1,file.extralen,*pfp))
    return 2;
  
  lua_pushnumber(L,pos);
  lua_pushinteger(L,0);
  return 2;
}

/***********************************************************************
*
* offset,err = zipw.dir(lem, {
*		modtime = modtime,	-- os.time()
*		crc     = crc,		-- MANDATORY
*		csize   = compresssize,	-- MANDATORY
*		usize   = normalsize,	-- MANDATORY
*		name    = name,		-- MANDATORY
*		extra   = {
*				luamin = ver,	-- _G._VERSION
*				luamax = ver,	-- _G__VERSION
*				version = ver,	-- '0.0'
*				os      = os,	-- 'none'
*				cpu     = cpu,	-- 'none'
*				license = lic,	-- 'unknown'
*			  },
*		comment = comment,
*		text    = true,		-- false (text file)
*		data    = true,		-- false (used to mark zip_data__s usage)
*		utf8    = true,		-- false (filename/comment in UTF-8)
*		offset  = lem:seek()	-- MANDATORY
*	})
*
*	lem - file open for binary writing
*
* This function writes a directory entry to the LEM file.  All such entries
* MUST appear consecutively in the file, but can appear anywhere except at
* the end of the file.
*
* This returns the offset of the directory entry in the file.
*
**************************************************************************/

static int zipwlua_dir(lua_State *L)
{
  FILE           **pfp;
  long             pos;
  zip_dir__s       dir;
  zip_lua_ext__s   ext;
  const char      *name;
  const char      *comment;
  
  pfp = luaL_checkudata(L,1,LUA_FILEHANDLE);
  pos = ftell(*pfp);
  luaL_checktype(L,2,LUA_TTABLE);
  
  dir.magic       = ZIP_MAGIC_CFILE;
  dir.byversion   = 20;		/* MS-DOS, ZIP version 2.0 */
  dir.forversion  = 20;
  dir.compression =  8;		/* deflate */
  dir.flags       =  0;
  dir.diskstart   =  0;
  dir.eattr       =  0;
  
  lua_getfield(L,2,"modtime");
  zwlua_tomodtime(L,-1,&dir.modtime,&dir.moddate);
  lua_getfield(L,2,"crc");
  dir.crc = luaL_checknumber(L,-1);
  lua_getfield(L,2,"csize");
  dir.csize = luaL_checknumber(L,-1);
  lua_getfield(L,2,"usize");
  dir.usize = luaL_checknumber(L,-1);
  lua_getfield(L,2,"name");
  dir.namelen = zwlua_tostring(L,-1,&name);
  lua_getfield(L,2,"extra");
  dir.extralen = zwlua_toextra(L,-1,&ext);
  lua_getfield(L,2,"comment");
  dir.commentlen = zwlua_tostring(L,-1,&comment);
  lua_getfield(L,2,"data");
  dir.flags |= lua_toboolean(L,-1) ? ZIPF_DATA : 0;
  lua_getfield(L,2,"utf8");
  dir.flags |= lua_toboolean(L,-1) ? ZIPF_UTF8 : 0;
  lua_getfield(L,2,"text");
  dir.iattr = lua_toboolean(L,-1) ? ZIPIA_TEXT : 0;
  lua_getfield(L,2,"offset");
  dir.offset = lua_tonumber(L,-1);
  
  /* FIXME: adjust byte order on big endian systems */

  if (!zwlib_fwrite(L,&dir,sizeof(dir),1,*pfp))
    return 2;
  if (!zwlib_fwrite(L,name,1,dir.namelen,*pfp))
    return 2;
  if (!zwlib_fwrite(L,&ext,1,dir.extralen,*pfp))
    return 2;
  if (!zwlib_fwrite(L,comment,1,dir.commentlen,*pfp))
    return 2;
  
  lua_pushnumber(L,pos);
  lua_pushinteger(L,0);
  return 2;
}

/***********************************************************************
*
*	err = zipw.eocd(lem,{
*		entries = #entries,	-- 0
*		size    = direize,	-- 0 (size in bytes of directory)
*		offset  = offset	-- 0 (offset in file of directory)
*	})
*
*	lem - file open for binary writing
*
* This function writes the End of the Central Directory.  This MUST be
* at the end of the file, and if everything is 0, then it creates an
* empty LEM file.
*
* If the data was written, return 0; otherwise, return an error.
*
**************************************************************************/

static int zipwlua_eocd(lua_State *L)
{
  zip_eocd__s   eocd;
  FILE        **pfp;
  
  pfp = luaL_checkudata(L,1,LUA_FILEHANDLE);
  luaL_checktype(L,2,LUA_TTABLE);
  
  eocd.magic      = ZIP_MAGIC_EOCD;
  eocd.disknum    = 0;
  eocd.diskstart  = 0;
  eocd.commentlen = 0;
  
  lua_getfield(L,2,"entries");
  eocd.totalentries = luaL_optnumber(L,-1,0);
  eocd.entries      = eocd.totalentries;
  lua_getfield(L,2,"size");
  eocd.size = luaL_optnumber(L,-1,0);
  lua_getfield(L,2,"offset");
  eocd.offset = luaL_optnumber(L,-1,0);
  
  /* FIXME: adjust byte order on big endian systems */
  
  fwrite(&eocd,sizeof(zip_eocd__s),1,*pfp);
  lua_pushinteger(L,errno);
  return 1;
}

/***********************************************************************/

static const luaL_Reg m_ziplua[] =
{
  { "data"	, zipwlua_data	} ,
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

