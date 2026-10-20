/* third_party/wrap/jpeg-6a/jerror.c: jpeg-6a/jerror.c, unchanged, with its static methods external as jpeg6a_jerror_<name> (see
   wrap/jpeg-6a/jcparam.c), as 6b's (wrap/jpeg-6b/jerror.c). They are also the names of jpeglib.h's jpeg_error_mgr members, so they are
   defined before the headers: the members are renamed with them, consistently, in this unit alone (the layout is the same).
   format_message's sprintf is the exe's C runtime's (0x67a650), as jmemansi.c's malloc and free are (wrap/jpeg-6a/jmemansi.c): it goes
   through src/game/lib/jpeg6a.cpp's jpeg6a_exe_sprintf. error_exit and output_message stay the exe's (they call its exit and
   fprintf(stderr)), and so does jpeg_std_error (the hand port Fn_64dd60), which stores the exe's methods and its 6a message table
   (0x7054b8): these copies are built but not hooked. */
#define error_exit jpeg6a_jerror_error_exit
#define emit_message jpeg6a_jerror_emit_message
#define output_message jpeg6a_jerror_output_message
#define format_message jpeg6a_jerror_format_message
#define reset_error_mgr jpeg6a_jerror_reset_error_mgr

#include "jinclude.h"
#include "jpeglib.h"

#undef METHODDEF
#define METHODDEF(type) type

extern int jpeg6a_exe_sprintf(char *buffer, const char *format, ...);
#define sprintf jpeg6a_exe_sprintf

#include "../../jpeg-6a/jerror.c"  /* (by its path: a plain "jerror.c" would find this file first) */
