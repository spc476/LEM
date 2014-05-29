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
-- ------------------------------------------------------------------
--
-- Loads and tests a few modules from the sample.lem file.
--
-- ********************************************************************

local errno = require "org.conman.errno"
local idiv  = require "org.conman.math".idiv
local zipr  = require "zipr"
local mz    = require "mz"

-- ***********************************************************************

local function luaversion(min,max)
  if min > 0 then
    if min == max then
      return string.format("Lua %d.%d",idiv(min,256))
    else
      minq,minr = idiv(min,256)
      maxq,maxr = idiv(max,256)
      return string.format("Lua %d.%d-%d.%d",minq,minr,maxq,maxr)
    end
  else
    return ""
  end
end

-- ***********************************************************************

local function version(ver)
  if ver > 0 then
    return string.format("%d.%d",idiv(ver,256))
  else
    return ""
  end
end

-- ***********************************************************************

local LEM  = arg[1] or "sample.lem"
local lem  = io.open(LEM,"rb")
local eocd = zipr.eocd(lem);

lem:seek('set',eocd.offset)

for i = 1 , eocd.numentries do
  local dir,err = zipr.dir(lem)
  if not dir then error("%s: %s",LEM,errno[err]) end
  if dir.extra then
    luaversion(dir.extra.luamin,dir.extra.luamax)
    io.stdout:write(string.format(
    	"%-9s %-9s %-9s %-12s %-8s %s\n",
    	dir.extra.os,
    	dir.extra.cpu,
    	dir.extra.license,
    	luaversion(dir.extra.luamin,dir.extra.luamax),
    	version(dir.extra.version),
    	dir.name
    ))
  else
    io.stdout:write(string.format(
     	"%52s%s\n",
     	"",
     	dir.name
    ))
  end
end
