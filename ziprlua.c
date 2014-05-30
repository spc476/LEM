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
* The ZIP file reader.
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

#define abs_index(L,i) ((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : lua_gettop(L) + (i) + 1)

/***********************************************************************/

static bool zwlib_fread(
        lua_State *L,
        void      *data,
        size_t     size,
        size_t     nmemb,
        FILE      *fp
)
{
  if (nmemb > 0)
  {
    if (fread(data,size,nmemb,fp) == nmemb)
      return true;
    lua_pushnil(L);
    lua_pushinteger(L,errno);
  }
  
  return false;
}

/***********************************************************************/

static void zwlib_readstring(lua_State *L,FILE *fp,size_t len)
{
  if (len > 0)
  {
    luaL_Buffer buf;
  
    luaL_buffinit(L,&buf);
    while(len--)
      luaL_addchar(&buf,fgetc(fp));
    luaL_pushresult(&buf);
  }
  else
    lua_pushnil(L);
}

/***********************************************************************/

static void zwlua_pushlanguage(lua_State *L,uint16_t lang)
{
  switch(lang)
  {
    case ZIPE_LANG_LUA:    lua_pushliteral(L,"Lua");    break;
    case ZIPE_LANG_LUAJIT: lua_pushliteral(L,"LuaJIT"); break;
    case ZIPE_LANG_BASIC:  lua_pushliteral(L,"BASIC");  break;
    default:               lua_pushnil(L);              break;
  }
}

/***********************************************************************/

static void zwlua_pushos(lua_State *L,uint16_t os)
{
  switch(os)
  {
    case ZIPE_OS_LINUX:   lua_pushliteral(L,"Linux"); break;
    case ZIPE_OS_SOLARIS: lua_pushliteral(L,"SunOS"); break;
    case ZIPE_OS_NONE:    lua_pushliteral(L,"none");  break;
    default:              lua_pushnil(L);             break;
  }
}

/***********************************************************************/

static void zwlua_pushcpu(lua_State *L,uint16_t cpu,uint16_t os)
{
  switch(cpu)
  {
    case ZIPE_CPU_SPARC64: lua_pushliteral(L,"sparcv9"); break;
    case ZIPE_CPU_x86:     lua_pushliteral(L,"x86");     break;
    default:               lua_pushnil(L);               break;
    
    case ZIPE_CPU_NONE:
         if (os == ZIPE_OS_NONE)
           lua_pushliteral(L,"Lua");
         else
           lua_pushliteral(L,"none");
         break;
    
    case ZIPE_META_LEM:
         if (os == ZIPE_OS_NONE)
           lua_pushliteral(L,"_LEM");
         else
           lua_pushnil(L);
         break;
  }
}

/***********************************************************************/

static void zwlua_pushlicense(lua_State *L,uint16_t license)
{
  switch(license)
  {
    case ZIPE_LIC_LGPL3: lua_pushliteral(L,"LGPL3+"); break;
    case ZIPE_LIC_MIT:   lua_pushliteral(L,"MIT");    break;
    case ZIPE_LIC_NONE:  lua_pushliteral(L,"none");   break;
    default:             lua_pushnil(L);              break;
  }
}

/***********************************************************************/

static bool zwlua_pushext(lua_State *L,FILE *fp,size_t len)
{
  if (len == 0)
    lua_pushnil(L);
  else
  {
    if (len == sizeof(zip_lua_ext__s))
    {
      zip_lua_ext__s ext;
      
      if (!zwlib_fread(L,&ext,sizeof(zip_lua_ext__s),1,fp))
        return false;
      
      /* FIXME: adjust byte order on big endian systems */
      
      if (ext.id == ZIP_EXT_LUA)
      {
        lua_createtable(L,0,6);
  
        zwlua_pushlanguage(L,ext.lang);
        lua_setfield(L,-2,"language");
        lua_pushinteger(L,ext.lvmin);
        lua_setfield(L,-2,"luamin");
        lua_pushinteger(L,ext.lvmax);
        lua_setfield(L,-2,"luamax");
        lua_pushinteger(L,ext.version);
        lua_setfield(L,-2,"version");
        zwlua_pushos(L,ext.os);
        lua_setfield(L,-2,"os");
        zwlua_pushcpu(L,ext.cpu,ext.os);
        lua_setfield(L,-2,"cpu");
        zwlua_pushlicense(L,ext.license);
        lua_setfield(L,-2,"license");
      }
      else
        zwlib_readstring(L,fp,len);
    }
    else    
      zwlib_readstring(L,fp,len);
  }
  return true;
}

/***********************************************************************/

static void zwlua_pushmodtime(lua_State *L,uint16_t modtime,uint16_t moddate)
{
  struct tm stm;
  
  stm.tm_hour =   modtime >> 11;
  stm.tm_min  =  (modtime >>  5) & 0x3F;
  stm.tm_sec  = ((modtime      ) & 0x1F) << 1; /* 2 second increment */
  stm.tm_year = ((moddate >>  9) + 1980) - 1900;
  stm.tm_mon  =  (moddate >>  5) & 0x0F;
  stm.tm_mday =  (moddate      ) & 0x1F;
  lua_pushnumber(L,mktime(&stm));
}

/***********************************************************************
*
*	data,err = zipr.data(lem)
*
*	lem  - file open for binary reading
*	data - data descriptor 
*		.crc   - crc of normal data.
*		.csize - compressed size of file
*		.usize - normal size of file
*
* This function reads the data descriptor, which only exists if the data
* flag of a file is set.  If an error, returns nil and the error.
*
***************************************************************************/

static int ziprlua_data(lua_State *L)
{
  zip_data__s   data;
  FILE        **pfp;
  
  pfp = luaL_checkudata(L,1,LUA_FILEHANDLE);
  if (!zwlib_fread(L,&data,sizeof(zip_data__s),1,*pfp))
    return 2;
  
  if (data.magic != ZIP_MAGIC_DATA)
  {
    lua_pushnil(L);
    lua_pushinteger(L,EINVAL);
    return 2;
  }
  
  lua_createtable(L,0,3);
  lua_pushnumber(L,data.crc);
  lua_setfield(L,-2,"crc");
  lua_pushnumber(L,data.csize);
  lua_setfield(L,-2,"csize");
  lua_pushnumber(L,data.usize);
  lua_setfield(L,-2,"usize");
  
  lua_pushinteger(L,0);
  return 2;
}

/***********************************************************************
*
*	file,err = zipr.file(lem)
*
*	lem  - file open for binary reading
*	file - local file header
*		.modtime - modification time of file
*		.crc     - crc of file
*		.csize   - compressed size of file
*		.usize   - normal size of file
*		.name    - name of file
*		.data    - true if data descriptor for file is present
*		.utf8    - name/comment is in UTF-8
*		.extra   - extra data, raw data unless if Lua extension:
*			.luamin  - minimum Lua version supported
*			.luamax  - maximum Lua version supported
*			.version - version of file
*			.os      - OS required 
*			.cpu     - CPU required
*			.license - license of file
*
* This function reads a local file header.  If an error, returns nil and the
* error.
*
***************************************************************************/

static int ziprlua_file(lua_State *L)
{
  zip_file__s   file;
  FILE        **pfp;
  
  pfp = luaL_checkudata(L,1,LUA_FILEHANDLE);
  if (!zwlib_fread(L,&file,sizeof(zip_file__s),1,*pfp))
    return 2;
  
  /* FIXME: adjust byte order on big endian systems */

  if (
          (file.magic       != ZIP_MAGIC_FILE)
       || (file.compression !=  8)
       || (file.byversion   != 20)
     )
  {
    lua_pushnil(L);
    lua_pushinteger(L,EINVAL);
    return 2;
  }
  
  lua_createtable(L,0,8);

  zwlua_pushmodtime(L,file.modtime,file.moddate);
  lua_setfield(L,-2,"modtime");
  lua_pushnumber(L,file.crc);
  lua_setfield(L,-2,"crc");
  lua_pushnumber(L,file.csize);
  lua_setfield(L,-2,"csize");
  lua_pushnumber(L,file.usize);
  lua_setfield(L,-2,"usize");
  lua_pushboolean(L,(file.flags & ZIPF_DATA) == ZIPF_DATA);
  lua_setfield(L,-2,"data");
  lua_pushboolean(L,(file.flags & ZIPF_UTF8) == ZIPF_UTF8);
  lua_setfield(L,-2,"utf8");
  
  zwlib_readstring(L,*pfp,file.namelen);
  lua_setfield(L,-2,"name");
  if (!zwlua_pushext(L,*pfp,file.extralen))
    return 2;
  lua_setfield(L,-2,"extra");
  
  lua_pushinteger(L,0);
  return 2;  
}

/***********************************************************************
*
*	dir,err = zipr.dir(lem)
*
*	lem - file open for binary reading
*	dir - table of results:
*		.modtime - modification time of file
*		.crc     - crc of file
*		.csize   - compressed size of file
*		.usize   - normal size of file
*		.name    - name of file
*		.comment - file comment
*		.text    - true if file is text
*		.data    - true if data descriptor for file is present
*		.utf8    - name/comment is in UTF-8
*		.offset  - offset of local file header
*		.extra   - extra data, raw data unless if Lua extension:
*			.luamin  - minimum Lua version supported
*			.luamax  - maximum Lua version supported
*			.version - version of file
*			.os      - OS required 
*			.cpu     - CPU required
*			.license - license of file
*
* This function reads a directory entry.  If an error, returns nil and the
* error.
*
***************************************************************************/

static int ziprlua_dir(lua_State *L)
{
  zip_dir__s    dir;
  FILE        **pfp;
  
  pfp = luaL_checkudata(L,1,LUA_FILEHANDLE);
  if (!zwlib_fread(L,&dir,sizeof(zip_dir__s),1,*pfp))
    return 2;
  
  /* FIXME: adjust byte order on big endian systems */
  
  if (
          (dir.magic       != ZIP_MAGIC_CFILE)
       || (dir.compression !=  8)
       || (dir.forversion  != 20)
     )
  {
    lua_pushnil(L);
    lua_pushinteger(L,EINVAL);
    return 2;
  }
  
  lua_createtable(L,0,11);
  
  zwlua_pushmodtime(L,dir.modtime,dir.moddate);
  lua_setfield(L,-2,"modtime");
  lua_pushnumber(L,dir.crc);
  lua_setfield(L,-2,"crc");
  lua_pushnumber(L,dir.csize);
  lua_setfield(L,-2,"csize");
  lua_pushnumber(L,dir.usize);
  lua_setfield(L,-2,"usize");
  lua_pushnumber(L,dir.offset);
  lua_setfield(L,-2,"offset");
  lua_pushboolean(L,(dir.flags & ZIPF_DATA) == ZIPF_DATA);
  lua_setfield(L,-2,"data");
  lua_pushboolean(L,(dir.flags & ZIPF_UTF8) == ZIPF_UTF8);
  lua_setfield(L,-2,"utf8");
  lua_pushboolean(L,(dir.iattr & ZIPIA_TEXT) == ZIPIA_TEXT);
  lua_setfield(L,-2,"text");
  
  zwlib_readstring(L,*pfp,dir.namelen);
  lua_setfield(L,-2,"name");
  if (!zwlua_pushext(L,*pfp,dir.extralen))
    return 2;
  lua_setfield(L,-2,"extra");
  zwlib_readstring(L,*pfp,dir.commentlen);
  lua_setfield(L,-2,"comment");
  
  lua_pushinteger(L,0);
  return 2;
}

/***********************************************************************
*
*	eocd,err = zipr.eocd(lem)
*
*	lem  - file open for binary reading
*	eocd - table of results:
*		.entries - #directory entries
*		.size    - #bytes in directory
*		.offset  - offset if file of directory
*
* This function reads the End of Central Directory.  If it can't read the
* data, eocd is nil, and an error is returned.
*
***********************************************************************/

static int ziprlua_eocd(lua_State *L)
{
  FILE        **pfp;
  zip_eocd__s   eocd;
  
  pfp = luaL_checkudata(L,1,LUA_FILEHANDLE);
  
  if (fseek(*pfp,-(long)sizeof(zip_eocd__s),SEEK_END) < 0)
  {
    lua_pushnil(L);
    lua_pushinteger(L,errno);
    return 2;
  }
  
  if (!zwlib_fread(L,&eocd,sizeof(zip_eocd__s),1,*pfp))
    return 2;
    
  /* FIXME: adjust byte order on big endian systems */
  
  if (eocd.magic != ZIP_MAGIC_EOCD)
  {
    lua_pushnil(L);
    lua_pushinteger(L,EINVAL);
    return 2;
  }
  
  lua_createtable(L,0,3);
  lua_pushnumber(L,eocd.entries);
  lua_setfield(L,-2,"numentries");
  lua_pushnumber(L,eocd.size);
  lua_setfield(L,-2,"size");
  lua_pushnumber(L,eocd.offset);
  lua_setfield(L,-2,"offset");
  
  lua_pushinteger(L,0);
  return 2;  
}

/***********************************************************************/

static const luaL_Reg m_ziprlua[] =
{
  { "data"	, ziprlua_data	} ,
  { "file" 	, ziprlua_file	} ,
  { "dir"	, ziprlua_dir	} ,
  { "eocd"	, ziprlua_eocd	} ,
  { NULL	, NULL		}
};

int luaopen_zipr(lua_State *L)
{
  luaL_register(L,"zipr",m_ziprlua);
  return 1;
}

