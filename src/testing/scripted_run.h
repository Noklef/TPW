// TEST TOOLING, NOT PART OF THE DECOMPILATION.
//
// A run of the original game without a person: with tpw.ini [tpw] script=1 the mouse the game reads through DirectInput is
// the script's (the real mouse is left out), and the frame on the screen is saved as a .bmp each shot_ms (default 2000) into
// the folder [tpw] out (with its trailing backslash). The script: clicks through the front end to the last park, then the
// camera button and the walk-through view. Made to compare a view of the original with the recompilation's frame dump
// (tpw-recompilation, docs/improvements-2.md, entry 4). No screen capture, no input to other programs.
#pragma once

#include <windows.h>

namespace tpw::testing {

// Reads the switches from tpw.ini. Called from after_hooks.
void start_scripted_run(const char *ini);

// The DirectInput object the game just made: its CreateDevice is wrapped, to wrap the mouse device's two reads.
void wrap_direct_input(void *input);

}  // namespace tpw::testing
