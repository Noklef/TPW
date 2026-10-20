// IJG libjpeg 6b: see jpeg6b.h. The functions are the library's (third_party/jpeg-6b, tpw_jpeg6b); this unit pins the layouts the
// exe's copies share with the library's code while both run (the offsets are the listing's), and holds the method-address shim.
#include "jpeg6b.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>

extern "C" {
#define JPEG_INTERNALS
#include "jpeglib.h"
}

static_assert(sizeof(jpeg_decompress_struct) == 0x1b0, "jpeg_decompress_struct (jpeg_CreateDecompress checks 0x1b0)");
static_assert(sizeof(jpeg_component_info) == 0x54, "jpeg_component_info");
static_assert(sizeof(JCOEF) == 2 && sizeof(JSAMPLE) == 1 && sizeof(JDIMENSION) == 4, "the sample and coefficient types");
// Fields the exe's code addresses directly (listing offsets).
static_assert(offsetof(jpeg_decompress_struct, sample_range_limit) == 0x120, "cinfo->sample_range_limit (the IDCTs' range_limit)");
static_assert(offsetof(jpeg_component_info, dct_table) == 0x50, "compptr->dct_table (the IDCTs' quantptr)");
static_assert(offsetof(jpeg_decompress_struct, mem) == 0x4, "cinfo->mem (alloc_small, its first method)");
static_assert(offsetof(jpeg_decompress_struct, num_components) == 0x24, "cinfo->num_components (null_convert)");
static_assert(offsetof(jpeg_decompress_struct, output_width) == 0x5c, "cinfo->output_width (the converters' num_cols)");
static_assert(offsetof(jpeg_decompress_struct, output_height) == 0x60, "cinfo->output_height (start_pass_merged_upsample)");
static_assert(offsetof(jpeg_decompress_struct, upsample) == 0x1a0, "cinfo->upsample (jdmerge.c's my_upsampler)");
static_assert(offsetof(jpeg_decompress_struct, cconvert) == 0x1a4, "cinfo->cconvert (jdcolor.c's my_color_deconverter)");
static_assert(offsetof(jpeg_decompress_struct, comp_info) == 0xc4, "cinfo->comp_info (sep_upsample)");
static_assert(offsetof(jpeg_decompress_struct, max_v_samp_factor) == 0x114, "cinfo->max_v_samp_factor (the upsamplers' rows)");
static_assert(offsetof(jpeg_component_info, component_index) == 0x4, "compptr->component_index (int_upsample)");
static_assert(offsetof(jpeg_component_info, downsampled_width) == 0x28, "compptr->downsampled_width (the fancy upsamplers)");
static_assert(offsetof(jpeg_decompress_struct, idct) == 0x19c, "cinfo->idct (jddctmgr.c's my_idct_controller)");
static_assert(offsetof(jpeg_decompress_struct, entropy) == 0x198, "cinfo->entropy (jdhuff.c's huff_entropy_decoder)");
static_assert(offsetof(jpeg_decompress_struct, unread_marker) == 0x17c, "cinfo->unread_marker (jpeg_fill_bit_buffer)");
static_assert(offsetof(jpeg_decompress_struct, marker) == 0x194, "cinfo->marker (process_restart's read_restart_marker)");
static_assert(offsetof(jpeg_decompress_struct, blocks_in_MCU) == 0x140, "cinfo->blocks_in_MCU (decode_mcu)");
static_assert(offsetof(jpeg_decompress_struct, coef_bits) == 0x8c, "cinfo->coef_bits (jinit_phuff_decoder)");

// The method-address shim. Every module's init (and jddctmgr.c's start_pass) stores the addresses of its methods in the structs the
// exe's code also reads and calls through. The library stores its own functions' addresses; the exe's copy of the same code stores the
// exe's. So the library's function that stores them is built under another name (its wrapper unit, third_party/wrap/jpeg-6b, renames
// it <module>_lib_<name>), and the hook row points the exe's address at a shim here with the library's name, which calls it and then
// puts back, field by field, the exe's address of each method the library stored: the memory is then exactly the exe's. In the game
// those addresses are hooked, so a call through one still runs the library's method (or the exe's own where it stays the exe's:
// jpeg_idct_float). The private method fields (jdmerge.c's upmethod, jdsample.c's methods[]) are mapped by offset; their wrapper units
// pin the offsets.
extern "C" {
void __cdecl jdcolor_start_pass_dcolor(j_decompress_ptr cinfo);
void __cdecl jdcolor_lib_jinit_color_deconverter(j_decompress_ptr cinfo);
void __cdecl jdmerge_lib_jinit_merged_upsampler(j_decompress_ptr cinfo);
void __cdecl jdsample_lib_jinit_upsampler(j_decompress_ptr cinfo);
void __cdecl jddctmgr_lib_jinit_inverse_dct(j_decompress_ptr cinfo);
void __cdecl jddctmgr_lib_start_pass(j_decompress_ptr cinfo);
void __cdecl jdhuff_lib_jinit_huff_decoder(j_decompress_ptr cinfo);
void __cdecl jdphuff_lib_jinit_phuff_decoder(j_decompress_ptr cinfo);
void __cdecl jdphuff_lib_start_pass_phuff_decoder(j_decompress_ptr cinfo);
void __cdecl jpeg_idct_float(jpeg_decompress_struct *cinfo, jpeg6b_component_info *compptr, short *coef_block, unsigned char **output_buf,
                             unsigned int output_col);
}

namespace {
struct ExeMethod {
    const void *library;
    std::uintptr_t exe;
};

template <class F> const void *fn(F *f) { return reinterpret_cast<const void *>(f); }

// Each library method an init stores, and the exe's address of the same method (re/libmap.csv).
const ExeMethod kExeMethods[] = {
    // jdcolor.c: start_pass_dcolor is empty, the exe's folded into the shared bare RET at 0x631270
    {fn(&jdcolor_start_pass_dcolor), 0x631270},
    {fn(&jdcolor_ycc_rgb_convert), 0x646b00},
    {fn(&jdcolor_null_convert), 0x646c40},
    {fn(&jdcolor_grayscale_convert), 0x646cd0},
    {fn(&jdcolor_gray_rgb_convert), 0x646d00},
    {fn(&jdcolor_ycck_cmyk_convert), 0x646d70},
    // jdmerge.c
    {fn(&jdmerge_start_pass_merged_upsample), 0x647040},
    {fn(&jdmerge_merged_2v_upsample), 0x647060},
    {fn(&jdmerge_merged_1v_upsample), 0x647130},
    {fn(&jdmerge_h2v1_merged_upsample), 0x647170},
    {fn(&jdmerge_h2v2_merged_upsample), 0x6472f0},
    // jdsample.c
    {fn(&jdsample_start_pass_upsample), 0x6462e0},
    {fn(&jdsample_sep_upsample), 0x646300},
    {fn(&jdsample_fullsize_upsample), 0x646400},
    {fn(&jdsample_noop_upsample), 0x646410},
    {fn(&jdsample_int_upsample), 0x646420},
    {fn(&jdsample_h2v1_upsample), 0x646540},
    {fn(&jdsample_h2v2_upsample), 0x6465a0},
    {fn(&jdsample_h2v1_fancy_upsample), 0x646630},
    {fn(&jdsample_h2v2_fancy_upsample), 0x646700},
    // jddctmgr.c, and the IDCTs its start_pass selects
    {fn(&jddctmgr_lib_start_pass), 0x645b00},
    {fn(&jpeg_idct_islow), 0x64a5c0},
    {fn(&jpeg_idct_ifast), 0x64abb0},
    {fn(&jpeg_idct_float), 0x64b040},
    {fn(&jpeg_idct_4x4), 0x64b5b0},
    {fn(&jpeg_idct_2x2), 0x64b970},
    {fn(&jpeg_idct_1x1), 0x64bb90},
    // jdhuff.c
    {fn(&jdhuff_start_pass_huff_decoder), 0x644710},
    {fn(&jdhuff_decode_mcu), 0x644860},
    // jdphuff.c
    {fn(&jdphuff_lib_start_pass_phuff_decoder), 0x644db0},
    {fn(&jdphuff_decode_mcu_DC_first), 0x645030},
    {fn(&jdphuff_decode_mcu_AC_first), 0x645300},
    {fn(&jdphuff_decode_mcu_DC_refine), 0x645560},
    {fn(&jdphuff_decode_mcu_AC_refine), 0x645660},
};

// The method pointer at slot: the exe's address in place of a library method's (anything else is left as it is).
void exe_method_at(void *slot) {
    void *&p = *static_cast<void **>(slot);
    for (const ExeMethod &m : kExeMethods)
        if (p == m.library) {
            p = reinterpret_cast<void *>(m.exe);
            return;
        }
}

template <class F> void exe_method(F &field) { exe_method_at(&field); }

void *at(void *base, std::size_t offset) { return static_cast<unsigned char *>(base) + offset; }

constexpr std::size_t kMergedUpmethod = 0xc;  // jdmerge.c's my_upsampler.upmethod (pinned in wrap/jpeg-6b/jdmerge.c)
constexpr std::size_t kSampleMethods = 0x34;  // jdsample.c's my_upsampler.methods[] (pinned in wrap/jpeg-6b/jdsample.c)
}  // namespace

extern "C" {

void __cdecl jinit_color_deconverter(j_decompress_ptr cinfo) {
    jdcolor_lib_jinit_color_deconverter(cinfo);
    exe_method(cinfo->cconvert->start_pass);
    exe_method(cinfo->cconvert->color_convert);
}

void __cdecl jinit_merged_upsampler(j_decompress_ptr cinfo) {
    jdmerge_lib_jinit_merged_upsampler(cinfo);
    exe_method(cinfo->upsample->start_pass);
    exe_method(cinfo->upsample->upsample);
    exe_method_at(at(cinfo->upsample, kMergedUpmethod));
}

void __cdecl jinit_upsampler(j_decompress_ptr cinfo) {
    jdsample_lib_jinit_upsampler(cinfo);
    exe_method(cinfo->upsample->start_pass);
    exe_method(cinfo->upsample->upsample);
    for (int ci = 0; ci < cinfo->num_components; ++ci)  // (the library's loop sets methods[ci] for each component)
        exe_method_at(at(cinfo->upsample, kSampleMethods + 4 * ci));
}

void __cdecl jinit_inverse_dct(j_decompress_ptr cinfo) {
    jddctmgr_lib_jinit_inverse_dct(cinfo);
    exe_method(cinfo->idct->start_pass);
}

void __cdecl jddctmgr_start_pass(j_decompress_ptr cinfo) {
    jddctmgr_lib_start_pass(cinfo);
    for (int ci = 0; ci < cinfo->num_components; ++ci)  // (it sets inverse_DCT[ci] for each component)
        exe_method(cinfo->idct->inverse_DCT[ci]);
}

void __cdecl jinit_huff_decoder(j_decompress_ptr cinfo) {
    jdhuff_lib_jinit_huff_decoder(cinfo);
    exe_method(cinfo->entropy->start_pass);
    exe_method(cinfo->entropy->decode_mcu);
}

void __cdecl jinit_phuff_decoder(j_decompress_ptr cinfo) {
    jdphuff_lib_jinit_phuff_decoder(cinfo);
    exe_method(cinfo->entropy->start_pass);
}

void __cdecl jdphuff_start_pass_phuff_decoder(j_decompress_ptr cinfo) {
    jdphuff_lib_start_pass_phuff_decoder(cinfo);
    exe_method(cinfo->entropy->decode_mcu);
}
}
