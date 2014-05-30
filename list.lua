
-- *************************************************************************
--
-- Each entry is a table with the following fields:
--
--	module = "name"		-- MANDATORY
--		The module name as it would appear in a require() statment.
--
--	file = "/path/to/file"	-- MANDATORY
--		The absolute path to the module file.
--
--	os = "operatingsystem"	-- MANDATORY
--		Should be "none" if a Lua file.  Otherwise, it needs
--		to be the value of org.conman.sys._SYSNAME
--
--	cpu = "cpu"		-- MANDATORY
--		Should be "none" if a Lua file.  Otherwise, it needs
--		to be the value of org.conman.sys._CPU
--
--	version = "1.0"		-- optional
--		Defaults to "0.0" (i.e. not available).  Max version
--		number suported is 255.255.
--
--	license = "license"	-- optinal
--		Defaults to "LGPL3+" (LGPL v3 or higher).  Currently,
--		it can be that, or "MIT".  Others will be added as needed.
--
--	lvmin = "5.1"		-- optional
--		Defaults to "5.1".  The minimum version of Lua that can
--		run the module.
--
--	lvmax = "5.1"		-- optional
--		Defaults to "5.1".  The maximum version of Lua that can
--		run the module.  A Lua module that can only run on one
--		version of Lua should set the lvmin and lvmax to the same
--		value.
--
-- The following list is an example.  It is expected for you to modify
-- the list as it suits your needs.
--
-- This file is used by the mktzip.lua script to generate the sample.lem
-- file.
-- *************************************************************************

list = 
{
  -- ********************************************************
  -- modules for Solaris 64b
  -- ********************************************************
  
  {
    module = "org.conman.base64",
    os = "SunOS",
    cpu = "sparcv9",
    file = "/home/spc/projects/LEM/misc/base64.so",
  },
  {
    module = "org.conman.crc",
    os = "SunOS",
    cpu = "sparcv9",
    file = "/home/spc/projects/LEM/misc/crc.so",
  },
  {
    module = "org.conman.env",
    os = "SunOS",
    cpu = "sparcv9",
    file = "/home/spc/projects/LEM/misc/env.so",
    version = "5.1",
  },
  {
    module = "org.conman.errno",
    os = "SunOS",
    cpu = "sparcv9",
    file = "/home/spc/projects/LEM/misc/errno.so",
    version = "5.1",
  },
  {
    module = "org.conman.fsys",
    os = "SunOS",
    cpu = "sparcv9",
    file = "/home/spc/projects/LEM/misc/fsys.so",
  },
  {
    module = "org.conman.hash",
    os = "SunOS",
    cpu = "sparcv9",
    file = "/home/spc/projects/LEM/misc/hash.so",
  },
  {
    module = "org.conman.iconv",
    os = "SunOS",
    cpu = "sparcv9",
    file = "/home/spc/projects/LEM/misc/iconv.so",
    version = "1.1",
  },
  {
    module = "org.conman.math",
    os = "SunOS",
    cpu = "sparcv9",
    file = "/home/spc/projects/LEM/misc/math.so",
    version = "5.1",
  },
  {
    module = "org.conman.net",
    os = "SunOS",
    cpu = "sparcv9",
    file = "/home/spc/projects/LEM/misc/net.so",
  },
  {
    module = "org.conman.pollset",
    os = "SunOS",
    cpu = "sparcv9",
    file = "/home/spc/projects/LEM/misc/pollset.so",
  },
  {
    module = "org.conman.process",
    os = "SunOS",
    cpu = "sparcv9",
    file = "/home/spc/projects/LEM/misc/process.so",
  },
  {
    module = "org.conman.strcore",
    os = "SunOS",
    cpu = "sparcv9",
    file = "/home/spc/projects/LEM/misc/strcore.so",
  },
  {
    module = "org.conman.sys",
    os = "SunOS",
    cpu = "sparcv9",
    file = "/home/spc/projects/LEM/misc/sys.so",
  },
  {
    module = "org.conman.syslog",
    os = "SunOS",
    cpu = "sparcv9",
    file = "/home/spc/projects/LEM/misc/syslog.so",
    version = "5.1",
  },

  -- ********************************************************
  -- modules for Linux 32b
  -- ********************************************************
  
  {
    module = "org.conman.base64",
    os = "Linux",
    cpu = "x86",
    file = "/usr/local/lib/lua/5.1/org/conman/base64.so",
  },
  {
    module = "org.conman.crc",
    os = "Linux",
    cpu = "x86",
    file = "/usr/local/lib/lua/5.1/org/conman/crc.so",
  },
  {
    module = "org.conman.env",
    os = "Linux",
    cpu = "x86",
    file = "/usr/local/lib/lua/5.1/org/conman/env.so",
    version = "5.1",
  },
  {
    module = "org.conman.errno",
    os = "Linux",
    cpu = "x86",
    file = "/usr/local/lib/lua/5.1/org/conman/errno.so",
    version = "5.1",
  },
  {
    module = "org.conman.fsys",
    os = "Linux",
    cpu = "x86",
    file = "/usr/local/lib/lua/5.1/org/conman/fsys.so",
  },
  {
    module = "org.conman.fsys.magic",
    os = "Linux",
    cpu = "x86",
    file = "/usr/local/lib/lua/5.1/org/conman/fsys/magic.so",
  },
  {
    module = "org.conman.hash",
    os = "Linux",
    cpu = "x86",
    file = "/usr/local/lib/lua/5.1/org/conman/hash.so",
  },
  {
    module = "org.conman.iconv",
    os = "Linux",
    cpu = "x86",
    file = "/usr/local/lib/lua/5.1/org/conman/iconv.so",
    version = "1.1",
  },
  {
    module = "org.conman.math",
    os = "Linux",
    cpu = "x86",
    file = "/usr/local/lib/lua/5.1/org/conman/math.so",
    version = "5.1",
  },
  {
    module = "org.conman.net",
    os = "Linux",
    cpu = "x86",
    file = "/usr/local/lib/lua/5.1/org/conman/net.so",
  },
  {
    module = "org.conman.net.ipacl",
    os = "Linux",
    cpu = "x86",
    file = "/usr/local/lib/lua/5.1/org/conman/net/ipacl.so",
  },
  {
    module = "org.conman.pollset",
    os = "Linux",
    cpu = "x86",
    file = "/usr/local/lib/lua/5.1/org/conman/pollset.so",
  },
  {
    module = "org.conman.process",
    os = "Linux",
    cpu = "x86",
    file = "/usr/local/lib/lua/5.1/org/conman/process.so",
  },
  {
    module = "org.conman.strcore",
    os = "Linux",
    cpu = "x86",
    file = "/usr/local/lib/lua/5.1/org/conman/strcore.so",
  },
  {
    module = "org.conman.sys",
    os = "Linux",
    cpu = "x86",
    file = "/usr/local/lib/lua/5.1/org/conman/sys.so",
  },
  {
    module = "org.conman.syslog",
    os = "Linux",
    cpu = "x86",
    file = "/usr/local/lib/lua/5.1/org/conman/syslog.so",
    version = "5.1",
  },
  {
    module = "org.conman.tcc",
    os = "Linux",
    cpu = "x86",
    file = "/usr/local/lib/lua/5.1/org/conman/tcc.so",
  },

  -- ********************************************************
  -- Modules written in Lua
  -- ********************************************************
  
  {
    module = "org.conman.cc",
    os = "none",
    cpu = "none",
    file = "/usr/local/share/lua/5.1/org/conman/cc.lua",
  },
  {
    module = "org.conman.date",
    os = "none",
    cpu = "none",
    file = "/usr/local/share/lua/5.1/org/conman/date.lua",
  },
  {
    module = "org.conman.debug",
    os = "none",
    cpu = "none",
    file = "/usr/local/share/lua/5.1/org/conman/debug.lua",
  },
  {
    module = "org.conman.dns.resolv",
    os = "none",
    cpu = "none",
    file = "/usr/local/share/lua/5.1/org/conman/dns/resolv.lua",
  },
  {
    module = "org.conman.getopt",
    os = "none",
    cpu = "none",
    file = "/usr/local/share/lua/5.1/org/conman/getopt.lua",
  },
  {
    module = "org.conman.string",
    os = "none",
    cpu = "none",
    file = "/usr/local/share/lua/5.1/org/conman/string.lua",
  },
  {
    module = "org.conman.table",
    os = "none",
    cpu = "none",
    file = "/usr/local/share/lua/5.1/org/conman/table.lua",
  },
  {
    module = "org.conman.unix",
    os = "none",
    cpu = "none",
    file = "/usr/local/share/lua/5.1/org/conman/unix.lua",
  },
  
  -- ********************************************************
  -- Version 0.10.2 of LPeg
  -- ********************************************************

  {
    module = "lpeg",
    os = "Linux",
    cpu = "x86",
    file = "/usr/local/lib/lua/5.1/lpeg.so",
    version = "0.10",
    license = "MIT",
  },
  {
    module = "re",
    os = "none",
    cpu = "none",
    file = "/usr/local/share/lua/5.1/re.lua",
    version = "0.10",
    license = "MIT",
  },

  -- ********************************************************
  -- Version 0.12 of LPeg
  -- ********************************************************
  
  {
    module = "lpeg",
    os = "Linux",
    cpu = "x86",
    file = "/home/spc/.luarocks/lib/lua/5.1/lpeg.so",
    version = "0.12",
    lvmin  = "5.1",
    lvmax  = "5.1",
    license = "MIT",
  },
  {
    module = "lpeg",
    os = "Linux",
    cpu = "x86",
    file = "/home/spc/.luarocks/lib/lua/5.1/lpeg.so",
    version = "0.12",
    lvmin  = "5.2",
    lvmax  = "5.2",
    license = "MIT",
  },
  {
    module = "re",
    os = "none",
    cpu = "none",
    file = "/home/spc/.luarocks/share/lua/5.1/re.lua",
    version = "0.12",
    lvmin  = "5.1",
    lvmax  = "5.2",
    license = "MIT",
  },  

  -- ********************************************************
  -- Now, let's include some files
  -- ********************************************************
  
  {
    file = "APPNOTE.TXT"
  },
  {
    file = "COPYING"
  },
  {
    file = "README"
  },
}

