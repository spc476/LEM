
-- *************************************************************************
--
-- Each entry is a table with the following fields (all optional)
--
--	module = "name"
--		The module name as it would appear in a require() statment.
--		If this is set, the resulting module is placed under the
--		"MODULES/" directory.
--
--	file = "/path/to/file"
--		The absolute path to the module file.  This is optional
--		if the module name is specified (and in that case, the
--		file will be located from package.path or package.cpath),
--		otherwise, it's mandatory.
--
--	os = "operatingsystem"
--		Should only be specified if the binary module is being
--		packaged on a different system.  Othersise, this will
--		be set (if needed) to org.conman.sys.SYSNAME .
--
--	osver = "4.3.2"
--		The version of the operating system.  This should only be
--		specified if needed, and it defaults to
--		org.conman.sys.RELEASE .
--
--	cpu = "cpu"
--		Should only be specified if the binary module is being
--		packaged on a different system.  Otherwise, this will
--		be set (if needed) to org.conman.sys.CPU .
--
--	version = "1.0"
--		Version of the module.  
--
--	license = "license"
--		License of the module, for instance, "GPLv3" or "MIT".
--
--	lvmin = "5.1"
--		Defaults to "5.1".  The minimum version of Lua that can
--		run the module.
--
--	lvmax = "5.1"
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

LIST = 
{
  -- ********************************************************
  -- modules for Solaris 64b
  -- ********************************************************
  
  {
    module = "org.conman.base64",
    os     = "SunOS",
    cpu    = "sparcv9",
    file   = "/home/spc/projects/LEM/misc/base64.so",
  },
  {
    module = "org.conman.crc",
    os     = "SunOS",
    cpu    = "sparcv9",
    file   = "/home/spc/projects/LEM/misc/crc.so",
  },
  {
    module  = "org.conman.env",
    os      = "SunOS",
    cpu     = "sparcv9",
    file    = "/home/spc/projects/LEM/misc/env.so",
    version = "5.1",
  },
  {
    module  = "org.conman.errno",
    os      = "SunOS",
    cpu     = "sparcv9",
    file    = "/home/spc/projects/LEM/misc/errno.so",
    version = "5.1",
  },
  {
    module = "org.conman.fsys",
    os     = "SunOS",
    cpu    = "sparcv9",
    file   = "/home/spc/projects/LEM/misc/fsys.so",
  },
  {
    module = "org.conman.hash",
    os     = "SunOS",
    cpu    = "sparcv9",
    file   = "/home/spc/projects/LEM/misc/hash.so",
  },
  {
    module  = "org.conman.iconv",
    os      = "SunOS",
    cpu     = "sparcv9",
    file    = "/home/spc/projects/LEM/misc/iconv.so",
    version = "1.1",
  },
  {
    module  = "org.conman.math",
    os      = "SunOS",
    cpu     = "sparcv9",
    file    = "/home/spc/projects/LEM/misc/math.so",
    version = "5.1",
  },
  {
    module = "org.conman.net",
    os     = "SunOS",
    cpu    = "sparcv9",
    file   = "/home/spc/projects/LEM/misc/net.so",
  },
  {
    module = "org.conman.pollset",
    os     = "SunOS",
    cpu    = "sparcv9",
    file   = "/home/spc/projects/LEM/misc/pollset.so",
  },
  {
    module = "org.conman.process",
    os     = "SunOS",
    cpu    = "sparcv9",
    file   = "/home/spc/projects/LEM/misc/process.so",
  },
  {
    module = "org.conman.strcore",
    os     = "SunOS",
    cpu    = "sparcv9",
    file   = "/home/spc/projects/LEM/misc/strcore.so",
  },
  {
    module  = "org.conman.sys",
    os      = "SunOS",
    cpu     = "sparcv9",
    file    = "/home/spc/projects/LEM/misc/sys.so",
    version = "1.0",
  },
  {
    module  = "org.conman.syslog",
    os      = "SunOS",
    cpu     = "sparcv9",
    file    = "/home/spc/projects/LEM/misc/syslog.so",
    version = "5.1",
  },

  -- ********************************************************
  -- modules for Linux 32b
  -- ********************************************************
  
  "org.conman.base64",
  "org.conman.crc",
  {
    module  = "org.conman.env",
    version = "1.0.0",
  },
  {
    module  = "org.conman.errno",
    version = "1.0.0",
  },
  "org.conman.fsys",
  "org.conman.fsys.magic",
  "org.conman.hash",
  {
    module  = "org.conman.iconv",
    version = "1.1.1",
  },
  {
    module  = "org.conman.math",
    version = "5.1",
  },
  "org.conman.net",
  "org.conman.net.ipacl",
  "org.conman.pollset",
  "org.conman.process",
  "org.conman.strcore",
  {
    module  = "org.conman.sys",
    version = "1.1.1",
  },
  {
    module  = "org.conman.syslog",
    version = "1.0.2",
  },
  "org.conman.tcc",

  -- ********************************************************
  -- Modules written in Lua
  -- ********************************************************
  
  "org.conman.cc",
  "org.conman.date",
  "org.conman.debug",
  "org.conman.dns.resolv",
  "org.conman.getopt",
  "org.conman.string",
  "org.conman.table",
  "org.conman.unix",
  
  -- ********************************************************
  -- Version 0.10.2 of LPeg
  -- ********************************************************

  {
    module  = "lpeg",
    file    = "/usr/local/lib/lua/5.1/lpeg.so",
    version = "0.10",
    license = "MIT",
  },
  {
    module  = "re",
    file    = "/usr/local/share/lua/5.1/re.lua",
    version = "0.10",
    license = "MIT",
  },

  -- ********************************************************
  -- Version 0.12 of LPeg
  -- ********************************************************
  
  {
    module  = "lpeg",
    version = "0.12",
    lvmin   = "5.1",
    lvmax   = "5.1",
    license = "MIT",
  },
  {
    module  = "lpeg",
    version = "0.12",
    lvmin   = "5.2",
    lvmax   = "5.2",
    license = "MIT",
  },
  {
    module  = "re",
    version = "0.12",
    lvmin   = "5.1",
    lvmax   = "5.2",
    license = "MIT",
  },  

  -- ********************************************************
  -- Now, let's include some files
  -- ********************************************************
  
  {
    file = "APPNOTE.TXT",
    comment = "The ZIP file format",
    
  },
  {
    file = "COPYING"
  },
  {
    file = "README",
    comment = "Much Ado About Nothing",
  },
  {
    file = "/home/spc/docs/SPELLS.TXT",
    name = "Miscellaneous Things About Nothing",
    comment = "If you this this has significance, think otherwise",
  }
}

