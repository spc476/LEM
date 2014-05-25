
local idiv  = require "org.conman.math".idiv
local sys   = require "org.conman.sys"
local ddt   = require "org.conman.debug"
local errno = require "org.conman.errno"
local zipr  = require "zipr"
local mz    = require "mz"
local dump  = require "org.conman.table".dump

-- ***********************************************************************

local function toversion(version)
  return string.format("%d.%d",idiv(version,256))
end

-- ***********************************************************************

local function error(...)
  io.stderr:write(string.format(...),"\n")
  os.exit(1)
end

-- ***********************************************************************

local VER  = 5 * 256 + 1
local MODS = {}
local META

do
  local _LEM
  local function store(entry)
    local function add()
      if VER < entry.luamin or VER > entry.luamax then
        return
      end
      
      if MODS[entry.module] then
        if entry.version < MODS[entry.module].version then
          return
        end
      end
      
      MODS[entry.module] = entry
    end
    
    if entry.file == "_LEM" then
      _LEM = entry
      return
    end
    
    if entry.os == 'none' then
      add()
    else
      if entry.os == sys._SYSNAME and entry.cpu == sys._CPU then
        add()
      end      
    end
  end
  
  local lem  = io.open("sample.lem","rb")
  local eocd = zipr.eocd(lem);

  lem:seek('set',eocd.offset)

  for i = 1 , eocd.numentries do
    local dir,err = zipr.dir(lem)
  
    if not dir then error("ERROR %s: %s","sample.lem",errno[err]) end
    store(dir)
  end
  
  lem:seek('set',_LEM.offset)
  local file = zipr.file(lem)
  local com  = lem:read(file.csize)
  local uncom = mz.inflate(com,file.usize,-15)

  local thelem,err = loadstring(uncom)
  if not thelem then error("_LEM: %s",err) end
  META = {}
  setfenv(thelem,META)
  thelem()
end

print(META.NOTES)

