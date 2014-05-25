
local ddt   = require "org.conman.debug"
local cc    = require "org.conman.cc"
local errno = require "org.conman.errno"
local zipr  = require "zipr"
local mz    = require "mz"

local dump = require "org.conman.table".dump





lem = io.open("sample.lem","rb")
eocd = zipr.eocd(lem);

dump("eocd",eocd)

lem:seek('set',eocd.offset)

modules = {}

for i = 1 , eocd.numentries do
  dir,err = zipr.dir(lem)
  modules[dir.module] = dir
end


meta = modules._LEM
dump("LEM",meta)

lem:seek('set',meta.offset)
file = zipr.file(lem)

dump("file",file)

com = lem:read(meta.csize)

print(#com)

uncom =  mz.inflate(com,meta.usize,-15)
print(uncom)

lem:close()
