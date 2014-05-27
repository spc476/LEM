#!/usr/bin/env lua
-- ***************************************************************
--
-- Copyright 2014 by Sean Conner.  All Rights Reserved.
-- 
-- This library is free software; you can redistribute it and/or modify it
-- under the terms of the GNU Lesser General Public License as published by
-- the Free Software Foundation; either version 3 of the License, or (at your
-- option) any later version.
-- 
-- This library is distributed in the hope that it will be useful, but
-- WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
-- or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
-- License for more details.
-- 
-- You should have received a copy of the GNU Lesser General Public License
-- along with this library; if not, see <http://www.gnu.org/licenses/>.
--
-- Comments, questions and criticisms can be sent to: sean@conman.org
--
-- ********************************************************************

local errno = require "org.conman.errno"
local fsys  = require "org.conman.fsys"
local zipw  = require "zipw"
local mz    = require "mz"

-- **************************************************************************
-- This code assumes that LuaRocks 2.1.2-1 has been downloaded and extracted
-- in /tmp.  Also that the rockspec for the same is also in /tmp.
-- **************************************************************************

local ROCK = arg[1] or "/tmp/luarocks-2.1.2-1.rockspec"
local SRC  = "/tmp/luarocks-2.1.2/"

-- **********************************************************************
-- Change the following as needed.
-- **********************************************************************

local SITE_CONFIG = [[
module("luarocks.site_config")
LUAROCKS_PREFIX="/usr/local"
LUA_INCDIR="/usr/local/include"
LUA_LIBDIR="/usr/local/lib"
LUA_BINDIR="/usr/local/bin"
LUAROCKS_SYSCONFDIR="."
LUAROCKS_ROCKS_TREE="/usr/local"
LUAROCKS_UNAME_S="Linux"
LUAROCKS_UNAME_M="i686"
LUAROCKS_DOWNLOADER="wget"
LUAROCKS_MD5CHECKER="md5sum"
]]

local CONFIG_5_1 = [[
rocks_trees = {
   home.."/.luarocks",
   "/usr/local"
}
]]

local CONFIG = [[
rocks_servers = {
  "http://luarocks.org/repositories/rocks"
}
rocks_trees = {
   home.."/.luarocks",
   "/usr/local"
}
]]

-- **********************************************************************

local function error(...)
  io.stderr:write(string.format(...),"\n")
  os.exit(1)
end

-- **********************************************************************

do
  local paths = {}
  for path in os.getenv("PATH"):gmatch("[^:]+") do
    table.insert(paths,path)
  end
  
  function find_cmd(name)
    for _,path in ipairs(paths) do
      local name = path .. "/" .. name
      local file = io.open(name)
      if file then
        file:close()
        return name
      end
    end
  end
end

-- **********************************************************************

do
  local DIR  = package.config:sub(1,1)
  local SEP  = package.config:sub(3,3)
  local MARK = package.config:sub(5,5)
  local IGN  = package.config:sub(9.9)
  
  function find_mod(name)
    local modname = name:gsub("%.",DIR)
    
    for path in package.path:gmatch("[^%" .. SEP .. "]+") do
      local fname = path:gsub("[%" .. MARK .. "]",modname)
      local file  = io.open(fname)
      if file then
        file:close()
        return fname
      end
    end
  end
end

-- **********************************************************************

function load_rockspec(rockspec)
  local e = {}
  local f = loadfile(rockspec)
  setfenv(f,e)
  f()
  return e
end

-- **********************************************************************

list = {}
rock = load_rockspec(ROCK)
lem  = io.open("luarocks.lem","wb")

if not fsys.chdir(SRC) then
  error("%s has a problem",SRC)
end

do
  local info  = fsys.stat(ROCK)
  local com   = {}
  local crc   = 0
  local f     = io.open(ROCK,"rb")
  
  local rc,err = mz.deflate(
    function()
      local d = f:read(8192)
      crc     = mz.crc(crc,d)
      return d
    end,
    function(s)
      table.insert(com,s)
    end
  )
  f:close()
  com = table.concat(com)
  
  table.insert(list, {
        extra   = true,
  	name    = "_LEM",
  	os      = "none",
  	cpu     = "_LEM",
  	version = "0.1",
  	luamin  = "5.1",
  	luamax  = "5.255",
  	crc     = crc,
  	csize   = #com,
  	usize   = info.st_size,
  	modtime = info.st_mtime,
  	license = rock.description.license,
  })
  
  list[1].offset,err = zipw.file(lem,list[1],true)
  
  if not list[1].offset then error("_LEM: %s",errno[err]) end
  lem:write(com)
end

for cmd,file in pairs(rock.build.install.bin) do
  local info,err = fsys.stat(file)
  if not info then
    error("%s %s",file,errno[err])
  end
  
  local com = {}
  local crc = 0
  local f   = io.open(file,"rb")
  
  local rc,err = mz.deflate(
    function()
      local d = f:read(8192)
      crc     = mz.crc(crc,d)
      return d
    end,
    function(s)
      table.insert(com,s)
    end
  )
  f:close()
  com = table.concat(com)
  
  table.insert(list, {
        extra   = true,
  	name    = "_APP/" .. cmd,
  	os      = "none",
  	cpu     = "none",
  	version = rock.version,
  	luamin  = "5.1",
  	luamax  = "5.255",
  	crc     = crc,
  	csize   = #com,
  	usize   = info.st_size,
  	modtime = info.st_mtime,
  	license = rock.description.license,
  })
  
  list[#list].offset,err = zipw.file(lem,list[#list],true)
  
  if not list[#list].offset then erorr("%s: %s",cmd,errno[err]) end
  lem:write(com)
end

for module,file in pairs(rock.build.modules) do
  local info,err = fsys.stat(file)
  if not info then
    error("%s: %s",file,errno[err])
  end
  
  local com = {}
  local crc = 0
  local f   = io.open(file,"rb")
  
  local rc,err = mz.deflate(
    function()
      local d = f:read(8192)
      crc     = mz.crc(crc,d)
      return d
    end,
    function(s)
      table.insert(com,s)
    end
  )
  f:close()
  com = table.concat(com)
  
  table.insert(list, {
        extra   = true,
  	name    = "_MODULES/" .. module,
  	os      = "none",
  	cpu     = "none",
  	version = rock.version,
  	luamin  = "5.1",
  	luamax  = "5.255",
  	crc     = crc,
  	csize   = #com,
  	usize   = info.st_size,
  	modtime = info.st_mtime,
  	license = rock.description.license,
  })
  
  list[#list].offset,err = zipw.file(lem,list[#list],true)
  
  if not list[#list].offset then error("%s: %s",module,errno[err]) end
  lem:write(com)
end

-- **********************************************************************

local function compress_string(text)
  local com = {}
  local crc = 0
  
  local rc,err = mz.deflate(
    function()
      crc = mz.crc(crc,text)
      local x = text
      text = nil
      return x
    end,
    function(s)
      table.insert(com,s)
    end
  )
  return table.concat(com),crc
end

local com,crc = compress_string(SITE_CONFIG)

table.insert(list,{
	extra = true,
	name  = "_MODULES/luarocks.site_config",
	os    = "none",
	cpu   = "none",
	version = rock.version,
	luamin  = "5.1",
	luamax  = "5.255",
	crc     = crc,
	csize   = #com,
	usize   = #SITE_CONFIG,
	modtime = os.time(),
	license = rock.description.license,
})

list[#list].offset,err = zipw.file(lem,list[#list],true)
if not list[#list].offset then error("site_config: %s",errno[err]) end
lem:write(com)
  
local com,crc = compress_string(CONFIG_5_1)
table.insert(list,{
	name = "_FILES/config-5.1.lua",
	crc  = crc,
	csize = #com,
	usize = #CONFIG_5_1,
	modtime = os.time(),
})

list[#list].offset,err = zipw.file(lem,list[#list])
if not list[#list].offset then error("site_config: %s",errno[err]) end
lem:write(com)

local com,crc = compress_string(CONFIG)
table.insert(list,{
	name = "_FILES/config.lua",
	crc  = crc,
	csize = #com,
	usize = #CONFIG,
	modtime = os.time(),
})

list[#list].offset,err = zipw.file(lem,list[#list])
if not list[#list].offset then error("site_config: %s",errno[err]) end
lem:write(com)

local sortkey = { _LEM = 1 , _APP = 1 , _MODULES = 2 , _FILES = 3 }

table.sort(list,function(a,b)
  local am,af = a.name:match("^([^/]+)/?(.*)")
  local bm,bf = b.name:match("^([^/]+)/?(.*)")
  local ak = sortkey[am]
  local bk = sortkey[bm]
  if ak < bk then return true end
  if ak > bk then return false end
  return af < bf
end)

for _,entry in ipairs(list) do
  local err
  entry.coffset,err = zipw.dir(lem,entry,entry.extra)
  if not entry.coffset then
    error("%s: %s",entry.name,errno[err])
  end
end

zipw.eocd(
	lem,
	#list,
	lem:seek() - list[1].coffset,
	list[1].coffset
)

lem:close()

