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
-- -------------------------------------------------------------------
--
-- Create a sample LEM file.
--
-- ********************************************************************

if #arg ~= 2 then
  io.stderr:write("usage: ",arg[0]," listfile [lemfile]\n")
  os.exit(1)
end

dump = require "org.conman.table".dump
zlib = require "zlib"
zipw = require "org.conman.zip.write"
fsys = require "org.conman.fsys"
sys  = require "org.conman.sys"

LIST = { sys = sys }
do
  local file = arg[1]
  local f    = loadfile(file)
  setfenv(f,LIST)
  f()
  
  if not LIST.SCRIPTS then
    LIST.SCRIPTS = {}
  end
  
  if not LIST.FILES then
    LIST.FILES = {}
  end  
  
  if not LIST.APP then
    LIST.APP = {}
  end
end

local DIR     = package.config:sub(1,1)
local SEP     = package.config:sub(3,3)
local MARK    = package.config:sub(5,5)
local VERSION = tonumber(_VERSION:match("^Lua (.*)"))
local SEGMENT = "([^%" .. SEP .. "]*)%" .. SEP .. "*"
local ZIP     = io.open(arg[2] or "sample.lem","wb")
local CDH     = {}

-- ***********************************************************************

local function path_module(module,luapath)
  local path = module:gsub("%.",DIR)
  for segment in luapath:gmatch(SEGMENT) do
    local file = segment:gsub(MARK,path)
    local info = fsys.stat(file)
    local f    = io.open(file,"rb")
    if f then
      return f,info
    end
  end
end

-- **********************************************************************

local function new_extra(entry)
  return {
  	language = entry.langauge or "Lua",
  	lvmin    = entry.lvmin    or _VERSION:match("^Lua (.*)"),
  	lvmax    = entry.lvmax    or _VERSION:match("^Lua (.*)"),
  	license  = entry.license  or "LGPL3+",
  	version  = entry.version,
  }
end

-- **********************************************************************

local function open_module(entry)
  local extra = new_extra(entry)
  local file
  
  if entry.contents then
    local info = { st_size = #entry.contents , st_mtime = os.time() }
    return nil,info,extra
  end
  
  if entry.file then
    local info  = fsys.stat(entry.file)
    extra.cpu   = entry.cpu
    extra.os    = entry.os
    extra.osver = entry.osversion
    return io.open(entry.file,"rb"),info,extra
  end
  
  local file,info = path_module(entry.name,package.cpath)
  if file then
    extra.cpu   = entry.cpu       or sys.CPU
    extra.os    = entry.os        or sys.SYSNAME
    extra.osver = entry.osversion or sys.RELEASE:match "^([^-]+)%-?"
    return file,info,extra
  end
  
  local file,info = path_module(entry.name,package.path)
  return file,info,extra
end

-- **********************************************************************

local function write_entry(entry,section,fp,info,extra)
  local file = zipw.new('file')
  file.name  = section .. "/" .. (entry.name or entry.file)
  
  local data
  
  if fp then
    data = fp:read("*a")
    fp:close()
  else
    data = entry.contents
  end
  
  local cmp = zlib.compress(data,nil,nil,-15)
  
  file.usize     = info.st_size
  file.csize     = #cmp
  file.crc       = zlib.crc32(0,data)
  file.modtime   = info.st_mtime
  file.extra     = { [0x454C] = extra }
  
  local dir      = zipw.new('dir',file)
  dir.comment    = entry.comment or ""
  dir.iattr.text = entry.cpu == nil
  dir.offset     = zipw.file(ZIP,file)
  
  table.insert(CDH,dir)
  ZIP:write(cmp)
end  

-- **********************************************************************

for _,mod in ipairs(LIST.MODULES) do
  if type(mod) == 'string' then
    mod = { name = mod }
  end
  
  local fp,info,extra = open_module(mod)
  write_entry(mod,"MODULES",fp,info,extra)
end

for _,list in ipairs { "APP" , "SCRIPTS" , "FILES" } do
  for _,file in ipairs(LIST[list]) do
    local fp
    local info
    
    if file.file then
      fp   = io.open(file.file)
      info = fsys.stat(file.file)
    else
      info = { st_size = #file.contents , st_mtime = os.time() }
    end
    
    local extra    = {}
    extra.version  = file.version
    extra.license  = file.license
    extra.language = file.language
    extra.lvmin    = file.lvmin
    extra.lvmax    = file.lvmax
    extra.cpu      = file.cpu
    extra.os       = file.os
    extra.osver    = file.osver
    write_entry(file,list,fp,info,extra)
  end
end
  
local offset,size = zipw.dir(ZIP,CDH)
local eocd        = zipw.new('eocd')
eocd.entries      = #CDH
eocd.totalentries = #CDH
eocd.offset       = offset
eocd.size         = size
eocd.comment      = "The Eagle Has Landed"

zipw.eocd(ZIP,eocd)
ZIP:close()
