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
*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
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

static void zwlua_pushext(lua_State *L,int idx,zip_lua_ext__s *ext)
{
  idx = abs_index(L,idx);
  
  lua_pushinteger(L,ext->luavmin);
  lua_setfield(L,idx,"luamin");
  lua_pushinteger(L,ext->luavmax);
  lua_setfield(L,idx,"luamax");
  lua_pushinteger(L,ext->version);
  lua_setfield(L,idx,"version");
  zwlua_pushos(L,ext->os);
  lua_setfield(L,idx,"os");
  zwlua_pushcpu(L,ext->cpu,ext->os);
  if (ext->os == ZIPE_OS_NONE)
    lua_setfield(L,idx,"file");
  else
    lua_setfield(L,idx,"cpu");
  zwlua_pushlicense(L,ext->license);
  lua_setfield(L,idx,"license");
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

/***********************************************************************/
 
static int ziprlua_file(lua_State *L)
{
  zip_file__s   file;
  FILE        **pfp;
  luaL_Buffer   buf;
  
  pfp = luaL_checkudata(L,1,LUA_FILEHANDLE);
  if (fread(&file,sizeof(zip_file__s),1,*pfp) != 1)
  {
    lua_pushnil(L);
    lua_pushinteger(L,errno);
    return 2;
  }
  
  if (
       (file.magic != ZIP_MAGIC_FILE)
       || (file.compression !=  8)
       || (file.byversion   != 20)
     )
  {
    lua_pushnil(L);
    lua_pushinteger(L,EINVAL);
    return 2;
  }
  
  lua_createtable(L,0,0);
  
  luaL_buffinit(L,&buf);
  while(file.namelen--)
    luaL_addchar(&buf,fgetc(*pfp));
  luaL_pushresult(&buf);
  lua_setfield(L,-2,"module");
  
  zwlua_pushmodtime(L,file.modtime,file.moddate);
  lua_setfield(L,-2,"modtime");
  lua_pushnumber(L,file.crc);
  lua_setfield(L,-2,"crc");
  lua_pushnumber(L,file.csize);
  lua_setfield(L,-2,"csize");
  lua_pushnumber(L,file.usize);
  lua_setfield(L,-2,"usize");
  
  if (file.extralen == sizeof(zip_lua_ext__s))
  {
    zip_lua_ext__s ext;
    
    if (fread(&ext,sizeof(zip_lua_ext__s),1,*pfp) != 1)
    {
      lua_pushnil(L);
      lua_pushinteger(L,errno);
      return 2;
    }
    
    if (
         (ext.id != ZIP_EXT_LUA)
         || (ext.size != sizeof(zip_lua_ext__s) - (sizeof(uint16_t) * 2))
      )
    {
      lua_pushnil(L);
      lua_pushinteger(L,EINVAL);
      return 2;
    }
    
    zwlua_pushext(L,-1,&ext);
  }
  else if (file.extralen > 0)
    while(file.extralen--)
      fgetc(*pfp);
  
  lua_pushinteger(L,0);
  return 2;
}

/***********************************************************************/

static int ziprlua_dir(lua_State *L)
{
  zip_dir__s    dir;
  FILE        **pfp;
  luaL_Buffer   buf;
  
  pfp = luaL_checkudata(L,1,LUA_FILEHANDLE);
  if (fread(&dir,sizeof(zip_dir__s),1,*pfp) != 1)
  {
    lua_pushnil(L);
    lua_pushinteger(L,errno);
    return 2;
  }
  
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
  
  luaL_buffinit(L,&buf);
  while(dir.namelen--)
    luaL_addchar(&buf,fgetc(*pfp));
  luaL_pushresult(&buf);
  lua_setfield(L,-2,"module");
  
  zwlua_pushmodtime(L,dir.modtime,dir.moddate);
  lua_setfield(L,-2,"modtime");
  lua_pushnumber(L,dir.crc);
  lua_setfield(L,-2,"crc");
  lua_pushnumber(L,dir.csize);
  lua_setfield(L,-2,"csize");
  lua_pushnumber(L,dir.usize);
  lua_setfield(L,-2,"usize");
  lua_pushboolean(L,(dir.iattr & ZIPIA_TEXT) == ZIPIA_TEXT);
  lua_setfield(L,-2,"text");
  lua_pushnumber(L,dir.offset);
  lua_setfield(L,-2,"offset");
  
  if (dir.extralen == sizeof(zip_lua_ext__s))
  {
    zip_lua_ext__s ext;
    
    if (fread(&ext,sizeof(zip_lua_ext__s),1,*pfp) != 1)
    {
      lua_pushnil(L);
      lua_pushinteger(L,errno);
      return 2;
    }
    
    if (
            (ext.id   != ZIP_EXT_LUA)
         || (ext.size != sizeof(zip_lua_ext__s) - (sizeof(uint16_t) * 2))
       )
    {
      lua_pushnil(L);
      lua_pushinteger(L,EINVAL);
      return 2;
    }
    
    zwlua_pushext(L,-1,&ext);
  }
  else if (dir.extralen > 0)
    while(dir.extralen--)
      fgetc(*pfp);

  lua_pushinteger(L,0);
  return 2;
}

/***********************************************************************/

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
  
  if (fread(&eocd,sizeof(zip_eocd__s),1,*pfp) != 1)
  {
    lua_pushnil(L);
    lua_pushinteger(L,ENODATA);
    return 2;
  }
  
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

