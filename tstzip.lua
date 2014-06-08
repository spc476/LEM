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
-- ------------------------------------------------------------------
--
-- Loads and tests a few modules from the sample.lem file.
--
-- ********************************************************************

local sys   = require "org.conman.sys"
local errno = require "org.conman.errno"
local zipr  = require "org.conman.zip.read"
local zlib  = require "zlib"
local dump  = require "org.conman.table".dump

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

local function read_data(entry,lem)
  lem:seek('set',entry.offset)
  local file = zipr.file(lem)
  local cmp  = lem:read(entry.csize)
  local data = zlib.decompress(cmp,-15)
  
  if #data ~= entry.usize then
    return nil,string.format("expecting %d bytes, got %d bytes",entry.usize,#data)
  end
  
  local crc = zlib.crc32(0,data)
  
  if crc ~= entry.crc then
    return nil,"CRC error"
  end
  
  return data
end

-- ***********************************************************************

local LANGUAGE = "Lua"
local VERSION  = _VERSION:match("^Lua (.*)")
local MODULES  = {}
local APP      = {}
local FILES    = {}
local META
local lem

do
  local function store(entry)
    local function add(list,name)
      if LANGUAGE ~= entry.extra[0x454C].language then
        return
      end
      
      if VERSION < entry.extra[0x454C].lvmin
      or VERSION > entry.extra[0x454C].lvmax
      then
        return
      end
      
      if list[name] then
        if entry.extra[0x454C].version < list[name].extra[0x454C].version then
          return
        end
      end
      
      list[name] = entry
    end
    
    local list,name = entry.name:match("^([^%/]+)%/(.*)")
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
    elseif not entry.extra[0x454C].os then
      add(dolist,name)
    else
      if  entry.extra[0x454C].os  == sys.SYSNAME 
      and entry.extra[0x454C].cpu == sys.CPU 
      then
        add(dolist,name)
      end      
    end
  end
  
  lem        = io.open("sample.lem","rb")
  local eocd = zipr.eocd(lem);
  local dir  = zipr.dir(lem,eocd.entries)

  for i = 1 , #dir do
    store(dir[i])
  end  
end

-- ***********************************************************************

local function zip_loader(name)
  if not MODULES[name] then return string.format("\n\tno file %s",name) end

  if not MODULES[name].extra[0x454C].os then
    local data,err = read_data(MODULES[name],lem)
    
    if not data then
      return err
    end
    
    local func,err = load(function() local d = data data = nil return d end,name)
    
    if not func then 
      return string.format("%s: %s",name,err)
    else
      return func
    end
  end
  
  local lib      = os.tmpname()
  local f        = io.open(lib,"wb")
  local data,err = read_data(MODULES[name],lem)
  
  if not data then
    return err
  end
  
  f:write(data)
  f:close()
  
  local func,err = package.loadlib(lib,"luaopen_" .. name:gsub("%.","_"))
  os.remove(lib)
  
  if not func then
    return err
  else
    return func
  end  
end

-- ***********************************************************************

table.insert(package.loaders,2,zip_loader)

print("PATH",package.path)
print("CPATH",package.cpath)

date = require "org.conman.date"

print()
print(date.tojulianday())
print()

unix = require "org.conman.unix"

lpeg = require "lpeg"
print("LPeg",lpeg.version())

dump("USER",unix.users[os.getenv("USER")])

for name,entry in pairs(FILES) do
  dump(name,entry)
end
