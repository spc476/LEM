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
  size_t      bsize;
  const char *blob   = luaL_checklstring(L,1,&bsize);
  size_t      big    = luaL_checknumber(L,2);
  z_stream    sin;
  Bytef      *out;
  int         rc;
  
  out = malloc(big);
  sin.zalloc    = Z_NULL;
  sin.zfree     = Z_NULL;
  sin.opaque    = Z_NULL;
  sin.avail_in  = 0;
  sin.avail_out = 0;
  
  rc = inflateInit2(&sin,-MAX_WBITS);
  if (rc != Z_OK) { printf("1 %d %s\n",rc,sin.msg); exit(1); }
  
  sin.next_in   = (Byte *)blob;
  sin.avail_in  = bsize;
  sin.next_out  = out;
  sin.avail_out = big;
  
  rc = inflate(&sin,Z_FINISH);
  if (rc != Z_STREAM_END) 
  { 
    printf("2 %d %s\n",rc,sin.msg);
    printf("2 %lu\n",sin.total_in);
    printf("2 %lu\n",sin.total_out);
    exit(1);
  }
  
  rc = inflateEnd(&sin);
  if (rc != Z_OK) { printf("3 %d %s\n",rc,sin.msg); exit(1); }
  
  lua_pushlstring(L,(char *)out,big);
  free(out);
  return 1;
}

/**************************************************************************/

static int mz_deflate(lua_State *L)
{
  size_t      size;
  const char *data  = luaL_checklstring(L,1,&size);
  int         level = luaL_optinteger(L,2,Z_DEFAULT_COMPRESSION);
  z_stream    sin;
  Bytef      *out;
  int         rc;
  
  out           = malloc(size);
  sin.zalloc    = Z_NULL;
  sin.zfree     = Z_NULL;
  sin.opaque    = Z_NULL;
  sin.avail_in  = 0;
  sin.avail_out = 0;
  
  rc = deflateInit2(
          &sin,
          level,
          Z_DEFLATED,
          -MAX_WBITS,
          DEF_MEM_LEVEL,
          Z_DEFAULT_STRATEGY
  );
  if (rc != Z_OK) { printf("1 %d %s\n",rc,sin.msg); exit(1); }
  
  sin.next_in   = (Bytef *)data;
  sin.avail_in  = size;
  sin.next_out  = out;
  sin.avail_out = size;
  
  rc = deflate(&sin,Z_FINISH);
  if (rc != Z_STREAM_END) { printf("2 %d %s\n",rc,sin.msg); exit(1); }
  
  rc = deflateEnd(&sin);
  if (rc != Z_OK) { printf("3 %d %s\n",rc,sin.msg); exit(1); }
  
  lua_pushlstring(L,(char *)out,(size_t)sin.total_out);
  free(out);
  return 1;
}

/**************************************************************************/

static int mz_crc32(lua_State *L)
{
  size_t      size;
  const char *data = luaL_checklstring(L,1,&size);
  
  lua_pushnumber(L,crc32(0,(Byte *)data,size));
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
