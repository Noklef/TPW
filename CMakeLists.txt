cmake_minimum_required(VERSION 3.20)
project(tpw CXX)

# The hook phase runs inside the original 32-bit tp.exe, and re/types.h is byte-exact for x86.
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "Build for 32-bit x86 (vcvars32 + Ninja, or -A Win32).")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Static CRT: players shouldn't need a VC++ redistributable.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
add_compile_options(/W4 /permissive- /utf-8)

# Reimplemented game code, shared by the hook DLL and the tests.
add_library(tpw_game STATIC
    src/game/rsse/rsse_vm.cpp
    src/game/core/app.cpp
    src/game/core/crt.cpp
    src/game/core/fpu.cpp
    src/game/core/libc.cpp
    src/game/core/list.cpp
    src/game/core/ptrlist.cpp
    src/game/core/result.cpp
    src/game/core/semaphore.cpp
    src/game/core/string.cpp
    src/game/files/dirsource.cpp
    src/game/files/file.cpp
    src/game/files/handles.cpp
    src/game/files/memsource.cpp
    src/game/files/zlibcodec.cpp
    src/game/files/refpack.cpp
    src/game/files/sources.cpp
    src/game/files/textwriter.cpp
    src/game/files/waddir.cpp
    src/game/files/wadfind.cpp
    src/game/files/wrapsources.cpp
    src/game/files/wadsource.cpp
    src/game/files/bufsource.cpp
    src/game/files/filestream.cpp
    src/game/gfx/palette.cpp
    src/game/gfx/surface.cpp
    src/game/gfx/pixconv.cpp
    src/game/gfx/invmap.cpp
    src/game/gfx/draw8.cpp
    src/game/gfx/drawrgb.cpp
    src/game/gfx/display.cpp
    src/game/gfx/screen.cpp
    src/game/gfx/cursor.cpp
    src/game/gfx/surfbuf.cpp
    src/game/gfx/pointer.cpp
    src/game/core/events.cpp
    src/game/core/filetime.cpp
    src/game/gfx/color.cpp
    src/game/gfx/convert.cpp
    src/game/gfx/jpeg.cpp
    src/game/gfx/imagecodec.cpp
    src/game/gfx/tga.cpp
    src/game/files/streamopen.cpp
    src/game/files/drives.cpp
    src/game/files/quickload.cpp
    src/game/files/textscan.cpp
    src/game/files/wtext.cpp
    src/game/core/regkey.cpp
    src/game/core/stdstring.cpp
    src/game/core/globalmem.cpp
    src/game/core/idtable.cpp
    src/game/core/keymap.cpp
    src/game/gfx/text.cpp
    src/game/gfx/canvas.cpp
    src/game/gfx/canvasdraw.cpp
    src/game/gfx/texdirs.cpp
    src/game/gfx/gauge.cpp
    src/game/gfx/barmesh.cpp
    src/game/gfx/overlay.cpp
    src/game/gfx/screensurfaces.cpp
    src/game/math/matrix.cpp
    src/game/math/vector.cpp
    src/game/math/rect.cpp
    src/game/math/sintable.cpp
    src/game/motion/curve.cpp
    src/game/motion/keys.cpp
    src/game/motion/quat.cpp
    src/game/motion/uvanim.cpp
    src/game/motion/vertexanim.cpp
    src/game/motion/track.cpp
    src/game/motion/animator.cpp
    src/game/motion/normals.cpp
    src/game/sprite/script.cpp
    src/game/sprite/draw.cpp
    src/game/sprite/sprite.cpp
    src/game/sprite/screenmatrix.cpp
    src/game/world/random.cpp
    src/game/world/things.cpp
    src/game/world/camera.cpp
    src/game/world/mousemode.cpp
    src/game/coaster/orient.cpp
    src/game/coaster/rating.cpp
    src/game/coaster/spline.cpp
    src/game/coaster/helpers.cpp
    src/game/coaster/cars.cpp
    src/game/coaster/nodes.cpp
    src/game/coaster/keys.cpp
    src/game/coaster/lists.cpp
    src/game/coaster/queue.cpp
    src/game/coaster/cursor.cpp
    src/game/coaster/registry.cpp
    src/game/coaster/contacts.cpp
    src/game/coaster/train.cpp
    src/game/coaster/save.cpp
    src/game/coaster/xsection.cpp
    src/game/coaster/setup.cpp
    src/game/coaster/ridebounds.cpp
    src/game/terrain/terrain_blend.cpp
    src/game/terrain/land.cpp
    src/game/terrain/procedural.cpp
    src/game/model/parts.cpp
    src/game/model/holder.cpp
    src/game/model/remap.cpp
    src/game/model/highlight.cpp
    src/game/model/partstate.cpp
    src/game/model/scratch.cpp
    src/game/model/clone.cpp
    src/game/model/slots.cpp
    src/game/model/worldverts.cpp
    src/game/model/faces.cpp
    src/game/model/release.cpp
    src/game/model/ridemodel.cpp
    src/game/model/bounds.cpp
    src/game/model/stripanim.cpp
    src/game/model/heightat.cpp
    src/game/render/mode.cpp
    src/game/render/frame.cpp
    src/game/render/screen.cpp
    src/game/online/servers.cpp
)

# Proxy dinput.dll that loads into tp.exe and installs the hooks.
add_library(tpw_hook SHARED
    src/hook/dllmain.cpp
    src/hook/dinput_proxy.cpp
    src/hook/hooks.cpp
    src/hook/patch.cpp
    src/hook/dinput.def
)
set_target_properties(tpw_hook PROPERTIES OUTPUT_NAME dinput)
target_link_libraries(tpw_hook PRIVATE tpw_game)

# Test tooling (tpw.log, the startup trace, the FPU watcher): NOT part of the decompilation.
# It lives in src/testing, links into the hook DLL only, and is never visible to tpw_game.
# OFF builds the DLL as the reimplementation alone. See .docs/06-in-game-testing.md.
option(TPW_TEST_TOOLING "Build the in-game test tooling into the hook DLL" ON)
if(TPW_TEST_TOOLING)
    add_library(tpw_testing STATIC
        src/testing/testing.cpp
        src/testing/log.cpp
        src/testing/trace.cpp
        src/testing/fpu_watch.cpp
    )
    target_link_libraries(tpw_hook PRIVATE tpw_testing)
    target_compile_definitions(tpw_hook PRIVATE TPW_TEST_TOOLING)
endif()

# Guard: game code must never reach the test tooling.
file(GLOB_RECURSE tpw_game_sources CONFIGURE_DEPENDS src/game/*.h src/game/*.cpp)
foreach(f IN LISTS tpw_game_sources)
    file(STRINGS "${f}" hits REGEX "testing/|tpw::testing|TPW_TEST_TOOLING")
    if(hits)
        message(FATAL_ERROR "${f} refers to the test tooling (src/testing), which game code must not use: ${hits}")
    endif()
endforeach()

add_executable(tpw_tests
    tests/test_main.cpp
    src/hook/hooks.cpp
    src/hook/patch.cpp
)
target_link_libraries(tpw_tests PRIVATE tpw_game)

# Comparison driver for tools/fncmp.py: calls hooked functions by their original address,
# fixed at the game's image base so absolute game addresses work as they do in tp.exe.
add_executable(tpw_fncmp
    tools/fncmp/driver.cpp
    tools/fncmp/crt.cpp
    src/hook/hooks.cpp
    src/hook/patch.cpp
)
target_link_libraries(tpw_fncmp PRIVATE tpw_game)
target_link_options(tpw_fncmp PRIVATE /FIXED /DYNAMICBASE:NO /BASE:0x400000)
