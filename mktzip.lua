
local fsys  = require "org.conman.fsys"
local errno = require "org.conman.errno"
local zlib  = require "zlib"
local zipw  = require "zipw"

dofile "list.lua"

for i = 1 , #list do
  local info,err = fsys.stat(list[i].file)
  if not info then
    print("ERROR",list[i].file,errno[err])
    return
  end
  
  list[i].usize   = info.st_size
  list[i].modtime = info.st_mtime
  
  if not list[i].luamin then
    list[i].luamin  = "5.1"
    list[i].luamax  = "5.1"
  end
  
  if not list[i].version then
    list[i].version = "0.0"
  end  
end

for _,entry in ipairs(list) do
  local f = io.open(entry.file,"rb")
  local d = f:read("*a")
  f:close()
  
  entry.zip   = zlib.compress(d,9):sub(3,-5)
  entry.crc   = zlib.crc32(0,d)
  entry.csize = #entry.zip
end

f = io.open("sample.lem","wb")
for _,entry in ipairs(list) do
  local err
  entry.offset,err = zipw.file(f,entry)
  if not entry.offset then
    entry.zip = nil
    dump(errno[err],entry)
    os.exit(1)
  end
end

for _,entry in ipairs(list) do
  local err
  entry.coffset,err = zipw.dir(f,entry)
  entry.zip = nil
  if not entry.coffset then
    dump(errno[err],entry)
    os.exit(2)
  end
end

zipw.eocd(
	f,
	#list,
	f:seek() - list[1].coffset,
	list[1].coffset
)
f:close()

local dump = require "org.conman.table".dump
dump("list",list)
