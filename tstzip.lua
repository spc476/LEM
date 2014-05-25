
local idiv  = require "org.conman.math".idiv
local sys   = require "org.conman.sys"
local ddt   = require "org.conman.debug"
local errno = require "org.conman.errno"
local zipr  = require "zipr"
local mz    = require "mz"

local dump = require "org.conman.table".dump

local function toversion(version)
  return string.format("%d.%d",idiv(version,256))
end

lem = io.open("sample.lem","rb")
eocd = zipr.eocd(lem);

dump("eocd",eocd)

lem:seek('set',eocd.offset)

modules = {}
seqmod = {}

for i = 1 , eocd.numentries do
  dir,err = zipr.dir(lem)
  modules[dir.module] = dir
  table.insert(seqmod,dir)
end

if false then
  meta = modules._LEM
  dump("LEM",meta)

  lem:seek('set',meta.offset)
  file = zipr.file(lem)

  dump("file",file)

  com = lem:read(meta.csize)

  print(#com)

  uncom =  mz.inflate(com,meta.usize,-15)
  print(uncom)
end

local _META
local lua = {}
local lib = {}

local VER = 5 * 256 + 1 

local function store(entry)
  local function add(list,entry)
    if VER < entry.luamin then
      return
    end
    
    if VER > entry.luamax then
      return
    end
  
    if list[entry.module] then
      if entry.version < list[entry.module].version then
        return
      end
    end
    list[entry.module] = entry
  end
  
  if entry.file == "_LEM" then
    _META = entry
    return
  end
  
  if entry.os == 'none' then
    add(lua,entry)
  else
    if entry.os ~= sys._SYSNAME then
      return
    end
    
    if entry.cpu ~= sys._CPU then
      return
    end
    
    add(lib,entry)
  end
end
    
for _,entry in ipairs(seqmod) do
  store(entry)
end

lem:seek('set',_META.offset)
file = zipr.file(lem)
com = lem:read(_META.csize)
uncom = mz.inflate(com,file.usize,-15)
print(uncom)

for name,meta in pairs(lua) do
  print("","",toversion(meta.version),name)
end

for name,meta in pairs(lib) do
  print(meta.os or "",meta.cpu or "",toversion(meta.version),name)
end

lem:close()
