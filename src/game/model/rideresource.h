// A ride's resource loaded: its files, its model, its sign and its height map, the model's parts marked and its slot in the table at 0x7ae6a8.
// Reimplementation of 0x4629d0. Name invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// Its calls (the ports, or the exe's where unported; the bench runs them as recorders).
struct RideResourceEnv {
    void(__cdecl *modelPath)(char *out);                                                                        // 0x46e650 (unported)
    u32(__cdecl *quickload)(const char *name, const char *folder, const char *c);                               // Quickload_LoadRide (0x461a60)
    u32(__cdecl *load)(const char *name, u32 flags, u8 **out, char *outName, u32 layers);                       // RideModel_Load (0x461f10)
    void(__cdecl *prepare)(const u8 *object, const u8 *owner, const u8 *cells, s32 width, s32 height);          // RideBase_Prepare (0x46a5a0)
    void(__cdecl *markTextures)(u32 unused, const u8 *face, u32 set);                                           // Face_MarkTextures (0x46a550)
    void(__cdecl *hoarding)(const char *level, u8 *object, const u8 *grid);                                     // HoardingMesh_Build (0x452e70)
    void(__cdecl *markFaces)(const u8 *const *holder);                                                          // Model_MarkFlag80Faces (0x44ba60)
    void(__cdecl *heightMap)(u8 *model, u32 width, u32 height, const char *name, const u8 *cells, u32 flag800); // HeightMap_LoadOrBuild (0x451640, unported)
    u32(__cdecl *fontsFolder)(char *out);                                                                       // GamePaths_FontsFolder (0x468250)
    u8 *(__fastcall *findDef)(u8 *defs, void *, u16 id);                                                        // ObjectDefs_FindAt410 (0x412f50)
    void(__cdecl *sign)(u8 *model, const char *folder, const char *name, const char *fonts, const u8 *def, u32 signArg);  // Sign_Apply (0x467a80, unported)
};
void RideResource_SetEnv(const RideResourceEnv *env);

// 0x4629d0 (cdecl; eleven arguments; name invented, unconfirmed): name with ".md2" unless it has an extension (a last '.' not before '\'), Quickload_LoadRide of that with folder and the
// path 0x46e650 gives; then, when that answers 1, RideModel_Load of name with the flags translated (0x10 -> 2, 0x20 -> 0x10, 0x40 -> 0x20, 2 -> 0x40, 0x100 -> 1, 0x200 -> 0x100, 4 -> 0x80,
// 0x4000 -> 0x400, 0x10000 -> 0x800, 0x40000 -> 0x1000, 0x100000 -> 0x2000, 0x200000 -> 0x4000, 0x400000 -> 0x8000, 0x1000000 -> 0x20000), layers 1 when 0; the model's +4 gets bit 1 whatever it answers
// (sic: a failed load's pointer is the stack's). With flag 0x20000, "p<name>" loaded the same way into its +0xd0. Anything but 1: 0.
// Width and height: their arguments, else the cells' first two dwords, else 0; kept with id at +0xf0, +0xf4, +0xf8. Flag 1 RideBase_Prepare; 0x2000 Face_MarkTextures on both models; the largest
// part count ever goes to 0x79f7d0 (+8's +0x2c's +0x58, plus each marked part's), and each extra (+8's +0x28's +4) of a part with flags 0x580f00, or under flag 0x400000, gets bit 3; flag 8 with a grid HoardingMesh_Build;
// 0x80 Model_MarkFlag80Faces; 0x400 HeightMap_LoadOrBuild of the name RideModel_Load gave, its extension ".hmp". With the fonts folder, Sign_Apply (def, signArg when def is given; else the def of id, 0).
// Then each part: flag 0x200 sets 0x80000010, 0x20040 sets the model's 0x80, without 0x200 its textures of bit 7 get bit 1; a chain of all flag 0x200 and +0xc zero sets 0x20. Each of its entries by +0x36.
// Returns the new count at 0x7a439c, the model in the table at 0x7ae6a8.
extern "C" {
u32 __cdecl Ride_LoadResource(const char *folder, const char *name, const u8 *cells, const u8 *grid, u32 flags, u32 id, u32 layers, const u8 *def, u32 signArg, u32 width, u32 height);
}
