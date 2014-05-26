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

#include <stdlib.h>

#include <zconf.h>
#include <zutil.h>
#include <zlib.h>
#include <lua.h>
#include <lauxlib.h>

/**************************************************************************/

static int mz_inflate(lua_State *L)
{
  z_stream sin;
  Bytef    out[LUAL_BUFFERSIZE];
  int      rc;

  luaL_checktype(L,1,LUA_TFUNCTION);
  luaL_checktype(L,2,LUA_TFUNCTION);
  
  sin.zalloc    = Z_NULL;
  sin.zfree     = Z_NULL;
  sin.opaque    = Z_NULL;
  sin.avail_in  = 0;
  sin.avail_out = 0;
  
  /*-----------------------------------------------------------------------
  ; The documentation for zlib() states that when using inflateInit2() with
  ; a windowBits value between -8 and -15 will not expect normal deflate
  ; data (with the typical header and hash) but the raw deflate data, which
  ; is what ZIP files use, thus the use of -MAX_WBITS here.
  ;-----------------------------------------------------------------------*/
  
  rc = inflateInit2(&sin,-MAX_WBITS);
  if (rc != Z_OK)
  {
    lua_pushinteger(L,rc);
    lua_pushstring(L,sin.msg);
    return 2;
  }
  
  do
  {
    /*-------------------------------------------------------------------
    ; we pull some data in (from the source function), do a little dance,
    ; then push some data out (to the sink function).  We keep doing this
    ; until we're done.
    ;--------------------------------------------------------------------*/
    
    if (sin.avail_in == 0)
    {  
      lua_pushvalue(L,1);
      lua_call(L,0,1);
      sin.next_in = (Byte *)luaL_checklstring(L,-1,&sin.avail_in);
      lua_pop(L,1);
    }

    sin.next_out  = out;
    sin.avail_out = sizeof(out);

    rc = inflate(&sin,Z_SYNC_FLUSH);
    
    if ((rc == Z_OK) || (rc == Z_STREAM_END))
    {
      lua_pushvalue(L,2);
      lua_pushlstring(L,(char *)out,sizeof(out) - sin.avail_out);
      lua_call(L,1,0);
    }
    else
    {
      lua_pushinteger(L,rc);
      lua_pushstring(L,sin.msg);
      return 2;
    }
  } while (rc != Z_STREAM_END);  

  rc = inflateEnd(&sin);
  if (rc != Z_OK)
  {
    lua_pushinteger(L,rc);
    lua_pushstring(L,sin.msg);
    return 2;
  }
  
  lua_pushinteger(L,0);
  lua_pushliteral(L,"Success");
  
  return 2;
}

/**************************************************************************/

static int mz_deflate(lua_State *L)
{
  z_stream sin;
  Bytef    out[LUAL_BUFFERSIZE];
  int      flush;
  int      rc;
  
  if (lua_gettop(L) == 2)
  {
    lua_pushinteger(L,Z_DEFAULT_COMPRESSION);
    lua_insert(L,1);
  }
  
  luaL_checktype(L,2,LUA_TFUNCTION);
  luaL_checktype(L,3,LUA_TFUNCTION);

  sin.zalloc    = Z_NULL;
  sin.zfree     = Z_NULL;
  sin.opaque    = Z_NULL;
  sin.avail_in  = 0;
  sin.avail_out = 0;
  
  /*-----------------------------------------------------------------------
  ; To keep from generating the deflate header and hash, we need to inform
  ; zlib() that we want just the raw data.  To do so, we call deflateInit2()
  ; with a bunch of default values, except for the windowBits paramter,
  ; which is negative, informing zlib() we just want the raw data.
  ;------------------------------------------------------------------------*/
  
  rc = deflateInit2(
          &sin,
          lua_tointeger(L,1),
          Z_DEFLATED,
          -MAX_WBITS,
          DEF_MEM_LEVEL,
          Z_DEFAULT_STRATEGY
  );
  if (rc != Z_OK)
  {
    lua_pushnil(L);
    lua_pushstring(L,sin.msg);
    return 2;
  }

  do
  {
    /*----------------------------------------
    ; again with the pull/push dance of data
    ;-----------------------------------------*/
    
    if (sin.avail_in == 0)
    {
      lua_pushvalue(L,2);
      lua_call(L,0,1);
      sin.next_in = (Byte *)lua_tolstring(L,-1,&sin.avail_in);
      lua_pop(L,1);
      flush = (sin.avail_in == 0) 
            ? Z_FINISH 
            : Z_SYNC_FLUSH
            ;
    }
    
    sin.next_out  = out;
    sin.avail_out = sizeof(out);
    
    rc = deflate(&sin,flush);
    
    if ((rc == Z_OK) || (rc == Z_STREAM_END))
    {
      lua_pushvalue(L,3);
      lua_pushlstring(L,(char *)out,sizeof(out) - sin.avail_out);
      lua_call(L,1,0);
    }
    else
    {
      lua_pushinteger(L,rc);
      lua_pushstring(L,sin.msg);
      return 2;
    }
  } while(rc != Z_STREAM_END);

  rc = deflateEnd(&sin);
  if (rc != Z_OK)
  {
    lua_pushinteger(L,rc);
    lua_pushstring(L,sin.msg);
    return 2;
  }
  
  lua_pushinteger(L,0);
  lua_pushliteral(L,"Success");
  return 2;
}

/**************************************************************************/

static int mz_crc32(lua_State *L)
{
  unsigned long  initcrc = luaL_checknumber(L,1);
  size_t         size;
  const char    *data    = luaL_checklstring(L,2,&size);
  
  lua_pushnumber(L,crc32(initcrc,(Byte *)data,size));
  return 1;
}

/**************************************************************************/

int luaopen_mz(lua_State *L)
{
  lua_createtable(L,0,0);
  lua_pushcfunction(L,mz_inflate);
  lua_setfield(L,-2,"inflate");
  lua_pushcfunction(L,mz_deflate);
  lua_setfield(L,-2,"deflate");
  lua_pushcfunction(L,mz_crc32);
  lua_setfield(L,-2,"crc");
  return 1;
}
