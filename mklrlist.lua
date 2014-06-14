
if #arg ~= 3 then
  io.stderr:write("usage: ",arg[0]," rockspec srcdir output\n")
  os.exit(1)
end

local errno = require "org.conman.errno"
local fsys  = require "org.conman.fsys"
local dump  = require "org.conman.table".dump_value

MODULES = {} 
SCRIPTS = {} 
APP     = {}

do
  ROCK    = {}
  local f = loadfile(arg[1])
  setfenv(f,ROCK)
  f()
  
  DIR     = arg[2]
  OUT     = io.open(arg[3],"w")
  VERSION = ROCK.version:match "^([^-]+)%-?"
end

for name,file in pairs(ROCK.build.modules) do
  table.insert(MODULES,{
  	name     = name,
  	file     = DIR .. "/" .. file,
  	version  = VERSION,
  	language = "Lua",
  	lvmin    = "5.1",
  	lvmax    = "5.2",
  	license  = ROCK.description.license,
  })
end

for name,file in pairs(ROCK.build.install.bin) do
  table.insert(APP,{
  	name     = name,
  	file     = DIR .. "/" .. file,
  	version  = VERSION,
  	language = "Lua",
  	lvmin    = "5.1",
  	lvmax    = "5.2",
  	license  = ROCK.description.license,
  })
end

table.insert(MODULES,{
	name     = "luarocks.site_config",
	version  = VERSION,
	license  = ROCK.description.license,
	language = "Lua",
	lvmin    = "5.1",
	lvmax    = "5.2",
	contents = [[
module("luarocks.site_config")
LUAROCKS_PREFIX="/usr/local"
LUA_INCDIR="/usr/local/include"
LUA_LIBDIR="/usr/local/lib"
LUA_BINDIR="/usr/local/bin"
LUAROCKS_SYSCONFDIR="."
LUAROCKS_ROCKS_TREE="/usr/local"
LUAROCKS_UNAME_S="Linux"
LUAROCKS_UNAME_M="i686"
LUAROCKS_DOWNLOADER="wget"
LUAROCKS_MD5CHECKER="md5sum"
]]
})

table.insert(SCRIPTS,{
	name     = "config-5.1.lua",
	language = "Lua",
	lvmin    = "5.1",
	lvmax    = "5.1",
	contents = [[
rocks_trees = { 
   home.."/.luarocks",
   "/usr/local"
}
]]
})

table.insert(SCRIPTS,{
	name     = "config.lua",
	language = "Lua",
	lvmin    = "5.1",
	lvmax    = "5.2",
	contents = [[
rocks_servers = {
  "http://luarocks.org/repositories/rocks"
}
rocks_trees = {
   home.."/.luarocks",
   "/usr/local"
}
]]
})

table.sort(MODULES,function(a,b)
  return a.name < b.name
end)


OUT:write(dump("MODULES",MODULES))
OUT:write(dump("SCRIPTS",SCRIPTS))
OUT:write(dump("APP",APP))
OUT:close()
