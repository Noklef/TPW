/* third_party/wrap/jpeg-6b/jmemmgr.c: jpeg-6b/jmemmgr.c, unchanged, with its static functions external as jmemmgr_<name> (see
   wrap/jpeg-6b/jdcolor.c). Its methods (alloc_small ... self_destruct) are also the names of jpeglib.h's jpeg_memory_mgr members, so
   they are defined before the headers: the members are renamed with them, consistently, in this unit alone (the layout is the same).
   jinit_memory_mgr stores the eleven method addresses, so it is built as jmemmgr_lib_jinit_memory_mgr: src/game/lib/jpeg6b.cpp's shim
   calls it and puts the exe's addresses in their place.
   What it calls outside the library is the exe's, as jmemansi.c's malloc and free are (wrap/jpeg-6b/jmemansi.c): getenv (0x67fbf0) and
   sscanf (0x67f620), the exe's C runtime's, read JPEGMEM; and jpeg_open_backing_store is the exe's (0x6429c0, it stays the exe's: its
   tmpfile() is a FILE of the exe's C runtime, which its read / write / close methods take), so realize_virt_arrays opens the exe's
   backing store. They go through jpeg6b.cpp's jpeg6b_exe_getenv / sscanf / open_backing_store. */
#define alloc_small jmemmgr_alloc_small
#define alloc_large jmemmgr_alloc_large
#define alloc_sarray jmemmgr_alloc_sarray
#define alloc_barray jmemmgr_alloc_barray
#define request_virt_sarray jmemmgr_request_virt_sarray
#define request_virt_barray jmemmgr_request_virt_barray
#define realize_virt_arrays jmemmgr_realize_virt_arrays
#define access_virt_sarray jmemmgr_access_virt_sarray
#define access_virt_barray jmemmgr_access_virt_barray
#define free_pool jmemmgr_free_pool
#define self_destruct jmemmgr_self_destruct

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
/* (not jmemsys.h: it has no include guard, and jmemmgr.c includes it) */

#undef METHODDEF
#define METHODDEF(type) type
#undef LOCAL
#define LOCAL(type) type

#define out_of_memory jmemmgr_out_of_memory
#define do_sarray_io jmemmgr_do_sarray_io
#define do_barray_io jmemmgr_do_barray_io
#define jinit_memory_mgr jmemmgr_lib_jinit_memory_mgr

extern char *jpeg6b_exe_getenv(const char *name);
extern int jpeg6b_exe_sscanf(const char *s, const char *format, long *value, char *ch);
#define getenv(name) jpeg6b_exe_getenv(name)
#define sscanf jpeg6b_exe_sscanf
#define jpeg_open_backing_store jpeg6b_exe_open_backing_store  /* (jmemsys.h's declaration is renamed with it) */

#include "../../jpeg-6b/jmemmgr.c"  /* (by its path: a plain "jmemmgr.c" would find this file first) */

/* The exe allocates 0x54 bytes (last_rowsperchunk at +0x50) and 0x78 a virtual array's control block (b_s_info at +0x28). */
typedef char jmemmgr_size_0x54[sizeof(my_memory_mgr) == 0x54 ? 1 : -1];
typedef char jmemmgr_sarray_0x78[sizeof(struct jvirt_sarray_control) == 0x78 ? 1 : -1];
typedef char jmemmgr_barray_0x78[sizeof(struct jvirt_barray_control) == 0x78 ? 1 : -1];
