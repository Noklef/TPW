/* third_party/wrap/jpeg-6a/jmemmgr.c: jpeg-6a/jmemmgr.c, unchanged, with its static functions external as jpeg6a_jmemmgr_<name> (see
   wrap/jpeg-6a/jcparam.c), as 6b's (wrap/jpeg-6b/jmemmgr.c). Its methods (alloc_small ... self_destruct) are also the names of
   jpeglib.h's jpeg_memory_mgr members, so they are defined before the headers: the members are renamed with them, consistently, in this
   unit alone (the layout is the same). jinit_memory_mgr stores the eleven method addresses, so it is built as
   jpeg6a_jmemmgr_lib_jinit_memory_mgr: src/game/lib/jpeg6a.cpp's shim calls it and puts the exe's addresses in their place.
   What it calls outside the library is the exe's, as jmemansi.c's malloc and free are (wrap/jpeg-6a/jmemansi.c): getenv (0x67fbf0) and
   sscanf (0x67f620), the exe's C runtime's, read JPEGMEM; and jpeg_open_backing_store is the exe's 6a copy (0x650265, it stays the
   exe's: its tmpfile() is a FILE of the exe's C runtime, which its read / write / close methods take), so realize_virt_arrays opens the
   exe's backing store. They go through jpeg6a.cpp's jpeg6a_exe_getenv / sscanf / open_backing_store. */
#define alloc_small jpeg6a_jmemmgr_alloc_small
#define alloc_large jpeg6a_jmemmgr_alloc_large
#define alloc_sarray jpeg6a_jmemmgr_alloc_sarray
#define alloc_barray jpeg6a_jmemmgr_alloc_barray
#define request_virt_sarray jpeg6a_jmemmgr_request_virt_sarray
#define request_virt_barray jpeg6a_jmemmgr_request_virt_barray
#define realize_virt_arrays jpeg6a_jmemmgr_realize_virt_arrays
#define access_virt_sarray jpeg6a_jmemmgr_access_virt_sarray
#define access_virt_barray jpeg6a_jmemmgr_access_virt_barray
#define free_pool jpeg6a_jmemmgr_free_pool
#define self_destruct jpeg6a_jmemmgr_self_destruct

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
/* (not jmemsys.h: it has no include guard, and jmemmgr.c includes it) */

#undef METHODDEF
#define METHODDEF(type) type
#undef LOCAL
#define LOCAL(type) type

#define out_of_memory jpeg6a_jmemmgr_out_of_memory
#define do_sarray_io jpeg6a_jmemmgr_do_sarray_io
#define do_barray_io jpeg6a_jmemmgr_do_barray_io
#undef jinit_memory_mgr  /* (config/jpeg6a_prefix.h's jpeg6a_jinit_memory_mgr: the shim's name) */
#define jinit_memory_mgr jpeg6a_jmemmgr_lib_jinit_memory_mgr

extern char *jpeg6a_exe_getenv(const char *name);
extern int jpeg6a_exe_sscanf(const char *s, const char *format, long *value, char *ch);
#define getenv(name) jpeg6a_exe_getenv(name)
#define sscanf jpeg6a_exe_sscanf
#undef jpeg_open_backing_store
#define jpeg_open_backing_store jpeg6a_exe_open_backing_store  /* (jmemsys.h's declaration is renamed with it) */

#include "../../jpeg-6a/jmemmgr.c"  /* (by its path: a plain "jmemmgr.c" would find this file first) */

/* The exe allocates 0x50 bytes (6a's jpeg_memory_mgr has no max_alloc_chunk: last_rowsperchunk at +0x4c), and a virtual array's
   control block is 6a's with int booleans (pre_zero, dirty, b_s_open). */
typedef char jpeg6a_jmemmgr_size_0x50[sizeof(my_memory_mgr) == 0x50 ? 1 : -1];
