/* third_party/wrap/jpeg-6a/jcmaster.c: jpeg-6a/jcmaster.c, unchanged, with its static functions external as jpeg6a_jcmaster_<name>
   (see wrap/jpeg-6a/jcparam.c). Two of its methods (prepare_for_pass, pass_startup) are also the names of jpeglib.h's
   jpeg_comp_master members, so they are defined before the headers: the members are renamed with them, consistently, in this unit
   alone (the layout is the same). jinit_c_master_control stores the three method addresses, so it is built as
   jpeg6a_jcmaster_lib_jinit_c_master_control: src/game/lib/jpeg6a.cpp's shim calls it and puts the exe's addresses in their place. */
#define prepare_for_pass jpeg6a_jcmaster_prepare_for_pass
#define pass_startup jpeg6a_jcmaster_pass_startup

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#undef METHODDEF
#define METHODDEF(type) type
#undef LOCAL
#define LOCAL(type) type

#define initial_setup jpeg6a_jcmaster_initial_setup
#define validate_script jpeg6a_jcmaster_validate_script
#define select_scan_parameters jpeg6a_jcmaster_select_scan_parameters
#define per_scan_setup jpeg6a_jcmaster_per_scan_setup
#define finish_pass_master jpeg6a_jcmaster_finish_pass_master
#undef jinit_c_master_control  /* (config/jpeg6a_prefix.h's jpeg6a_jinit_c_master_control: the shim's name) */
#define jinit_c_master_control jpeg6a_jcmaster_lib_jinit_c_master_control

#include "../../jpeg-6a/jcmaster.c"  /* (by its path: a plain "jcmaster.c" would find this file first) */

/* The exe allocates 0x24 bytes (my_comp_master: the public 0x14, pass_type, pass_number, total_passes, scan_number). */
typedef char jpeg6a_jcmaster_size_0x24[sizeof(my_comp_master) == 0x24 ? 1 : -1];
