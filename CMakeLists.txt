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
    src/game/core/libc.cpp
    src/game/core/ptrlist.cpp
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
    src/game/world/random.cpp
    src/game/world/things.cpp
    src/game/terrain/terrain_blend.cpp
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

add_executable(tpw_tests
    tests/test_main.cpp
    src/hook/hooks.cpp
    src/hook/patch.cpp
)
target_link_libraries(tpw_tests PRIVATE tpw_game)
