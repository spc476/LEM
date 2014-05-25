
local idiv  = require "org.conman.math".idiv
local sys   = require "org.conman.sys"
local ddt   = require "org.conman.debug"
local errno = require "org.conman.errno"
local zipr  = require "zipr"
local mz    = require "mz"
local dump  = require "org.conman.table".dump


-- *********************************************************************
-- Any module loaded past this point *WILL* come from the supplied LEM file. 
-- No ands, ifs or buts about it.
-- *********************************************************************

package.path  = ""
package.cpath = ""

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

local function read_data(entry,lem)
  lem:seek('set',entry.offset)
  local file = zipr.file(lem)
  local com  = lem:read(file.csize)
  return mz.inflate(com,file.usize,-15)
end

-- ***********************************************************************

local VER  = 5 * 256 + 1
local MODS = {}
local META
local lem

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
  
  lem  = io.open("sample.lem","rb")
  local eocd = zipr.eocd(lem);

  lem:seek('set',eocd.offset)

  for i = 1 , eocd.numentries do
    local dir,err = zipr.dir(lem)
  
    if not dir then error("ERROR %s: %s","sample.lem",errno[err]) end
    store(dir)
  end

  local thelem,err = loadstring(read_data(_LEM,lem))
  if not thelem then error("_LEM: %s",err) end
  META = {}
  setfenv(thelem,META)
  thelem()
end

-- ***********************************************************************

local function zip_loader(name)
  local data
  local function feed()
    local d = data
    data = nil
    return d
  end
  
  if not MODS[name] then return string.format("\n\tno file %s",name) end

  if MODS[name].os == 'none' then
    data = read_data(MODS[name],lem)
    return load(feed,name)
  end
  
  return "\n\tshared object files not supported"
end

-- ***********************************************************************

print(META.NOTES)

table.insert(package.loaders,2,zip_loader)

date = require "org.conman.date"

print(date.tojulianday())
print()
print("PATH",package.path)
print("CPATH",package.cpath)
print()

unix = require "org.conman.unix"
