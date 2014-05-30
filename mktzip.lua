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
-- Makes a sample LEM file.  Yes, this is ugly code.
--
-- ********************************************************************

local dump  = require "org.conman.table".dump
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
masterlist = {}

do
  local com = {}
  local x = LEM

  local rc,err = mz.deflate(
  	function()
  	  local s = x
  	  x = nil
  	  return s
  	end,
  	function(s)
  	  com[#com + 1] = s
  	end
  )
  
  com       = table.concat(com)
  local crc = mz.crc(0,LEM)
  local err

  table.insert(masterlist,{
  	modtime = os.time(),
  	crc     = crc,
  	csize   = #com,
  	usize   = #LEM,
  	name    = "_LEM",
  	extra   =
  	{
  	  cpu = "_LEM",
  	},
  	
  	text    = true,
  	comment = "Lovingly made by hand",  	
  })

  masterlist[1].offset,err = zipw.file(lem,masterlist[1])

  if not masterlist[1].offset then
    dump(errno[err],masterlist[1])
    os.exit(1)
  end

  lem:write(com)
end

for i = 1 , #list do
  local entry = { }
  local info,err = fsys.stat(list[i].file)
  if not info then
    print("ERROR",list[i].file,errno[err])
    return
  end
  
  entry.usize   = info.st_size
  entry.modtime = info.st_mtime
  
  if list[i].module then
    entry.name = "_MODULES/" .. list[i].module
    entry.extra = 
    {
      lvmin  = list[i].lvmin,
      lvmax  = list[i].lvmax,
      version = list[i].version,
      license = list[i].license or "LGPL3+",
      os      = list[i].os,
      cpu     = list[i].cpu,
    }
    entry.text = entry.extra.os == 'none'
  else
    entry.name = "_FILES/" .. list[i].file
    entry.text = true
  end  
  
  local f = io.open(list[i].file,"rb")
  local com = {}
  local crc = 0
  local rc,err = mz.deflate(
  	function()
  	  local d = f:read(8192)
  	  if d then
  	    crc = mz.crc(crc,d)
  	  end
  	  return d
  	end,
  	function(s)
  	  com[#com + 1] = s
  	end
  )
  f:close()
  
  com         = table.concat(com)
  entry.crc   = crc
  entry.csize = #com
  
  local err
  entry.offset,err = zipw.file(lem,entry)
  if not entry.offset then
    dump(errno[err],entry)
    os.exit(1)
  end

  lem:write(com)
  table.insert(masterlist,entry)
  
end

for _,entry in ipairs(masterlist) do
  local err
  entry.coffset,err = zipw.dir(lem,entry)
  if not entry.coffset then
    dump(errno[err],entry)
    os.exit(2)
  end
end

zipw.eocd(lem,{
	entries = #masterlist,
	size    = lem:seek() - masterlist[1].coffset,
	offset  = masterlist[1].coffset
})

lem:close()
