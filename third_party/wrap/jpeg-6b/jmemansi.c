/* third_party/wrap/jpeg-6b/jmemansi.c: jpeg-6b/jmemansi.c, unchanged, with its C runtime the exe's. The exe's jpeg_get_small /
   jpeg_get_large (one copy, 0x642980) call its C runtime's malloc (0x67dcd0) and jpeg_free_small / jpeg_free_large (0x642990, the same
   copy as zlib's zcfree) its free (0x67ad90): every block the decoder allocates is on the exe's CRT heap, and the exe's own code
   (zcfree) frees them there. The library's would use the DLL's C runtime, a different heap. So malloc and free are #defined, after
   the headers have declared the real ones, to src/game/lib/jpeg6b.cpp's jpeg6b_exe_malloc / jpeg6b_exe_free, which call the exe's
   (never replaced: they are the front of its heap). The backing store's tmpfile, fseek, fread, fwrite and fclose are the exe's too
   (jpeg6b_exe_*: its FILE is the exe's C runtime's), and ERREXITS's "" is the exe's (jpeg6b_exe_empty, a .bss byte).
   The backing store's methods are static: they are external here as jmemansi_<name> (see wrap/jpeg-6b/jdcolor.c), and
   jpeg_open_backing_store, which stores them, is built as jmemansi_lib_jpeg_open_backing_store: jpeg6b.cpp's shim calls it and puts
   the exe's addresses in their place. */
#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
/* (not jmemsys.h: it has no include guard, and jmemansi.c includes it) */

#undef METHODDEF
#define METHODDEF(type) type

extern void *jpeg6b_exe_malloc(size_t size);
extern void jpeg6b_exe_free(void *block);
extern void *jpeg6b_exe_tmpfile(void);
extern int jpeg6b_exe_fseek(void *file, long offset, int origin);
extern size_t jpeg6b_exe_fread(void *buffer, size_t size, size_t count, void *file);
extern size_t jpeg6b_exe_fwrite(const void *buffer, size_t size, size_t count, void *file);
extern int jpeg6b_exe_fclose(void *file);
extern const char *jpeg6b_exe_empty(void);
#define malloc(size) jpeg6b_exe_malloc(size)
#define free(block) jpeg6b_exe_free(block)
#define tmpfile() jpeg6b_exe_tmpfile()
#define fseek(file, offset, origin) jpeg6b_exe_fseek(file, offset, origin)
#define fread(buffer, size, count, file) jpeg6b_exe_fread(buffer, size, count, file)
#define fwrite(buffer, size, count, file) jpeg6b_exe_fwrite(buffer, size, count, file)
#define fclose(file) jpeg6b_exe_fclose(file)
#undef ERREXITS
#define ERREXITS(cinfo, code, str) \
  ((cinfo)->err->msg_code = (code), \
   strncpy((cinfo)->err->msg_parm.s, jpeg6b_exe_empty(), JMSG_STR_PARM_MAX), \
   (*(cinfo)->err->error_exit) ((j_common_ptr) (cinfo)))

#define read_backing_store jmemansi_read_backing_store
#define write_backing_store jmemansi_write_backing_store
#define close_backing_store jmemansi_close_backing_store
#define jpeg_open_backing_store jmemansi_lib_jpeg_open_backing_store  /* (jmemsys.h's declaration is renamed with it) */

#include "../../jpeg-6b/jmemansi.c"  /* (by its path: a plain "jmemansi.c" would find this file first) */
