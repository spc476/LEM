#!/usr/bin/env lua

dump = require "org.conman.table".dump
zlib = require "zlib"
zipw = require "org.conman.zip.write"
fsys = require "org.conman.fsys"
sys  = require "org.conman.sys"

dofile "list.lua"

local DIR     = package.config:sub(1,1)
local SEP     = package.config:sub(3,3)
local MARK    = package.config:sub(5,5)
local VERSION = tonumber(_VERSION:match("^Lua (.*)"))
local SEGMENT = "([^%" .. SEP .. "]*)%" .. SEP .. "*"
local ZIP     = io.open(arg[1] or "sample.lem","wb")
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
  
  if entry.file then
    local info      = fsys.stat(entry.file)
    extra.cpu       = entry.cpu
    extra.os        = entry.os
    extra.osversion = entry.osversion
    return io.open(entry.file,"rb"),info,extra
  end
  
  local file,info = path_module(entry.module,package.cpath)
  if file then
    extra.cpu       = entry.cpu       or sys.CPU
    extra.os        = entry.os        or sys.SYSNAME
    extra.osversion = entry.osversion or sys.RELEASE:match "^([^-]+)%-?"
    return file,info,extra
  end
  
  local file,info = path_module(entry.module,package.path)
  return file,info,extra
end

-- **********************************************************************

local function write_entry(entry)
  local fp
  local info
  local file
  local dir
  local data
  local cmd
  local extra
  
  file = zipw.new('file')
  
  if entry.module then
    fp,info,extra = open_module(entry)
    file.name     = "MODULES/" .. entry.module
  else
    fp        = io.open(entry.file,"rb")  
    info      = fsys.stat(entry.file)
    file.name = "FILES/" .. (entry.name or entry.file)

    if entry.file:match(".*$.lua$") then
      extra = new_extra(entry)
    else
      extra = {}
    end
  end
  
  if not fp then
    error()
    return
  end
  
  data = fp:read("*a")
  cmp  = zlib.compress(data,nil,nil,-15)
  
  file.usize     = info.st_size
  file.csize     = #cmp
  file.crc       = zlib.crc32(0,data)
  file.modtime   = info.st_mtime
  file.extra     = { [0x454C] = extra }
  
  dir            = zipw.new('dir',file)
  dir.comment    = entry.comment or ""
  dir.iattr.text = entry.cpu == nil
  dir.offset     = zipw.file(ZIP,file)
  
  table.insert(CDH,dir)
  ZIP:write(cmp)
  fp:close()
end  

-- **********************************************************************

for _,entry in ipairs(LIST) do
  if type(entry) == 'table' then
    write_entry(entry)
  else
    write_entry { module = entry }
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
