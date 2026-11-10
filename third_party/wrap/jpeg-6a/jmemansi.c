/* third_party/wrap/jpeg-6a/jmemansi.c: jpeg-6a/jmemansi.c, unchanged, with its C runtime the exe's, as 6b's
   (wrap/jpeg-6b/jmemansi.c). The exe's 6a jpeg_get_small / jpeg_get_large (0x650210, 0x650232) call its C runtime's malloc (0x67dcd0)
   and jpeg_free_small / jpeg_free_large (0x650221, 0x650243) its free (0x67ad90): the compressor's blocks are on the exe's CRT heap, and
   the exe's own code frees them there. So malloc and free are #defined, after the headers have declared the real ones, to
   src/game/lib/jpeg6a.cpp's jpeg6a_exe_malloc / jpeg6a_exe_free, which call the exe's. The backing store's tmpfile, fseek, fread,
   fwrite and fclose are the exe's too (jpeg6a_exe_*: its FILE is the exe's C runtime's), and ERREXITS's "" is the exe's 6a one
   (jpeg6a_exe_empty, a .bss byte).
   The backing store's methods are static: they are external here as jpeg6a_jmemansi_<name> (see wrap/jpeg-6a/jcparam.c), and
   jpeg_open_backing_store, which stores them, is built as jpeg6a_jmemansi_lib_jpeg_open_backing_store: jpeg6a.cpp's shim
   (jpeg6a_jpeg_open_backing_store) calls it and puts the exe's addresses in their place.
   The exe's 6a was built with DEFAULT_MAX_MEM 10000000 (jpeg_mem_init at 0x6503c1 answers 0x989680; the source's default is 1000000):
   third_party/CMakeLists.txt defines it for this unit. */
#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
/* (not jmemsys.h: it has no include guard, and jmemansi.c includes it) */

#undef METHODDEF
#define METHODDEF(type) type

extern void *jpeg6a_exe_malloc(size_t size);
extern void jpeg6a_exe_free(void *block);
extern void *jpeg6a_exe_tmpfile(void);
extern int jpeg6a_exe_fseek(void *file, long offset, int origin);
extern size_t jpeg6a_exe_fread(void *buffer, size_t size, size_t count, void *file);
extern size_t jpeg6a_exe_fwrite(const void *buffer, size_t size, size_t count, void *file);
extern int jpeg6a_exe_fclose(void *file);
extern const char *jpeg6a_exe_empty(void);
#define malloc(size) jpeg6a_exe_malloc(size)
#define free(block) jpeg6a_exe_free(block)
#define tmpfile() jpeg6a_exe_tmpfile()
#define fseek(file, offset, origin) jpeg6a_exe_fseek(file, offset, origin)
#define fread(buffer, size, count, file) jpeg6a_exe_fread(buffer, size, count, file)
#define fwrite(buffer, size, count, file) jpeg6a_exe_fwrite(buffer, size, count, file)
#define fclose(file) jpeg6a_exe_fclose(file)
#undef ERREXITS
#define ERREXITS(cinfo, code, str) \
  ((cinfo)->err->msg_code = (code), \
   strncpy((cinfo)->err->msg_parm.s, jpeg6a_exe_empty(), JMSG_STR_PARM_MAX), \
   (*(cinfo)->err->error_exit) ((j_common_ptr) (cinfo)))

#define read_backing_store jpeg6a_jmemansi_read_backing_store
#define write_backing_store jpeg6a_jmemansi_write_backing_store
#define close_backing_store jpeg6a_jmemansi_close_backing_store
#undef jpeg_open_backing_store
#define jpeg_open_backing_store jpeg6a_jmemansi_lib_jpeg_open_backing_store  /* (jmemsys.h's declaration is renamed with it) */

#include "../../jpeg-6a/jmemansi.c"  /* (by its path: a plain "jmemansi.c" would find this file first) */
