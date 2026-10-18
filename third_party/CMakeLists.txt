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

# libjpeg: makefile.vc's LIBOBJECTS, with jmemnobs as the memory manager. config/jconfig.h is jconfig.vc (see its header).
set(JPEG_LIBOBJECTS jcapimin jcapistd jctrans jcparam jdatadst jcinit jcmaster jcmarker jcmainct jcprepct jccoefct jccolor jcsample
    jchuff jcphuff jcdctmgr jfdctfst jfdctflt jfdctint jdapimin jdapistd jdtrans jdatasrc jdmaster jdinput jdmarker jdhuff jdphuff
    jdmainct jdcoefct jdpostct jddctmgr jidctfst jidctflt jidctint jidctred jdsample jdcolor jquant1 jquant2 jdmerge jcomapi jutils
    jerror jmemmgr jmemnobs)

function(tpw_jpeg target tree)
    set(sources)
    foreach(name ${JPEG_LIBOBJECTS})
        list(APPEND sources ${CMAKE_CURRENT_SOURCE_DIR}/${tree}/${name}.c)
    endforeach()
    add_library(${target} STATIC ${sources})
    # config/ first, so its jconfig.h is the one found (the trees have none of their own)
    target_include_directories(${target} BEFORE PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/config ${CMAKE_CURRENT_SOURCE_DIR}/${tree})
    target_compile_options(${target} PRIVATE ${TPW_THIRD_PARTY_OPTIONS})
endfunction()

# 6b keeps its names; 6a's are renamed jpeg6a_* (config/jpeg6a_prefix.h, written by tools/jpegprefix.py) so the two copies link together.
tpw_jpeg(tpw_jpeg6b jpeg-6b)
tpw_jpeg(tpw_jpeg6a jpeg-6a)
target_compile_options(tpw_jpeg6a PRIVATE /FI${CMAKE_CURRENT_SOURCE_DIR}/config/jpeg6a_prefix.h)
