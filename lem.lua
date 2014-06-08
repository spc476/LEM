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
-- -------------------------------------------------------------------
--
-- A Lua module to load modules from a LEM file.
--
-- ********************************************************************

local _VERSION     = _VERSION
local load         = load
local setmetatable = setmetatable

local sys    = require "org.conman.sys"
local zipr   = require "org.conman.zip.read"
local zlib   = require "zlib"

local io      = require "io"
local os      = require "os"
local package = require "package"
local string  = require "string"
local table   = require "table"
local package = require "package"

if _VERSION == "Lua 5.1" then
  module("lem")
end

local MODULES = {}
local APP     = {}
local FILES   = {}

-- ************************************************************************

local function lemloader(name)
  if not MODULES[name] then
    return string.format("\n\tno module %s found",name)
  end
  
  local chunk
  local lem = io.open(MODULES[name].FILE,"rb")

  if lem then
    lem:seek('set',MODULES[name].offset)
    local _        = zipr.file(lem)
    local compress = lem:read(MODULES[name].csize)
    chunk          = zlib.decompress(compress,-zlib.DEFAULT_WINDOWBITS)    
    lem:close()    
  else
    return string.format("\n\tmissing LEM file '%s' for %s",MODULES[name].FILE,name)
  end
  
  if not MODULES[name].extra[0x454C].os then
    local func,err = load(function() 
                            local d = chunk 
                            chunk = nil 
                            return d 
                          end,
                          name
                         )
    if not func then
      return string.format("\n\t%s: %s",name,err)
    else
      return func
    end
  else
    local lib = os.tmpname()
    local f   = io.open(lib,"wb")
    
    f:write(chunk)
    f:close()
    
    local func,err = package.loadlib(lib,"luaopen_" .. name:gsub("%.","_"))
    os.remove(lib)
    
    if not func then
      return string.format("\n\t%s: %s",name,err)
    else
      return func
    end
  end
end

-- ************************************************************************

local VERSION = _VERSION:match("^Lua (.*)")

local function store(entry,lemfile)
  local function add(list,name)
    if entry.extra[0x454C].language ~= "Lua" then
      return
    end
    
    if VERSION  < entry.extra[0x454C].lvmin
    or VERSION  > entry.extra[0x454C].lvmax then
      return
    end
    
    if list[name] then
      if entry.extra[0x454C].version < list[name].extra[0x454C].version then
        return
      end
    end
    
    entry.FILE = lemfile
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
    and entry.extra[0x454C].cpu == sys.CPU then
      add(dolist,name)
    end
  end
end

-- ************************************************************************

local function loadlem(lemfile)
  local lem  = io.open(lemfile,"rb")
  local eocd = zipr.eocd(lem)
  local dir  = zipr.dir(lem,eocd.entries)
  
  for i = 1 , #dir do
    store(dir[i],lemfile)
  end
  lem:close()
end

-- ************************************************************************

table.insert(package.loaders,2,lemloader)

return loadlem
