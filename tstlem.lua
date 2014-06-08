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

lem = require "lem"

-- *********************************************************************
-- Any module loaded past this point *WILL* come from the supplied LEM file. 
-- No ands, ifs or buts about it.
-- *********************************************************************

package.path  = ""
package.cpath = ""

print("PATH",package.path)
print("CPATH",package.cpath)
print()

lem("sample.lem")

date = require "org.conman.date"
print(date.tojulianday())
print()

lpeg = require "lpeg"
print("LPeg",lpeg.version())
print()

unix = require "org.conman.unix"
dump = require "org.conman.table".dump
dump("USER",unix.users[os.getenv("USER")])
