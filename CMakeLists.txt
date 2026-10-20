cmake_minimum_required(VERSION 3.20)
project(tpw C CXX)

# The hook phase runs inside the original 32-bit tp.exe, and re/types.h is byte-exact for x86.
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "Build for 32-bit x86 (vcvars32 + Ninja, or -A Win32).")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Static CRT: players shouldn't need a VC++ redistributable.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
add_compile_options(/W4 /permissive- /utf-8)

# The libraries the original links (zlib, libjpeg 6b and 6a), from their released sources.
add_subdirectory(third_party)

# Reimplemented game code, shared by the hook DLL and the tests.
add_library(tpw_game STATIC
    src/game/rsse/rsse_vm.cpp
    src/game/core/app.cpp
    src/game/core/crt.cpp
    src/game/core/fpu.cpp
    src/game/core/libc.cpp
    src/game/core/list.cpp
    src/game/core/defids.cpp
    src/game/core/gameheap.cpp
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
    src/game/files/gamepaths.cpp
    src/game/files/direntry.cpp
    src/game/files/userslots.cpp
    src/game/files/chunkfile.cpp
    src/game/core/regkey.cpp
    src/game/core/stdstring.cpp
    src/game/core/globalmem.cpp
    src/game/core/idtable.cpp
    src/game/core/keymap.cpp
    src/game/core/ime.cpp
    src/game/core/wtraits.cpp
    src/game/core/language.cpp
    src/game/core/options.cpp
    src/game/core/gameclock.cpp
    src/game/core/circbuf.cpp
    src/game/core/textarg.cpp
    src/game/core/soundset.cpp
    src/game/core/globalsave.cpp
    src/game/core/liveobjects.cpp
    src/game/core/trivial.cpp
    src/game/core/dequeiter.cpp
    src/game/core/accessors.cpp
    src/game/core/volumes.cpp
    src/game/core/misc.cpp
    src/game/core/cheatmodule.cpp
    src/game/core/forwarders.cpp
    src/game/core/base64.cpp
    src/game/core/selectors.cpp
    src/game/core/fieldops.cpp
    src/game/core/methods.cpp
    src/game/core/cursors.cpp
    src/game/core/seqs.cpp
    src/game/core/indexed.cpp
    src/game/core/records16.cpp
    src/game/core/vwrap.cpp
    src/game/core/vdtor.cpp
    src/game/core/vlist.cpp
    src/game/core/vstream.cpp
    src/game/core/vregs.cpp
    src/game/core/vobj2.cpp
    src/game/core/vobj3.cpp
    src/game/core/vdel.cpp
    src/game/core/stdlist.cpp
    src/game/core/codecvt.cpp
    src/game/core/savemods.cpp
    src/game/core/idmaps.cpp
    src/game/core/stdtree.cpp
    src/game/core/class6fd9d8.cpp
    src/game/core/wstringmap.cpp
    src/game/core/buffers.cpp
    src/game/core/framestubs.cpp
    src/game/core/globals.cpp
    src/game/core/initializers.cpp
    src/game/core/observers.cpp
    src/game/core/rings.cpp
    src/game/core/streambuf.cpp
    src/game/core/thunks.cpp
    src/game/core/voices.cpp
    src/game/core/vtables.cpp
    src/game/core/factories.cpp
    src/game/core/vanillatime.cpp
    src/game/core/actionrec.cpp
    src/game/core/namedobjects.cpp
    src/game/core/savegame.cpp
    src/game/core/balancefile.cpp
    src/game/core/timesig.cpp
    src/game/core/floatconsts.cpp
    src/game/core/funnytime.cpp
    src/game/core/sync.cpp
    src/game/core/cpuinfo.cpp
    src/game/core/stdwstring.cpp
    src/game/core/gameerror.cpp
    src/game/core/countedpair.cpp
    src/game/core/stringmapsrecord.cpp
    src/game/core/cmdline.cpp
    src/game/core/framecountdown.cpp
    src/game/ui/panel7cc328.cpp
    src/game/core/wscanner.cpp
    src/game/core/soundplay.cpp
    src/game/core/startcheck.cpp
    src/game/gfx/text.cpp
    src/game/gfx/canvas.cpp
    src/game/gfx/canvasdraw.cpp
    src/game/gfx/texdirs.cpp
    src/game/gfx/gauge.cpp
    src/game/gfx/barmesh.cpp
    src/game/gfx/overlay.cpp
    src/game/gfx/screensurfaces.cpp
    src/game/gfx/texcache.cpp
    src/game/gfx/texset.cpp
    src/game/gfx/texman.cpp
    src/game/gfx/texture.cpp
    src/game/gfx/spriteframe.cpp
    src/game/gfx/animtex.cpp
    src/game/gfx/wavelet.cpp
    src/game/gfx/texsurface.cpp
    src/game/gfx/array3d.cpp
    src/game/gfx/imageops.cpp
    src/game/gfx/statsgraph.cpp
    src/game/gfx/rleimage.cpp
    src/game/gfx/texsprite.cpp
    src/game/gfx/texatlas.cpp
    src/game/gfx/wctimage.cpp
    src/game/gfx/screengrab.cpp
    src/game/gfx/tvstatic.cpp
    src/game/gfx/loadscreen.cpp
    src/game/gfx/surfacepool.cpp
    src/game/gfx/textlayout.cpp
    src/game/math/matrix.cpp
    src/game/math/vector.cpp
    src/game/math/rect.cpp
    src/game/math/sintable.cpp
    src/game/math/intpair.cpp
    src/game/math/intsin.cpp
    src/game/math/mat4.cpp
    src/game/math/bits.cpp
    src/game/math/vec2.cpp
    src/game/math/geometry.cpp
    src/game/math/transform.cpp
    src/game/motion/curve.cpp
    src/game/motion/keys.cpp
    src/game/motion/quat.cpp
    src/game/motion/uvanim.cpp
    src/game/motion/vertexanim.cpp
    src/game/motion/track.cpp
    src/game/motion/animator.cpp
    src/game/motion/normals.cpp
    src/game/motion/placement.cpp
    src/game/sprite/script.cpp
    src/game/sprite/draw.cpp
    src/game/sprite/sprite.cpp
    src/game/sprite/screenmatrix.cpp
    src/game/sprite/scriptref.cpp
    src/game/sprite/loading.cpp
    src/game/world/random.cpp
    src/game/world/things.cpp
    src/game/world/camera.cpp
    src/game/world/mousemode.cpp
    src/game/world/objectdefs.cpp
    src/game/world/thingmsg.cpp
    src/game/world/staff.cpp
    src/game/world/advisor.cpp
    src/game/world/analyser.cpp
    src/game/world/mapcell.cpp
    src/game/world/challenges.cpp
    src/game/world/object.cpp
    src/game/world/person.cpp
    src/game/world/research.cpp
    src/game/world/nav.cpp
    src/game/world/world.cpp
    src/game/world/collobj.cpp
    src/game/world/trackrides.cpp
    src/game/world/flyingrides.cpp
    src/game/world/analysercell.cpp
    src/game/world/linestep.cpp
    src/game/world/personscripts.cpp
    src/game/world/dirstep.cpp
    src/game/world/thingscan.cpp
    src/game/world/ridelist.cpp
    src/game/world/trackpools.cpp
    src/game/world/money.cpp
    src/game/world/gamesystem.cpp
    src/game/world/ridesearch.cpp
    src/game/world/ridesweep.cpp
    src/game/world/objectmsg.cpp
    src/game/world/litter.cpp
    src/game/world/messagecentre.cpp
    src/game/world/steering.cpp
    src/game/world/tagsystem.cpp
    src/game/world/ridestate.cpp
    src/game/world/forecast.cpp
    src/game/world/headquarters.cpp
    src/game/world/wallfollow.cpp
    src/game/world/route.cpp
    src/game/world/focuscell.cpp
    src/game/world/hoardinggrid.cpp
    src/game/world/mapline.cpp
    src/game/world/cellrun.cpp
    src/game/world/parkmap.cpp
    src/game/world/cellmodel.cpp
    src/game/world/weather.cpp
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
    src/game/coaster/tracktypes.cpp
    src/game/coaster/edittools.cpp
    src/game/terrain/terrain_blend.cpp
    src/game/terrain/land.cpp
    src/game/terrain/procedural.cpp
    src/game/terrain/landscape.cpp
    src/game/terrain/texcache.cpp
    src/game/terrain/normals.cpp
    src/game/terrain/nodecells.cpp
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
    src/game/model/m3d2.cpp
    src/game/model/objmodel.cpp
    src/game/model/heightmap.cpp
    src/game/render/mode.cpp
    src/game/render/frame.cpp
    src/game/render/screen.cpp
    src/game/render/particles.cpp
    src/game/render/state.cpp
    src/game/render/weatherfx.cpp
    src/game/render/renderer.cpp
    src/game/render/backdrop.cpp
    src/game/render/meshqueue.cpp
    src/game/render/spritequeue.cpp
    src/game/render/drawbuffer.cpp
    src/game/render/texformat.cpp
    src/game/render/lightlist.cpp
    src/game/render/softrender.cpp
    src/game/render/meshlist.cpp
    src/game/render/batch.cpp
    src/game/render/rasteriser.cpp
    src/game/render/dynfaces.cpp
    src/game/render/lightsource.cpp
    src/game/render/clipcodes.cpp
    src/game/render/dxerrors.cpp
    src/game/render/viewport.cpp
    src/game/render/heightgrid.cpp
    src/game/render/vertexfog.cpp
    src/game/render/quad.cpp
    src/game/render/d3dprobe.cpp
    src/game/render/device.cpp
    src/game/render/engineerrors.cpp
    src/game/render/texcoords.cpp
    src/game/render/graphicssettings.cpp
    src/game/render/textparts.cpp
    src/game/render/worldview.cpp
    src/game/render/fixedverts.cpp
    src/game/render/vertexblock.cpp
    src/game/render/clipedge.cpp
    src/game/render/viewquad.cpp
    src/game/render/depthsort.cpp
    src/game/render/clippoly.cpp
    src/game/render/transformer.cpp
    src/game/render/meshdraws.cpp
    src/game/online/servers.cpp
    src/game/online/session.cpp
    src/game/online/itemlist.cpp
    src/game/online/items.cpp
    src/game/online/onlinelist.cpp
    src/game/online/chatavatar.cpp
    src/game/online/ftp.cpp
    src/game/online/chat.cpp
    src/game/frontend/lobbymodel.cpp
    src/game/frontend/helpbg.cpp
    src/game/frontend/lobbyscene.cpp
    src/game/frontend/viewcontrols.cpp
    src/game/input/mouse.cpp
    src/game/ui/dialog7ca1c8.cpp
    src/game/ui/window.cpp
    src/game/ui/dialogs.cpp
    src/game/ui/guimanager.cpp
    src/game/ui/guirect.cpp
    src/game/ui/guidialog.cpp
    src/game/ui/msgqueue.cpp
    src/game/ui/textpool.cpp
    src/game/ui/guitimers.cpp
    src/game/ui/panel7b9940.cpp
    src/game/ui/windowreg.cpp
    src/game/ui/windowtimers.cpp
    src/game/ui/window705a50.cpp
    src/game/ui/thingpreview.cpp
    src/game/ui/listrows.cpp
    src/game/ui/frame.cpp
    src/game/ui/dialog7cb398.cpp
    src/game/ui/panel7ca228.cpp
    src/game/ui/cursor.cpp
    src/game/ui/textbutton.cpp
    src/game/ui/focus.cpp
    src/game/ui/radiogroup.cpp
    src/game/ui/panel7cc298.cpp
    src/game/ui/dialog7cb2fc.cpp
    src/game/ui/window705b20.cpp
    src/game/ui/dialogbase.cpp
    src/game/ui/window705b98.cpp
    src/game/ui/window705c58.cpp
    src/game/ui/window705bf8.cpp
    src/game/ui/window705c28.cpp
    src/game/ui/window705af0.cpp
    src/game/ui/window705bc8.cpp
    src/game/ui/window705a20.cpp
    src/game/ui/window705950.cpp
    src/game/ui/window7059f0.cpp
    src/game/ui/windowfactory.cpp
    src/game/ui/windowscript.cpp
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
# Not incremental: an incremental link keeps an old, larger .bss when a static shrinks, and a case's wild pointer reads the driver's own
# memory natively where the emulator has none, so the layout has to follow the source.
target_link_options(tpw_fncmp PRIVATE /FIXED /DYNAMICBASE:NO /BASE:0x400000 /MAP /INCREMENTAL:NO)  # (the map names where FNCMP_CRASHAT's crashes are)
