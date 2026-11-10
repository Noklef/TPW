/* third_party/wrap/jpeg-6b/jerror.c: jpeg-6b/jerror.c, unchanged, with its static methods external as jerror_<name> (see
   wrap/jpeg-6b/jdcolor.c). They are also the names of jpeglib.h's jpeg_error_mgr members, so they are defined before the headers: the
   members are renamed with them, consistently, in this unit alone (the layout is the same).
   What it calls in the C runtime is the exe's (0x67a650 sprintf, 0x67f920 exit, 0x67dad0 fprintf), as jmemansi.c's malloc and free are
   (wrap/jpeg-6b/jmemansi.c): format_message's sprintf goes through jpeg6b.cpp's jpeg6b_exe_sprintf, error_exit's exit through
   jpeg6b_exe_exit, and output_message's fprintf(stderr, "%s\n", buffer) through jpeg6b_exe_print, which passes the exe's stderr and
   its "%s\n". jpeg_std_error stays the exe's (Codec631280_Init), which stores the exe's methods and message table: this copy is built
   but not hooked. */
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
extern void jpeg6b_exe_exit(int status);
extern int jpeg6b_exe_print(const char *text);
#define sprintf jpeg6b_exe_sprintf
#define exit jpeg6b_exe_exit
#define fprintf(file, format, text) jpeg6b_exe_print(text)

#include "../../jpeg-6b/jerror.c"  /* (by its path: a plain "jerror.c" would find this file first) */
