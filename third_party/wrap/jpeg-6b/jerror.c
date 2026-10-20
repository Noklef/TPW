/* third_party/wrap/jpeg-6b/jerror.c: jpeg-6b/jerror.c, unchanged, with its static methods external as jerror_<name> (see
   wrap/jpeg-6b/jdcolor.c). They are also the names of jpeglib.h's jpeg_error_mgr members, so they are defined before the headers: the
   members are renamed with them, consistently, in this unit alone (the layout is the same).
   format_message's sprintf is the exe's C runtime's (0x67a650), as jmemansi.c's malloc and free are (wrap/jpeg-6b/jmemansi.c): it goes
   through jpeg6b.cpp's jpeg6b_exe_sprintf. error_exit and output_message stay the exe's (they call its exit and fprintf(stderr)), and so
   does jpeg_std_error (Codec631280_Init), which stores the exe's methods and message table: these copies are built but not hooked. */
#define error_exit jerror_error_exit
#define emit_message jerror_emit_message
#define output_message jerror_output_message
#define format_message jerror_format_message
#define reset_error_mgr jerror_reset_error_mgr

#include "jinclude.h"
#include "jpeglib.h"

#undef METHODDEF
#define METHODDEF(type) type

extern int jpeg6b_exe_sprintf(char *buffer, const char *format, ...);
#define sprintf jpeg6b_exe_sprintf

#include "../../jpeg-6b/jerror.c"  /* (by its path: a plain "jerror.c" would find this file first) */
