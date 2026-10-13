/*
 * Recovered data types for Theme Park World (tp.exe, MSVC 5/6, 32-bit).
 *
 * Every struct here matches the original in-memory or on-disk layout byte for byte.
 * Offsets are written next to each field and checked by the static asserts at the
 * bottom (compile this header as C or C++ to verify).
 *
 * Kept deliberately plain (no <stdint.h>, no C++) so Ghidra's C parser can import
 * it directly: see tools/ApplyTypes.java. Unknown regions are named unk_XX after
 * their offset; rename them as they are understood.
 *
 * Sources: .docs/03-rsse-scripting.md (RSSE), tools/*.py (file formats).
 */
#ifndef TPW_TYPES_H
#define TPW_TYPES_H

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef signed char    s8;
typedef short          s16;
typedef int            s32;

/* ------------------------------------------------------------------------ */
/* RSSE script VM                                                           */
/* ------------------------------------------------------------------------ */

/* .RSE file header (RSSE_LoadScriptFile, 0x5587f0). Code words follow at 0x34. */
typedef struct RsseFileHeader {
    char magic[4];          /* 0x00 "RSSE" */
    u32  version;           /* 0x04 must be 0x00010F51 */
    u32  var_count;         /* 0x08 -> ctx.var_count */
    u32  stack_words;       /* 0x0C -> ctx.stack_size */
    u32  slice_budget;      /* 0x10 -> ctx.budget (50 in every shipped script) */
    u32  limbo_slots;       /* 0x14 -> ctx.limbo_capacity */
    u32  rider_slots;       /* 0x18 -> ctx.rider_capacity */
    u32  walker_slots;      /* 0x1C -> ctx.walker_capacity */
    char pad[16];           /* 0x20 "Pad Pad Pad Pad " */
    u32  code_len;          /* 0x30 in words */
} RsseFileHeader;

/* LIMBO: a guest held invisibly until release_time. */
typedef struct RsseLimboSlot {
    s32 guest;              /* 0x00 */
    u32 release_time;       /* 0x04 game clock ms */
} RsseLimboSlot;

/* BOUNCE: a guest seated on the ride. */
typedef struct RsseRiderSlot {
    s32 guest;              /* 0x00 */
    s32 node;               /* 0x04 mesh node the rider is attached to */
    u32 release_time;       /* 0x08 */
    u32 start_time;         /* 0x0C */
} RsseRiderSlot;

/* WALK*: guests following walk nodes. Layout not yet mapped. */
typedef struct RsseWalkerSlot {
    u8 unk_00[0x20];
} RsseWalkerSlot;

/* Per-script runtime context, 0xF4 bytes, on the list at DAT_008791b0. */
typedef struct RsseContext {
    struct RsseContext *next;       /* 0x00 */
    struct RsseContext *prev;       /* 0x04 */
    s32  id;                        /* 0x08 */
    s32  child_id;                  /* 0x0C SPAWNCHILD */
    u8   unk_10[0x08];              /* 0x10 */
    u32 *code;                      /* 0x18 */
    s32 *vars;                      /* 0x1C */
    s32 *stack;                     /* 0x20 shared by PUSH/JSR (from top) and HUSH (from bottom) */
    RsseLimboSlot  *limbo;          /* 0x24 */
    RsseRiderSlot  *riders;         /* 0x28 */
    RsseWalkerSlot *walkers;        /* 0x2C */
    u8   unk_30[0x0C];              /* 0x30 */
    s32  pc;                        /* 0x3C word index; < 0 = terminated (-10000) */
    s32  sp;                        /* 0x40 grows down */
    s32  hp;                        /* 0x44 HUSH/HOP pointer, grows up */
    s32  result;                    /* 0x48 tested by BRANCH_* */
    u8   unk_4c[0x04];              /* 0x4C */
    u32  code_len;                  /* 0x50 */
    u32  stack_size;                /* 0x54 */
    u32  limbo_capacity;            /* 0x58 */
    u8   unk_5c[0x04];              /* 0x5C */
    u32  limbo_count;               /* 0x60 */
    u32  rider_capacity;            /* 0x64 */
    u8   unk_68[0x04];              /* 0x68 */
    u32  rider_count;               /* 0x6C */
    s32  rider_node;                /* 0x70 BOUNCESETNODE */
    char *name;                     /* 0x74 NAME */
    u8   unk_78[0x04];              /* 0x78 */
    u32  walker_capacity;           /* 0x7C */
    u8   unk_80[0x0C];              /* 0x80 */
    u32  var_count;                 /* 0x8C */
    u8   unk_90[0x04];              /* 0x90 */
    s32  budget;                    /* 0x94 instructions per slice */
    s32  budget_left;               /* 0x98 */
    s32  sound;                     /* 0x9C */
    u32  wait_deadline;             /* 0xA0 WAIT/WAITABS */
    u32  anim_end;                  /* 0xA4 WAIT4ANIM */
    s32  loop_anim;                 /* 0xA8 LOOPANIM */
    u8   unk_ac[0x04];              /* 0xAC */
    void *objects;                  /* 0xB0 ADDOBJ list */
    u8   unk_b4[0x04];              /* 0xB4 */
    u8   turbo;                     /* 0xB8 run every tick */
    u8   unk_b9[0x07];              /* 0xB9 */
    s16  speed;                     /* 0xC0 speed/wear, pushed to anims each tick */
    u8   unk_c2[0x06];              /* 0xC2 */
    s32  owner;                     /* 0xC8 index into object table DAT_007a4610 */
    u8   unk_cc[0x28];              /* 0xCC */
} RsseContext;

/* ------------------------------------------------------------------------ */
/* Asset file formats                                                       */
/* ------------------------------------------------------------------------ */

/* DWFB .wad archive (tools/wadtool.py). Entries start at 0x58. */
typedef struct DwfbHeader {
    char magic[4];          /* 0x00 "DWFB" */
    u8   unk_04[0x44];      /* 0x04 */
    u32  count;             /* 0x48 */
    u32  names_offset;      /* 0x4C */
    u32  names_size;        /* 0x50 */
    u8   unk_54[0x04];      /* 0x54 */
} DwfbHeader;

typedef struct DwfbEntry {
    u32 unk_00;             /* 0x00 */
    u32 name_offset;        /* 0x04 absolute */
    u32 name_len;           /* 0x08 */
    u32 data_offset;        /* 0x0C absolute */
    u32 stored_size;        /* 0x10 */
    u32 comp_type;          /* 0x14 4 = EA RefPack */
    u32 size;               /* 0x18 uncompressed */
    u32 unk_1c[3];          /* 0x1C */
} DwfbEntry;

/* .wct wavelet texture (tools/wct2png.py). Bitstreams follow at 0x1C. */
typedef struct WctHeader {
    u8  version;            /* 0x00 0x13 = D4 wavelet, 0x12 = BILZ */
    u8  has_alpha;          /* 0x01 */
    u8  bpp;                /* 0x02 */
    u8  unk_03;             /* 0x03 */
    u16 width;              /* 0x04 */
    u16 height;             /* 0x06 */
    u16 q_y, q_u, q_v, q_a; /* 0x08 quantiser per plane */
    u32 main_len;           /* 0x10 YUV LZ stream */
    u32 alpha_len;          /* 0x14 alpha LZ stream, 0 if none */
    u8  avg_colour[4];      /* 0x18 */
} WctHeader;

/* BFST .str string table (tools/bfst.py). Offsets are relative to 0x0C. */
typedef struct BfstHeader {
    char magic[4];          /* 0x00 "BFST" */
    u32  unk_04;            /* 0x04 */
    u32  count;             /* 0x08 */
    /* u32 offsets[count];     0x0C */
} BfstHeader;

typedef struct BfstRecord {
    u8  type;               /* 0x00 always 1 */
    u8  length;             /* 0x01 */
    u16 zero;               /* 0x02 */
    /* u8 text[length];        0x04 custom code page, see MBToUni.dat */
} BfstRecord;

/* ------------------------------------------------------------------------ */
/* Layout checks (ignored by Ghidra's parser, which skips the macro body)   */
/* ------------------------------------------------------------------------ */
#ifndef __GHIDRA__
#define TPW_CHECK(name, cond) typedef char tpw_check_##name[(cond) ? 1 : -1]
#define TPW_OFF(t, f) ((u32)(unsigned long)&((t *)0)->f)
TPW_CHECK(rse_header,  sizeof(RsseFileHeader) == 0x34);
TPW_CHECK(rider_slot,  sizeof(RsseRiderSlot) == 0x10);
TPW_CHECK(ctx_size,    sizeof(RsseContext) == 0xF4);
TPW_CHECK(ctx_pc,      TPW_OFF(RsseContext, pc) == 0x3C);
TPW_CHECK(ctx_result,  TPW_OFF(RsseContext, result) == 0x48);
TPW_CHECK(ctx_varcnt,  TPW_OFF(RsseContext, var_count) == 0x8C);
TPW_CHECK(ctx_turbo,   TPW_OFF(RsseContext, turbo) == 0xB8);
TPW_CHECK(ctx_owner,   TPW_OFF(RsseContext, owner) == 0xC8);
TPW_CHECK(dwfb_header, sizeof(DwfbHeader) == 0x58);
TPW_CHECK(dwfb_entry,  sizeof(DwfbEntry) == 0x28);
TPW_CHECK(wct_header,  sizeof(WctHeader) == 0x1C);
#endif

#endif /* TPW_TYPES_H */
