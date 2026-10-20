// IJG libjpeg 6b: see jpeg6b.h. The functions are the library's (third_party/jpeg-6b, tpw_jpeg6b); this unit pins the layouts the
// exe's copies share with the library's code while both run (the offsets are the listing's), and holds the method-address shim.
#include "jpeg6b.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>

// (jpeglib.h declares the two table allocators with its untagged JQUANT_TBL / JHUFF_TBL, which jpeg6b.h names by tags of its own:
// jpeglib.h's declarations of them are renamed away here, where nothing calls them)
#define jpeg_alloc_quant_table jpeglib_h_jpeg_alloc_quant_table
#define jpeg_alloc_huff_table jpeglib_h_jpeg_alloc_huff_table
extern "C" {
#define JPEG_INTERNALS
#include "jpeglib.h"
}
#undef jpeg_alloc_quant_table
#undef jpeg_alloc_huff_table

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
static_assert(offsetof(jpeg_decompress_struct, post) == 0x18c, "cinfo->post (jdpostct.c's my_post_controller)");
static_assert(offsetof(jpeg_decompress_struct, main) == 0x184, "cinfo->main (jdmainct.c's my_main_controller)");
static_assert(offsetof(jpeg_decompress_struct, coef) == 0x188, "cinfo->coef (the main controller's decompress_data)");
static_assert(offsetof(jpeg_decompress_struct, min_DCT_scaled_size) == 0x118, "cinfo->min_DCT_scaled_size (the row groups)");
static_assert(offsetof(jpeg_decompress_struct, inputctl) == 0x190, "cinfo->inputctl (the coefficient controller's consume_input)");
static_assert(offsetof(jpeg_decompress_struct, idct) == 0x19c, "cinfo->idct (the coefficient controller's inverse_DCT)");
static_assert(offsetof(jpeg_decompress_struct, Ss) == 0x16c, "cinfo->Ss (decompress_smooth_data)");
static_assert(offsetof(jpeg_decompress_struct, cquantize) == 0x1a8, "cinfo->cquantize (the post-processor's color_quantize)");
static_assert(offsetof(jpeg_decompress_struct, dither_mode) == 0x4c, "cinfo->dither_mode (start_pass_1_quant)");
static_assert(offsetof(jpeg_decompress_struct, desired_number_of_colors) == 0x54, "cinfo->desired_number_of_colors (select_ncolors)");
static_assert(offsetof(jpeg_decompress_struct, out_color_components) == 0x64, "cinfo->out_color_components (the quantizers' nc)");
static_assert(offsetof(jpeg_decompress_struct, enable_2pass_quant) == 0x5a, "cinfo->enable_2pass_quant (jinit_2pass_quantizer)");
static_assert(offsetof(jpeg_decompress_struct, actual_number_of_colors) == 0x70, "cinfo->actual_number_of_colors (start_pass_2_quant)");
static_assert(offsetof(jpeg_decompress_struct, colormap) == 0x74, "cinfo->colormap (find_nearby_colors)");

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
void __cdecl jdpostct_lib_jinit_d_post_controller(j_decompress_ptr cinfo, boolean need_full_buffer);
void __cdecl jdpostct_lib_start_pass_dpost(j_decompress_ptr cinfo, J_BUF_MODE pass_mode);
void __cdecl jdmainct_lib_jinit_d_main_controller(j_decompress_ptr cinfo, boolean need_full_buffer);
void __cdecl jdmainct_lib_start_pass_main(j_decompress_ptr cinfo, J_BUF_MODE pass_mode);
void __cdecl jdcoefct_lib_jinit_d_coef_controller(j_decompress_ptr cinfo, boolean need_full_buffer);
void __cdecl jdcoefct_lib_start_output_pass(j_decompress_ptr cinfo);
int __cdecl jdcoefct_dummy_consume_data(j_decompress_ptr cinfo);  // (not a hook: its exe copy is a shared return 0)
void __cdecl jdmarker_lib_jinit_marker_reader(j_decompress_ptr cinfo);
void __cdecl jdinput_lib_jinit_input_controller(j_decompress_ptr cinfo);
void __cdecl jdinput_lib_finish_input_pass(j_decompress_ptr cinfo);
void __cdecl jdinput_lib_reset_input_controller(j_decompress_ptr cinfo);
void __cdecl jquant1_lib_jinit_1pass_quantizer(j_decompress_ptr cinfo);
void __cdecl jquant1_lib_start_pass_1_quant(j_decompress_ptr cinfo, boolean is_pre_scan);
void __cdecl jquant1_finish_pass_1_quant(j_decompress_ptr cinfo);  // (not a hook: its exe copy is the shared bare RET)
void __cdecl jquant2_lib_jinit_2pass_quantizer(j_decompress_ptr cinfo);
void __cdecl jquant2_lib_start_pass_2_quant(j_decompress_ptr cinfo, boolean is_pre_scan);
void __cdecl jquant2_finish_pass2(j_decompress_ptr cinfo);  // (not a hook: its exe copy is the shared bare RET)
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
    // jdpostct.c (its PASS_THRU post_process_data without quantizing is cinfo->upsample->upsample, the exe's already)
    {fn(&jdpostct_lib_start_pass_dpost), 0x645de0},
    {fn(&jdpostct_post_process_1pass), 0x645ee0},
    {fn(&jdpostct_post_process_prepass), 0x645f60},
    {fn(&jdpostct_post_process_2pass), 0x646010},
    // jdmainct.c
    {fn(&jdmainct_lib_start_pass_main), 0x642cc0},
    {fn(&jdmainct_process_data_simple_main), 0x642ea0},
    {fn(&jdmainct_process_data_context_main), 0x642f10},
    {fn(&jdmainct_process_data_crank_post), 0x643210},
    // jdcoefct.c (its dummy_consume_data, a bare JPEG_SUSPENDED, is the exe's 0x66f4d0: a return 0 the linker shares)
    {fn(&jdcoefct_start_input_pass), 0x643350},
    {fn(&jdcoefct_lib_start_output_pass), 0x6433d0},
    {fn(&jdcoefct_decompress_onepass), 0x643420},
    {fn(&jdcoefct_consume_data), 0x643680},
    {fn(&jdcoefct_decompress_data), 0x643890},
    {fn(&jdcoefct_decompress_smooth_data), 0x643b70},
    {fn(&jdcoefct_dummy_consume_data), 0x66f4d0},
    // jdmarker.c
    {fn(&jdmarker_reset_marker_reader), 0x63d3a0},
    {fn(&jdmarker_read_markers), 0x63c0e0},
    {fn(&jdmarker_read_restart_marker), 0x63d300},
    {fn(&jdmarker_skip_variable), 0x63c030},
    {fn(&jdmarker_get_interesting_appn), 0x63bb80},
    // jdinput.c (its start_input_pass stores the coefficient controller's consume_data, the exe's already)
    {fn(&jdinput_consume_markers), 0x63b5e0},
    {fn(&jdinput_lib_reset_input_controller), 0x63b8b0},
    {fn(&jdinput_start_input_pass), 0x63b310},
    {fn(&jdinput_lib_finish_input_pass), 0x63b5c0},
    // jquant1.c (its finish_pass_1_quant is empty: the shared bare RET at 0x631270)
    {fn(&jquant1_lib_start_pass_1_quant), 0x648db0},
    {fn(&jquant1_finish_pass_1_quant), 0x631270},
    {fn(&jquant1_new_color_map_1_quant), 0x649530},
    {fn(&jquant1_color_quantize), 0x648f80},
    {fn(&jquant1_color_quantize3), 0x649040},
    {fn(&jquant1_quantize_ord_dither), 0x649100},
    {fn(&jquant1_quantize3_ord_dither), 0x649230},
    {fn(&jquant1_quantize_fs_dither), 0x649360},
    // jquant2.c (its finish_pass2 is empty: the shared bare RET at 0x631270)
    {fn(&jquant2_lib_start_pass_2_quant), 0x6476c0},
    {fn(&jquant2_new_color_map_2_quant), 0x6488e0},
    {fn(&jquant2_prescan_quantize), 0x6477e0},
    {fn(&jquant2_finish_pass1), 0x6480a0},
    {fn(&jquant2_pass2_no_dither), 0x647860},
    {fn(&jquant2_pass2_fs_dither), 0x647db0},
    {fn(&jquant2_finish_pass2), 0x631270},
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
constexpr std::size_t kMarkerProcessCOM = 0x18;   // jdmarker.c's my_marker_reader.process_COM (pinned in wrap/jpeg-6b/jdmarker.c)
constexpr std::size_t kMarkerProcessAPPn = 0x1c;  // and its process_APPn[16]
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

void __cdecl jinit_d_post_controller(j_decompress_ptr cinfo, boolean need_full_buffer) {
    jdpostct_lib_jinit_d_post_controller(cinfo, need_full_buffer);
    exe_method(cinfo->post->start_pass);
}

void __cdecl jdpostct_start_pass_dpost(j_decompress_ptr cinfo, int pass_mode) {
    jdpostct_lib_start_pass_dpost(cinfo, static_cast<J_BUF_MODE>(pass_mode));
    exe_method(cinfo->post->post_process_data);
}

void __cdecl jinit_d_main_controller(j_decompress_ptr cinfo, boolean need_full_buffer) {
    jdmainct_lib_jinit_d_main_controller(cinfo, need_full_buffer);
    exe_method(cinfo->main->start_pass);
}

void __cdecl jdmainct_start_pass_main(j_decompress_ptr cinfo, int pass_mode) {
    jdmainct_lib_start_pass_main(cinfo, static_cast<J_BUF_MODE>(pass_mode));
    exe_method(cinfo->main->process_data);
}

void __cdecl jinit_d_coef_controller(j_decompress_ptr cinfo, boolean need_full_buffer) {
    jdcoefct_lib_jinit_d_coef_controller(cinfo, need_full_buffer);
    exe_method(cinfo->coef->start_input_pass);
    exe_method(cinfo->coef->consume_data);
    exe_method(cinfo->coef->start_output_pass);
    exe_method(cinfo->coef->decompress_data);
}

void __cdecl jdcoefct_start_output_pass(j_decompress_ptr cinfo) {
    jdcoefct_lib_start_output_pass(cinfo);
    exe_method(cinfo->coef->decompress_data);
}

void __cdecl jinit_marker_reader(j_decompress_ptr cinfo) {
    jdmarker_lib_jinit_marker_reader(cinfo);
    exe_method(cinfo->marker->reset_marker_reader);
    exe_method(cinfo->marker->read_markers);
    exe_method(cinfo->marker->read_restart_marker);
    exe_method_at(at(cinfo->marker, kMarkerProcessCOM));
    for (int i = 0; i < 16; ++i)
        exe_method_at(at(cinfo->marker, kMarkerProcessAPPn + 4 * i));
}

void __cdecl jinit_input_controller(j_decompress_ptr cinfo) {
    jdinput_lib_jinit_input_controller(cinfo);
    exe_method(cinfo->inputctl->consume_input);
    exe_method(cinfo->inputctl->reset_input_controller);
    exe_method(cinfo->inputctl->start_input_pass);
    exe_method(cinfo->inputctl->finish_input_pass);
}

void __cdecl jdinput_reset_input_controller(j_decompress_ptr cinfo) {
    jdinput_lib_reset_input_controller(cinfo);
    exe_method(cinfo->inputctl->consume_input);
}

void __cdecl jdinput_finish_input_pass(j_decompress_ptr cinfo) {
    jdinput_lib_finish_input_pass(cinfo);
    exe_method(cinfo->inputctl->consume_input);
}

void __cdecl jinit_1pass_quantizer(j_decompress_ptr cinfo) {
    jquant1_lib_jinit_1pass_quantizer(cinfo);
    exe_method(cinfo->cquantize->start_pass);
    exe_method(cinfo->cquantize->finish_pass);
    exe_method(cinfo->cquantize->new_color_map);
}

void __cdecl jquant1_start_pass_1_quant(j_decompress_ptr cinfo, boolean is_pre_scan) {
    jquant1_lib_start_pass_1_quant(cinfo, is_pre_scan);
    exe_method(cinfo->cquantize->color_quantize);
}

void __cdecl jinit_2pass_quantizer(j_decompress_ptr cinfo) {
    jquant2_lib_jinit_2pass_quantizer(cinfo);
    exe_method(cinfo->cquantize->start_pass);
    exe_method(cinfo->cquantize->new_color_map);
}

void __cdecl jquant2_start_pass_2_quant(j_decompress_ptr cinfo, boolean is_pre_scan) {
    jquant2_lib_start_pass_2_quant(cinfo, is_pre_scan);
    exe_method(cinfo->cquantize->color_quantize);
    exe_method(cinfo->cquantize->finish_pass);
}
}

// jmemansi.c's malloc and free (third_party/wrap/jpeg-6b/jmemansi.c #defines them to these two): the exe's, so the blocks the library's
// jpeg_get_small hands out are the exe's heap's, which the exe's own code (zcfree, jmemmgr.c while it stays the exe's) frees there.
namespace {
const Jpeg6bHeapEnv kExeHeap = {reinterpret_cast<void *(__cdecl *)(size_t)>((uintptr_t)0x0067dcd0),
                                reinterpret_cast<void(__cdecl *)(void *)>((uintptr_t)0x0067ad90)};
const Jpeg6bHeapEnv *g_heap = &kExeHeap;
}  // namespace

void Jpeg6b_SetHeapEnv(const Jpeg6bHeapEnv *env) { g_heap = env ? env : &kExeHeap; }

extern "C" {
void *__cdecl jpeg6b_exe_malloc(size_t size) { return g_heap->alloc(size); }
void __cdecl jpeg6b_exe_free(void *block) { g_heap->free(block); }
}
