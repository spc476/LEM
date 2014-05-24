
local fsys  = require "org.conman.fsys"
local errno = require "org.conman.errno"
local zlib  = require "zlib"
local zipw  = require "zipw"

dofile "list.lua"
lem = io.open("sample.lem","wb")

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

  local f = io.open(list[i].file,"rb")
  local d = f:read("*a")
  f:close()
  
  list[i].zip   = zlib.compress(d,9):sub(3,-5)
  list[i].crc   = zlib.crc32(0,d)
  list[i].csize = #list[i].zip
  
  local err
  list[i].offset,err = zipw.file(lem,list[i])
  if not list[i].offset then
    list[i].zip = nil
    dump(errno[err],list[i])
    os.exit(1)
  end
  
  list[i].zip = nil
end

for _,entry in ipairs(list) do
  local err
  entry.coffset,err = zipw.dir(lem,entry)
  if not entry.coffset then
    dump(errno[err],entry)
    os.exit(2)
  end
end

zipw.eocd(
	lem,
	#list,
	lem:seek() - list[1].coffset,
	list[1].coffset
)

lem:close()
