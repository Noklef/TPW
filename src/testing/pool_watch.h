// Double-release watcher (test tooling): names the second release of a sound stream / sample block, which the exit crash in the MP2
// sample pool's purge (tpw.log 2026-10-28, a free-list block in decommitted heap) points at. It watches the game's block pools
// (src/game/core/streampool's lists) from outside: the LeaveCriticalSection import slot sees each pool's list change under its lock, and the
// heap's free (0x67ad90) and operator new (0x67acc0) are patched to note what is freed. Logged to tpw.log, only when wrong:
//   pool watch: <pool> block B released twice ...      it was on the pool's free list already
//   pool watch: <pool> block B released, already freed ...  the heap had it (and no operator new gave it back since)
//   pool watch: heap free of <pool> block B ...        it is on the pool's free list
// Each line names the callers: "game XXXXXX" an address in tp.exe, "dll+XXXXX" an offset for tools/dllsym.py. It changes nothing the game
// sees. tpw.ini [tpw] poolwatch=0 turns it off.
#pragma once

#include <windows.h>

namespace tpw::testing {

// `dll` is the hook DLL (its offsets are what the log prints).
void start_pool_watch(HINSTANCE dll);

}  // namespace tpw::testing
