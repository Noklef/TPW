/* third_party/wrap/jpeg-6b/jdmarker.c: jpeg-6b/jdmarker.c, unchanged, with its static functions external as jdmarker_<name> (see
   wrap/jpeg-6b/jdcolor.c). reset_marker_reader, read_markers and read_restart_marker are also the names of struct members (jpeglib.h's
   jpeg_marker_reader), so they are defined before the headers: the members are renamed with them, consistently, in this unit alone.
   jinit_marker_reader stores method addresses, so it is built as jdmarker_lib_jinit_marker_reader: src/game/lib/jpeg6b.cpp's shim
   (jinit_marker_reader) calls it and puts the exe's addresses in their place. get_dac is skip_variable without arithmetic coding
   (jdmarker.c's own #define). save_marker, jpeg_save_markers and jpeg_set_marker_processor are not in the exe. */
#define reset_marker_reader jdmarker_reset_marker_reader
#define read_markers jdmarker_read_markers
#define read_restart_marker jdmarker_read_restart_marker

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#undef METHODDEF
#define METHODDEF(type) type
#undef LOCAL
#define LOCAL(type) type

#define get_soi jdmarker_get_soi
#define get_sof jdmarker_get_sof
#define get_sos jdmarker_get_sos
#define get_dht jdmarker_get_dht
#define get_dqt jdmarker_get_dqt
#define get_dri jdmarker_get_dri
#define examine_app0 jdmarker_examine_app0
#define examine_app14 jdmarker_examine_app14
#define get_interesting_appn jdmarker_get_interesting_appn
#define save_marker jdmarker_save_marker
#define skip_variable jdmarker_skip_variable
#define next_marker jdmarker_next_marker
#define first_marker jdmarker_first_marker
#define jinit_marker_reader jdmarker_lib_jinit_marker_reader

#include "../../jpeg-6b/jdmarker.c"  /* (by its path: a plain "jdmarker.c" would find this file first) */

/* The private methods src/game/lib/jpeg6b.cpp's shim maps by their offsets: process_COM at +0x18, process_APPn[16] at +0x1c (the exe
   allocates 0xa8 bytes). */
typedef char jdmarker_process_COM_at_0x18[offsetof(my_marker_reader, process_COM) == 0x18 ? 1 : -1];
typedef char jdmarker_process_APPn_at_0x1c[offsetof(my_marker_reader, process_APPn) == 0x1c ? 1 : -1];
typedef char jdmarker_size_0xa8[sizeof(my_marker_reader) == 0xa8 ? 1 : -1];
