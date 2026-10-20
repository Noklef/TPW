/* third_party/wrap/jpeg-6b/jmemansi.c: jpeg-6b/jmemansi.c, unchanged, with its malloc and free the exe's. The exe's jpeg_get_small /
   jpeg_get_large (one copy, 0x642980) call its C runtime's malloc (0x67dcd0) and jpeg_free_small / jpeg_free_large (0x642990, the same
   copy as zlib's zcfree) its free (0x67ad90): every block the decoder allocates is on the exe's CRT heap, and the exe's own code
   (zcfree, and jmemmgr.c while it stays the exe's) frees them there. The library's would use the DLL's C runtime, a different heap.
   So malloc and free are #defined, after the headers have declared the real ones, to src/game/lib/jpeg6b.cpp's jpeg6b_exe_malloc /
   jpeg6b_exe_free, which call the exe's (never replaced: they are the front of its heap). The backing store (tmpfile, fread, fwrite,
   fclose) is left as it is: jpeg_open_backing_store and its methods stay the exe's (see docs/porting.md). */
#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
/* (not jmemsys.h: it has no include guard, and jmemansi.c includes it) */

extern void *jpeg6b_exe_malloc(size_t size);
extern void jpeg6b_exe_free(void *block);
#define malloc(size) jpeg6b_exe_malloc(size)
#define free(block) jpeg6b_exe_free(block)

#include "../../jpeg-6b/jmemansi.c"  /* (by its path: a plain "jmemansi.c" would find this file first) */
