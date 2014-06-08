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
-- Displays contents of a LEM file.
--
-- ********************************************************************

local errno = require "org.conman.errno"
local zipr  = require "org.conman.zip.read"

-- ***********************************************************************

local LEM  = arg[1] or "sample.lem"
local lem  = io.open(LEM,"rb")
local eocd = zipr.eocd(lem);

if eocd.comment then print(eocd.comment) end

local list = zipr.dir(lem,eocd.entries)

for i = 1 , #list do
  if list[i].extra[0x454C] then
    lua = list[i].extra[0x454C]
    
    io.stdout:write(string.format(
      "%-9s %-9s %-9s %6s %-3s %-3s %-6s %s\n",
      lua.os       or "",
      lua.cpu      or "",
      lua.license  or "",
      lua.language or "",
      lua.lvmin    or "",
      lua.lvmax    or "",
      lua.version  or "",
      list[i].name
    )) 
  else
    io.stdout:write(string.format(
    	"%52s%s\n",
    	"",
    	list[i].name
    ))
  end  
end
