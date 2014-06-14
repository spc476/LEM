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
-- -----------------------------------------------------------------
--
-- Test the excution of LuaRocks from a LEM file.
--
-- ********************************************************************

lem  = require "lem"

-- *********************************************************************
-- Any module loaded past this point *WILL* come from the supplied LEM file. 
-- No ands, ifs or buts about it.
-- *********************************************************************

package.path  = ""
package.cpath = ""

local luarocks,err = lem("luarocks.lem","luarocks")

if not luarocks then
  io.stderr:write("luarocks: ",err)
  os.exit(1)
end

arg[0] = "luarocks"
luarocks(unpack(arg))
