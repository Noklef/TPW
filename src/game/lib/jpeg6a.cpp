// IJG libjpeg 6a: see jpeg6a.h. The functions are the library's (third_party/jpeg-6a, tpw_jpeg6a); this unit checks that its
// declarations are the library's and pins the layouts the exe's copies share with it.
#include "jpeg6a.h"

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>

// 6a's own headers, by path (the include path finds 6b's jpeglib.h and jconfig.h first), under the library's external names. Its
// struct tags are renamed to jpeg6a.h's (6b's jpeg6b.h has the same tags, laid out differently), and jpeglib.h's declarations of the
// functions jpeg6a.h types with tags of its own for the untagged JQUANT_TBL / JHUFF_TBL are renamed away (nothing here calls them).
#include "jpeg6a_prefix.h"
#define jpeg_common_struct jpeg6a_common_struct
#define jpeg_compress_struct jpeg6a_compress_struct
#define jvirt_sarray_control jpeg6a_jvirt_sarray_control
#define jvirt_barray_control jpeg6a_jvirt_barray_control
#define jpeg6a_jpeg_alloc_quant_table jpeglib_h_jpeg6a_jpeg_alloc_quant_table
#define jpeg6a_jpeg_alloc_huff_table jpeglib_h_jpeg6a_jpeg_alloc_huff_table
#define jpeg6a_jpeg_set_colorspace jpeglib_h_jpeg6a_jpeg_set_colorspace  // (a J_COLOR_SPACE, an int in jpeg6a.h)
extern "C" {
#define JPEG_INTERNALS
#include "../../../third_party/config/jpeg-6a/jconfig.h"
#define JCONFIG_INCLUDED
#include "../../../third_party/jpeg-6a/jpeglib.h"
#include "../../../third_party/jpeg-6a/jdct.h"
}
#undef jpeg6a_jpeg_alloc_quant_table
#undef jpeg6a_jpeg_alloc_huff_table
#undef jpeg6a_jpeg_set_colorspace

static_assert(JPEG_LIB_VERSION == 61, "libjpeg 6a (jpeg_CreateCompress checks 61)");
static_assert(sizeof(jpeg_compress_struct) == 0x168, "jpeg_compress_struct (jpeg_CreateCompress checks 0x168)");
static_assert(sizeof(DCTELEM) == 4 && sizeof(JCOEF) == 2 && sizeof(JSAMPLE) == 1 && sizeof(JDIMENSION) == 4,
              "the sample, coefficient and DCT types");
static_assert(sizeof(boolean) == 4, "boolean (an int in the exe's 6a: third_party/config/jpeg-6a/jconfig.h)");
// Fields the exe's code addresses directly (listing offsets).
static_assert(offsetof(jpeg_common_struct, mem) == 0x4 && offsetof(jpeg_common_struct, is_decompressor) == 0xc &&
                  offsetof(jpeg_common_struct, global_state) == 0x10,
              "cinfo->mem, is_decompressor, global_state (jpeg_abort 0x64f3c0, jpeg_destroy)");
static_assert(sizeof(JQUANT_TBL) == 0x84 && offsetof(JQUANT_TBL, sent_table) == 0x80, "JQUANT_TBL (jpeg_alloc_quant_table 0x64f43e)");
static_assert(sizeof(JHUFF_TBL) == 0x118 && offsetof(JHUFF_TBL, sent_table) == 0x114, "JHUFF_TBL (jpeg_alloc_huff_table 0x64f46f)");
static_assert(offsetof(jpeg_compress_struct, input_components) == 0x20 && offsetof(jpeg_compress_struct, in_color_space) == 0x24 &&
                  offsetof(jpeg_compress_struct, data_precision) == 0x30 && offsetof(jpeg_compress_struct, num_components) == 0x34 &&
                  offsetof(jpeg_compress_struct, jpeg_color_space) == 0x38 && offsetof(jpeg_compress_struct, comp_info) == 0x3c,
              "the colour fields and comp_info (jpeg_set_colorspace 0x64d23b)");
static_assert(offsetof(jpeg_compress_struct, quant_tbl_ptrs) == 0x40 && offsetof(jpeg_compress_struct, dc_huff_tbl_ptrs) == 0x50 &&
                  offsetof(jpeg_compress_struct, ac_huff_tbl_ptrs) == 0x60 && offsetof(jpeg_compress_struct, arith_dc_L) == 0x70 &&
                  offsetof(jpeg_compress_struct, num_scans) == 0xa0 && offsetof(jpeg_compress_struct, scan_info) == 0xa4,
              "the tables and the scan script (jpeg_set_defaults 0x64cf20)");
static_assert(offsetof(jpeg_compress_struct, raw_data_in) == 0xa8 && offsetof(jpeg_compress_struct, CCIR601_sampling) == 0xb4 &&
                  offsetof(jpeg_compress_struct, write_JFIF_header) == 0xc8 && offsetof(jpeg_compress_struct, density_unit) == 0xcc &&
                  offsetof(jpeg_compress_struct, Y_density) == 0xd0 && offsetof(jpeg_compress_struct, write_Adobe_marker) == 0xd4,
              "the int booleans and the JFIF fields (jpeg_set_defaults)");
static_assert(sizeof(jpeg_component_info) == 0x54 && sizeof(jpeg_scan_info) == 0x24, "a component (10 at 0x348), a scan");
static_assert(offsetof(jpeg_memory_mgr, free_pool) == 0x24 && offsetof(jpeg_memory_mgr, self_destruct) == 0x28,
              "mem->free_pool, self_destruct (jcomapi.c)");

// The method-address shim, as jpeg6b.cpp's: an init that stores the library's methods is built as jpeg6a_<module>_lib_<name> (its
// wrapper unit), and the shim of its name calls it and puts the exe's address of each method in place of the library's, so the exe's
// code that compares or calls them sees its own (the game's hooks then run the library's anyway).
extern "C" void __cdecl jpeg6a_jmemmgr_lib_jinit_memory_mgr(jpeg6a_common_struct *cinfo);
extern "C" void __cdecl jpeg6a_jcmarker_lib_jinit_marker_writer(jpeg6a_compress_struct *cinfo);

namespace {
struct ExeMethod {
    const void *library;
    std::uintptr_t exe;
};

template <class F> const void *fn(F *f) { return reinterpret_cast<const void *>(f); }

// Each library method an init stores, and the exe's address of the same method (re/libmap.csv).
const ExeMethod kExeMethods[] = {
    // jmemmgr.c
    {fn(&jpeg6a_jmemmgr_alloc_small), 0x64e21d},
    {fn(&jpeg6a_jmemmgr_alloc_large), 0x64e40d},
    {fn(&jpeg6a_jmemmgr_alloc_sarray), 0x64e4ff},
    {fn(&jpeg6a_jmemmgr_alloc_barray), 0x64e600},
    {fn(&jpeg6a_jmemmgr_request_virt_sarray), 0x64e715},
    {fn(&jpeg6a_jmemmgr_request_virt_barray), 0x64e7ba},
    {fn(&jpeg6a_jmemmgr_realize_virt_arrays), 0x64e85f},
    {fn(&jpeg6a_jmemmgr_access_virt_sarray), 0x64eb3a},
    {fn(&jpeg6a_jmemmgr_access_virt_barray), 0x64ee6f},
    {fn(&jpeg6a_jmemmgr_free_pool), 0x64f1aa},
    {fn(&jpeg6a_jmemmgr_self_destruct), 0x64f360},
    // jcmarker.c
    {fn(&jpeg6a_jcmarker_write_any_marker), 0x64f520},
    {fn(&jpeg6a_jcmarker_write_file_header), 0x64f644},
    {fn(&jpeg6a_jcmarker_write_frame_header), 0x64f852},
    {fn(&jpeg6a_jcmarker_write_scan_header), 0x64fc34},
    {fn(&jpeg6a_jcmarker_write_file_trailer), 0x65000f},
    {fn(&jpeg6a_jcmarker_write_tables_only), 0x650025},
};

bool g_native_methods = false;  // (Jpeg6a_SetNativeMethods: the tests' driver, which cannot call the exe's addresses)

// The method pointer at slot: the exe's address in place of a library method's (anything else is left as it is).
void exe_method_at(void *slot) {
    if (g_native_methods) return;
    void *&p = *static_cast<void **>(slot);
    for (const ExeMethod &m : kExeMethods)
        if (p == m.library) {
            p = reinterpret_cast<void *>(m.exe);
            return;
        }
}

template <class F> void exe_method(F &field) { exe_method_at(&field); }
}  // namespace

extern "C" {
void __cdecl jpeg6a_jinit_memory_mgr(j_common_ptr cinfo) {
    jpeg6a_jmemmgr_lib_jinit_memory_mgr(cinfo);
    jpeg_memory_mgr *mem = cinfo->mem;
    exe_method(mem->alloc_small);
    exe_method(mem->alloc_large);
    exe_method(mem->alloc_sarray);
    exe_method(mem->alloc_barray);
    exe_method(mem->request_virt_sarray);
    exe_method(mem->request_virt_barray);
    exe_method(mem->realize_virt_arrays);
    exe_method(mem->access_virt_sarray);
    exe_method(mem->access_virt_barray);
    exe_method(mem->free_pool);
    exe_method(mem->self_destruct);
}

void __cdecl jpeg6a_jinit_marker_writer(j_compress_ptr cinfo) {
    jpeg6a_jcmarker_lib_jinit_marker_writer(cinfo);
    jpeg_marker_writer *marker = cinfo->marker;
    exe_method(marker->write_any_marker);
    exe_method(marker->write_file_header);
    exe_method(marker->write_frame_header);
    exe_method(marker->write_scan_header);
    exe_method(marker->write_file_trailer);
    exe_method(marker->write_tables_only);
}
}

void Jpeg6a_SetNativeMethods(bool native) { g_native_methods = native; }

std::uint32_t Jpeg6a_ExeMethod(std::uint32_t address) {
    for (const ExeMethod &m : kExeMethods)
        if ((std::uintptr_t)m.library == address) return (std::uint32_t)m.exe;
    return 0;
}

// What the library calls outside itself (jpeg6a.h's Jpeg6aCrtEnv), as jpeg6b.cpp's: jmemansi.c's malloc and free
// (third_party/wrap/jpeg-6a/jmemansi.c #defines them to jpeg6a_exe_malloc / free), so the compressor's blocks are the exe's heap's;
// jmemmgr.c's getenv, sscanf and jpeg_open_backing_store (wrap/jpeg-6a/jmemmgr.c); jerror.c's sprintf (wrap/jpeg-6a/jerror.c).
namespace {
const Jpeg6aCrtEnv kExeCrt = {
    reinterpret_cast<void *(__cdecl *)(size_t)>((uintptr_t)0x0067dcd0),
    reinterpret_cast<void(__cdecl *)(void *)>((uintptr_t)0x0067ad90),
    reinterpret_cast<char *(__cdecl *)(const char *)>((uintptr_t)0x0067fbf0),
    reinterpret_cast<int(__cdecl *)(const char *, const char *, long *, char *)>((uintptr_t)0x0067f620),
    reinterpret_cast<void(__cdecl *)(jpeg6a_common_struct *, void *, long)>((uintptr_t)0x00650265),
    reinterpret_cast<int(__cdecl *)(char *, const char *, const char *)>((uintptr_t)0x0067a650),
    reinterpret_cast<int(__cdecl *)(char *, const char *, int, int, int, int, int, int, int, int)>((uintptr_t)0x0067a650)};
const Jpeg6aCrtEnv *g_crt = &kExeCrt;
}  // namespace

void Jpeg6a_SetCrtEnv(const Jpeg6aCrtEnv *env) { g_crt = env ? env : &kExeCrt; }

extern "C" {
void *__cdecl jpeg6a_exe_malloc(size_t size) { return g_crt->alloc(size); }
void __cdecl jpeg6a_exe_free(void *block) { g_crt->free(block); }
char *__cdecl jpeg6a_exe_getenv(const char *name) { return g_crt->getenv(name); }
int __cdecl jpeg6a_exe_sscanf(const char *s, const char *format, long *value, char *ch) { return g_crt->sscanf(s, format, value, ch); }
void __cdecl jpeg6a_exe_open_backing_store(jpeg6a_common_struct *cinfo, void *info, long total_bytes_needed) {
    g_crt->open_backing_store(cinfo, info, total_bytes_needed);
}
// format_message's two sprintf calls: the message's string parameter when its first '%' is a %s (format_message's own test, so the
// call has the arguments the source passes), else the eight ints. The exe's sprintf is variadic, so either typed pointer calls it.
int __cdecl jpeg6a_exe_sprintf(char *buffer, const char *format, ...) {
    bool isstring = false;
    for (const char *p = format; *p; p++)
        if (*p == '%') {
            isstring = p[1] == 's';
            break;
        }
    va_list ap;
    va_start(ap, format);
    int n;
    if (isstring) {
        n = g_crt->sprintf_string(buffer, format, va_arg(ap, const char *));
    } else {
        int i[8];
        for (int k = 0; k < 8; k++) i[k] = va_arg(ap, int);
        n = g_crt->sprintf_ints(buffer, format, i[0], i[1], i[2], i[3], i[4], i[5], i[6], i[7]);
    }
    va_end(ap);
    return n;
}
}
