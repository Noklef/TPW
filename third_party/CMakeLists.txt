# The original's libraries (README.md), as static libraries built from the unmodified trees. Nothing links them yet: the game's copies
# are mapped and benched against these first (tools/fncmp), then the game's addresses can be pointed at them.

# Released C, so the project's own warning level is not theirs (the inherited /W4 is dropped rather than overridden, which cl warns about).
get_directory_property(TPW_INHERITED_OPTIONS COMPILE_OPTIONS)
list(REMOVE_ITEM TPW_INHERITED_OPTIONS /W4)
set_directory_properties(PROPERTIES COMPILE_OPTIONS "${TPW_INHERITED_OPTIONS}")
set(TPW_THIRD_PARTY_OPTIONS /W0 /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_WARNINGS)

# zlib 1.1.3 (Makefile's OBJS; minigzip, example and maketree are programs).
set(ZLIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/zlib-1.1.3)
add_library(tpw_zlib STATIC
    ${ZLIB_DIR}/adler32.c ${ZLIB_DIR}/compress.c ${ZLIB_DIR}/crc32.c ${ZLIB_DIR}/gzio.c ${ZLIB_DIR}/uncompr.c ${ZLIB_DIR}/deflate.c
    ${ZLIB_DIR}/trees.c ${ZLIB_DIR}/zutil.c ${ZLIB_DIR}/inflate.c ${ZLIB_DIR}/infblock.c ${ZLIB_DIR}/inftrees.c ${ZLIB_DIR}/infcodes.c
    ${ZLIB_DIR}/infutil.c ${ZLIB_DIR}/inffast.c)
target_include_directories(tpw_zlib PUBLIC ${ZLIB_DIR})
target_compile_options(tpw_zlib PRIVATE ${TPW_THIRD_PARTY_OPTIONS})
# -Dlocal= (zutil.h's own switch) gives the static functions and tables external names, so the hook table can point the exe's copies
# of them at these. The code is the same.
target_compile_definitions(tpw_zlib PRIVATE local=)
# config/zlib_shims.h renames the functions src/game/lib/zlib.cpp shims (they leave the library's own tables, strings or adler32 where
# the exe's code looks) to zlib_*, so the shims can have the library's names. The code is the same.
target_include_directories(tpw_zlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/config)
target_compile_options(tpw_zlib PRIVATE /FI${CMAKE_CURRENT_SOURCE_DIR}/config/zlib_shims.h)

# libjpeg: makefile.vc's LIBOBJECTS, and the memory manager's system-dependent backend as the copy in the exe has it: 6b's is
# jmemansi.c (jpeg_mem_init's 1000000 at 0x642b20, jpeg_mem_available's max_memory_to_use - already_allocated, a tmpfile() backing
# store), 6a keeps jmemnobs.c until its own libmap rows say. config/jconfig.h is jconfig.vc (see its header).
set(JPEG_LIBOBJECTS jcapimin jcapistd jctrans jcparam jdatadst jcinit jcmaster jcmarker jcmainct jcprepct jccoefct jccolor jcsample
    jchuff jcphuff jcdctmgr jfdctfst jfdctflt jfdctint jdapimin jdapistd jdtrans jdatasrc jdmaster jdinput jdmarker jdhuff jdphuff
    jdmainct jdcoefct jdpostct jddctmgr jidctfst jidctflt jidctint jidctred jdsample jdcolor jquant1 jquant2 jdmerge jcomapi jutils
    jerror jmemmgr)

# A module with a wrapper unit, wrap/<tree>/<module>.c, is compiled through it: the wrapper #includes the module's .c, unchanged, and
# makes its static functions external as <module>_<name> (jmorecfg.h defines METHODDEF and LOCAL as static unconditionally, and the
# modules reuse static names such as start_pass), so the hook table can point the exe's copies of them at the library's.
function(tpw_jpeg target tree backend)
    set(sources)
    foreach(name ${JPEG_LIBOBJECTS} ${backend})
        if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/wrap/${tree}/${name}.c)
            list(APPEND sources ${CMAKE_CURRENT_SOURCE_DIR}/wrap/${tree}/${name}.c)
        else()
            list(APPEND sources ${CMAKE_CURRENT_SOURCE_DIR}/${tree}/${name}.c)
        endif()
    endforeach()
    add_library(${target} STATIC ${sources})
    # config/ first, so its jconfig.h is the one found (the trees have none of their own)
    target_include_directories(${target} BEFORE PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/config ${CMAKE_CURRENT_SOURCE_DIR}/${tree})
    target_compile_options(${target} PRIVATE ${TPW_THIRD_PARTY_OPTIONS})
endfunction()

# 6b keeps its names; 6a's are renamed jpeg6a_* (config/jpeg6a_prefix.h, written by tools/jpegprefix.py) so the two copies link together.
tpw_jpeg(tpw_jpeg6b jpeg-6b jmemansi)
tpw_jpeg(tpw_jpeg6a jpeg-6a jmemnobs)
target_compile_options(tpw_jpeg6a PRIVATE /FI${CMAKE_CURRENT_SOURCE_DIR}/config/jpeg6a_prefix.h)
