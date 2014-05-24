
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

/***********************************************************************/

#define ZIP_MAGIC_EOCD	0x06054B50
#define ZIP_MAGIC_CFILE	0x02014B50
#define ZIP_MAGIC_FILE  0x04034B50

typedef enum
{
  ZIPOS_MSDOS,
  ZIPOS_AMIGA,
  ZIPOS_OPENVMS,
  ZIPOS_UNIX,
  ZIPOS_VM_CMS,
  ZIPOS_ATARI,
  ZIPOS_OS2,
  ZIPOS_MAC,
  ZIPOS_ZSYSTEM,
  ZIPOS_CPM,
  ZIPOS_WINDOW,
  ZIPOS_MVS,
  ZIPOS_VSE,
  ZIPOS_ACORN,
  ZIPOS_VFAT,
  ZIPOS_ALTMVS,
  ZIPOS_BEOS,
  ZIPOS_TANDEM,
  ZIPOS_OS400,
  ZIPOS_OSX,
} zipfile_os__e;

typedef enum
{
  ZIPCOMP_NONE,
  ZIPCOMP_SHRUNK,
  ZIPCOMP_REDUCED1,
  ZIPCOMP_REDUCED2,
  ZIPCOMP_REDUCED3,
  ZIPCOMP_REDUCED4,
  ZIPCOMP_IMPLODED,
  ZIPCOMP_TOKEN,
  ZIPCOMP_DEFLATED,	/* zlib */
  ZIPCOMP_DEFLATE64,
  ZIPCOMP_PKWARE,
  ZIPCOMP_rsvp,
  ZIPCOMP_BZIP2,
  ZIPCOMP_rsvp2,
  ZIPCOMP_LZMA,
  ZIPCOMP_rsvp3,
  ZIPCOMP_rsvp4,
  ZIPCOMP_rsvp5,
  ZIPCOMP_IBM,
  ZIPCOMP_LZ77,
  ZIPCOMP_WAVPACK = 97,
  ZIPCOMP_PPMd = 98,
} zipfile_comp__e; 

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

typedef struct
{
  struct
  {
    zipfile_os__e os;
    int           major;
    int           minor;
  } by;
  
  struct
  {
    zipfile_os__e os;
    int           major;
    int           minor;
  } need;
  
} zipfile_version__s;

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

#define ZIPF_DATADESCR	(1 <<  3)
#define ZIPF_UTF8	(1 << 11)

#define ZIPIA_TEXT	(1 << 0)

typedef struct
{
  uint16_t id;		/* 0x4C45 */
  uint16_t size;
  uint16_t luavmin;	/* Lua minimum version support */
  uint16_t luavmax;	/* Lua maximum version support */
  uint16_t os;
  uint16_t cpu;
  uint16_t version;	/* Version */
} __attribute__((packed)) zip_lua_ext__s;

typedef union
{
  uint32_t    magic;
  zip_dir__s  cfile;
} zip_rec__u;

typedef struct
{
  char          filename[FILENAME_MAX];
  FILE         *fp;
  zip_eocd__s  *eocd;
  zip_dir__s  **dir;
} zipfile__s;

/***********************************************************************/

int	zipfile_open	(zipfile__s *,const char *);
int	zipfile_close	(zipfile__s *);

int	zipfile_version	(uint16_t,char *,size_t,char *,size_t);

/***********************************************************************/

int main(int argc,char *argv[])
{
  if (argc == 1)
  {
    fprintf(stderr,"usage: %s zipfile...\n",argv[0]);
    return EXIT_FAILURE;
  }
  
  for (int i = 1 ; i < argc ; i++)
  {
    zipfile__s zipfile;
    int        rc;
    
    rc = zipfile_open(&zipfile,argv[i]);
    if (rc != 0)
      fprintf(stderr,"%s: %s\n",argv[i],strerror(rc));
    else
    {
      printf(
      	"%s\n"
      	"\tdisk num:      %10d\n"
      	"\tdisk start:    %10d\n"
      	"\tentries:       %10d\n"
      	"\ttotal entries: %10d\n"
      	"\tsize of dir:   %10lu\n"
      	"\toffset of dir: %10lu\n"
      	"\n",
      	argv[i],
      	zipfile.eocd->disknum,
      	zipfile.eocd->diskstart,
      	zipfile.eocd->entries,
      	zipfile.eocd->totalentries,
      	(unsigned long)zipfile.eocd->size,
      	(unsigned long)zipfile.eocd->offset
      );
      
      for (size_t i = 0 ; zipfile.dir[i] != NULL ; i++)
      {
        zip_lua_ext__s ext;
        char           byos  [64];
        char           byver [64];
        char           foros [64];
        char           forver[64];
        
        zipfile_version(zipfile.dir[i]->byversion,byos,sizeof(byos),byver,sizeof(byver));
        zipfile_version(zipfile.dir[i]->forversion,foros,sizeof(foros),forver,sizeof(forver));
        
        if (zipfile.dir[i]->extralen > 0)
        {
          memcpy(
                  &ext,
                  &zipfile.dir[i]->data[zipfile.dir[i]->namelen],
                  sizeof(zip_lua_ext__s)
                );
          if (ext.id == 0x4C45)
          {
            const char *os;
            const char *cpu;
            char        luaver [16];
            char        version[12];
            
            switch(ext.os)
            {
              case ZIPE_OS_NONE:    os = "";          break;
              case ZIPE_OS_LINUX:   os = "Linux";     break;
              case ZIPE_OS_SOLARIS: os = "Solaris";   break;
              default:              os = "(unknown)"; break;
            }
            
            switch(ext.cpu)
            {
              case ZIPE_CPU_NONE:    cpu = "";          break;
              case ZIPE_CPU_x86:     cpu = "x86";       break;
              case ZIPE_CPU_SPARC64: cpu = "sparcv9";   break;
              default:               cpu = "(unknown)"; break;
            }
            
            if (ext.luavmin > 0)
            {
              if (ext.luavmin == ext.luavmax)
              {
                snprintf(
                  luaver,
                  sizeof(luaver),
                  "Lua %d.%d",
                  ext.luavmin >> 8,
                  ext.luavmin & 0xFF
                );
              }
              else
              {
                snprintf(
                  luaver,
                  sizeof(luaver),
                  "Lua %d.%d - %d.%d",
                  ext.luavmin >> 8,
                  ext.luavmin & 0xFF,
                  ext.luavmax >> 8,
                  ext.luavmax & 0xFF
                );
              }
            }
            else
              luaver[0] = '\0';
            
            snprintf(version,sizeof(version),"%d.%d",ext.version >> 8,ext.version & 0xFF);
            
            printf(
              "%-9s %-9s %-16s %-12s %.*s\n",
              os,
              cpu,
              luaver,
              version,
              (int)zipfile.dir[i]->namelen,zipfile.dir[i]->data
            );
          }
        }
        else
        {        
          printf(
        	"name:        %.*s\n"
        	"namelen:     %10lu\n"
        	"size:        %10lu\n"
        	"flags:       %04X\n"
        	"compression: %04X\n"
        	"iattr:       %04X\n"
        	"eattr:       %08X\n"
        	"byversion:   %s %s\n"
        	"forversion:  %s %s\n"
        	"\n",
        	(int)zipfile.dir[i]->namelen,zipfile.dir[i]->data,
        	(unsigned long)zipfile.dir[i]->namelen,
        	(unsigned long)zipfile.dir[i]->usize,
        	zipfile.dir[i]->flags,
        	zipfile.dir[i]->compression,
        	zipfile.dir[i]->iattr,
        	zipfile.dir[i]->eattr,
        	byos,byver,
        	foros,forver
	  );
        }
      }
            
      zipfile_close(&zipfile);
    }
  }

  return EXIT_SUCCESS;
}

/***********************************************************************/

int zipfile_open(zipfile__s *zf,const char *fname)
{
  zip_eocd__s eocd;
  
  assert(zf    != NULL);
  assert(fname != NULL);
  
  memset(zf,0,sizeof(zipfile__s));
  zf->fp   = NULL;
  zf->eocd = NULL;
  zf->dir  = NULL;
  
  strcpy(zf->filename,fname);
  zf->fp = fopen(fname,"rb");
  if (zf->fp == NULL)
    return errno;
  
  if (fseek(zf->fp,-(long)sizeof(zip_eocd__s),SEEK_END) < 0)
    return errno;
  
  if (fread(&eocd,sizeof(zip_eocd__s),1,zf->fp) != 1)
  {
    if (ferror(zf->fp)) return errno;
    return ENODATA;
  }
  
  if (eocd.magic != ZIP_MAGIC_EOCD)
    return EINVAL;
  
  zf->eocd = calloc(1,sizeof(zip_eocd__s) + eocd.commentlen);
  if (zf->eocd == NULL)
    return ENODATA;
  
  memcpy(zf->eocd,&eocd,sizeof(zip_eocd__s));
  fread(&zf->eocd->data,1,eocd.commentlen,zf->fp);
  
  zf->dir = calloc(zf->eocd->entries + 1,sizeof(zip_dir__s *));
  
  if (zf->dir == NULL)
    return ENOMEM;
  
  if (fseek(zf->fp,zf->eocd->offset,SEEK_SET) < 0)
    return errno;
  
  fprintf(stderr,"%08X\n",zf->eocd->offset);
  
  for (size_t i = 0 ; i < zf->eocd->entries ; i++)
  {
    zip_dir__s dir;
    size_t     len;
    
    fread(&dir,sizeof(zip_dir__s),1,zf->fp);
    len = sizeof(zip_dir__s) + dir.namelen + dir.extralen + dir.commentlen;
    zf->dir[i] = malloc(len);
    if (zf->dir[i] == NULL)
      return ENOMEM;
    
    memcpy(zf->dir[i],&dir,sizeof(zip_dir__s));
    fread(zf->dir[i]->data,1,dir.namelen + dir.extralen + dir.commentlen,zf->fp);
  }

  return 0;
}

/***********************************************************************/

int zipfile_close(zipfile__s *zf)
{
  if (zf->eocd)
    free(zf->eocd);
    
  if (zf->dir)
  {
    for (size_t i = 0 ; zf->dir[i] != NULL ; i++)
      free(zf->dir[i]);
    free(zf->dir);
  }
  
  if (zf->fp)
    fclose(zf->fp);
  
  return 0;
}

/***********************************************************************/

int zipfile_version(
	uint16_t    ver,
	char       *os,
	size_t      oslen,
	char       *version,
	size_t      verlen
)
{
  static const char *const osmap[] =
  {
    "MS-DOS,OS/2",
    "Amiga",
    "OpenVMS",
    "UNIX",
    "VM/CMS",
    "Atari ST",
    "OS/2 HPFS",
    "Macintosh",
    "Z-System",
    "CP/M",
    "Windows NTFS",
    "MVS (OS/390-Z/OS)",
    "VSE",
    "Acorn Risc",
    "VFAT",
    "alternative MVS",
    "BeOS",
    "Tandem",
    "OS/400",
    "OS X",
  };
  
  int   vos;
  div_t mmv;
    
  assert(os      != NULL);
  assert(oslen   >= 40);
  assert(version != NULL);
  assert(verlen  >= 40);
  
  if ((oslen < 40) || (verlen < 40))
    return ENOMEM;
  
  vos = ver >> 8;
  if (vos < 20)
    strcpy(os,osmap[vos]);
  else
    snprintf(os,oslen,"unknown %d",vos);
  
  mmv = div(ver & 0xFF,10);
  snprintf(version,verlen,"%d.%d",mmv.quot,mmv.rem);
  return 0;
}

/***********************************************************************/

