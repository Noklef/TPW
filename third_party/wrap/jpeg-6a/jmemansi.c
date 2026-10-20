/* third_party/wrap/jpeg-6a/jmemansi.c: jpeg-6a/jmemansi.c, unchanged, with its malloc and free the exe's, as 6b's
   (wrap/jpeg-6b/jmemansi.c). The exe's 6a jpeg_get_small / jpeg_get_large (0x650210, 0x650232) call its C runtime's malloc (0x67dcd0)
   and jpeg_free_small / jpeg_free_large (0x650221, 0x650243) its free (0x67ad90): the compressor's blocks are on the exe's CRT heap, and
   the exe's own code frees them there. So malloc and free are #defined, after the headers have declared the real ones, to
   src/game/lib/jpeg6a.cpp's jpeg6a_exe_malloc / jpeg6a_exe_free, which call the exe's. The backing store (tmpfile, fread, fwrite,
   fclose) is left as it is: jpeg_open_backing_store and its methods stay the exe's (docs/porting-libraries.md).
   The exe's 6a was built with DEFAULT_MAX_MEM 10000000 (jpeg_mem_init at 0x6503c1 answers 0x989680; the source's default is 1000000):
   third_party/CMakeLists.txt defines it for this unit. */
#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
/* (not jmemsys.h: it has no include guard, and jmemansi.c includes it) */

extern void *jpeg6a_exe_malloc(size_t size);
extern void jpeg6a_exe_free(void *block);
#define malloc(size) jpeg6a_exe_malloc(size)
#define free(block) jpeg6a_exe_free(block)

#include "../../jpeg-6a/jmemansi.c"  /* (by its path: a plain "jmemansi.c" would find this file first) */
