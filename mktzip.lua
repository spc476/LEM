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

  table.insert(list,1, {
	module  = "_LEM",
	os      = "none",
	cpu     = "_LEM",
	version = "0.1",
	luamin  = "5.1",
	luamax  = "5.1",
	crc     = crc,
	csize   = #com,
	usize   = #LEM,
	modtime = os.time(),
	license = "none",
  })

  list[1].offset,err = zipw.file(lem,list[1],true)

  if not list[1].offset then
    dump(errno[err],list[1])
    os.exit(1)
  end

  lem:write(com)
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
  
  com           = table.concat(com)
  list[i].crc   = crc
  list[i].csize = #com
  
  local err
  list[i].offset,err = zipw.file(lem,list[i],true)
  if not list[i].offset then
    list[i].zip = nil
    dump(errno[err],list[i])
    os.exit(1)
  end

  lem:write(com)
  
end

for _,entry in ipairs(list) do
  local err
  entry.coffset,err = zipw.dir(lem,entry,true)
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
