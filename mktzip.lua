
local fsys  = require "org.conman.fsys"
local errno = require "org.conman.errno"
local zipw  = require "zipw"
local mz    = require "mz"

-- ************************************************************************
-- The excessive use of '=' in the next line is to work around a bug in my
-- preferred editor.  --sean@conman.org
-- ************************************************************************

LEM = [=====[
NOTES   = [[
A sample LEM file.  This isn't an application per se; it's more of a
"proof-of-concept" and to show off some features of the file spec.
  	]]
X_OTHER_DATA = true
]=====]

dofile "list.lua"
lem = io.open("sample.lem","wb")

do
  local com   = mz.deflate(LEM)
  local meta  = com
  local crc   = mz.crc(LEM)
  local err

  table.insert(list,1, {
	module  = "_LEM",
	os      = "none",
	cpu     = "_LEM",
	version = "0.1",
	luamin  = "5.1",
	luamax  = "5.1",
	crc     = crc,
	csize   = #meta,
	usize   = #LEM,
	modtime = os.time(),
	license = "none",
	zip     = meta
  })

  list[1].offset,err = zipw.file(lem,list[1])
  if not list[1].offset then
    dump(errno[err],list[1])
    os.exit(1)
  end
end

for i = 2 , #list do
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

  if not list[i].license then
    list[i].license = "LGPL3+"
  end
  
  local f = io.open(list[i].file,"rb")
  local d = f:read("*a")
  f:close()
  
  local com     = mz.deflate(d)
  list[i].zip   = com
  list[i].crc   = mz.crc(d)
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
