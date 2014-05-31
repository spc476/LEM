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
-- -----------------------------------------------------------------
--
-- Test the excution of LuaRocks from a LEM file.
--
-- ********************************************************************

local dump  = require "org.conman.table".dump
local fsys  = require "org.conman.fsys"
local sys   = require "org.conman.sys"
local errno = require "org.conman.errno"
local zipr  = require "zipr"
local mz    = require "mz"

if #arg < 2 then
  io.stderr:write("usage: ",arg[0]," lemfile script ...","\n")
  os.exit(1)
end

LEMFILE = arg[1]
SCRIPT  = arg[2]

-- ***********************************************************************
-- Adjust the argument vector to remove this script and the LEM file name.
-- The rest of the arguments are then passed to LuaRocks.
-- ***********************************************************************

arg[0] = SCRIPT
table.remove(arg,1)
table.remove(arg,1)

-- *********************************************************************
-- Any module loaded past this point *WILL* come from the supplied LEM file. 
-- No ands, ifs or buts about it.
-- *********************************************************************

package.path  = ""
package.cpath = ""

-- ***********************************************************************

local function error(...)
  io.stderr:write(string.format(...),"\n")
  os.exit(1)
end

-- ***********************************************************************

local function read_data(entry,lem,sink)
  lem:seek('set',entry.offset)
  local file = zipr.file(lem)
  
  local rc,err = mz.inflate(function()  
    return lem:read(file.csize)
  end,sink)  
end

-- ***********************************************************************
-- Function to remove our working directory.  We clean up after ourselves.
-- ***********************************************************************

local function rmdir()
  local f = {}
  local d = {}
  
  for entry in fsys.dir() do
    local info = fsys.stat(entry)
    if info.type == 'file' then
      f[#f + 1] = entry
    elseif info.type == 'dir' then
      d[#d + 1] = entry
    end
  end

  for _,file in ipairs(f) do
    os.remove(file)
  end
  for _,sub in ipairs(d) do
    fsys.chdir(sub)
    rmdir()
    fsys.chdir("..")
    fsys.rmdir(sub)
  end
end

-- ***********************************************************************

local VER     = 5 * 256 + 1
local MODULES = {}
local APP     = {}
local FILES   = {}
local META
local lem

do
  local _LEM
  local function store(entry)
    local function add(list,name)
      if VER < entry.extra.lvmin 
      or VER > entry.extra.lvmax 
      then
        return
      end
      
      if list[name] then
        if entry.extra.version < list[name].extra.version then
          return
        end
      end
      
      list[name] = entry
    end
    
    if entry.extra and entry.extra.cpu == "_LEM" then
      _LEM = entry
      return
    end
    
    local list,name = entry.name:match("^_([^%/]+)%/(.*)")
    local dolist
    
    if list == 'MODULES' then
      dolist = MODULES
    elseif list == 'APP' then
      dolist = APP
    else
      dolist = FILES
      entry.name = name
    end
    
    if dolist == FILES then
      dolist[name] = entry
    elseif entry.extra.os == 'none' then
      add(dolist,name)
    else
      if  entry.extra.os  == sys.SYSNAME 
      and entry.extra.cpu == sys.CPU 
      then
        add(dolist,name)
      end      
    end
  end
  
  lem  = io.open(LEMFILE,"rb")
  local eocd = zipr.eocd(lem);

  lem:seek('set',eocd.offset)

  for i = 1 , eocd.numentries do
    local dir,err = zipr.dir(lem)
    if not dir then error("ERROR %s: %s","sample.lem",errno[err]) end
    store(dir)
  end

  local data = {}
  read_data(_LEM,lem,function(s) data[#data + 1] = s end)
  local thelem,err = load(function() return table.remove(data,1) end,"_LEM")
  
  if not thelem then error("_LEM: %s",err) end
  
  META = {}
  setfenv(thelem,META)
  thelem()
end

-- ***********************************************************************
-- Insert our new module loader.
-- ***********************************************************************

table.insert(package.loaders,2,function(name)
  if not MODULES[name] then return string.format("\n\tno file %s",name) end

  if MODULES[name].extra.os == 'none' then
    local data = {}
    read_data(MODULES[name],lem,function(s) data[#data + 1] = s end)
    local func,err = load(function() return table.remove(data,1) end,name)
    if not func then error("%s: %s",name,err) end
    return func
  end
    
  local lib  = os.tmpname()
  local f    = io.open(lib,"wb")
  
  read_data(MODULES[name],lem,function(s) f:write(s) end)
  
  local func,err = package.loadlib(lib,"luaopen_" .. name:gsub("%.","_"))
  os.remove(lib)
  
  if not func then
    return err
  else
    return func
  end  
end)

-- ***********************************************************************
-- Create some temporary space, then dump all the entries in _FILES there. 
-- While I can intercept io.open(), there are other ways that the files can
-- be loaded.  This is a quick workaround until I identify all the functions
-- that need to be overriden---if at all.
-- ***********************************************************************

_WORKDIR = os.tmpname()
os.remove(_WORKDIR)
if not fsys.mkdir(_WORKDIR) then error("cannot create workdir %s",_WORKDIR) end
fsys.chdir(_WORKDIR)

for name,info in pairs(FILES) do
  local f,err = io.open(info.name,"wb")
  if not f then error("%s: %s",name,err) end
  read_data(info,lem,function(s) f:write(s) end)
  f:close()  
end

-- ***********************************************************************
-- Patch os.exit() to clean up after ourselves.  
--
-- Monkeypatching.  Sigh.
-- ***********************************************************************

local sysexit = os.exit
os.exit = function(...)
  rmdir()
  fsys.rmdir(_WORKDIR)
  sysexit(...)
end

-- ***********************************************************************

if not APP[SCRIPT] then
  error("%s: not found",SCRIPT)
end

local data = {}
read_data(APP[SCRIPT],lem,function(s) data[#data + 1] = s end)

data[1] = data[1]:match("^#[^%c]+%c(.*)")

local func,err = load(function() return table.remove(data,1) end,SCRIPT)
if not func then error("%s: %s",SCRIPT,err) end

func(unpack(arg))
os.exit(0)
