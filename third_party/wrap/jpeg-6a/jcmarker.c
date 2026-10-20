/* third_party/wrap/jpeg-6a/jcmarker.c: jpeg-6a/jcmarker.c, unchanged, with its static functions external as jpeg6a_jcmarker_<name>
   (see wrap/jpeg-6a/jcparam.c). Its methods (write_any_marker ... write_tables_only) are also the names of jpeglib.h's
   jpeg_marker_writer members, so they are defined before the headers: the members are renamed with them, consistently, in this unit
   alone (the layout is the same). jinit_marker_writer stores the six method addresses, so it is built as
   jpeg6a_jcmarker_lib_jinit_marker_writer: src/game/lib/jpeg6a.cpp's shim calls it and puts the exe's addresses in their place.
   emit_dac is empty in 6a without arithmetic coding; the exe's (0x64fe7d) is a hand port, this one is built but not hooked. */
#define write_any_marker jpeg6a_jcmarker_write_any_marker
#define write_file_header jpeg6a_jcmarker_write_file_header
#define write_frame_header jpeg6a_jcmarker_write_frame_header
#define write_scan_header jpeg6a_jcmarker_write_scan_header
#define write_file_trailer jpeg6a_jcmarker_write_file_trailer
#define write_tables_only jpeg6a_jcmarker_write_tables_only

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#undef METHODDEF
#define METHODDEF(type) type
#undef LOCAL
#define LOCAL(type) type

#define emit_byte jpeg6a_jcmarker_emit_byte
#define emit_marker jpeg6a_jcmarker_emit_marker
#define emit_2bytes jpeg6a_jcmarker_emit_2bytes
#define emit_dqt jpeg6a_jcmarker_emit_dqt
#define emit_dht jpeg6a_jcmarker_emit_dht
#define emit_dac jpeg6a_jcmarker_emit_dac
#define emit_dri jpeg6a_jcmarker_emit_dri
#define emit_sof jpeg6a_jcmarker_emit_sof
#define emit_sos jpeg6a_jcmarker_emit_sos
#define emit_jfif_app0 jpeg6a_jcmarker_emit_jfif_app0
#define emit_adobe_app14 jpeg6a_jcmarker_emit_adobe_app14
#undef jinit_marker_writer  /* (config/jpeg6a_prefix.h's jpeg6a_jinit_marker_writer: the shim's name) */
#define jinit_marker_writer jpeg6a_jcmarker_lib_jinit_marker_writer

#include "../../jpeg-6a/jcmarker.c"  /* (by its path: a plain "jcmarker.c" would find this file first) */
