
#ifndef LEM_H
#define LEM_H

#include <stdint.h>

#define ZIP_MAGIC_EOCD	0x06054B50
#define ZIP_MAGIC_CFILE	0x02014B50
#define ZIP_MAGIC_FILE  0x04034B50

#define ZIPE_META_LEM	65535u

typedef enum
{
  ZIPE_CPU_NONE,
  ZIPE_CPU_ALPHA,
  ZIPE_CPU_ARM,
  ZIPE_CPU_AVR32,
  ZIPE_CPU_ETRAX,
  ZIPE_CPU_C6X,
  ZIPE_CPU_H8,
  ZIPE_CPU_HPPA,
  ZIPE_CPU_IBM390,
  ZIPE_CPU_IBMZ,
  ZIPE_CPU_ITANIUM,
  ZIPE_CPU_68000,
  ZIPE_CPU_68010,
  ZIPE_CPU_68020,
  ZIPE_CPU_68030,
  ZIPE_CPU_68040,
  ZIPE_CPU_M32R,
  ZIPE_CPU_MICROBLAZE,
  ZIPE_CPU_MIPS,
  ZIPE_CPU_PPC,
  ZIPE_CPU_SH3EB,
  ZIPE_CPU_SH3EI,
  ZIPE_CPU_SPARC,
  ZIPE_CPU_SPARC64,
  ZIPE_CPU_VAX,
  ZIPE_CPU_i386,
  ZIPE_CPU_i486,
  ZIPE_CPU_x86,
  ZIPE_CPU_x86_64,
} zipe_cpu__e;

typedef enum
{
  ZIPE_OS_NONE,
  ZIPE_OS_AIX,
  ZIPE_OS_BSD,
  ZIPE_OS_FREEBSD,
  ZIPE_OS_LINUX,
  ZIPE_OS_MACOSX,
  ZIPE_OS_MINGW,
  ZIPE_OS_SOLARIS
} zipe_os__e;

typedef enum
{
  ZIPE_LIC_UNKNOWN,
  ZIPE_LIC_NONE,	/* possible?		*/
  ZIPE_LIC_PUBLIC_DOMAIN,
  
  ZIPE_LIC_GPL1,	/* GPL  v1 or higher	*/
  ZIPE_LIC_GPL1e,	/* GPL  v1 only 	*/
  ZIPE_LIC_FDL1,	/* FDL  v1 or higher	*/
  ZIPE_LIC_FDL1e,	/* FDL  v1 only		*/
  ZIPE_LIC_GPL2,	/* GPL  v2 or higher	*/
  ZIPE_LIC_GPL2e,	/* GPL  v2 only		*/
  ZIPE_LIC_LGPL2,	/* LGPL v2.1 or higher	*/
  ZIPE_LIC_LGPL2e,	/* LGPL v2.1 only	*/
  ZIPE_LIC_FDL2,	/* FDL  v2 or higher	*/
  ZIPE_LIC_FDL2e,	/* FDL  v2 only		*/
  ZIPE_LIC_GPL3,	/* GPL  v3 or higher	*/
  ZIPE_LIC_GPL3e,	/* GPL  v3 only		*/
  ZIPE_LIC_LGPL3,	/* LGPL v3.1 or higher	*/
  ZIPE_LIC_LGPL3e,	/* LGPL v3.1 only	*/
  ZIPE_LIC_FDL3,	/* FDL  v3 or higher	*/
  ZIPE_LIC_FDL3e,	/* FDL  v3 only		*/
  ZIPE_LIC_AGPL3,	/* AGPL v3 or higher	*/
  ZIPE_LIC_AGPL3e,	/* AGPL v3 only		*/
  
  ZIPE_LIC_MIT,		/* MIT license		*/
} zipe_license__e;

typedef struct
{
  uint32_t magic;	/* 0x06054b50 */
  uint16_t disknum;
  uint16_t diskstart;
  uint16_t entries;
  uint16_t totalentries;
  uint32_t size;
  uint32_t offset;
  uint16_t commentlen;
  uint8_t  data[];
} __attribute__((packed)) zip_eocd__s;

typedef struct
{
  uint32_t magic;	/* 0x02014b50 */
  uint16_t byversion;
  uint16_t forversion;
  uint16_t flags;
  uint16_t compression;
  uint16_t modtime;
  uint16_t moddate;
  uint32_t crc;
  uint32_t csize;
  uint32_t usize;
  uint16_t namelen;
  uint16_t extralen;
  uint16_t commentlen;
  uint16_t diskstart;
  uint16_t iattr;
  uint32_t eattr;
  uint32_t offset;
  uint8_t  data[];
} __attribute__((packed)) zip_dir__s;

typedef struct
{
  uint32_t magic;	/* 0x04034b50 */
  uint16_t byversion;
  uint16_t flags;
  uint16_t compression;
  uint16_t modtime;
  uint16_t moddate;
  uint32_t crc;
  uint32_t csize;
  uint32_t usize;
  uint16_t namelen;
  uint16_t extralen;
  uint8_t  data[];
} __attribute__((packed)) zip_file__s;

#define ZIPIA_TEXT	(1 << 0)
#define ZIP_EXT_LUA	0x454C

typedef struct
{
  uint16_t id;		/* 0x4C45 */
  uint16_t size;
  uint16_t luavmin;	/* Lua minimum version support */
  uint16_t luavmax;	/* Lua maximum version support */
  uint16_t os;
  uint16_t cpu;
  uint16_t version;	/* Version */
  uint16_t license;	/* License */
} __attribute__((packed)) zip_lua_ext__s;

typedef struct
{
  uint16_t modtime;
  uint16_t moddate;
} modtime__s;

#endif
