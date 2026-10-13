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

/* BEGIN GENERATED by tools/gen_structs.py: do not edit by hand */
/* Saved game objects. Offsets from the serializers' load code, types from tools/tpsave.py. */
typedef struct TpwU64 { u32 lo; u32 hi; } TpwU64;  /* 64-bit time values */
typedef struct Vec2 { float x, y; } Vec2;
typedef struct Vec3 { float x, y, z; } Vec3;

/* Base of every world object (0x50b090). Not polymorphic: Thing_Update and
 * Thing_HandleMessage switch on `model` (1 Visitor, 3 Object, 4..8 staff, ...). The world
 * update skips things whose unk_03 is non-zero. */
typedef struct Thing {
    u16 unk_00;                          /* 0x00 */
    u8 model;                            /* 0x02 */
    u8 unk_03;                           /* 0x03 */
    u16 mX;                              /* 0x04 */
    u16 mY;                              /* 0x06 */
    u16 mMapParent;                      /* 0x08 */
    u16 mMapChild;                       /* 0x0a */
} Thing;

/* Sprite animation state (0x4d3ee0). */
typedef struct SpriteAnim {
    u32 mSpriteScript;                   /* 0x00 */
    u32 mNextAnim;                       /* 0x04 */
    u32 mNextServiceInterval;            /* 0x08 */
} SpriteAnim;

/* Recent thoughts and events of a person (0x50bad0). */
typedef struct Thoughts {
    u32 mLastThought;                    /* 0x00 */
    u32 mEventHistory[32];               /* 0x04 */
    u32 mThoughtScript;                  /* 0x84 */
    u32 mActionHistIndex;                /* 0x88 */
    u32 mTimeBubbleShown;                /* 0x8c */
} Thoughts;

/* Steering / path following (0x50d150). */
typedef struct Nav {
    float mass;                          /* 0x00 */
    float radius;                        /* 0x04 */
    Vec2 pos;                            /* 0x08 */
    Vec2 vel;                            /* 0x10 */
    float max_force;                     /* 0x18 */
    float max_speed;                     /* 0x1c */
    Vec2 local_xaxis;                    /* 0x20 */
    Vec2 local_yaxis;                    /* 0x28 */
    u8 unk_30[0x8];
    Vec2 formation_pos;                  /* 0x38 */
    Vec2 force;                          /* 0x40 */
    u32 path_timestamp;                  /* 0x48 */
    Vec2 path_target_pos;                /* 0x4c */
    u32 path_total_count;                /* 0x54 */
    u32 path_buffer_count;               /* 0x58 */
    u32 path_count;                      /* 0x5c */
    u8 path_finished;                    /* 0x60 */
    u8 unk_61[0x3];
    Vec2 subpath_buffer[5];              /* 0x64 */
    float subpath_dist[5];               /* 0x8c */
    float path_total_dist;               /* 0xa0 */
    float path_subpath_dist;             /* 0xa4 */
    float path_tail_dist;                /* 0xa8 */
    u32 path_last_progress;              /* 0xac */
    u32 path_stuck_buffer;               /* 0xb0 */
    u32 nav_mode;                        /* 0xb4 */
    u32 mCantReachDest;                  /* 0xb8 */
} Nav;

/* A walking person: base of Visitor and Staff (0x4f8b10). */
typedef struct Sprite {
    Thing base;                          /* 0x00 */
    SpriteAnim mAnim;                    /* 0x0c */
    u16 mAccurateDestX;                  /* 0x18 */
    u16 mAccurateDestY;                  /* 0x1a */
    u32 mSpriteAngle;                    /* 0x1c */
    u32 mSpriteID;                       /* 0x20 */
    u32 mESPSprite;                      /* 0x24 */
    u32 mSpriteUnderRideCtrl;            /* 0x28 */
    u8 mCount;                           /* 0x2c */
    u8 unk_2d[0x3];
    Thoughts mThoughts;                  /* 0x30 */
    u16 mBaseSpeed;                      /* 0xc0 */
    u16 mPurposeSpeed;                   /* 0xc2 */
    u16 mAdjustorSpeed;                  /* 0xc4 */
    u8 unk_c6[0x2];
    u32 mPreviousSpeed;                  /* 0xc8 */
    u16 mLastRecordedMapId;              /* 0xcc */
    u8 unk_ce[0x2];
    u32 mSetDestSuccessfully;            /* 0xd0 */
    Nav mNav;                            /* 0xd4 */
    u32 mPreviousX;                      /* 0x190 */
    u32 mPreviousY;                      /* 0x194 */
    u32 mStrandedTime;                   /* 0x198 */
} Sprite;

/* Thing model 1 (0x4fb530). The needs (mHappiness .. mTiredness) are 0..100 floats saved as
 * 0..255 under the placeholder name "pv".
 * Only the saved fields are known: the real object may be larger. */
typedef struct Visitor {
    Sprite base;                         /* 0x00 */
    float mHappiness;                    /* 0x19c: "Sideshow won - happiness up" adds here */
    s32 mCash;                           /* 0x1a0 */
    float mThirst;                       /* 0x1a4: 0..100; drinks subtract ObjectDef.mThirstEffect */
    float mHunger;                       /* 0x1a8: 0..100; food subtracts ObjectDef.mHungerEffect */
    float mToilet;                       /* 0x1ac: zeroed by a toilet; grows over time */
    float mVomit;                        /* 0x1b0: ObjectDef.mVomitEffect adds; sick above 90 */
    float mLitter;                       /* 0x1b4: "Litter gone up by %d" adds ObjectDef.mLitterEffect */
    float mTiredness;                    /* 0x1b8: name from its sort slot; only ever zeroed and saved */
    u32 mExitLevel;                      /* 0x1bc */
    u8 mPrankeryIndex;                   /* 0x1c0 */
    u8 unk_1c1[0x3];
    u32 mNumRides;                       /* 0x1c4 */
    u32 mNumShops;                       /* 0x1c8 */
    u32 mNumSideshows;                   /* 0x1cc */
    u32 mNumSideshowsWon;                /* 0x1d0 */
    u32 mArrivalDate;                    /* 0x1d4 */
    u32 mArrivalIndex;                   /* 0x1d8 */
    u16 mMajorDest;                      /* 0x1dc */
    u16 mSavedMajorDest;                 /* 0x1de */
    u16 mPreviousRides[4];               /* 0x1e0 */
    u16 mPreviousTemporaryRides[4];      /* 0x1e8 */
    u8 mPersonType;                      /* 0x1f0 */
    u8 mQueuePos;                        /* 0x1f1 */
    u8 unk_1f2[0x2];
    u32 mQueueMoveDelay;                 /* 0x1f4 */
    u32 mBeenAdmitted;                   /* 0x1f8 */
    u32 mTimeStartedIdling;              /* 0x1fc */
    u32 mParkOpeningWaitingTime;         /* 0x200 */
    u32 mPaidAdmission;                  /* 0x204 */
    u32 mTimeOfLastSpotAnim;             /* 0x208 */
    u8 unk_20c[0x4];
    u32 mBalloonScript;                  /* 0x210 */
    u32 mRemainingBalloonLife;           /* 0x214 */
    u32 mLastPosX;                       /* 0x218 */
    u32 mLastPosY;                       /* 0x21c */
    u32 mState;                          /* 0x220 */
    u32 mSavedState;                     /* 0x224 */
    u16 mQNext;                          /* 0x228 */
    u16 mQPrev;                          /* 0x22a */
} Visitor;

/* Base of the five staff types (0x504de0). */
typedef struct Staff {
    Sprite base;                         /* 0x00 */
    u32 mState;                          /* 0x19c */
    u16 mName[33];                       /* 0x1a0 */
    u8 unk_1e2[0x2];
    u32 mCurrentPayGrade;                /* 0x1e4 */
    u8 mPercentageThroughGrade;          /* 0x1e8 */
    u8 unk_1e9[0x7];
    TpwU64 mTimeHired;                   /* 0x1f0 */
    float mHappiness;                    /* 0x1f8: probably; drains faster on low pay grades */
    float mEnergy;                       /* 0x1fc: too tired below a balance threshold (0x506680) */
    u32 mTimeStartedIdling;              /* 0x200 */
    u32 mJobsDone;                       /* 0x204 */
    u16 mRestArea;                       /* 0x208 */
    u16 mPatrolRegionBL;                 /* 0x20a */
    u16 mPatrolRegionTR;                 /* 0x20c */
    u8 unk_20e[0x2];
} Staff;

/* Thing model 4 (0x4da110).
 * Only the saved fields are known: the real object may be larger. */
typedef struct Mechanic {
    Staff base;                          /* 0x00 */
    u16 mNext;                           /* 0x210 */
    u8 unk_212[0x2];
    u32 mDurationOfRepair;               /* 0x214 */
    u16 mObjectToRepair;                 /* 0x218 */
    u8 unk_21a[0x2];
} Mechanic;

/* Thing model 5 (0x4d6d60).
 * Only the saved fields are known: the real object may be larger. */
typedef struct Handyman {
    Staff base;                          /* 0x00 */
    u16 mNext;                           /* 0x210 */
    u8 unk_212[0x2];
    u32 mTimeStartedCleaning;            /* 0x214 */
    u16 mTargetLitterCell;               /* 0x218 */
    u16 mToiletToClean;                  /* 0x21a */
} Handyman;

/* Thing model 6 (0x4d4460).
 * Only the saved fields are known: the real object may be larger. */
typedef struct Entertainer {
    Staff base;                          /* 0x00 */
    u16 mNext;                           /* 0x210 */
    u8 unk_212[0x2];
    u32 mTimeStartedEntertaining;        /* 0x214 */
} Entertainer;

/* Thing model 7 (0x4d6000).
 * Only the saved fields are known: the real object may be larger. */
typedef struct Guard {
    Staff base;                          /* 0x00 */
    u16 mNext;                           /* 0x210 */
    u16 mPerp;                           /* 0x212 */
    u32 mProsecutionTimestamp;           /* 0x214 */
} Guard;

/* Thing model 8 (0x502760).
 * Only the saved fields are known: the real object may be larger. */
typedef struct Researcher {
    Staff base;                          /* 0x00 */
    u16 mNext;                           /* 0x210 */
    u8 unk_212[0x2];
    u32 mTimeStartedResearching;         /* 0x214 */
} Researcher;

/* Ring buffer of 30 values (0x4e2c10); Object keeps six of them. */
typedef struct CircBufU32_30 {
    u32 mTemp;                           /* 0x00 */
    u32 mData[30];                       /* 0x04 */
    u32 mCurrentEntry;                   /* 0x7c */
    u32 mNumEntries;                     /* 0x80 */
    u8 mWrappedAround;                   /* 0x84 */
    u8 unk_85[0x3];
} CircBufU32_30;

/* Thing model 3: a ride, shop, sideshow or feature (0x4db7d0). The six history buffers are
 * saved in the order 0x70, 0xf8, 0x1a8, 0x230, 0x2b8, 0x340; their meaning is unknown.
 * Only the saved fields are known: the real object may be larger. */
typedef struct Object {
    Thing base;                          /* 0x00 */
    u16 mNext;                           /* 0x0c */
    u16 mId;                             /* 0x0e */
    u32 mAngle;                          /* 0x10 */
    u8 unk_14[0xc];
    u32 MeshInstanceID;                  /* 0x20 */
    u32 mRideScriptHandle;               /* 0x24 */
    u32 mTrackRideHandle;                /* 0x28 */
    u32 mIsTrackRideValid;               /* 0x2c */
    u16 mUpgradeParent;                  /* 0x30 */
    u16 mFlags;                          /* 0x32 */
    u16 mTopLeft;                        /* 0x34 */
    u16 mEntryPos;                       /* 0x36 */
    u16 mExitPos;                        /* 0x38 */
    u16 mBackOfQueue;                    /* 0x3a */
    u16 mFirstInQ;                       /* 0x3c */
    u8 unk_3e[0x2];
    u32 mQueueSizeInCells;               /* 0x40 */
    u8 unk_44[0xc];
    u8 mUpgradeLevel;                    /* 0x50 */
    u8 unk_51[0x3];
    u32 mUpgradeBalloonSprite;           /* 0x54 */
    u32 mOperatingSpeed;                 /* 0x58 */
    u8 mOperatingDuration;               /* 0x5c */
    u8 mOperatingCapacity;               /* 0x5d */
    u16 mAssignedStaffMember;            /* 0x5e */
    u32 mTimeMarkedForMaintenance;       /* 0x60 */
    u32 mRequestedService;               /* 0x64 */
    u32 mCanLoad;                        /* 0x68 */
    u16 mPersonBeingLoaded;              /* 0x6c */
    u8 unk_6e[0x2];
    CircBufU32_30 mHist0;                /* 0x70 */
    CircBufU32_30 mHist1;                /* 0xf8 */
    u32 mTotalTakings;                   /* 0x180 */
    u32 mTotalCosts;                     /* 0x184 */
    u32 mCostOfGoods;                    /* 0x188 */
    u32 mQualityOfGoods;                 /* 0x18c */
    u32 mChanceOfWinning;                /* 0x190 */
    u32 mPricePerUse;                    /* 0x194 */
    u32 mAmountOfSpecialIngredient;      /* 0x198 */
    u32 mState;                          /* 0x19c */
    u32 mNumCustomers;                   /* 0x1a0 */
    u32 mNumWalkAways;                   /* 0x1a4 */
    CircBufU32_30 mHist2;                /* 0x1a8 */
    CircBufU32_30 mHist3;                /* 0x230 */
    CircBufU32_30 mHist4;                /* 0x2b8 */
    CircBufU32_30 mHist5;                /* 0x340 */
    u16 mNameA[33];                      /* 0x3c8 */
    u16 mNameB[33];                      /* 0x40a */
} Object;

/* Common part of map and track cells (0x4d0b30). */
typedef struct MapCellBase {
    u8 unk_00[0x4];
    u32 mMeshInstance;                   /* 0x04 */
    u32 mType;                           /* 0x08 */
    u8 mNeighbours;                      /* 0x0c */
    u8 mDirection;                       /* 0x0d */
    u16 mFlags;                          /* 0x0e */
    u16 mParentID;                       /* 0x10 */
    u8 unk_12[0x2];
    Vec3 mTileData;                      /* 0x14 */
    u16 mOverlapCounter;                 /* 0x20 */
    u8 mHoardingNeighbours;              /* 0x22 */
    u8 unk_23[0x1];
} MapCellBase;

/* One of the 128x128 map cells (0x4d8dd0).
 * Only the saved fields are known: the real object may be larger. */
typedef struct MapCell {
    MapCellBase base;                    /* 0x00 */
    u16 mWho;                            /* 0x24 */
    u8 mStatusFlags;                     /* 0x26 */
    u8 unk_27[0x1];
    u32 mLitterScript;                   /* 0x28 */
    u32 mLitter;                         /* 0x2c */
    u16 mLitterCollector;                /* 0x30 */
    u8 unk_32[0x2];
    u32 mTimeMarkedForLitterCollection;  /* 0x34 */
    u32 mLitterScript_38;                /* 0x38 */
    u8 unk_3c[0x4];
    u16 mPylonIndex;                     /* 0x40 */
    u8 unk_42[0x2];
} MapCell;

/* One of the 128x128 track cells (0x50c200).
 * Only the saved fields are known: the real object may be larger. */
typedef struct TrackCell {
    MapCellBase base;                    /* 0x00 */
    u16 mSegmentNumber;                  /* 0x24 */
    u8 unk_26[0x2];
} TrackCell;

/* Weather forecast entry (0x5132a0). */
typedef struct Forecast {
    TpwU64 mTime;                        /* 0x00 */
    u32 mQuality;                        /* 0x08 */
    u8 mValid;                           /* 0x0c */
    u8 unk_0d[0x3];
} Forecast;

/* Thing model 15 (0x512010).
 * Only the saved fields are known: the real object may be larger. */
typedef struct Weather {
    Thing base;                          /* 0x00 */
    u32 mQuality;                        /* 0x0c */
    TpwU64 mLastChanged;                 /* 0x10 */
    TpwU64 mLastForecasted;              /* 0x18 */
    Forecast mForecast;                  /* 0x20 */
    u32 mCurrentDrops;                   /* 0x30 */
    u32 mTargetDrops;                    /* 0x34 */
    u8 mLightning;                       /* 0x38 */
    u8 unk_39[0x3];
    u32 mLightningCountdown;             /* 0x3c */
    u32 mLightningNumThisPeriod;         /* 0x40 */
    u32 mThunderCountdown;               /* 0x44 */
    Vec3 mLastLightningLocation;         /* 0x48 */
    u32 mRainSoundHandle;                /* 0x54 */
    u32 mPType;                          /* 0x58 */
    float mWindDir;                      /* 0x5c */
    float mWindForce;                    /* 0x60 */
    u8 mOverridden;                      /* 0x64 */
    u8 unk_65[0x3];
} Weather;

/* One park challenge (0x4d0f90). */
typedef struct Challenge {
    u32 mType;                           /* 0x00 */
    u32 mTargetTime;                     /* 0x04 */
    u32 mTargetVal;                      /* 0x08 */
    u16 mTargetObj;                      /* 0x0c */
    u16 mTargetObj2;                     /* 0x0e */
    u32 mTargetStaffType;                /* 0x10 */
    u32 mPrize;                          /* 0x14 */
    u32 mDoubleOrNothingFollowup;        /* 0x18 */
    u8 mCheckAtEndOnly;                  /* 0x1c */
    u8 mIndependent;                     /* 0x1d */
    u8 unk_1e[0x2];
    u32 mChallengeOffered;               /* 0x20 */
    u32 mChallengeLost;                  /* 0x24 */
    u32 mChallengeDeclined;              /* 0x28 */
    u8 mHaveWon;                         /* 0x2c */
    u8 unk_2d[0x1];
    u16 mThingIdForFollowup;             /* 0x2e */
} Challenge;

/* Thing model 19 (0x4d2320).
 * Only the saved fields are known: the real object may be larger. */
typedef struct Challenges {
    Thing base;                          /* 0x00 */
    Challenge mChallenges[20];           /* 0x0c */
    u32 mCurrentChallenge;               /* 0x3cc */
    TpwU64 mNextChallengeEventTime;      /* 0x3d0 */
    u8 mSystemSwitchedOn;                /* 0x3d8 */
    u8 mChallengeOn;                     /* 0x3d9 */
    u8 mWaitingForReply;                 /* 0x3da */
    u8 unk_3db[0x1];
    u32 mValue;                          /* 0x3dc */
    u16 mActualThing;                    /* 0x3e0 */
    u8 unk_3e2[0x2];
} Challenges;

/* Research project (0x504210). */
typedef struct ResearchProject {
    u16 mObjId;                          /* 0x00 */
    u8 unk_02[0x2];
    u32 mCat;                            /* 0x04 */
    u8 mResearchGroup;                   /* 0x08 */
    u8 mUpgradeLevel;                    /* 0x09 */
    u8 unk_0a[0x2];
    float mCost;                         /* 0x0c */
    float mResearchDone;                 /* 0x10 */
} ResearchProject;

/* Research group (0x504760). */
typedef struct ResearchGroup {
    u32 effort;                          /* 0x00 */
    u32 output;                          /* 0x04 */
    u32 currentProject;                  /* 0x08 */
    u8 highestGroupAvailable;            /* 0x0c */
    u8 unk_0d[0x3];
} ResearchGroup;

/* Thing model 14 (0x502dc0).
 * Only the saved fields are known: the real object may be larger. */
typedef struct ResearchLab {
    Thing base;                          /* 0x00 */
    ResearchProject mProjects[256];      /* 0x0c */
    ResearchGroup mGroups[5];            /* 0x140c */
    u32 mNumProjects;                    /* 0x145c */
    u32 mWorkLoad;                       /* 0x1460 */
    u32 mMostExpensiveProject;           /* 0x1464 */
    u8 mLabActive;                       /* 0x1468 */
    u8 unk_1469[0x3];
} ResearchLab;

/* Loan slot (saved inside 0x4cf920); see tools/tpsave.py BANK for the other field names. */
typedef struct BankLoan {
    u32 loan_available;                  /* 0x00 */
    u32 unk_04[7];                       /* 0x04 */
} BankLoan;

/* Thing model 16 (0x4cf920).
 * Only the saved fields are known: the real object may be larger. */
typedef struct BankAccount {
    Thing base;                          /* 0x00 */
    s32 mBalance;                        /* 0x0c */
    s32 mBatchBalance;                   /* 0x10 */
    BankLoan mLoans[8];                  /* 0x14 */
    u32 mWithdrawalsEnabled;             /* 0x114 */
    u32 mAdmissionFee;                   /* 0x118 */
    s32 mLastBalance;                    /* 0x11c */
    u32 mTurnEnteredRed;                 /* 0x120 */
    s32 mProfitThisYear;                 /* 0x124 */
} BankAccount;

/* Game clock in the world header (0x4f7f30). */
typedef struct GameClock {
    TpwU64 mFunnyTimeStart;              /* 0x00 */
    TpwU64 mSessionStart;                /* 0x08 */
    u32 mMonthAtLastUpdate;              /* 0x10 */
    u32 mDayAtLastUpdate;                /* 0x14 */
    u8 unk_18[0x4];
    u32 mFunnySecsPerRealSec;            /* 0x1c */
} GameClock;

/* One row of ObjectDef.mUpgrades: the Upgrades[i].* keys. */
typedef struct ObjectDefUpgrade {
    u32 mCostOfResearch;                 /* 0x00 */
    s32 mEstimatedPriceVariance;         /* 0x04 */
    u32 mScrapValueYear1;                /* 0x08 */
    u32 mScrapValueYear2;                /* 0x0c */
    u32 mScrapValueYear3;                /* 0x10 */
    u32 mScrapValueYear4;                /* 0x14 */
    u32 mInitCapacity;                   /* 0x18 */
    u32 mRedLineCapacity;                /* 0x1c */
    u32 mInitDuration;                   /* 0x20 */
    u32 mRedLineDuration;                /* 0x24 */
    u32 mInitSpeed;                      /* 0x28 */
    u32 mRedLineSpeed;                   /* 0x2c */
    u32 mWearRate;                       /* 0x30 */
    float mQueueWaitTimeConstant;        /* 0x34 */
    u32 mCostOfUpgrade;                  /* 0x38 */
    u32 mDurationOfUpgrade;              /* 0x3c */
} ObjectDefUpgrade;

/* A ride/shop/sideshow/feature description, loaded from its .sam file (0x413c10) and
 * found by id with ObjectDefs_Find (0x412e90). Fields are the .sam keys in schema order
 * (tools/samtool.py schema object); e.g. UsageInfo.ThirstEffect is mThirstEffect.
 * The vtable and mSlotInfo belong to the generic balance-file base class (0x401030),
 * which starts at +4. Runtime state follows mSamPath (up to ~0x500).
 * Only the saved fields are known: the real object may be larger. */
typedef struct ObjectDef {
    u16 mLookupId;                       /* 0x00 */
    u8 unk_02[0x2];
    void * vtable;                       /* 0x04 */
    void * mSlotInfo;                    /* 0x08 */
    u8 unk_0c[0x4];
    u32 mId;                             /* 0x10 */
    char * mName;                        /* 0x14 */
    void * mShape;                       /* 0x18 */
    void * mHoarding;                    /* 0x1c */
    u32 mEngineMapOffsetOverrideX;       /* 0x20 */
    u32 mEngineMapOffsetOverrideY;       /* 0x24 */
    u32 mEngineFootprintWidthOverride;   /* 0x28 */
    u32 mEngineFootprintHeightOverride;  /* 0x2c */
    u32 mMapOffsetX;                     /* 0x30 */
    u32 mMapOffsetY;                     /* 0x34 */
    s32 mDontApplyOffset;                /* 0x38 */
    s32 mIsChoosable;                    /* 0x3c */
    s32 mHasQueue;                       /* 0x40 */
    s32 mCanBreakDown;                   /* 0x44 */
    s32 mRunsContinuously;               /* 0x48 */
    s32 mWhichUIType;                    /* 0x4c */
    s32 mDontDeformBase;                 /* 0x50 */
    u32 mOverwritePriority;              /* 0x54 */
    u32 mAttractionValue;                /* 0x58 */
    s32 mNewAttractionDecayTime;         /* 0x5c */
    s32 mCreateParticleEffect;           /* 0x60 */
    s32 mDestroyParticleEffect;          /* 0x64 */
    s32 mRepairParticleEffect;           /* 0x68 */
    s32 mUpgradeParticleEffect;          /* 0x6c */
    u32 mDurationUnit;                   /* 0x70 */
    u32 mUsesCars;                       /* 0x74 */
    char * mPreviewAnimType;             /* 0x78 */
    u32 mPreviewAnimNum;                 /* 0x7c */
    s32 mFirstPersonSprites;             /* 0x80 */
    s32 mDoHeadProcessing;               /* 0x84 */
    s32 mTurnOffAnimatingTextures;       /* 0x88 */
    s32 mTurnOffScrollingTextures;       /* 0x8c */
    u32 mRideTypeStringIndex;            /* 0x90 */
    s32 mDownloadedRideNumber;           /* 0x94 */
    s32 mIsPreviewRide;                  /* 0x98 */
    s32 mWhichTrackType;                 /* 0x9c */
    s32 mBumperType;                     /* 0xa0 */
    s32 mNorthXAdjust;                   /* 0xa4 */
    s32 mNorthYAdjust;                   /* 0xa8 */
    s32 mEastXAdjust;                    /* 0xac */
    s32 mEastYAdjust;                    /* 0xb0 */
    s32 mSouthXAdjust;                   /* 0xb4 */
    s32 mSouthYAdjust;                   /* 0xb8 */
    s32 mWestXAdjust;                    /* 0xbc */
    s32 mWestYAdjust;                    /* 0xc0 */
    u32 mGoldenTicketCost;               /* 0xc4 */
    u32 mLength;                         /* 0xc8 */
    u32 mWidth;                          /* 0xcc */
    u32 mHeight;                         /* 0xd0 */
    float mEntryCellStandPosX;           /* 0xd4 */
    float mEntryCellStandPosY;           /* 0xd8 */
    float mExitCellAppearPosX;           /* 0xdc */
    float mExitCellAppearPosY;           /* 0xe0 */
    u32 mInitPricePerUse;                /* 0xe4 */
    u32 mFatigueEffect;                  /* 0xe8 */
    s32 mInitChanceOfLoosing;            /* 0xec */
    s32 mProvidesSecurity;               /* 0xf0 */
    s32 mProvidesRelief;                 /* 0xf4 */
    s32 mHoldsLitter;                    /* 0xf8 */
    s32 mChillsYouOut;                   /* 0xfc */
    s32 mRequiresTeleport;               /* 0x100 */
    s32 mRideHandlesSprite;              /* 0x104 */
    s32 mRequiresWatering;               /* 0x108 */
    s32 mNonWinnableSideshow;            /* 0x10c */
    s32 mIsFireworks;                    /* 0x110 */
    s32 mISIndoors;                      /* 0x114 */
    s32 mCannotRide;                     /* 0x118 */
    s32 mConstrainCamera;                /* 0x11c */
    u32 mPukeReductionLevel;             /* 0x120 */
    u32 mMinCapacity;                    /* 0x124 */
    u32 mMaxCapacity;                    /* 0x128 */
    u32 mMinDuration;                    /* 0x12c */
    u32 mMaxDuration;                    /* 0x130 */
    u32 mMinSpeed;                       /* 0x134 */
    u32 mMaxSpeed;                       /* 0x138 */
    s32 mExcitementLevel;                /* 0x13c */
    u32 mInitCostOfGoods;                /* 0x140 */
    s32 mThirstEffect;                   /* 0x144 */
    s32 mHungerEffect;                   /* 0x148 */
    s32 mVomitEffect;                    /* 0x14c */
    s32 mHappinessEffect;                /* 0x150 */
    s32 mLitterEffect;                   /* 0x154 */
    s32 mSpecialIngredient;              /* 0x158 */
    s32 mAppearanceEffect;               /* 0x15c */
    u32 mInitPrizeValue;                 /* 0x160 */
    s32 mShopType;                       /* 0x164 */
    s32 mExciteFactor;                   /* 0x168 */
    u32 mRipOffOK;                       /* 0x16c */
    u32 mNumSimultAnims;                 /* 0x170 */
    u32 mCost;                           /* 0x174 */
    s32 mGroup;                          /* 0x178 */
    s32 mCategory;                       /* 0x17c */
    ObjectDefUpgrade mUpgrades[3];       /* 0x180 */
    u8 unk_240[0x4];
    char * mSupplementalMeshes[8];       /* 0x244 */
    u8 unk_264[0x4];
    u32 mAttractionNewBonus[3];          /* 0x268 */
    u8 unk_274[0x4];
    char * mSignTextures[2];             /* 0x278 */
    u8 unk_280[0x4];
    u32 mUpgradesId;                     /* 0x284 */
    u32 mUpgradeType;                    /* 0x288 */
    u8 mSamPath[260];                    /* 0x28c */
} ObjectDef;

/* One row of BalanceLevel.mPeepTypes: the PeepTypes[i].* keys. */
typedef struct BalanceLevelPeepType {
    s32 mPreferredExcitement;            /* 0x00 */
    u32 mStartingCash;                   /* 0x04 */
    s32 mBoredomThreshold;               /* 0x08 */
} BalanceLevelPeepType;

/* One row of BalanceLevel.mLoanInfo: the LoanInfo[i].* keys. */
typedef struct BalanceLevelLoanInfo {
    u32 mLoanAmount;                     /* 0x00 */
    s32 mAPRInPercent;                   /* 0x04 */
    u32 mRepaymentPeriodInMonths;        /* 0x08 */
    u32 mLendername;                     /* 0x0c */
} BalanceLevelLoanInfo;

/* One row of BalanceLevel.mPerGradeStaffConsts: the PerGradeStaffConsts[i].* keys. */
typedef struct BalanceLevelPerGradeStaffConst {
    u32 mBaseWage;                       /* 0x00 */
    u32 mIdleDuration;                   /* 0x04 */
    float mRecuperationRate;             /* 0x08 */
    float mHappinessRecuperationRate;    /* 0x0c */
} BalanceLevelPerGradeStaffConst;

/* One row of BalanceLevel.mEntertainerConstsPerGrade: the EntertainerConstsPerGrade[i].* keys. */
typedef struct BalanceLevelEntertainerConstsPerGrade {
    u32 mWorkDuration;                   /* 0x00 */
    u32 mHappinessEffectOnCell;          /* 0x04 */
    u32 mActivationDistance;             /* 0x08 */
    u32 mPoundsPerTrainingPoint;         /* 0x0c */
} BalanceLevelEntertainerConstsPerGrade;

/* One row of BalanceLevel.mHandymanConstsPerGrade: the HandymanConstsPerGrade[i].* keys. */
typedef struct BalanceLevelHandymanConstsPerGrade {
    u32 mWorkDuration;                   /* 0x00 */
    u32 mDetectionRange;                 /* 0x04 */
    u32 mPoundsPerTrainingPoint;         /* 0x08 */
} BalanceLevelHandymanConstsPerGrade;

/* One row of BalanceLevel.mMechanicConstsPerGrade: the MechanicConstsPerGrade[i].* keys. */
typedef struct BalanceLevelMechanicConstsPerGrade {
    u32 mWorkDuration;                   /* 0x00 */
    u32 mPoundsPerTrainingPoint;         /* 0x04 */
} BalanceLevelMechanicConstsPerGrade;

/* One row of BalanceLevel.mResearcherConstsPerGrade: the ResearcherConstsPerGrade[i].* keys. */
typedef struct BalanceLevelResearcherConstsPerGrade {
    u32 mWorkDuration;                   /* 0x00 */
    u32 mResearchAbility;                /* 0x04 */
    u32 mPoundsPerTrainingPoint;         /* 0x08 */
} BalanceLevelResearcherConstsPerGrade;

/* One row of BalanceLevel.mGuardConstsPerGrade: the GuardConstsPerGrade[i].* keys. */
typedef struct BalanceLevelGuardConstsPerGrade {
    u32 mWorkDuration;                   /* 0x00 */
    u32 mPoundsPerTrainingPoint;         /* 0x04 */
} BalanceLevelGuardConstsPerGrade;

/* One row of BalanceLevel.mSeasons: the Seasons[i].* keys. */
typedef struct BalanceLevelSeason {
    s32 mAvgWeatherQuality;              /* 0x00 */
    s32 mNormalTolerance;                /* 0x04 */
    s32 mChanceForExceptionalWeather;    /* 0x08 */
} BalanceLevelSeason;

/* One row of BalanceLevel.mRegionFX: the RegionFX[i].* keys. */
typedef struct BalanceLevelRegionFX {
    u32 mRadius;                         /* 0x00 */
    s32 mHappiness;                      /* 0x04 */
    s32 mIllness;                        /* 0x08 */
    s32 mHunger;                         /* 0x0c */
    s32 mSecurity;                       /* 0x10 */
    s32 mAttraction;                     /* 0x14 */
} BalanceLevelRegionFX;

/* One row of BalanceLevel.mThemeAdvisorCostumes: the ThemeAdvisorCostumes[i].* keys. */
typedef struct BalanceLevelThemeAdvisorCostume {
    s32 mAddOns;                         /* 0x00 */
    s32 mSubOffs;                        /* 0x04 */
} BalanceLevelThemeAdvisorCostume;

/* One row of BalanceLevel.mSkyObjects: the SkyObjects[i].* keys. */
typedef struct BalanceLevelSkyObject {
    float mX;                            /* 0x00 */
    float mY;                            /* 0x04 */
    float mZ;                            /* 0x08 */
    float mSize;                         /* 0x0c */
    u32 mFrontFlag;                      /* 0x10 */
    char * mFilename;                    /* 0x14 */
} BalanceLevelSkyObject;

/* Values of Standard.sam (peeps, staff, bank, map, ...); static instance gBalanceLevel at 0x785040. Fields are the .sam keys
 * in schema order (tools/samtool.py schema level); vtable and mSlotInfo
 * belong to the balance-file base class (0x401030). */
typedef struct BalanceLevel {
    void * vtable;                       /* 0x00 */
    void * mSlotInfo;                    /* 0x04 */
    u8 unk_08[0x4];
    u32 mExitLevel;                      /* 0x0c */
    u32 mExitLevelVar;                   /* 0x10 */
    s32 mStartingCashVarPc;              /* 0x14 */
    u32 mSmallHappinessChange;           /* 0x18 */
    u32 mMediumHappinessChange;          /* 0x1c */
    u32 mBigHappinessChange;             /* 0x20 */
    u32 mPerfectRide;                    /* 0x24 */
    u32 mGoodRide;                       /* 0x28 */
    u32 mOKRide;                         /* 0x2c */
    u32 mRideVomitDivisor;               /* 0x30 */
    u32 mToiletDesparate;                /* 0x34 */
    u32 mVomitCapacity;                  /* 0x38 */
    u32 mExcitementToCostDivisor;        /* 0x3c */
    u32 mMinimumEntryFee;                /* 0x40 */
    float mCheapPriceMultiplier;         /* 0x44 */
    float mAveragePriceMultiplier;       /* 0x48 */
    float mExpensivePriceMultiplier;     /* 0x4c */
    u32 mDecisionVariable1;              /* 0x50 */
    u32 mDecisionVariable2;              /* 0x54 */
    u32 mDecisionVariable3;              /* 0x58 */
    u32 mDecisionVariable4;              /* 0x5c */
    u32 mDecisionVariable5;              /* 0x60 */
    u32 mDecisionVariable6;              /* 0x64 */
    u32 mDecisionVariable7;              /* 0x68 */
    u32 mDecisionVariable8;              /* 0x6c */
    u32 mDecisionVarDistWeight;          /* 0x70 */
    u32 mDecisionVarQueueWeight;         /* 0x74 */
    u32 mDecisionVarExcitementWeight;    /* 0x78 */
    u32 mDecisionVarThirstWeight;        /* 0x7c */
    u32 mDecisionVarHungerWeight;        /* 0x80 */
    u32 mDecisionVarToiletWeight;        /* 0x84 */
    u32 mDecisionVarIllnessWeight;       /* 0x88 */
    s32 mOpinionForMuchTooHigh;          /* 0x8c */
    s32 mOpinionForTooHigh;              /* 0x90 */
    s32 mOpinionForTooLow;               /* 0x94 */
    s32 mOpinionForAboutRight;           /* 0x98 */
    s32 mPrankeryLikelihood;             /* 0x9c */
    s32 mStinkbombLikelihood;            /* 0xa0 */
    BalanceLevelPeepType mPeepTypes[20]; /* 0xa4 */
    u8 unk_194[0x4];
    s32 mInitialCash;                    /* 0x198 */
    u32 mInitialAdmissionFee;            /* 0x19c */
    BalanceLevelLoanInfo mLoanInfo[10];  /* 0x1a0 */
    u8 unk_240[0x4];
    s32 mChanceToGetGreatHandymen;       /* 0x244 */
    s32 mChanceToGetGreatMechanics;      /* 0x248 */
    s32 mChanceToGetGreatEntertainers;   /* 0x24c */
    s32 mChanceToGetGreatGuards;         /* 0x250 */
    s32 mChanceToGetGreatResearchers;    /* 0x254 */
    s32 mAvgGradeOfHandymen;             /* 0x258 */
    s32 mAvgGradeOfMechanics;            /* 0x25c */
    s32 mAvgGradeOfEntertainers;         /* 0x260 */
    s32 mAvgGradeOfGuards;               /* 0x264 */
    s32 mAvgGradeOfResearchers;          /* 0x268 */
    u32 mBeginningNumberOfHandymen;      /* 0x26c */
    u32 mBeginningNumberOfMechanics;     /* 0x270 */
    u32 mBeginningNumberOfEntertainers;  /* 0x274 */
    u32 mBeginningNumberOfGuards;        /* 0x278 */
    u32 mBeginningNumberOfResearchers;   /* 0x27c */
    u32 mMaxHandymen;                    /* 0x280 */
    u32 mMaxMechanics;                   /* 0x284 */
    u32 mMaxEntertainers;                /* 0x288 */
    u32 mMaxGuards;                      /* 0x28c */
    u32 mMaxResearchers;                 /* 0x290 */
    u32 mMinHandymenInPool;              /* 0x294 */
    u32 mMinMechanicsInPool;             /* 0x298 */
    u32 mMinEntertainersInPool;          /* 0x29c */
    u32 mMinGuardsInPool;                /* 0x2a0 */
    u32 mMinResearchersInPool;           /* 0x2a4 */
    u32 mMaxHandymenInPark;              /* 0x2a8 */
    u32 mMaxMechanicsInPark;             /* 0x2ac */
    u32 mMaxEntertainersInPark;          /* 0x2b0 */
    u32 mMaxGuardsInPark;                /* 0x2b4 */
    u32 mMaxResearchersInPark;           /* 0x2b8 */
    u32 mBaseCostPerStaff;               /* 0x2bc */
    u32 mCostPerQualityLevel;            /* 0x2c0 */
    u32 mTimeBetweenStaffUpdates;        /* 0x2c4 */
    u32 mMaxNumberOfStaffPerUpdate;      /* 0x2c8 */
    u32 mStaffTimeoutTime;               /* 0x2cc */
    u32 mMinPeople;                      /* 0x2d0 */
    u32 mTimeBetweenArrivals;            /* 0x2d4 */
    s32 mFixedRate;                      /* 0x2d8 */
    s32 mNewParkBonus;                   /* 0x2dc */
    u32 mPointsPerVisitor;               /* 0x2e0 */
    s32 mRestLevel;                      /* 0x2e4 */
    u32 mHappyHitCosNoRestArea;          /* 0x2e8 */
    BalanceLevelPerGradeStaffConst mPerGradeStaffConsts[5]; /* 0x2ec */
    u8 unk_33c[0x4];
    u32 mPerTypeStaffConstsPayMultiplier[5]; /* 0x340 */
    u8 unk_354[0x4];
    BalanceLevelEntertainerConstsPerGrade mEntertainerConstsPerGrade[5]; /* 0x358 */
    u8 unk_3a8[0x4];
    BalanceLevelHandymanConstsPerGrade mHandymanConstsPerGrade[5]; /* 0x3ac */
    u8 unk_3e8[0x4];
    BalanceLevelMechanicConstsPerGrade mMechanicConstsPerGrade[5]; /* 0x3ec */
    u8 unk_414[0x4];
    BalanceLevelResearcherConstsPerGrade mResearcherConstsPerGrade[5]; /* 0x418 */
    u8 unk_454[0x4];
    BalanceLevelGuardConstsPerGrade mGuardConstsPerGrade[5]; /* 0x458 */
    u8 unk_480[0x4];
    BalanceLevelSeason mSeasons[4];      /* 0x484 */
    u8 unk_4b4[0x4];
    u32 mDaysOfWarning;                  /* 0x4b8 */
    s32 mDaysBetweenChanges;             /* 0x4bc */
    s32 mSpeedOfChange;                  /* 0x4c0 */
    s32 mQualityForSnowLow;              /* 0x4c4 */
    s32 mQualityForSnowHigh;             /* 0x4c8 */
    s32 mQualityForRainLow;              /* 0x4cc */
    s32 mQualityForRainHigh;             /* 0x4d0 */
    s32 mQualityForLightningLow;         /* 0x4d4 */
    s32 mQualityForLightningHigh;        /* 0x4d8 */
    s32 mLightningCountdownSpeed;        /* 0x4dc */
    s32 mLightningCountdownFrom;         /* 0x4e0 */
    u32 mLightningTime;                  /* 0x4e4 */
    s32 mThunderStartsThisFarAway;       /* 0x4e8 */
    s32 mThunderRandomiser;              /* 0x4ec */
    s32 mMaxRaindrops;                   /* 0x4f0 */
    u32 mMinTimeAnyMessage;              /* 0x4f4 */
    u32 mMinTimeSameMessage;             /* 0x4f8 */
    s32 mResearchTechPercentageForThisTech[10]; /* 0x4fc */
    u8 unk_524[0x4];
    s32 mResearchCategoriesEffort[5];    /* 0x528 */
    u8 unk_53c[0x4];
    s32 mStartingWorkLoad;               /* 0x540 */
    s32 mHeightfieldXStart;              /* 0x544 */
    s32 mHeightfieldYStart;              /* 0x548 */
    s32 mHeightfieldWidth;               /* 0x54c */
    s32 mHeightfieldHeight;              /* 0x550 */
    s32 mFixedItemOriginX;               /* 0x554 */
    s32 mFixedItemOriginY;               /* 0x558 */
    u32 mTicketBoothAPosX;               /* 0x55c */
    u32 mTicketBoothAPosY;               /* 0x560 */
    u32 mTicketBoothBPosX;               /* 0x564 */
    u32 mTicketBoothBPosY;               /* 0x568 */
    u32 mBusStopAPosX;                   /* 0x56c */
    u32 mBusStopAPosY;                   /* 0x570 */
    u32 mBusStopBPosX;                   /* 0x574 */
    u32 mBusStopBPosY;                   /* 0x578 */
    u32 mEntranceAPosX;                  /* 0x57c */
    u32 mEntranceAPosY;                  /* 0x580 */
    u32 mEntranceBPosX;                  /* 0x584 */
    u32 mEntranceBPosY;                  /* 0x588 */
    u32 mCrossingParkSideAPosX;          /* 0x58c */
    u32 mCrossingParkSideAPosY;          /* 0x590 */
    u32 mCrossingParkSideBPosX;          /* 0x594 */
    u32 mCrossingParkSideBPosY;          /* 0x598 */
    u32 mCrossingBSSideAPosX;            /* 0x59c */
    u32 mCrossingBSSideAPosY;            /* 0x5a0 */
    u32 mCrossingBSSideBPosX;            /* 0x5a4 */
    u32 mCrossingBSSideBPosY;            /* 0x5a8 */
    u32 mStrikeAreaStartX;               /* 0x5ac */
    u32 mStrikeAreaStartY;               /* 0x5b0 */
    u32 mStrikeAreaSizeX;                /* 0x5b4 */
    u32 mStrikeAreaSizeY;                /* 0x5b8 */
    u32 mQueueCell;                      /* 0x5bc */
    u32 mPathCell;                       /* 0x5c0 */
    u32 mKartTrackCell;                  /* 0x5c4 */
    u32 mWaterTrackCell;                 /* 0x5c8 */
    u32 mMapCell;                        /* 0x5cc */
    BalanceLevelRegionFX mRegionFX[8];   /* 0x5d0 */
    u8 unk_690[0x4];
    s32 mFogColour;                      /* 0x694 */
    s32 mAmbientLightLevel;              /* 0x698 */
    s32 mDirectionalLightLevel;          /* 0x69c */
    s32 mProceduralTexturingXOffset;     /* 0x6a0 */
    s32 mProceduralTexturingYOffset;     /* 0x6a4 */
    float mLightNormalX;                 /* 0x6a8 */
    float mLightNormalY;                 /* 0x6ac */
    float mLightNormalZ;                 /* 0x6b0 */
    BalanceLevelThemeAdvisorCostume mThemeAdvisorCostumes[4]; /* 0x6b4 */
    u8 unk_6d4[0x4];
    float mWaterStartPosX;               /* 0x6d8 */
    float mWaterStartPosY;               /* 0x6dc */
    float mWaterStartPosZ;               /* 0x6e0 */
    u32 mn;                              /* 0x6e4 */
    BalanceLevelSkyObject mSkyObjects[4]; /* 0x6e8 */
    u8 unk_748[0x4];
    s32 mReverb;                         /* 0x74c */
    s32 mVisitors;                       /* 0x750 */
    s32 mPeopleInPark;                   /* 0x754 */
    s32 mHappiness;                      /* 0x758 */
    u32 mAtLeastThisManyHappyPeople;     /* 0x75c */
    s32 mProfitYear;                     /* 0x760 */
    s32 mRecentVisitors;                 /* 0x764 */
    s32 mRecentVisitorMonths;            /* 0x768 */
    s32 mCoasterHeight;                  /* 0x76c */
    s32 mGokartExcitement;               /* 0x770 */
    s32 mWaterLength;                    /* 0x774 */
    s32 mMinCellsOwned;                  /* 0x778 */
    s32 mMinCellsCovered;                /* 0x77c */
    s32 mDaysAfterCompletedChallenge;    /* 0x780 */
    s32 mDaysAfterDeclinedChallenge;     /* 0x784 */
    s32 mDaysUntilFirstChallenge;        /* 0x788 */
    s32 mDeclinesToForfeit;              /* 0x78c */
    s32 mShortTimeLeftWarningAt;         /* 0x790 */
    s32 mChallengesInThisLevelChallengeType[20]; /* 0x794 */
    u8 unk_7e4[0x4];
} BalanceLevel;

/* Values of Online.sam; static instance gBalanceOnline at 0x785828. Fields are the .sam keys
 * in schema order (tools/samtool.py schema online); vtable and mSlotInfo
 * belong to the balance-file base class (0x401030). */
typedef struct BalanceOnline {
    void * vtable;                       /* 0x00 */
    void * mSlotInfo;                    /* 0x04 */
    u8 unk_08[0x4];
    s32 mMailPort;                       /* 0x0c */
    s32 mChatPort;                       /* 0x10 */
    s32 mLoginPort;                      /* 0x14 */
    s32 mCityPort;                       /* 0x18 */
    s32 mNewsPort;                       /* 0x1c */
    s32 mVotePort;                       /* 0x20 */
    char * mMailServer;                  /* 0x24 */
    char * mChatServer;                  /* 0x28 */
    char * mLoginServer;                 /* 0x2c */
    char * mCityServer;                  /* 0x30 */
    char * mNewsServer;                  /* 0x34 */
    char * mVoteServer;                  /* 0x38 */
    char * mComplainEmail;               /* 0x3c */
} BalanceOnline;

/* Values of low/med/high.sam; static instance gBalanceGraphics at 0x785868. Fields are the .sam keys
 * in schema order (tools/samtool.py schema graphics); vtable and mSlotInfo
 * belong to the balance-file base class (0x401030). */
typedef struct BalanceGraphics {
    void * vtable;                       /* 0x00 */
    void * mSlotInfo;                    /* 0x04 */
    u8 unk_08[0x4];
    s32 mTEXTUREQUALITY;                 /* 0x0c */
    s32 mTEXTUREFILTERING;               /* 0x10 */
    s32 mPROCEDURALTEXTURING;            /* 0x14 */
    s32 mPROCEDURALTEXTURECACHE;         /* 0x18 */
    s32 mANIMATINGTEXTURES;              /* 0x1c */
    s32 mSKYQUALITY;                     /* 0x20 */
    s32 mSKYDETAIL;                      /* 0x24 */
    s32 mSKYSHADOW;                      /* 0x28 */
    s32 mMESHSHADOW;                     /* 0x2c */
    s32 mSPRITESHADOW;                   /* 0x30 */
    s32 mTRIPLEBUFFER;                   /* 0x34 */
    s32 mFOG;                            /* 0x38 */
    s32 mMIPMAP;                         /* 0x3c */
    s32 mRENDER32;                       /* 0x40 */
    s32 mTEXTURE32;                      /* 0x44 */
    s32 mBUMPMAPPING;                    /* 0x48 */
    s32 mFIRSTPERSONVIEWDISTANCE;        /* 0x4c */
    s32 mLIGHTING;                       /* 0x50 */
    s32 mFORCELOWALPHAREF;               /* 0x54 */
    s32 mCOASTERSMOOTHNESS;              /* 0x58 */
    s32 mCOASTERTRACKDETAIL;             /* 0x5c */
    s32 mPARTICLEDENSITY;                /* 0x60 */
    s32 mTOTALPARTICLES;                 /* 0x64 */
    s32 mNUMKIDS;                        /* 0x68 */
    s32 mWEATHER;                        /* 0x6c */
    s32 mWATER;                          /* 0x70 */
    s32 mLOBBYOBJECTS;                   /* 0x74 */
} BalanceGraphics;

/* Values of Resolution.sam; static instance gBalanceResolution at 0x7858e0. Fields are the .sam keys
 * in schema order (tools/samtool.py schema resolution); vtable and mSlotInfo
 * belong to the balance-file base class (0x401030). */
typedef struct BalanceResolution {
    void * vtable;                       /* 0x00 */
    void * mSlotInfo;                    /* 0x04 */
    u8 unk_08[0x4];
    s32 mRESOLUTION;                     /* 0x0c */
} BalanceResolution;

/* One row of BalanceSound.mChannelInfo: the ChannelInfo[i].* keys. */
typedef struct BalanceSoundChannelInfo {
    s32 mCHANNELS;                       /* 0x00 */
    s32 mSWITCH;                         /* 0x04 */
} BalanceSoundChannelInfo;

/* One row of BalanceSound.mRadiusInfo: the RadiusInfo[i].* keys. */
typedef struct BalanceSoundRadiusInfo {
    float mMINRADIUS;                    /* 0x00 */
    s32 mSWITCH;                         /* 0x04 */
} BalanceSoundRadiusInfo;

/* Values of the sound settings; static instance gBalanceSound at 0x7858f0. Fields are the .sam keys
 * in schema order (tools/samtool.py schema sound); vtable and mSlotInfo
 * belong to the balance-file base class (0x401030). */
typedef struct BalanceSound {
    void * vtable;                       /* 0x00 */
    void * mSlotInfo;                    /* 0x04 */
    u8 unk_08[0x4];
    s32 mQSOUND;                         /* 0x0c */
    s32 mREAR;                           /* 0x10 */
    s32 mCROSSOVER;                      /* 0x14 */
    s32 mEAX;                            /* 0x18 */
    s32 mGEOMETRY;                       /* 0x1c */
    s32 mMPEGRATE;                       /* 0x20 */
    s32 mDUCKINGLEVEL;                   /* 0x24 */
    s32 mDROPPABLELEVEL;                 /* 0x28 */
    s32 mDEFAULTQUALITY;                 /* 0x2c */
    s32 mFADETOGGLE;                     /* 0x30 */
    s32 mSOUNDON;                        /* 0x34 */
    s32 mSFX;                            /* 0x38 */
    s32 mMUSIC;                          /* 0x3c */
    s32 mSPEECH;                         /* 0x40 */
    s32 mMOVIE;                          /* 0x44 */
    BalanceSoundChannelInfo mChannelInfo[3]; /* 0x48 */
    u8 unk_60[0x4];
    BalanceSoundRadiusInfo mRadiusInfo[3]; /* 0x64 */
    u8 unk_7c[0x4];
} BalanceSound;

/* One row of BalanceHoardings.mHoardingClicks: the HoardingClicks[i].* keys. */
typedef struct BalanceHoardingsHoardingClick {
    s32 mX;                              /* 0x00 */
    s32 mY;                              /* 0x04 */
} BalanceHoardingsHoardingClick;

/* Values of Hoardings.sam / EndHoardings.sam (one each); static instance gBalanceHoardings at 0x784e30, 0x784c20. Fields are the .sam keys
 * in schema order (tools/samtool.py schema hoardings); vtable and mSlotInfo
 * belong to the balance-file base class (0x401030). */
typedef struct BalanceHoardings {
    void * vtable;                       /* 0x00 */
    void * mSlotInfo;                    /* 0x04 */
    u8 unk_08[0x4];
    BalanceHoardingsHoardingClick mHoardingClicks[64]; /* 0x0c */
    u8 unk_20c[0x4];
} BalanceHoardings;

/* One row of BalanceTrack.masTextureData: the asTextureData[i].* keys. */
typedef struct BalanceTrackAsTextureData {
    char * mpcTextureFilename;           /* 0x00 */
    s32 mbIsSelfIlluminating;            /* 0x04 */
    float mfScrollRate;                  /* 0x08 */
} BalanceTrackAsTextureData;

/* One row of BalanceTrack.masCrossSectionPointsN: the asCrossSectionPointsN[i].* keys. */
typedef struct BalanceTrackAsCrossSectionPoint {
    float mfX;                           /* 0x00 */
    float mfY;                           /* 0x04 */
    float mfU;                           /* 0x08 */
    float mfNx;                          /* 0x0c */
    float mfNy;                          /* 0x10 */
} BalanceTrackAsCrossSectionPoint;

/* One row of BalanceTrack.masCrossSectionEdgesN: the asCrossSectionEdgesN[i].* keys. */
typedef struct BalanceTrackAsCrossSectionEdge {
    s32 musTexture;                      /* 0x00 */
    s32 musCSPoint1;                     /* 0x04 */
    s32 musCSPoint2;                     /* 0x08 */
} BalanceTrackAsCrossSectionEdge;

/* One row of BalanceTrack.masCrossSections: the asCrossSections[i].* keys. */
typedef struct BalanceTrackAsCrossSection {
    s32 musCSPointBlockIndex;            /* 0x00 */
    s32 musCSEdgeBlockIndex;             /* 0x04 */
    s32 musTrackEdge;                    /* 0x08 */
    s32 musBlendToCSPointBlockIndex;     /* 0x0c */
    float mfBlendLength;                 /* 0x10 */
} BalanceTrackAsCrossSection;

/* One row of BalanceTrack.masCrossSectionSelects: the asCrossSectionSelects[i].* keys. */
typedef struct BalanceTrackAsCrossSectionSelect {
    float mfSpeedRangeMin;               /* 0x00 */
    float mfSpeedRangeMax;               /* 0x04 */
    float mfDirectionRangeMin;           /* 0x08 */
    float mfDirectionRangeMax;           /* 0x0c */
    float mfPitchRangeMin;               /* 0x10 */
    float mfPitchRangeMax;               /* 0x14 */
    float mfBankRangeMin;                /* 0x18 */
    float mfBankRangeMax;                /* 0x1c */
    s32 musCrossSectionIndex;            /* 0x20 */
    s32 musBlendFromCrossSectionIndex;   /* 0x24 */
    s32 musBlendToCrossSectionIndex;     /* 0x28 */
} BalanceTrackAsCrossSectionSelect;

/* One row of BalanceTrack.masPylonControls: the asPylonControls[i].* keys. */
typedef struct BalanceTrackAsPylonControl {
    char * mpcMeshFilename;              /* 0x00 */
    u32 muiPylonTypeUniqueID;            /* 0x04 */
    s32 mbDontDisableBaseOnStack;        /* 0x08 */
    float mfCollisionPossibleRadius;     /* 0x0c */
    float mfPylonStraightDotThreshold;   /* 0x10 */
    s32 muiPylonWidthCells;              /* 0x14 */
    s32 muiPylonLengthCells;             /* 0x18 */
    s32 muiMaxStackedPylons;             /* 0x1c */
    float mfMinHeight;                   /* 0x20 */
    float mfMaxHeight;                   /* 0x24 */
    float mfMaxHeightDiffPerGSquareDist; /* 0x28 */
    float mfEntranceMinXZAngleAtMinDist; /* 0x2c */
    float mfEntranceMaxXZAngleAtMinDist; /* 0x30 */
    float mfEntranceMinXZAngleAtMaxDist; /* 0x34 */
    float mfEntranceMaxXZAngleAtMaxDist; /* 0x38 */
    float mfMinXZAngleAtMinDist;         /* 0x3c */
    float mfMaxXZAngleAtMinDist;         /* 0x40 */
    float mfMinXZAngleAtMaxDist;         /* 0x44 */
    float mfMaxXZAngleAtMaxDist;         /* 0x48 */
    float mfMaxRotnAngleDeviance;        /* 0x4c */
    float mfMaxLoopRotnAngleDeviance;    /* 0x50 */
    u32 muiMinPlaceDist;                 /* 0x54 */
    u32 muiMaxPlaceDist;                 /* 0x58 */
    float mfMaxLoopHeightDifference;     /* 0x5c */
    float mfCostPerUnit;                 /* 0x60 */
    float mfCostPer2DLength;             /* 0x64 */
    float mfCostPerHeight;               /* 0x68 */
    float mfLoftAnimStartCap;            /* 0x6c */
    float mfLoftAnimEndCap;              /* 0x70 */
    float mfLoftAnimMultiplier;          /* 0x74 */
    float mfLoftAnimAdd;                 /* 0x78 */
    float mfRotateAnimStartCap;          /* 0x7c */
    float mfRotateAnimEndCap;            /* 0x80 */
    float mfRotateAnimMultiplier;        /* 0x84 */
    float mfRotateAnimAdd;               /* 0x88 */
    float mfInclineAnimStartCap;         /* 0x8c */
    float mfInclineAnimEndCap;           /* 0x90 */
    float mfInclineAnimMultiplier;       /* 0x94 */
    float mfInclineAnimAdd;              /* 0x98 */
    float mfBankAnimStartCap;            /* 0x9c */
    float mfBankAnimEndCap;              /* 0xa0 */
    float mfBankAnimMultiplier;          /* 0xa4 */
    float mfBankAnimAdd;                 /* 0xa8 */
    float mfLoopRotateAnimStartCap;      /* 0xac */
    float mfLoopRotateAnimEndCap;        /* 0xb0 */
    float mfLoopRotateAnimMultiplier;    /* 0xb4 */
    float mfLoopRotateAnimAdd;           /* 0xb8 */
} BalanceTrackAsPylonControl;

/* One row of BalanceTrack.masCarTypes: the asCarTypes[i].* keys. */
typedef struct BalanceTrackAsCarType {
    s32 mbBendCar;                       /* 0x00 */
    u32 muiCarTypeUniqueID;              /* 0x04 */
    char * mpcMeshFilename;              /* 0x08 */
    float mfFrontDistanceAdd;            /* 0x0c */
    float mfRearDistanceAdd;             /* 0x10 */
    float mfYOffset;                     /* 0x14 */
    float mfAnimSpeedMultiplier;         /* 0x18 */
} BalanceTrackAsCarType;

/* One row of BalanceTrack.masTrackCollisionPoints: the asTrackCollisionPoints[i].* keys. */
typedef struct BalanceTrackAsTrackCollisionPoint {
    float mfCollisionX;                  /* 0x00 */
    float mfCollisionY;                  /* 0x04 */
} BalanceTrackAsTrackCollisionPoint;

/* Values of a coaster/track type .sam (textures, cross sections, cars),
 * read into a stack object (vtable 0x6fd8b0) and copied out. Fields are the .sam keys in schema order
 * (tools/samtool.py schema track); vtable and mSlotInfo belong to the
 * balance-file base class (0x401030). */
typedef struct BalanceTrack {
    void * vtable;                       /* 0x00 */
    void * mSlotInfo;                    /* 0x04 */
    u8 unk_08[0x4];
    BalanceTrackAsTextureData masTextureData[20]; /* 0x0c */
    u8 unk_fc[0x4];
    BalanceTrackAsCrossSectionPoint masCrossSectionPoints1[20]; /* 0x100 */
    u8 unk_290[0x4];
    BalanceTrackAsCrossSectionPoint masCrossSectionPoints2[20]; /* 0x294 */
    u8 unk_424[0x4];
    BalanceTrackAsCrossSectionPoint masCrossSectionPoints3[20]; /* 0x428 */
    u8 unk_5b8[0x4];
    BalanceTrackAsCrossSectionPoint masCrossSectionPoints4[20]; /* 0x5bc */
    u8 unk_74c[0x4];
    BalanceTrackAsCrossSectionPoint masCrossSectionPoints5[20]; /* 0x750 */
    u8 unk_8e0[0x4];
    BalanceTrackAsCrossSectionPoint masCrossSectionPoints6[20]; /* 0x8e4 */
    u8 unk_a74[0x4];
    BalanceTrackAsCrossSectionPoint masCrossSectionPoints7[20]; /* 0xa78 */
    u8 unk_c08[0x4];
    BalanceTrackAsCrossSectionPoint masCrossSectionPoints8[20]; /* 0xc0c */
    u8 unk_d9c[0x4];
    BalanceTrackAsCrossSectionPoint masCrossSectionPoints9[20]; /* 0xda0 */
    u8 unk_f30[0x4];
    BalanceTrackAsCrossSectionPoint masCrossSectionPoints10[20]; /* 0xf34 */
    u8 unk_10c4[0x4];
    BalanceTrackAsCrossSectionPoint masCrossSectionPoints11[20]; /* 0x10c8 */
    u8 unk_1258[0x4];
    BalanceTrackAsCrossSectionPoint masCrossSectionPoints12[20]; /* 0x125c */
    u8 unk_13ec[0x4];
    BalanceTrackAsCrossSectionPoint masCrossSectionPoints13[20]; /* 0x13f0 */
    u8 unk_1580[0x4];
    BalanceTrackAsCrossSectionPoint masCrossSectionPoints14[20]; /* 0x1584 */
    u8 unk_1714[0x4];
    BalanceTrackAsCrossSectionPoint masCrossSectionPoints15[20]; /* 0x1718 */
    u8 unk_18a8[0x4];
    BalanceTrackAsCrossSectionPoint masCrossSectionPoints16[20]; /* 0x18ac */
    u8 unk_1a3c[0x4];
    BalanceTrackAsCrossSectionPoint masCrossSectionPoints17[20]; /* 0x1a40 */
    u8 unk_1bd0[0x4];
    BalanceTrackAsCrossSectionPoint masCrossSectionPoints18[20]; /* 0x1bd4 */
    u8 unk_1d64[0x4];
    BalanceTrackAsCrossSectionPoint masCrossSectionPoints19[20]; /* 0x1d68 */
    u8 unk_1ef8[0x4];
    BalanceTrackAsCrossSectionPoint masCrossSectionPoints20[20]; /* 0x1efc */
    u8 unk_208c[0x4];
    BalanceTrackAsCrossSectionEdge masCrossSectionEdges1[20]; /* 0x2090 */
    u8 unk_2180[0x4];
    BalanceTrackAsCrossSectionEdge masCrossSectionEdges2[20]; /* 0x2184 */
    u8 unk_2274[0x4];
    BalanceTrackAsCrossSectionEdge masCrossSectionEdges3[20]; /* 0x2278 */
    u8 unk_2368[0x4];
    BalanceTrackAsCrossSectionEdge masCrossSectionEdges4[20]; /* 0x236c */
    u8 unk_245c[0x4];
    BalanceTrackAsCrossSectionEdge masCrossSectionEdges5[20]; /* 0x2460 */
    u8 unk_2550[0x4];
    BalanceTrackAsCrossSectionEdge masCrossSectionEdges6[20]; /* 0x2554 */
    u8 unk_2644[0x4];
    BalanceTrackAsCrossSectionEdge masCrossSectionEdges7[20]; /* 0x2648 */
    u8 unk_2738[0x4];
    BalanceTrackAsCrossSectionEdge masCrossSectionEdges8[20]; /* 0x273c */
    u8 unk_282c[0x4];
    BalanceTrackAsCrossSectionEdge masCrossSectionEdges9[20]; /* 0x2830 */
    u8 unk_2920[0x4];
    BalanceTrackAsCrossSectionEdge masCrossSectionEdges10[20]; /* 0x2924 */
    u8 unk_2a14[0x4];
    BalanceTrackAsCrossSectionEdge masCrossSectionEdges11[20]; /* 0x2a18 */
    u8 unk_2b08[0x4];
    BalanceTrackAsCrossSectionEdge masCrossSectionEdges12[20]; /* 0x2b0c */
    u8 unk_2bfc[0x4];
    BalanceTrackAsCrossSectionEdge masCrossSectionEdges13[20]; /* 0x2c00 */
    u8 unk_2cf0[0x4];
    BalanceTrackAsCrossSectionEdge masCrossSectionEdges14[20]; /* 0x2cf4 */
    u8 unk_2de4[0x4];
    BalanceTrackAsCrossSectionEdge masCrossSectionEdges15[20]; /* 0x2de8 */
    u8 unk_2ed8[0x4];
    BalanceTrackAsCrossSectionEdge masCrossSectionEdges16[20]; /* 0x2edc */
    u8 unk_2fcc[0x4];
    BalanceTrackAsCrossSectionEdge masCrossSectionEdges17[20]; /* 0x2fd0 */
    u8 unk_30c0[0x4];
    BalanceTrackAsCrossSectionEdge masCrossSectionEdges18[20]; /* 0x30c4 */
    u8 unk_31b4[0x4];
    BalanceTrackAsCrossSectionEdge masCrossSectionEdges19[20]; /* 0x31b8 */
    u8 unk_32a8[0x4];
    BalanceTrackAsCrossSectionEdge masCrossSectionEdges20[20]; /* 0x32ac */
    u8 unk_339c[0x4];
    BalanceTrackAsCrossSection masCrossSections[20]; /* 0x33a0 */
    u8 unk_3530[0x4];
    BalanceTrackAsCrossSectionSelect masCrossSectionSelects[20]; /* 0x3534 */
    u8 unk_38a4[0x4];
    BalanceTrackAsPylonControl masPylonControls[2]; /* 0x38a8 */
    u8 unk_3a20[0x4];
    BalanceTrackAsCarType masCarTypes[3]; /* 0x3a24 */
    u8 unk_3a78[0x4];
    u32 muiMaxCars;                      /* 0x3a7c */
    s32 muiFrontCarTypeIndex;            /* 0x3a80 */
    s32 muiCentreCarTypeIndex;           /* 0x3a84 */
    s32 muiRearCarTypeIndex;             /* 0x3a88 */
    float mfFrontCarSpacing;             /* 0x3a8c */
    float mfStdCarSpacing;               /* 0x3a90 */
    float mfEndCarSpacing;               /* 0x3a94 */
    BalanceTrackAsTrackCollisionPoint masTrackCollisionPoints[6]; /* 0x3a98 */
    u8 unk_3ac8[0x4];
    float mfCurviness;                   /* 0x3acc */
    float mfStraightDivisor;             /* 0x3ad0 */
    float mfAutoBankDivisor;             /* 0x3ad4 */
    float mfAccelerationPerHeight;       /* 0x3ad8 */
    float mfUphillAccelModifier;         /* 0x3adc */
    float mfDownhillAccelModifier;       /* 0x3ae0 */
    float mfInitialPylonLoftHeight;      /* 0x3ae4 */
    s32 muiMaxTrains;                    /* 0x3ae8 */
    u32 muiTrainReleaseRate;             /* 0x3aec */
    s32 mbSpecialIsTrough;               /* 0x3af0 */
    s32 mbIsSuspendedCoaster;            /* 0x3af4 */
    s32 mbNoAutoBanking;                 /* 0x3af8 */
    s32 mbVisibleEndPylon;               /* 0x3afc */
    s32 muiInvalidCrossSection;          /* 0x3b00 */
    s32 muiWinchCrossSection;            /* 0x3b04 */
    float mfTrackHeightAdjust;           /* 0x3b08 */
    float mfWinchSpeed;                  /* 0x3b0c */
    float mfMinSpeed;                    /* 0x3b10 */
    float mfMaxSpeedAtMinSetting;        /* 0x3b14 */
    float mfMaxSpeedAtMaxSetting;        /* 0x3b18 */
    float mfFrictionMultiplier;          /* 0x3b1c */
    float mfBendElevationLow;            /* 0x3b20 */
    float mfBendElevationHigh;           /* 0x3b24 */
    float mfGravityX;                    /* 0x3b28 */
    float mfGravityY;                    /* 0x3b2c */
    float mfGravityZ;                    /* 0x3b30 */
    float mfForceMultiplier;             /* 0x3b34 */
    float mfInitialSickness;             /* 0x3b38 */
    float mfInitialExcitement;           /* 0x3b3c */
    float mfInitialUncomfortableness;    /* 0x3b40 */
    float mfGForceSicknessModifier;      /* 0x3b44 */
    float mfUpsideDownSicknessModifier;  /* 0x3b48 */
    float mfSpeedExcitementModifier;     /* 0x3b4c */
    float mfGForceExcitementModifier;    /* 0x3b50 */
    float mfUpsideDownExcitementModifier; /* 0x3b54 */
    float mfBankExcitementModifier;      /* 0x3b58 */
    float mfXGForceUncomfortableModifier; /* 0x3b5c */
    float mfInvertedUncomfortableModifier; /* 0x3b60 */
    float mfNormalScreamStartExciteDelta; /* 0x3b64 */
    float mfTerrorScreamStartExciteDelta; /* 0x3b68 */
    float mfNormalScreamSustainExcitement; /* 0x3b6c */
    float mfTerrorScreamSustainExcitement; /* 0x3b70 */
    float mfEntranceSizeInterval;        /* 0x3b74 */
    s32 miTrackSectionMeshVerts;         /* 0x3b78 */
    s32 miTrackSectionMeshFaces;         /* 0x3b7c */
} BalanceTrack;

/* One row of BalanceChallenges.mChallenges: the Challenges[i].* keys. */
typedef struct BalanceChallengesChallenge {
    s32 mType;                           /* 0x00 */
    s32 mFollowupType;                   /* 0x04 */
    s32 mTargetTime;                     /* 0x08 */
    s32 mTargetVal;                      /* 0x0c */
    s32 mTargetObj;                      /* 0x10 */
    s32 mTargetObj2;                     /* 0x14 */
    s32 mTargetStaffType;                /* 0x18 */
    s32 mPrize;                          /* 0x1c */
    s32 mCheckAtEndOnly;                 /* 0x20 */
    s32 mIndependent;                    /* 0x24 */
} BalanceChallengesChallenge;

/* Values of data/challenges.sam, read into a stack object
 * (vtable 0x700398) by the challenge loader 0x4d1ce0. Fields are the .sam keys in schema order
 * (tools/samtool.py schema challenges); vtable and mSlotInfo belong to the
 * balance-file base class (0x401030). */
typedef struct BalanceChallenges {
    void * vtable;                       /* 0x00 */
    void * mSlotInfo;                    /* 0x04 */
    u8 unk_08[0x4];
    BalanceChallengesChallenge mChallenges[60]; /* 0x0c */
    u8 unk_96c[0x4];
} BalanceChallenges;

/* Values of a theme's .sam (tickets, keys, park gate); heap object (vtable
 * 0x702800) whose owner keeps it at +0x1c. Fields are the .sam keys in schema order
 * (tools/samtool.py schema theme); vtable and mSlotInfo belong to the
 * balance-file base class (0x401030). */
typedef struct BalanceTheme {
    void * vtable;                       /* 0x00 */
    void * mSlotInfo;                    /* 0x04 */
    u8 unk_08[0x4];
    s32 mCanEarnTicketsInTheme;          /* 0x0c */
    s32 mCanSpendTicketsInTheme;         /* 0x10 */
    s32 mCostToEnter;                    /* 0x14 */
    u32 mGateObjectId;                   /* 0x18 */
} BalanceTheme;

/* One row of AdvisorScoring.mMessageGroups: the MessageGroups[i].* keys. */
typedef struct AdvisorScoringMessageGroup {
    u32 mMinTimeSameMessage;             /* 0x00 */
    s32 mSayOnlyOnce;                    /* 0x04 */
    u32 mDiscardAfterSlaps;              /* 0x08 */
} AdvisorScoringMessageGroup;

/* data/advisor/advisor.sam: how the advisor scores and picks messages. A function-static
 * singleton at 0xfb3540 (AdvisorScoring_Construct 0x59bea0, guard bit 0xfb2766 & 1);
 * the balance-file base class sits at +0xc, so the .sam keys (tools/samtool.py schema
 * advisor) start at +0x14. */
typedef struct AdvisorScoring {
    u8 unk_00[0xc];
    void * vtable;                       /* 0x0c */
    void * mSlotInfo;                    /* 0x10 */
    u8 unk_14[0x4];
    u32 mMinTimeAnyMessage;              /* 0x18 */
    u32 mMinTimeSameMessage;             /* 0x1c */
    u32 mMinScoreForConsideration;       /* 0x20 */
    AdvisorScoringMessageGroup mMessageGroups[10]; /* 0x24 */
    u8 unk_9c[0x4];
    s32 mWelcomeScore;                   /* 0xa0 */
    s32 mScorePerWaitingPerson;          /* 0xa4 */
    s32 mCloseParkScore;                 /* 0xa8 */
    s32 mScorePerThirstyPerson;          /* 0xac */
    s32 mThirstierThan;                  /* 0xb0 */
    s32 mScorePerHungryPerson;           /* 0xb4 */
    s32 mHungrierThan;                   /* 0xb8 */
    s32 mScorePerWornRide;               /* 0xbc */
    s32 mPoorerStateThan;                /* 0xc0 */
    s32 mScorePerBrokenRide;             /* 0xc4 */
    s32 mIdealNumberOfRidesPerMechanic;  /* 0xc8 */
    s32 mScorePerExtraRide;              /* 0xcc */
    s32 mStaffHireHandymen1ScorePerLitterPoint; /* 0xd0 */
    s32 mStaffHireHandymen1LitterPointCutoffLow; /* 0xd4 */
    s32 mLitterPointCutoffHigh;          /* 0xd8 */
    s32 mStaffHireHandymen2ScorePerLitterPoint; /* 0xdc */
    s32 mStaffHireHandymen2LitterPointCutoffLow; /* 0xe0 */
    s32 mIdealDevelopedCellsPerHandyman; /* 0xe4 */
    s32 mStaffHireHandymen3ScorePerExtra10Cells; /* 0xe8 */
    s32 mStaffHireHandymen3MinCellsForConsideration; /* 0xec */
    s32 mStaffHireGuards1AtLeastThisBusy; /* 0xf0 */
    s32 mStaffHireGuards1ScorePerExtraBusyPoint; /* 0xf4 */
    s32 mStaffHireGuards2Score;          /* 0xf8 */
    s32 mAdditionalPerMisbehavingKid;    /* 0xfc */
    s32 mIdealDevelopedCellsPerGuard;    /* 0x100 */
    s32 mStaffHireGuards3ScorePerExtra10Cells; /* 0x104 */
    s32 mStaffHireGuards3MinCellsForConsideration; /* 0x108 */
    s32 mStaffHireEntertainers1AtLeastThisBusy; /* 0x10c */
    s32 mStaffHireEntertainers1ScorePerExtraBusyPoint; /* 0x110 */
    s32 mMost;                           /* 0x114 */
    s32 mPointsPerExtraPct;              /* 0x118 */
    s32 mStaffHireResearchers2Score;     /* 0x11c */
    u32 mStaffHireResearchers3MonthsIntoGameForConsideration; /* 0x120 */
    s32 mSlowerThan;                     /* 0x124 */
    s32 mStaffHireResearchers3Score;     /* 0x128 */
    s32 mStaffSetMechanicAreaAtLeastThisMany; /* 0x12c */
    s32 mStaffSetMechanicAreaDontBotherIfPatrolledUp; /* 0x130 */
    s32 mStaffSetMechanicAreaScorePerUnpatrolledPct; /* 0x134 */
    s32 mStaffSetMechanicAreaMonthsIntoGameForConsideration; /* 0x138 */
    s32 mStaffMechanicCoverageScore;     /* 0x13c */
    s32 mStaffMechanicCoverageScorePerNonCoveredPercent; /* 0x140 */
    s32 mStaffSetHandymanAreaAtLeastThisMany; /* 0x144 */
    s32 mStaffSetHandymanAreaDontBotherIfPatrolledUp; /* 0x148 */
    s32 mStaffSetHandymanAreaScorePerUnpatrolledPct; /* 0x14c */
    s32 mStaffSetHandymanAreaMonthsIntoGameForConsideration; /* 0x150 */
    s32 mStaffHandymanCoverageScore;     /* 0x154 */
    s32 mStaffHandymanCoverageScorePerNonCoveredPercent; /* 0x158 */
    u32 mAcceptableTrouble;              /* 0x15c */
    s32 mStaffGuardBuildCameraScore;     /* 0x160 */
    s32 mStaffSetGuardAreaAtLeastThisMany; /* 0x164 */
    s32 mStaffSetGuardAreaDontBotherIfPatrolledUp; /* 0x168 */
    s32 mStaffSetGuardAreaScorePerUnpatrolledPct; /* 0x16c */
    s32 mStaffSetGuardAreaMonthsIntoGameForConsideration; /* 0x170 */
    s32 mStaffGuardCoverageScore;        /* 0x174 */
    s32 mStaffGuardCoverageScorePerNonCoveredPercent; /* 0x178 */
    s32 mStaffSetEntertainerAreaAtLeastThisMany; /* 0x17c */
    s32 mStaffSetEntertainerAreaDontBotherIfPatrolledUp; /* 0x180 */
    s32 mStaffSetEntertainerAreaScorePerUnpatrolledPct; /* 0x184 */
    s32 mStaffSetEntertainerAreaMonthsIntoGameForConsideration; /* 0x188 */
    s32 mStaffEntertainerCoverageScore;  /* 0x18c */
    s32 mStaffEntertainerCoverageScorePerNonCoveredPercent; /* 0x190 */
    s32 mStaffTrainMechanicsWorseThan;   /* 0x194 */
    s32 mStaffTrainMechanicsScore;       /* 0x198 */
    s32 mStaffTrainHandymenWorseThan;    /* 0x19c */
    s32 mStaffTrainHandymenScore;        /* 0x1a0 */
    s32 mStaffTrainGuardsWorseThan;      /* 0x1a4 */
    s32 mStaffTrainGuardsScore;          /* 0x1a8 */
    s32 mStaffTrainEntertainersWorseThan; /* 0x1ac */
    s32 mStaffTrainEntertainersScore;    /* 0x1b0 */
    s32 mStaffTrainResearchersWorseThan; /* 0x1b4 */
    s32 mStaffTrainResearchersScore;     /* 0x1b8 */
    s32 mStaffUnhappyMechanicsScore;     /* 0x1bc */
    s32 mStaffUnhappyHandymenScore;      /* 0x1c0 */
    s32 mStaffUnhappyGuardsScore;        /* 0x1c4 */
    s32 mStaffUnhappyEntertainersScore;  /* 0x1c8 */
    s32 mStaffUnhappyResearchersScore;   /* 0x1cc */
    s32 mStaffVeryUnhappyMechanicsScore; /* 0x1d0 */
    s32 mStaffVeryUnhappyHandymenScore;  /* 0x1d4 */
    s32 mStaffVeryUnhappyGuardsScore;    /* 0x1d8 */
    s32 mStaffVeryUnhappyEntertainersScore; /* 0x1dc */
    s32 mStaffVeryUnhappyResearchersScore; /* 0x1e0 */
    s32 mStaffStrikingMechanicsScore;    /* 0x1e4 */
    s32 mStaffStrikingHandymenScore;     /* 0x1e8 */
    s32 mStaffStrikingGuardsScore;       /* 0x1ec */
    s32 mStaffStrikingEntertainersScore; /* 0x1f0 */
    s32 mStaffStrikingResearchersScore;  /* 0x1f4 */
    s32 mStaffHappierMechanicsScore;     /* 0x1f8 */
    s32 mStaffHappierHandymenScore;      /* 0x1fc */
    s32 mStaffHappierGuardsScore;        /* 0x200 */
    s32 mStaffHappierEntertainersScore;  /* 0x204 */
    s32 mStaffHappierResearchersScore;   /* 0x208 */
    s32 mStrikeEndBadMechanicsScore;     /* 0x20c */
    s32 mStrikeEndBadHandymenScore;      /* 0x210 */
    s32 mStrikeEndBadGuardsScore;        /* 0x214 */
    s32 mStrikeEndBadEntertainersScore;  /* 0x218 */
    s32 mStrikeEndBadResearchersScore;   /* 0x21c */
    s32 mRidesPoorSelectionPoorerThan;   /* 0x220 */
    s32 mMoreRidesThan;                  /* 0x224 */
    s32 mRidesPoorSelectionScore;        /* 0x228 */
    s32 mRidesResearchMoreScore;         /* 0x22c */
    s32 mRidesBuildRidesScore;           /* 0x230 */
    s32 mRidesBuildRidesAdditionalPerDay; /* 0x234 */
    u32 mRidesNoUpgradesMonthsIntoGameForConsideration; /* 0x238 */
    s32 mRidesNoUpgradesScore;           /* 0x23c */
    s32 mLessAvailableThan;              /* 0x240 */
    s32 mRidesMoreUpgradesScore;         /* 0x244 */
    s32 mRidesBreakdownImminentScore;    /* 0x248 */
    s32 mRidesBreakdownNoMechanicsScore; /* 0x24c */
    s32 mRidesBreakdownBusyMechanicsScore; /* 0x250 */
    s32 mRidesBreakdownMechanicOnItScore; /* 0x254 */
    s32 mShopsPoorSelectionPoorerThan;   /* 0x258 */
    s32 mMoreShopsThan;                  /* 0x25c */
    s32 mShopsPoorSelectionScore;        /* 0x260 */
    s32 mShopsResearchMoreScore;         /* 0x264 */
    s32 mShopsBuildShopsScore;           /* 0x268 */
    s32 mShopsBuildShopsAdditionalPerDay; /* 0x26c */
    float mShopsPerRideRatio;            /* 0x270 */
    s32 mShopsBuildMoreShopsWealthMoreThan; /* 0x274 */
    s32 mShopsBuildMoreShopsScore;       /* 0x278 */
    s32 mSideshowsPoorSelectionPoorerThan; /* 0x27c */
    s32 mMoreSideshowsThan;              /* 0x280 */
    s32 mSideshowsPoorSelectionScore;    /* 0x284 */
    s32 mSideshowsResearchMoreScore;     /* 0x288 */
    s32 mSideshowsBuildSideshowsScore;   /* 0x28c */
    s32 mSideshowsBuildSideshowsAdditionalPerDay; /* 0x290 */
    float mSideshowsPerRideRatio;        /* 0x294 */
    s32 mSideshowsBuildMoreSideshowsWealthMoreThan; /* 0x298 */
    s32 mSideshowsBuildMoreSideshowsScore; /* 0x29c */
    s32 mFeaturesPoorSelectionPoorerThan; /* 0x2a0 */
    s32 mMoreFeaturesThan;               /* 0x2a4 */
    s32 mFeaturesPoorSelectionScore;     /* 0x2a8 */
    s32 mFeaturesResearchMoreScore;      /* 0x2ac */
    s32 mFeaturesBuildFeaturesScore;     /* 0x2b0 */
    s32 mFeaturesBuildFeaturesAdditionalPerDay; /* 0x2b4 */
    s32 mNeedingMoreThan;                /* 0x2b8 */
    s32 mMorePeopleThanThisNeeding;      /* 0x2bc */
    s32 mFeaturesNoToiletsScore;         /* 0x2c0 */
    s32 mFeaturesFarToToiletsScore;      /* 0x2c4 */
    float mAverageQueueLongerThan;       /* 0x2c8 */
    s32 mFeaturesNotEnoughToiletsScore;  /* 0x2cc */
    s32 mFeaturesDirtyToilets1FilthierThan; /* 0x2d0 */
    s32 mFeaturesDirtyToilets1PointsPerFilthPct; /* 0x2d4 */
    s32 mFeaturesDirtyToilets2FilthierThan; /* 0x2d8 */
    s32 mFeaturesDirtyToilets2PointsPerFilthPct; /* 0x2dc */
    s32 mFeaturesDirtyToilets3FilthierThan; /* 0x2e0 */
    s32 mFeaturesDirtyToilets3PointsPerFilthPct; /* 0x2e4 */
    s32 mFeaturesDirtyToilets4FilthierThan; /* 0x2e8 */
    s32 mFeaturesDirtyToilets4PointsPerFilthPct; /* 0x2ec */
    s32 mTicketSatisfactionUnder;        /* 0x2f0 */
    s32 mPricingTicketTooExpensiveScore; /* 0x2f4 */
    s32 mTicketSatisfactionOver;         /* 0x2f8 */
    s32 mPricingTicketTooCheapScore;     /* 0x2fc */
    s32 mResearchRideResearchedScore;    /* 0x300 */
    s32 mResearchAddonResearchedScore;   /* 0x304 */
    s32 mResearchShopResearchedScore;    /* 0x308 */
    s32 mResearchSideshowResearchedScore; /* 0x30c */
    s32 mResearchFeatureResearchedScore; /* 0x310 */
    s32 mMoreThanThisGotAway;            /* 0x314 */
    s32 mScorePerGetAway;                /* 0x318 */
    s32 mGeneralWonGoldenTicketScore;    /* 0x31c */
    s32 mAwardGivenScore;                /* 0x320 */
    s32 mAwardMovedScore;                /* 0x324 */
    s32 mHelpMessageScore;               /* 0x328 */
    s32 mContextHelpScore;               /* 0x32c */
    s32 mMoreMoneyThan;                  /* 0x330 */
    s32 mWealthHighScore;                /* 0x334 */
    float mMargin;                       /* 0x338 */
    s32 mWealthHighRepayLoanScore;       /* 0x33c */
    s32 mTakeLoanScore;                  /* 0x340 */
    s32 mInTheRedSixMonthsScore;         /* 0x344 */
    s32 mInTheRedThreeMonthsScore;       /* 0x348 */
    s32 mInTheRedMonthLeftScore;         /* 0x34c */
    s32 mBankruptedScore;                /* 0x350 */
    float mWagesTimesHigherThanIncome;   /* 0x354 */
    s32 mAverageOverMonths;              /* 0x358 */
    s32 mWagesHighScore;                 /* 0x35c */
    s32 mLotsHandymanFudgeFactor;        /* 0x360 */
    s32 mLotsHandymanScore;              /* 0x364 */
    s32 mLotsEntertainersFudgeFactor;    /* 0x368 */
    s32 mLotsEntertainersScore;          /* 0x36c */
    s32 mLotsMechanicsFudgeFactor;       /* 0x370 */
    s32 mLotsMechanicsScore;             /* 0x374 */
    s32 mLotsSecurityFudgeFactor;        /* 0x378 */
    s32 mLotsSecurityScore;              /* 0x37c */
    s32 mLotsResearchersFudgeFactor;     /* 0x380 */
    s32 mLotsResearchersScore;           /* 0x384 */
    s32 mQueueUnconnectedScore;          /* 0x388 */
    s32 mMaxHandymenMoreThan;            /* 0x38c */
    s32 mMaxHandymenScore;               /* 0x390 */
    s32 mMaxHandymenV2Score;             /* 0x394 */
    s32 mMaxEntertainersMoreThan;        /* 0x398 */
    s32 mMaxEntertainersScore;           /* 0x39c */
    s32 mMaxEntertainersV2Score;         /* 0x3a0 */
    s32 mMaxMechanicsMoreThan;           /* 0x3a4 */
    s32 mMaxMechanicsScore;              /* 0x3a8 */
    s32 mMaxMechanicsV2Score;            /* 0x3ac */
    s32 mMaxGuardsMoreThan;              /* 0x3b0 */
    s32 mMaxGuardsScore;                 /* 0x3b4 */
    s32 mMaxGuardsV2Score;               /* 0x3b8 */
    s32 mMaxResearchersMoreThan;         /* 0x3bc */
    s32 mMaxResearchersScore;            /* 0x3c0 */
    s32 mMaxResearchersV2Score;          /* 0x3c4 */
    s32 mMaxMonths;                      /* 0x3c8 */
    s32 mDroppedPercent;                 /* 0x3cc */
    s32 mHypeNewparkScore;               /* 0x3d0 */
    s32 mPrankSbombScore;                /* 0x3d4 */
    s32 mPrankLitterScore;               /* 0x3d8 */
    s32 mPrankBalloonScore;              /* 0x3dc */
    s32 mParkNowOpenScore;               /* 0x3e0 */
    s32 mParkNowClosedScore;             /* 0x3e4 */
    s32 mUpgradeHireMechanicScore;       /* 0x3e8 */
    s32 mUpgradeStikingMechanicsScore;   /* 0x3ec */
    s32 mNewUpgradeNoMechanicScore;      /* 0x3f0 */
    s32 mNewResearchGroupRideScore;      /* 0x3f4 */
    s32 mNewResearchGroupShopScore;      /* 0x3f8 */
    s32 mNewResearchGroupSideshowScore;  /* 0x3fc */
    s32 mNewResearchGroupFeatureScore;   /* 0x400 */
    s32 mBusArrivedScore;                /* 0x404 */
    s32 mPlaneArrivedScore;              /* 0x408 */
    s32 mBoatArrivedScore;               /* 0x40c */
    s32 mHowBigIsBig;                    /* 0x410 */
    s32 mBigBusloadArrivedScore;         /* 0x414 */
    s32 mAverageScarierThan;             /* 0x418 */
    s32 mMinimumRides;                   /* 0x41c */
    s32 mVisitorsTooScaredScore;         /* 0x420 */
    s32 mOnlineNewItemScore;             /* 0x424 */
    s32 mOnlineMessageScore;             /* 0x428 */
    s32 mMinimumStaff;                   /* 0x42c */
    s32 mCongratStaffHappyHappierThan;   /* 0x430 */
    s32 mCongratStaffHappyScore;         /* 0x434 */
    s32 mMinimumVisitors;                /* 0x438 */
    s32 mCongratVisitorsHappyHappierThan; /* 0x43c */
    s32 mCongratVisitorsHappyScore;      /* 0x440 */
    s32 mMinimumGuards;                  /* 0x444 */
    s32 mCongratSecurityTrainedBetterThan; /* 0x448 */
    s32 mCongratSecurityTrainedScore;    /* 0x44c */
    s32 mMinimumResearchers;             /* 0x450 */
    s32 mCongratResearchersTrainedBetterThan; /* 0x454 */
    s32 mCongratResearchersTrainedScore; /* 0x458 */
    s32 mMinimumHandymen;                /* 0x45c */
    s32 mCongratHandymenTrainedBetterThan; /* 0x460 */
    s32 mCongratHandymenTrainedScore;    /* 0x464 */
    s32 mMinimumEntertainers;            /* 0x468 */
    s32 mCongratEntertainersTrainedBetterThan; /* 0x46c */
    s32 mCongratEntertainersTrainedScore; /* 0x470 */
    s32 mMinimumMechanics;               /* 0x474 */
    s32 mCongratMechanicsTrainedBetterThan; /* 0x478 */
    s32 mCongratMechanicsTrainedScore;   /* 0x47c */
    s32 mCongratPublishParkValueBetterThan; /* 0x480 */
    s32 mCongratPublishParkHappinessGreaterThan; /* 0x484 */
    s32 mCongratPublishParkVisitorsMoreThan; /* 0x488 */
    s32 mCongratPublishParkScore;        /* 0x48c */
    s32 mCongratSendPostcardValueBetterThan; /* 0x490 */
    s32 mCongratSendPostcardHappinessGreaterThan; /* 0x494 */
    s32 mCongratSendPostcardVisitorsMoreThan; /* 0x498 */
    s32 mCongratSendPostcardScore;       /* 0x49c */
    s32 mCongratBalloonsMoreThan;        /* 0x4a0 */
    s32 mCongratBalloonsScore;           /* 0x4a4 */
    s32 mCongratCostumesMoreThan;        /* 0x4a8 */
    s32 mCongratCostumesScore;           /* 0x4ac */
    s32 mCongratAllCompleteScore;        /* 0x4b0 */
    s32 mCongratAllCompleteLeavingScore; /* 0x4b4 */
    s32 mPctOfStaffTired;                /* 0x4b8 */
    s32 mNeedStaffRoomScore;             /* 0x4bc */
    s32 mHireStaffGeneralScore;          /* 0x4c0 */
    s32 mMinVisitorsEver;                /* 0x4c4 */
    s32 mSetPatrolsGeneralMinStaffForMessage; /* 0x4c8 */
    s32 mSetPatrolsGeneralScore;         /* 0x4cc */
    s32 mTrainStaffGeneralMinStaffForMessage; /* 0x4d0 */
    s32 mTrainStaffGeneralScore;         /* 0x4d4 */
    s32 mFoodPoorValueValueWorseThan;    /* 0x4d8 */
    s32 mFoodPoorValueScorePerPoint;     /* 0x4dc */
    s32 mShopsPoorValueValueWorseThan;   /* 0x4e0 */
    s32 mShopsPoorValueScorePerPoint;    /* 0x4e4 */
    s32 mRestaurantPoorValueValueWorseThan; /* 0x4e8 */
    s32 mRestaurantPoorValueScorePerPoint; /* 0x4ec */
    s32 mDrinksPoorValueValueWorseThan;  /* 0x4f0 */
    s32 mDrinksPoorValueScorePerPoint;   /* 0x4f4 */
    s32 mSideshowsPoorValueValueWorseThan; /* 0x4f8 */
    s32 mSideshowsPoorValueScorePerPoint; /* 0x4fc */
    s32 mFoodPoorSatisfactionSatisfactionWorseThan; /* 0x500 */
    s32 mFoodPoorSatisfactionScorePerPoint; /* 0x504 */
    s32 mShopsPoorSatisfactionSatisfactionWorseThan; /* 0x508 */
    s32 mShopsPoorSatisfactionScorePerPoint; /* 0x50c */
    s32 mRestaurantPoorSatisfactionSatisfactionWorseThan; /* 0x510 */
    s32 mRestaurantPoorSatisfactionScorePerPoint; /* 0x514 */
    s32 mDrinksPoorSatisfactionSatisfactionWorseThan; /* 0x518 */
    s32 mDrinksPoorSatisfactionScorePerPoint; /* 0x51c */
    s32 mSideshowsPoorSatisfactionSatisfactionWorseThan; /* 0x520 */
    s32 mSideshowsPoorSatisfactionScorePerPoint; /* 0x524 */
    s32 mFoodGreatValueValueBetterThan;  /* 0x528 */
    s32 mFoodGreatValueScorePerPoint;    /* 0x52c */
    s32 mShopsGreatValueValueBetterThan; /* 0x530 */
    s32 mShopsGreatValueScorePerPoint;   /* 0x534 */
    s32 mRestaurantGreatValueValueBetterThan; /* 0x538 */
    s32 mRestaurantGreatValueScorePerPoint; /* 0x53c */
    s32 mDrinksGreatValueValueBetterThan; /* 0x540 */
    s32 mDrinksGreatValueScorePerPoint;  /* 0x544 */
    s32 mSideshowsGreatValueValueBetterThan; /* 0x548 */
    s32 mSideshowsGreatValueScorePerPoint; /* 0x54c */
    s32 mFoodGreatSatisfactionSatisfactionBetterThan; /* 0x550 */
    s32 mFoodGreatSatisfactionScorePerPoint; /* 0x554 */
    s32 mShopsGreatSatisfactionSatisfactionBetterThan; /* 0x558 */
    s32 mShopsGreatSatisfactionScorePerPoint; /* 0x55c */
    s32 mRestaurantGreatSatisfactionSatisfactionBetterThan; /* 0x560 */
    s32 mRestaurantGreatSatisfactionScorePerPoint; /* 0x564 */
    s32 mDrinksGreatSatisfactionSatisfactionBetterThan; /* 0x568 */
    s32 mDrinksGreatSatisfactionScorePerPoint; /* 0x56c */
    s32 mSideshowsGreatSatisfactionSatisfactionBetterThan; /* 0x570 */
    s32 mSideshowsGreatSatisfactionScorePerPoint; /* 0x574 */
    s32 mRideCondemnedScore;             /* 0x578 */
    u32 mAtLeastThisManyRides;           /* 0x57c */
    s32 mRidesClosedMoreThanThisPercentClosed; /* 0x580 */
    s32 mRidesClosedScore;               /* 0x584 */
    s32 mShopsClosedMoreThanThisPercentClosed; /* 0x588 */
    s32 mShopsClosedScore;               /* 0x58c */
    s32 mSideshowsClosedMoreThanThisPercentClosed; /* 0x590 */
    s32 mSideshowsClosedScore;           /* 0x594 */
    s32 mToiletsClosedMoreThanThisPercentClosed; /* 0x598 */
    s32 mToiletsClosedScore;             /* 0x59c */
    s32 mStaffroomClosedMoreThanThisPercentClosed; /* 0x5a0 */
    s32 mStaffroomClosedScore;           /* 0x5a4 */
    s32 mMorePrankstersThan;             /* 0x5a8 */
    s32 mLotsOfPranksScore;              /* 0x5ac */
    s32 mRideExitBlockedScore;           /* 0x5b0 */
    s32 mWonFirstGoldenTicketScore;      /* 0x5b4 */
    float mGoldTicketNearToFirstOnePeopleMultiplier; /* 0x5b8 */
    s32 mGoldTicketNearToFirstOneScore;  /* 0x5bc */
    s32 mGoldTicketCluesForRemainderScore; /* 0x5c0 */
    float mGoldTicketNearToXPeepsPeopleMultiplier; /* 0x5c4 */
    s32 mGoldTicketNearToXPeepsScore;    /* 0x5c8 */
    float mHappinessMultiplier;          /* 0x5cc */
    s32 mGoldTicketNearToXHappinessScore; /* 0x5d0 */
    float mProfitMultiplier;             /* 0x5d4 */
    s32 mGoldTicketNearToXProfitScore;   /* 0x5d8 */
    u32 mTutorialUIPanelAfterTime;       /* 0x5dc */
    u32 mTutorialUIPanelCutoffTime;      /* 0x5e0 */
    s32 mTutorialUIPanelScore;           /* 0x5e4 */
    u32 mTutorialExplainTooltipsAfterTime; /* 0x5e8 */
    u32 mTutorialExplainTooltipsCutoffTime; /* 0x5ec */
    s32 mTutorialExplainTooltipsScore;   /* 0x5f0 */
    s32 mGoldTicketExplanationScore;     /* 0x5f4 */
    s32 mPrebuiltParkScore;              /* 0x5f8 */
    s32 mGTAllResearchedAHigherPctThanThis; /* 0x5fc */
    s32 mGTAllResearchedAScore;          /* 0x600 */
    s32 mGTAllResearchedBHigherPctThanThis; /* 0x604 */
    s32 mGTAllResearchedBScore;          /* 0x608 */
    s32 mGTAllResearchedCHigherPctThanThis; /* 0x60c */
    s32 mGTAllResearchedCScore;          /* 0x610 */
    s32 mGTAmountVisitorsAHigherPctThanThis; /* 0x614 */
    s32 mGTAmountVisitorsAScore;         /* 0x618 */
    s32 mGTAmountVisitorsBHigherPctThanThis; /* 0x61c */
    s32 mGTAmountVisitorsBScore;         /* 0x620 */
    s32 mGTAmountVisitorsCHigherPctThanThis; /* 0x624 */
    s32 mGTAmountVisitorsCScore;         /* 0x628 */
    s32 mChallengeScore;                 /* 0x62c */
    u32 mDaysForStaff;                   /* 0x630 */
    u32 mDaysForObjects;                 /* 0x634 */
} AdvisorScoring;

#ifndef __GHIDRA__
#define TPW_CHECK(name, cond) typedef char tpw_check_##name[(cond) ? 1 : -1]
#define TPW_OFF(t, f) ((u32)(unsigned long)&((t *)0)->f)
TPW_CHECK(Thing_unk_00, TPW_OFF(Thing, unk_00) == 0x0);
TPW_CHECK(Thing_model, TPW_OFF(Thing, model) == 0x2);
TPW_CHECK(Thing_unk_03, TPW_OFF(Thing, unk_03) == 0x3);
TPW_CHECK(Thing_mX, TPW_OFF(Thing, mX) == 0x4);
TPW_CHECK(Thing_mY, TPW_OFF(Thing, mY) == 0x6);
TPW_CHECK(Thing_mMapParent, TPW_OFF(Thing, mMapParent) == 0x8);
TPW_CHECK(Thing_mMapChild, TPW_OFF(Thing, mMapChild) == 0xa);
TPW_CHECK(Thing_size, sizeof(Thing) == 0xc);
TPW_CHECK(SpriteAnim_mSpriteScript, TPW_OFF(SpriteAnim, mSpriteScript) == 0x0);
TPW_CHECK(SpriteAnim_mNextAnim, TPW_OFF(SpriteAnim, mNextAnim) == 0x4);
TPW_CHECK(SpriteAnim_mNextServiceInterval, TPW_OFF(SpriteAnim, mNextServiceInterval) == 0x8);
TPW_CHECK(SpriteAnim_size, sizeof(SpriteAnim) == 0xc);
TPW_CHECK(Thoughts_mLastThought, TPW_OFF(Thoughts, mLastThought) == 0x0);
TPW_CHECK(Thoughts_mEventHistory, TPW_OFF(Thoughts, mEventHistory) == 0x4);
TPW_CHECK(Thoughts_mThoughtScript, TPW_OFF(Thoughts, mThoughtScript) == 0x84);
TPW_CHECK(Thoughts_mActionHistIndex, TPW_OFF(Thoughts, mActionHistIndex) == 0x88);
TPW_CHECK(Thoughts_mTimeBubbleShown, TPW_OFF(Thoughts, mTimeBubbleShown) == 0x8c);
TPW_CHECK(Thoughts_size, sizeof(Thoughts) == 0x90);
TPW_CHECK(Nav_mass, TPW_OFF(Nav, mass) == 0x0);
TPW_CHECK(Nav_radius, TPW_OFF(Nav, radius) == 0x4);
TPW_CHECK(Nav_pos, TPW_OFF(Nav, pos) == 0x8);
TPW_CHECK(Nav_vel, TPW_OFF(Nav, vel) == 0x10);
TPW_CHECK(Nav_max_force, TPW_OFF(Nav, max_force) == 0x18);
TPW_CHECK(Nav_max_speed, TPW_OFF(Nav, max_speed) == 0x1c);
TPW_CHECK(Nav_local_xaxis, TPW_OFF(Nav, local_xaxis) == 0x20);
TPW_CHECK(Nav_local_yaxis, TPW_OFF(Nav, local_yaxis) == 0x28);
TPW_CHECK(Nav_formation_pos, TPW_OFF(Nav, formation_pos) == 0x38);
TPW_CHECK(Nav_force, TPW_OFF(Nav, force) == 0x40);
TPW_CHECK(Nav_path_timestamp, TPW_OFF(Nav, path_timestamp) == 0x48);
TPW_CHECK(Nav_path_target_pos, TPW_OFF(Nav, path_target_pos) == 0x4c);
TPW_CHECK(Nav_path_total_count, TPW_OFF(Nav, path_total_count) == 0x54);
TPW_CHECK(Nav_path_buffer_count, TPW_OFF(Nav, path_buffer_count) == 0x58);
TPW_CHECK(Nav_path_count, TPW_OFF(Nav, path_count) == 0x5c);
TPW_CHECK(Nav_path_finished, TPW_OFF(Nav, path_finished) == 0x60);
TPW_CHECK(Nav_subpath_buffer, TPW_OFF(Nav, subpath_buffer) == 0x64);
TPW_CHECK(Nav_subpath_dist, TPW_OFF(Nav, subpath_dist) == 0x8c);
TPW_CHECK(Nav_path_total_dist, TPW_OFF(Nav, path_total_dist) == 0xa0);
TPW_CHECK(Nav_path_subpath_dist, TPW_OFF(Nav, path_subpath_dist) == 0xa4);
TPW_CHECK(Nav_path_tail_dist, TPW_OFF(Nav, path_tail_dist) == 0xa8);
TPW_CHECK(Nav_path_last_progress, TPW_OFF(Nav, path_last_progress) == 0xac);
TPW_CHECK(Nav_path_stuck_buffer, TPW_OFF(Nav, path_stuck_buffer) == 0xb0);
TPW_CHECK(Nav_nav_mode, TPW_OFF(Nav, nav_mode) == 0xb4);
TPW_CHECK(Nav_mCantReachDest, TPW_OFF(Nav, mCantReachDest) == 0xb8);
TPW_CHECK(Nav_size, sizeof(Nav) == 0xbc);
TPW_CHECK(Sprite_base, TPW_OFF(Sprite, base) == 0x0);
TPW_CHECK(Sprite_mAnim, TPW_OFF(Sprite, mAnim) == 0xc);
TPW_CHECK(Sprite_mAccurateDestX, TPW_OFF(Sprite, mAccurateDestX) == 0x18);
TPW_CHECK(Sprite_mAccurateDestY, TPW_OFF(Sprite, mAccurateDestY) == 0x1a);
TPW_CHECK(Sprite_mSpriteAngle, TPW_OFF(Sprite, mSpriteAngle) == 0x1c);
TPW_CHECK(Sprite_mSpriteID, TPW_OFF(Sprite, mSpriteID) == 0x20);
TPW_CHECK(Sprite_mESPSprite, TPW_OFF(Sprite, mESPSprite) == 0x24);
TPW_CHECK(Sprite_mSpriteUnderRideCtrl, TPW_OFF(Sprite, mSpriteUnderRideCtrl) == 0x28);
TPW_CHECK(Sprite_mCount, TPW_OFF(Sprite, mCount) == 0x2c);
TPW_CHECK(Sprite_mThoughts, TPW_OFF(Sprite, mThoughts) == 0x30);
TPW_CHECK(Sprite_mBaseSpeed, TPW_OFF(Sprite, mBaseSpeed) == 0xc0);
TPW_CHECK(Sprite_mPurposeSpeed, TPW_OFF(Sprite, mPurposeSpeed) == 0xc2);
TPW_CHECK(Sprite_mAdjustorSpeed, TPW_OFF(Sprite, mAdjustorSpeed) == 0xc4);
TPW_CHECK(Sprite_mPreviousSpeed, TPW_OFF(Sprite, mPreviousSpeed) == 0xc8);
TPW_CHECK(Sprite_mLastRecordedMapId, TPW_OFF(Sprite, mLastRecordedMapId) == 0xcc);
TPW_CHECK(Sprite_mSetDestSuccessfully, TPW_OFF(Sprite, mSetDestSuccessfully) == 0xd0);
TPW_CHECK(Sprite_mNav, TPW_OFF(Sprite, mNav) == 0xd4);
TPW_CHECK(Sprite_mPreviousX, TPW_OFF(Sprite, mPreviousX) == 0x190);
TPW_CHECK(Sprite_mPreviousY, TPW_OFF(Sprite, mPreviousY) == 0x194);
TPW_CHECK(Sprite_mStrandedTime, TPW_OFF(Sprite, mStrandedTime) == 0x198);
TPW_CHECK(Sprite_size, sizeof(Sprite) == 0x19c);
TPW_CHECK(Visitor_base, TPW_OFF(Visitor, base) == 0x0);
TPW_CHECK(Visitor_mHappiness, TPW_OFF(Visitor, mHappiness) == 0x19c);
TPW_CHECK(Visitor_mCash, TPW_OFF(Visitor, mCash) == 0x1a0);
TPW_CHECK(Visitor_mThirst, TPW_OFF(Visitor, mThirst) == 0x1a4);
TPW_CHECK(Visitor_mHunger, TPW_OFF(Visitor, mHunger) == 0x1a8);
TPW_CHECK(Visitor_mToilet, TPW_OFF(Visitor, mToilet) == 0x1ac);
TPW_CHECK(Visitor_mVomit, TPW_OFF(Visitor, mVomit) == 0x1b0);
TPW_CHECK(Visitor_mLitter, TPW_OFF(Visitor, mLitter) == 0x1b4);
TPW_CHECK(Visitor_mTiredness, TPW_OFF(Visitor, mTiredness) == 0x1b8);
TPW_CHECK(Visitor_mExitLevel, TPW_OFF(Visitor, mExitLevel) == 0x1bc);
TPW_CHECK(Visitor_mPrankeryIndex, TPW_OFF(Visitor, mPrankeryIndex) == 0x1c0);
TPW_CHECK(Visitor_mNumRides, TPW_OFF(Visitor, mNumRides) == 0x1c4);
TPW_CHECK(Visitor_mNumShops, TPW_OFF(Visitor, mNumShops) == 0x1c8);
TPW_CHECK(Visitor_mNumSideshows, TPW_OFF(Visitor, mNumSideshows) == 0x1cc);
TPW_CHECK(Visitor_mNumSideshowsWon, TPW_OFF(Visitor, mNumSideshowsWon) == 0x1d0);
TPW_CHECK(Visitor_mArrivalDate, TPW_OFF(Visitor, mArrivalDate) == 0x1d4);
TPW_CHECK(Visitor_mArrivalIndex, TPW_OFF(Visitor, mArrivalIndex) == 0x1d8);
TPW_CHECK(Visitor_mMajorDest, TPW_OFF(Visitor, mMajorDest) == 0x1dc);
TPW_CHECK(Visitor_mSavedMajorDest, TPW_OFF(Visitor, mSavedMajorDest) == 0x1de);
TPW_CHECK(Visitor_mPreviousRides, TPW_OFF(Visitor, mPreviousRides) == 0x1e0);
TPW_CHECK(Visitor_mPreviousTemporaryRides, TPW_OFF(Visitor, mPreviousTemporaryRides) == 0x1e8);
TPW_CHECK(Visitor_mPersonType, TPW_OFF(Visitor, mPersonType) == 0x1f0);
TPW_CHECK(Visitor_mQueuePos, TPW_OFF(Visitor, mQueuePos) == 0x1f1);
TPW_CHECK(Visitor_mQueueMoveDelay, TPW_OFF(Visitor, mQueueMoveDelay) == 0x1f4);
TPW_CHECK(Visitor_mBeenAdmitted, TPW_OFF(Visitor, mBeenAdmitted) == 0x1f8);
TPW_CHECK(Visitor_mTimeStartedIdling, TPW_OFF(Visitor, mTimeStartedIdling) == 0x1fc);
TPW_CHECK(Visitor_mParkOpeningWaitingTime, TPW_OFF(Visitor, mParkOpeningWaitingTime) == 0x200);
TPW_CHECK(Visitor_mPaidAdmission, TPW_OFF(Visitor, mPaidAdmission) == 0x204);
TPW_CHECK(Visitor_mTimeOfLastSpotAnim, TPW_OFF(Visitor, mTimeOfLastSpotAnim) == 0x208);
TPW_CHECK(Visitor_mBalloonScript, TPW_OFF(Visitor, mBalloonScript) == 0x210);
TPW_CHECK(Visitor_mRemainingBalloonLife, TPW_OFF(Visitor, mRemainingBalloonLife) == 0x214);
TPW_CHECK(Visitor_mLastPosX, TPW_OFF(Visitor, mLastPosX) == 0x218);
TPW_CHECK(Visitor_mLastPosY, TPW_OFF(Visitor, mLastPosY) == 0x21c);
TPW_CHECK(Visitor_mState, TPW_OFF(Visitor, mState) == 0x220);
TPW_CHECK(Visitor_mSavedState, TPW_OFF(Visitor, mSavedState) == 0x224);
TPW_CHECK(Visitor_mQNext, TPW_OFF(Visitor, mQNext) == 0x228);
TPW_CHECK(Visitor_mQPrev, TPW_OFF(Visitor, mQPrev) == 0x22a);
TPW_CHECK(Staff_base, TPW_OFF(Staff, base) == 0x0);
TPW_CHECK(Staff_mState, TPW_OFF(Staff, mState) == 0x19c);
TPW_CHECK(Staff_mName, TPW_OFF(Staff, mName) == 0x1a0);
TPW_CHECK(Staff_mCurrentPayGrade, TPW_OFF(Staff, mCurrentPayGrade) == 0x1e4);
TPW_CHECK(Staff_mPercentageThroughGrade, TPW_OFF(Staff, mPercentageThroughGrade) == 0x1e8);
TPW_CHECK(Staff_mTimeHired, TPW_OFF(Staff, mTimeHired) == 0x1f0);
TPW_CHECK(Staff_mHappiness, TPW_OFF(Staff, mHappiness) == 0x1f8);
TPW_CHECK(Staff_mEnergy, TPW_OFF(Staff, mEnergy) == 0x1fc);
TPW_CHECK(Staff_mTimeStartedIdling, TPW_OFF(Staff, mTimeStartedIdling) == 0x200);
TPW_CHECK(Staff_mJobsDone, TPW_OFF(Staff, mJobsDone) == 0x204);
TPW_CHECK(Staff_mRestArea, TPW_OFF(Staff, mRestArea) == 0x208);
TPW_CHECK(Staff_mPatrolRegionBL, TPW_OFF(Staff, mPatrolRegionBL) == 0x20a);
TPW_CHECK(Staff_mPatrolRegionTR, TPW_OFF(Staff, mPatrolRegionTR) == 0x20c);
TPW_CHECK(Staff_size, sizeof(Staff) == 0x210);
TPW_CHECK(Mechanic_base, TPW_OFF(Mechanic, base) == 0x0);
TPW_CHECK(Mechanic_mNext, TPW_OFF(Mechanic, mNext) == 0x210);
TPW_CHECK(Mechanic_mDurationOfRepair, TPW_OFF(Mechanic, mDurationOfRepair) == 0x214);
TPW_CHECK(Mechanic_mObjectToRepair, TPW_OFF(Mechanic, mObjectToRepair) == 0x218);
TPW_CHECK(Handyman_base, TPW_OFF(Handyman, base) == 0x0);
TPW_CHECK(Handyman_mNext, TPW_OFF(Handyman, mNext) == 0x210);
TPW_CHECK(Handyman_mTimeStartedCleaning, TPW_OFF(Handyman, mTimeStartedCleaning) == 0x214);
TPW_CHECK(Handyman_mTargetLitterCell, TPW_OFF(Handyman, mTargetLitterCell) == 0x218);
TPW_CHECK(Handyman_mToiletToClean, TPW_OFF(Handyman, mToiletToClean) == 0x21a);
TPW_CHECK(Entertainer_base, TPW_OFF(Entertainer, base) == 0x0);
TPW_CHECK(Entertainer_mNext, TPW_OFF(Entertainer, mNext) == 0x210);
TPW_CHECK(Entertainer_mTimeStartedEntertaining, TPW_OFF(Entertainer, mTimeStartedEntertaining) == 0x214);
TPW_CHECK(Guard_base, TPW_OFF(Guard, base) == 0x0);
TPW_CHECK(Guard_mNext, TPW_OFF(Guard, mNext) == 0x210);
TPW_CHECK(Guard_mPerp, TPW_OFF(Guard, mPerp) == 0x212);
TPW_CHECK(Guard_mProsecutionTimestamp, TPW_OFF(Guard, mProsecutionTimestamp) == 0x214);
TPW_CHECK(Researcher_base, TPW_OFF(Researcher, base) == 0x0);
TPW_CHECK(Researcher_mNext, TPW_OFF(Researcher, mNext) == 0x210);
TPW_CHECK(Researcher_mTimeStartedResearching, TPW_OFF(Researcher, mTimeStartedResearching) == 0x214);
TPW_CHECK(CircBufU32_30_mTemp, TPW_OFF(CircBufU32_30, mTemp) == 0x0);
TPW_CHECK(CircBufU32_30_mData, TPW_OFF(CircBufU32_30, mData) == 0x4);
TPW_CHECK(CircBufU32_30_mCurrentEntry, TPW_OFF(CircBufU32_30, mCurrentEntry) == 0x7c);
TPW_CHECK(CircBufU32_30_mNumEntries, TPW_OFF(CircBufU32_30, mNumEntries) == 0x80);
TPW_CHECK(CircBufU32_30_mWrappedAround, TPW_OFF(CircBufU32_30, mWrappedAround) == 0x84);
TPW_CHECK(CircBufU32_30_size, sizeof(CircBufU32_30) == 0x88);
TPW_CHECK(Object_base, TPW_OFF(Object, base) == 0x0);
TPW_CHECK(Object_mNext, TPW_OFF(Object, mNext) == 0xc);
TPW_CHECK(Object_mId, TPW_OFF(Object, mId) == 0xe);
TPW_CHECK(Object_mAngle, TPW_OFF(Object, mAngle) == 0x10);
TPW_CHECK(Object_MeshInstanceID, TPW_OFF(Object, MeshInstanceID) == 0x20);
TPW_CHECK(Object_mRideScriptHandle, TPW_OFF(Object, mRideScriptHandle) == 0x24);
TPW_CHECK(Object_mTrackRideHandle, TPW_OFF(Object, mTrackRideHandle) == 0x28);
TPW_CHECK(Object_mIsTrackRideValid, TPW_OFF(Object, mIsTrackRideValid) == 0x2c);
TPW_CHECK(Object_mUpgradeParent, TPW_OFF(Object, mUpgradeParent) == 0x30);
TPW_CHECK(Object_mFlags, TPW_OFF(Object, mFlags) == 0x32);
TPW_CHECK(Object_mTopLeft, TPW_OFF(Object, mTopLeft) == 0x34);
TPW_CHECK(Object_mEntryPos, TPW_OFF(Object, mEntryPos) == 0x36);
TPW_CHECK(Object_mExitPos, TPW_OFF(Object, mExitPos) == 0x38);
TPW_CHECK(Object_mBackOfQueue, TPW_OFF(Object, mBackOfQueue) == 0x3a);
TPW_CHECK(Object_mFirstInQ, TPW_OFF(Object, mFirstInQ) == 0x3c);
TPW_CHECK(Object_mQueueSizeInCells, TPW_OFF(Object, mQueueSizeInCells) == 0x40);
TPW_CHECK(Object_mUpgradeLevel, TPW_OFF(Object, mUpgradeLevel) == 0x50);
TPW_CHECK(Object_mUpgradeBalloonSprite, TPW_OFF(Object, mUpgradeBalloonSprite) == 0x54);
TPW_CHECK(Object_mOperatingSpeed, TPW_OFF(Object, mOperatingSpeed) == 0x58);
TPW_CHECK(Object_mOperatingDuration, TPW_OFF(Object, mOperatingDuration) == 0x5c);
TPW_CHECK(Object_mOperatingCapacity, TPW_OFF(Object, mOperatingCapacity) == 0x5d);
TPW_CHECK(Object_mAssignedStaffMember, TPW_OFF(Object, mAssignedStaffMember) == 0x5e);
TPW_CHECK(Object_mTimeMarkedForMaintenance, TPW_OFF(Object, mTimeMarkedForMaintenance) == 0x60);
TPW_CHECK(Object_mRequestedService, TPW_OFF(Object, mRequestedService) == 0x64);
TPW_CHECK(Object_mCanLoad, TPW_OFF(Object, mCanLoad) == 0x68);
TPW_CHECK(Object_mPersonBeingLoaded, TPW_OFF(Object, mPersonBeingLoaded) == 0x6c);
TPW_CHECK(Object_mHist0, TPW_OFF(Object, mHist0) == 0x70);
TPW_CHECK(Object_mHist1, TPW_OFF(Object, mHist1) == 0xf8);
TPW_CHECK(Object_mTotalTakings, TPW_OFF(Object, mTotalTakings) == 0x180);
TPW_CHECK(Object_mTotalCosts, TPW_OFF(Object, mTotalCosts) == 0x184);
TPW_CHECK(Object_mCostOfGoods, TPW_OFF(Object, mCostOfGoods) == 0x188);
TPW_CHECK(Object_mQualityOfGoods, TPW_OFF(Object, mQualityOfGoods) == 0x18c);
TPW_CHECK(Object_mChanceOfWinning, TPW_OFF(Object, mChanceOfWinning) == 0x190);
TPW_CHECK(Object_mPricePerUse, TPW_OFF(Object, mPricePerUse) == 0x194);
TPW_CHECK(Object_mAmountOfSpecialIngredient, TPW_OFF(Object, mAmountOfSpecialIngredient) == 0x198);
TPW_CHECK(Object_mState, TPW_OFF(Object, mState) == 0x19c);
TPW_CHECK(Object_mNumCustomers, TPW_OFF(Object, mNumCustomers) == 0x1a0);
TPW_CHECK(Object_mNumWalkAways, TPW_OFF(Object, mNumWalkAways) == 0x1a4);
TPW_CHECK(Object_mHist2, TPW_OFF(Object, mHist2) == 0x1a8);
TPW_CHECK(Object_mHist3, TPW_OFF(Object, mHist3) == 0x230);
TPW_CHECK(Object_mHist4, TPW_OFF(Object, mHist4) == 0x2b8);
TPW_CHECK(Object_mHist5, TPW_OFF(Object, mHist5) == 0x340);
TPW_CHECK(Object_mNameA, TPW_OFF(Object, mNameA) == 0x3c8);
TPW_CHECK(Object_mNameB, TPW_OFF(Object, mNameB) == 0x40a);
TPW_CHECK(MapCellBase_mMeshInstance, TPW_OFF(MapCellBase, mMeshInstance) == 0x4);
TPW_CHECK(MapCellBase_mType, TPW_OFF(MapCellBase, mType) == 0x8);
TPW_CHECK(MapCellBase_mNeighbours, TPW_OFF(MapCellBase, mNeighbours) == 0xc);
TPW_CHECK(MapCellBase_mDirection, TPW_OFF(MapCellBase, mDirection) == 0xd);
TPW_CHECK(MapCellBase_mFlags, TPW_OFF(MapCellBase, mFlags) == 0xe);
TPW_CHECK(MapCellBase_mParentID, TPW_OFF(MapCellBase, mParentID) == 0x10);
TPW_CHECK(MapCellBase_mTileData, TPW_OFF(MapCellBase, mTileData) == 0x14);
TPW_CHECK(MapCellBase_mOverlapCounter, TPW_OFF(MapCellBase, mOverlapCounter) == 0x20);
TPW_CHECK(MapCellBase_mHoardingNeighbours, TPW_OFF(MapCellBase, mHoardingNeighbours) == 0x22);
TPW_CHECK(MapCellBase_size, sizeof(MapCellBase) == 0x24);
TPW_CHECK(MapCell_base, TPW_OFF(MapCell, base) == 0x0);
TPW_CHECK(MapCell_mWho, TPW_OFF(MapCell, mWho) == 0x24);
TPW_CHECK(MapCell_mStatusFlags, TPW_OFF(MapCell, mStatusFlags) == 0x26);
TPW_CHECK(MapCell_mLitterScript, TPW_OFF(MapCell, mLitterScript) == 0x28);
TPW_CHECK(MapCell_mLitter, TPW_OFF(MapCell, mLitter) == 0x2c);
TPW_CHECK(MapCell_mLitterCollector, TPW_OFF(MapCell, mLitterCollector) == 0x30);
TPW_CHECK(MapCell_mTimeMarkedForLitterCollection, TPW_OFF(MapCell, mTimeMarkedForLitterCollection) == 0x34);
TPW_CHECK(MapCell_mLitterScript_38, TPW_OFF(MapCell, mLitterScript_38) == 0x38);
TPW_CHECK(MapCell_mPylonIndex, TPW_OFF(MapCell, mPylonIndex) == 0x40);
TPW_CHECK(TrackCell_base, TPW_OFF(TrackCell, base) == 0x0);
TPW_CHECK(TrackCell_mSegmentNumber, TPW_OFF(TrackCell, mSegmentNumber) == 0x24);
TPW_CHECK(Forecast_mTime, TPW_OFF(Forecast, mTime) == 0x0);
TPW_CHECK(Forecast_mQuality, TPW_OFF(Forecast, mQuality) == 0x8);
TPW_CHECK(Forecast_mValid, TPW_OFF(Forecast, mValid) == 0xc);
TPW_CHECK(Forecast_size, sizeof(Forecast) == 0x10);
TPW_CHECK(Weather_base, TPW_OFF(Weather, base) == 0x0);
TPW_CHECK(Weather_mQuality, TPW_OFF(Weather, mQuality) == 0xc);
TPW_CHECK(Weather_mLastChanged, TPW_OFF(Weather, mLastChanged) == 0x10);
TPW_CHECK(Weather_mLastForecasted, TPW_OFF(Weather, mLastForecasted) == 0x18);
TPW_CHECK(Weather_mForecast, TPW_OFF(Weather, mForecast) == 0x20);
TPW_CHECK(Weather_mCurrentDrops, TPW_OFF(Weather, mCurrentDrops) == 0x30);
TPW_CHECK(Weather_mTargetDrops, TPW_OFF(Weather, mTargetDrops) == 0x34);
TPW_CHECK(Weather_mLightning, TPW_OFF(Weather, mLightning) == 0x38);
TPW_CHECK(Weather_mLightningCountdown, TPW_OFF(Weather, mLightningCountdown) == 0x3c);
TPW_CHECK(Weather_mLightningNumThisPeriod, TPW_OFF(Weather, mLightningNumThisPeriod) == 0x40);
TPW_CHECK(Weather_mThunderCountdown, TPW_OFF(Weather, mThunderCountdown) == 0x44);
TPW_CHECK(Weather_mLastLightningLocation, TPW_OFF(Weather, mLastLightningLocation) == 0x48);
TPW_CHECK(Weather_mRainSoundHandle, TPW_OFF(Weather, mRainSoundHandle) == 0x54);
TPW_CHECK(Weather_mPType, TPW_OFF(Weather, mPType) == 0x58);
TPW_CHECK(Weather_mWindDir, TPW_OFF(Weather, mWindDir) == 0x5c);
TPW_CHECK(Weather_mWindForce, TPW_OFF(Weather, mWindForce) == 0x60);
TPW_CHECK(Weather_mOverridden, TPW_OFF(Weather, mOverridden) == 0x64);
TPW_CHECK(Challenge_mType, TPW_OFF(Challenge, mType) == 0x0);
TPW_CHECK(Challenge_mTargetTime, TPW_OFF(Challenge, mTargetTime) == 0x4);
TPW_CHECK(Challenge_mTargetVal, TPW_OFF(Challenge, mTargetVal) == 0x8);
TPW_CHECK(Challenge_mTargetObj, TPW_OFF(Challenge, mTargetObj) == 0xc);
TPW_CHECK(Challenge_mTargetObj2, TPW_OFF(Challenge, mTargetObj2) == 0xe);
TPW_CHECK(Challenge_mTargetStaffType, TPW_OFF(Challenge, mTargetStaffType) == 0x10);
TPW_CHECK(Challenge_mPrize, TPW_OFF(Challenge, mPrize) == 0x14);
TPW_CHECK(Challenge_mDoubleOrNothingFollowup, TPW_OFF(Challenge, mDoubleOrNothingFollowup) == 0x18);
TPW_CHECK(Challenge_mCheckAtEndOnly, TPW_OFF(Challenge, mCheckAtEndOnly) == 0x1c);
TPW_CHECK(Challenge_mIndependent, TPW_OFF(Challenge, mIndependent) == 0x1d);
TPW_CHECK(Challenge_mChallengeOffered, TPW_OFF(Challenge, mChallengeOffered) == 0x20);
TPW_CHECK(Challenge_mChallengeLost, TPW_OFF(Challenge, mChallengeLost) == 0x24);
TPW_CHECK(Challenge_mChallengeDeclined, TPW_OFF(Challenge, mChallengeDeclined) == 0x28);
TPW_CHECK(Challenge_mHaveWon, TPW_OFF(Challenge, mHaveWon) == 0x2c);
TPW_CHECK(Challenge_mThingIdForFollowup, TPW_OFF(Challenge, mThingIdForFollowup) == 0x2e);
TPW_CHECK(Challenge_size, sizeof(Challenge) == 0x30);
TPW_CHECK(Challenges_base, TPW_OFF(Challenges, base) == 0x0);
TPW_CHECK(Challenges_mChallenges, TPW_OFF(Challenges, mChallenges) == 0xc);
TPW_CHECK(Challenges_mCurrentChallenge, TPW_OFF(Challenges, mCurrentChallenge) == 0x3cc);
TPW_CHECK(Challenges_mNextChallengeEventTime, TPW_OFF(Challenges, mNextChallengeEventTime) == 0x3d0);
TPW_CHECK(Challenges_mSystemSwitchedOn, TPW_OFF(Challenges, mSystemSwitchedOn) == 0x3d8);
TPW_CHECK(Challenges_mChallengeOn, TPW_OFF(Challenges, mChallengeOn) == 0x3d9);
TPW_CHECK(Challenges_mWaitingForReply, TPW_OFF(Challenges, mWaitingForReply) == 0x3da);
TPW_CHECK(Challenges_mValue, TPW_OFF(Challenges, mValue) == 0x3dc);
TPW_CHECK(Challenges_mActualThing, TPW_OFF(Challenges, mActualThing) == 0x3e0);
TPW_CHECK(ResearchProject_mObjId, TPW_OFF(ResearchProject, mObjId) == 0x0);
TPW_CHECK(ResearchProject_mCat, TPW_OFF(ResearchProject, mCat) == 0x4);
TPW_CHECK(ResearchProject_mResearchGroup, TPW_OFF(ResearchProject, mResearchGroup) == 0x8);
TPW_CHECK(ResearchProject_mUpgradeLevel, TPW_OFF(ResearchProject, mUpgradeLevel) == 0x9);
TPW_CHECK(ResearchProject_mCost, TPW_OFF(ResearchProject, mCost) == 0xc);
TPW_CHECK(ResearchProject_mResearchDone, TPW_OFF(ResearchProject, mResearchDone) == 0x10);
TPW_CHECK(ResearchProject_size, sizeof(ResearchProject) == 0x14);
TPW_CHECK(ResearchGroup_effort, TPW_OFF(ResearchGroup, effort) == 0x0);
TPW_CHECK(ResearchGroup_output, TPW_OFF(ResearchGroup, output) == 0x4);
TPW_CHECK(ResearchGroup_currentProject, TPW_OFF(ResearchGroup, currentProject) == 0x8);
TPW_CHECK(ResearchGroup_highestGroupAvailable, TPW_OFF(ResearchGroup, highestGroupAvailable) == 0xc);
TPW_CHECK(ResearchGroup_size, sizeof(ResearchGroup) == 0x10);
TPW_CHECK(ResearchLab_base, TPW_OFF(ResearchLab, base) == 0x0);
TPW_CHECK(ResearchLab_mProjects, TPW_OFF(ResearchLab, mProjects) == 0xc);
TPW_CHECK(ResearchLab_mGroups, TPW_OFF(ResearchLab, mGroups) == 0x140c);
TPW_CHECK(ResearchLab_mNumProjects, TPW_OFF(ResearchLab, mNumProjects) == 0x145c);
TPW_CHECK(ResearchLab_mWorkLoad, TPW_OFF(ResearchLab, mWorkLoad) == 0x1460);
TPW_CHECK(ResearchLab_mMostExpensiveProject, TPW_OFF(ResearchLab, mMostExpensiveProject) == 0x1464);
TPW_CHECK(ResearchLab_mLabActive, TPW_OFF(ResearchLab, mLabActive) == 0x1468);
TPW_CHECK(BankLoan_loan_available, TPW_OFF(BankLoan, loan_available) == 0x0);
TPW_CHECK(BankLoan_unk_04, TPW_OFF(BankLoan, unk_04) == 0x4);
TPW_CHECK(BankLoan_size, sizeof(BankLoan) == 0x20);
TPW_CHECK(BankAccount_base, TPW_OFF(BankAccount, base) == 0x0);
TPW_CHECK(BankAccount_mBalance, TPW_OFF(BankAccount, mBalance) == 0xc);
TPW_CHECK(BankAccount_mBatchBalance, TPW_OFF(BankAccount, mBatchBalance) == 0x10);
TPW_CHECK(BankAccount_mLoans, TPW_OFF(BankAccount, mLoans) == 0x14);
TPW_CHECK(BankAccount_mWithdrawalsEnabled, TPW_OFF(BankAccount, mWithdrawalsEnabled) == 0x114);
TPW_CHECK(BankAccount_mAdmissionFee, TPW_OFF(BankAccount, mAdmissionFee) == 0x118);
TPW_CHECK(BankAccount_mLastBalance, TPW_OFF(BankAccount, mLastBalance) == 0x11c);
TPW_CHECK(BankAccount_mTurnEnteredRed, TPW_OFF(BankAccount, mTurnEnteredRed) == 0x120);
TPW_CHECK(BankAccount_mProfitThisYear, TPW_OFF(BankAccount, mProfitThisYear) == 0x124);
TPW_CHECK(GameClock_mFunnyTimeStart, TPW_OFF(GameClock, mFunnyTimeStart) == 0x0);
TPW_CHECK(GameClock_mSessionStart, TPW_OFF(GameClock, mSessionStart) == 0x8);
TPW_CHECK(GameClock_mMonthAtLastUpdate, TPW_OFF(GameClock, mMonthAtLastUpdate) == 0x10);
TPW_CHECK(GameClock_mDayAtLastUpdate, TPW_OFF(GameClock, mDayAtLastUpdate) == 0x14);
TPW_CHECK(GameClock_mFunnySecsPerRealSec, TPW_OFF(GameClock, mFunnySecsPerRealSec) == 0x1c);
TPW_CHECK(GameClock_size, sizeof(GameClock) == 0x20);
TPW_CHECK(ObjectDefUpgrade_mCostOfResearch, TPW_OFF(ObjectDefUpgrade, mCostOfResearch) == 0x0);
TPW_CHECK(ObjectDefUpgrade_mEstimatedPriceVariance, TPW_OFF(ObjectDefUpgrade, mEstimatedPriceVariance) == 0x4);
TPW_CHECK(ObjectDefUpgrade_mScrapValueYear1, TPW_OFF(ObjectDefUpgrade, mScrapValueYear1) == 0x8);
TPW_CHECK(ObjectDefUpgrade_mScrapValueYear2, TPW_OFF(ObjectDefUpgrade, mScrapValueYear2) == 0xc);
TPW_CHECK(ObjectDefUpgrade_mScrapValueYear3, TPW_OFF(ObjectDefUpgrade, mScrapValueYear3) == 0x10);
TPW_CHECK(ObjectDefUpgrade_mScrapValueYear4, TPW_OFF(ObjectDefUpgrade, mScrapValueYear4) == 0x14);
TPW_CHECK(ObjectDefUpgrade_mInitCapacity, TPW_OFF(ObjectDefUpgrade, mInitCapacity) == 0x18);
TPW_CHECK(ObjectDefUpgrade_mRedLineCapacity, TPW_OFF(ObjectDefUpgrade, mRedLineCapacity) == 0x1c);
TPW_CHECK(ObjectDefUpgrade_mInitDuration, TPW_OFF(ObjectDefUpgrade, mInitDuration) == 0x20);
TPW_CHECK(ObjectDefUpgrade_mRedLineDuration, TPW_OFF(ObjectDefUpgrade, mRedLineDuration) == 0x24);
TPW_CHECK(ObjectDefUpgrade_mInitSpeed, TPW_OFF(ObjectDefUpgrade, mInitSpeed) == 0x28);
TPW_CHECK(ObjectDefUpgrade_mRedLineSpeed, TPW_OFF(ObjectDefUpgrade, mRedLineSpeed) == 0x2c);
TPW_CHECK(ObjectDefUpgrade_mWearRate, TPW_OFF(ObjectDefUpgrade, mWearRate) == 0x30);
TPW_CHECK(ObjectDefUpgrade_mQueueWaitTimeConstant, TPW_OFF(ObjectDefUpgrade, mQueueWaitTimeConstant) == 0x34);
TPW_CHECK(ObjectDefUpgrade_mCostOfUpgrade, TPW_OFF(ObjectDefUpgrade, mCostOfUpgrade) == 0x38);
TPW_CHECK(ObjectDefUpgrade_mDurationOfUpgrade, TPW_OFF(ObjectDefUpgrade, mDurationOfUpgrade) == 0x3c);
TPW_CHECK(ObjectDefUpgrade_size, sizeof(ObjectDefUpgrade) == 0x40);
TPW_CHECK(ObjectDef_mLookupId, TPW_OFF(ObjectDef, mLookupId) == 0x0);
TPW_CHECK(ObjectDef_vtable, TPW_OFF(ObjectDef, vtable) == 0x4);
TPW_CHECK(ObjectDef_mSlotInfo, TPW_OFF(ObjectDef, mSlotInfo) == 0x8);
TPW_CHECK(ObjectDef_mId, TPW_OFF(ObjectDef, mId) == 0x10);
TPW_CHECK(ObjectDef_mName, TPW_OFF(ObjectDef, mName) == 0x14);
TPW_CHECK(ObjectDef_mShape, TPW_OFF(ObjectDef, mShape) == 0x18);
TPW_CHECK(ObjectDef_mHoarding, TPW_OFF(ObjectDef, mHoarding) == 0x1c);
TPW_CHECK(ObjectDef_mEngineMapOffsetOverrideX, TPW_OFF(ObjectDef, mEngineMapOffsetOverrideX) == 0x20);
TPW_CHECK(ObjectDef_mEngineMapOffsetOverrideY, TPW_OFF(ObjectDef, mEngineMapOffsetOverrideY) == 0x24);
TPW_CHECK(ObjectDef_mEngineFootprintWidthOverride, TPW_OFF(ObjectDef, mEngineFootprintWidthOverride) == 0x28);
TPW_CHECK(ObjectDef_mEngineFootprintHeightOverride, TPW_OFF(ObjectDef, mEngineFootprintHeightOverride) == 0x2c);
TPW_CHECK(ObjectDef_mMapOffsetX, TPW_OFF(ObjectDef, mMapOffsetX) == 0x30);
TPW_CHECK(ObjectDef_mMapOffsetY, TPW_OFF(ObjectDef, mMapOffsetY) == 0x34);
TPW_CHECK(ObjectDef_mDontApplyOffset, TPW_OFF(ObjectDef, mDontApplyOffset) == 0x38);
TPW_CHECK(ObjectDef_mIsChoosable, TPW_OFF(ObjectDef, mIsChoosable) == 0x3c);
TPW_CHECK(ObjectDef_mHasQueue, TPW_OFF(ObjectDef, mHasQueue) == 0x40);
TPW_CHECK(ObjectDef_mCanBreakDown, TPW_OFF(ObjectDef, mCanBreakDown) == 0x44);
TPW_CHECK(ObjectDef_mRunsContinuously, TPW_OFF(ObjectDef, mRunsContinuously) == 0x48);
TPW_CHECK(ObjectDef_mWhichUIType, TPW_OFF(ObjectDef, mWhichUIType) == 0x4c);
TPW_CHECK(ObjectDef_mDontDeformBase, TPW_OFF(ObjectDef, mDontDeformBase) == 0x50);
TPW_CHECK(ObjectDef_mOverwritePriority, TPW_OFF(ObjectDef, mOverwritePriority) == 0x54);
TPW_CHECK(ObjectDef_mAttractionValue, TPW_OFF(ObjectDef, mAttractionValue) == 0x58);
TPW_CHECK(ObjectDef_mNewAttractionDecayTime, TPW_OFF(ObjectDef, mNewAttractionDecayTime) == 0x5c);
TPW_CHECK(ObjectDef_mCreateParticleEffect, TPW_OFF(ObjectDef, mCreateParticleEffect) == 0x60);
TPW_CHECK(ObjectDef_mDestroyParticleEffect, TPW_OFF(ObjectDef, mDestroyParticleEffect) == 0x64);
TPW_CHECK(ObjectDef_mRepairParticleEffect, TPW_OFF(ObjectDef, mRepairParticleEffect) == 0x68);
TPW_CHECK(ObjectDef_mUpgradeParticleEffect, TPW_OFF(ObjectDef, mUpgradeParticleEffect) == 0x6c);
TPW_CHECK(ObjectDef_mDurationUnit, TPW_OFF(ObjectDef, mDurationUnit) == 0x70);
TPW_CHECK(ObjectDef_mUsesCars, TPW_OFF(ObjectDef, mUsesCars) == 0x74);
TPW_CHECK(ObjectDef_mPreviewAnimType, TPW_OFF(ObjectDef, mPreviewAnimType) == 0x78);
TPW_CHECK(ObjectDef_mPreviewAnimNum, TPW_OFF(ObjectDef, mPreviewAnimNum) == 0x7c);
TPW_CHECK(ObjectDef_mFirstPersonSprites, TPW_OFF(ObjectDef, mFirstPersonSprites) == 0x80);
TPW_CHECK(ObjectDef_mDoHeadProcessing, TPW_OFF(ObjectDef, mDoHeadProcessing) == 0x84);
TPW_CHECK(ObjectDef_mTurnOffAnimatingTextures, TPW_OFF(ObjectDef, mTurnOffAnimatingTextures) == 0x88);
TPW_CHECK(ObjectDef_mTurnOffScrollingTextures, TPW_OFF(ObjectDef, mTurnOffScrollingTextures) == 0x8c);
TPW_CHECK(ObjectDef_mRideTypeStringIndex, TPW_OFF(ObjectDef, mRideTypeStringIndex) == 0x90);
TPW_CHECK(ObjectDef_mDownloadedRideNumber, TPW_OFF(ObjectDef, mDownloadedRideNumber) == 0x94);
TPW_CHECK(ObjectDef_mIsPreviewRide, TPW_OFF(ObjectDef, mIsPreviewRide) == 0x98);
TPW_CHECK(ObjectDef_mWhichTrackType, TPW_OFF(ObjectDef, mWhichTrackType) == 0x9c);
TPW_CHECK(ObjectDef_mBumperType, TPW_OFF(ObjectDef, mBumperType) == 0xa0);
TPW_CHECK(ObjectDef_mNorthXAdjust, TPW_OFF(ObjectDef, mNorthXAdjust) == 0xa4);
TPW_CHECK(ObjectDef_mNorthYAdjust, TPW_OFF(ObjectDef, mNorthYAdjust) == 0xa8);
TPW_CHECK(ObjectDef_mEastXAdjust, TPW_OFF(ObjectDef, mEastXAdjust) == 0xac);
TPW_CHECK(ObjectDef_mEastYAdjust, TPW_OFF(ObjectDef, mEastYAdjust) == 0xb0);
TPW_CHECK(ObjectDef_mSouthXAdjust, TPW_OFF(ObjectDef, mSouthXAdjust) == 0xb4);
TPW_CHECK(ObjectDef_mSouthYAdjust, TPW_OFF(ObjectDef, mSouthYAdjust) == 0xb8);
TPW_CHECK(ObjectDef_mWestXAdjust, TPW_OFF(ObjectDef, mWestXAdjust) == 0xbc);
TPW_CHECK(ObjectDef_mWestYAdjust, TPW_OFF(ObjectDef, mWestYAdjust) == 0xc0);
TPW_CHECK(ObjectDef_mGoldenTicketCost, TPW_OFF(ObjectDef, mGoldenTicketCost) == 0xc4);
TPW_CHECK(ObjectDef_mLength, TPW_OFF(ObjectDef, mLength) == 0xc8);
TPW_CHECK(ObjectDef_mWidth, TPW_OFF(ObjectDef, mWidth) == 0xcc);
TPW_CHECK(ObjectDef_mHeight, TPW_OFF(ObjectDef, mHeight) == 0xd0);
TPW_CHECK(ObjectDef_mEntryCellStandPosX, TPW_OFF(ObjectDef, mEntryCellStandPosX) == 0xd4);
TPW_CHECK(ObjectDef_mEntryCellStandPosY, TPW_OFF(ObjectDef, mEntryCellStandPosY) == 0xd8);
TPW_CHECK(ObjectDef_mExitCellAppearPosX, TPW_OFF(ObjectDef, mExitCellAppearPosX) == 0xdc);
TPW_CHECK(ObjectDef_mExitCellAppearPosY, TPW_OFF(ObjectDef, mExitCellAppearPosY) == 0xe0);
TPW_CHECK(ObjectDef_mInitPricePerUse, TPW_OFF(ObjectDef, mInitPricePerUse) == 0xe4);
TPW_CHECK(ObjectDef_mFatigueEffect, TPW_OFF(ObjectDef, mFatigueEffect) == 0xe8);
TPW_CHECK(ObjectDef_mInitChanceOfLoosing, TPW_OFF(ObjectDef, mInitChanceOfLoosing) == 0xec);
TPW_CHECK(ObjectDef_mProvidesSecurity, TPW_OFF(ObjectDef, mProvidesSecurity) == 0xf0);
TPW_CHECK(ObjectDef_mProvidesRelief, TPW_OFF(ObjectDef, mProvidesRelief) == 0xf4);
TPW_CHECK(ObjectDef_mHoldsLitter, TPW_OFF(ObjectDef, mHoldsLitter) == 0xf8);
TPW_CHECK(ObjectDef_mChillsYouOut, TPW_OFF(ObjectDef, mChillsYouOut) == 0xfc);
TPW_CHECK(ObjectDef_mRequiresTeleport, TPW_OFF(ObjectDef, mRequiresTeleport) == 0x100);
TPW_CHECK(ObjectDef_mRideHandlesSprite, TPW_OFF(ObjectDef, mRideHandlesSprite) == 0x104);
TPW_CHECK(ObjectDef_mRequiresWatering, TPW_OFF(ObjectDef, mRequiresWatering) == 0x108);
TPW_CHECK(ObjectDef_mNonWinnableSideshow, TPW_OFF(ObjectDef, mNonWinnableSideshow) == 0x10c);
TPW_CHECK(ObjectDef_mIsFireworks, TPW_OFF(ObjectDef, mIsFireworks) == 0x110);
TPW_CHECK(ObjectDef_mISIndoors, TPW_OFF(ObjectDef, mISIndoors) == 0x114);
TPW_CHECK(ObjectDef_mCannotRide, TPW_OFF(ObjectDef, mCannotRide) == 0x118);
TPW_CHECK(ObjectDef_mConstrainCamera, TPW_OFF(ObjectDef, mConstrainCamera) == 0x11c);
TPW_CHECK(ObjectDef_mPukeReductionLevel, TPW_OFF(ObjectDef, mPukeReductionLevel) == 0x120);
TPW_CHECK(ObjectDef_mMinCapacity, TPW_OFF(ObjectDef, mMinCapacity) == 0x124);
TPW_CHECK(ObjectDef_mMaxCapacity, TPW_OFF(ObjectDef, mMaxCapacity) == 0x128);
TPW_CHECK(ObjectDef_mMinDuration, TPW_OFF(ObjectDef, mMinDuration) == 0x12c);
TPW_CHECK(ObjectDef_mMaxDuration, TPW_OFF(ObjectDef, mMaxDuration) == 0x130);
TPW_CHECK(ObjectDef_mMinSpeed, TPW_OFF(ObjectDef, mMinSpeed) == 0x134);
TPW_CHECK(ObjectDef_mMaxSpeed, TPW_OFF(ObjectDef, mMaxSpeed) == 0x138);
TPW_CHECK(ObjectDef_mExcitementLevel, TPW_OFF(ObjectDef, mExcitementLevel) == 0x13c);
TPW_CHECK(ObjectDef_mInitCostOfGoods, TPW_OFF(ObjectDef, mInitCostOfGoods) == 0x140);
TPW_CHECK(ObjectDef_mThirstEffect, TPW_OFF(ObjectDef, mThirstEffect) == 0x144);
TPW_CHECK(ObjectDef_mHungerEffect, TPW_OFF(ObjectDef, mHungerEffect) == 0x148);
TPW_CHECK(ObjectDef_mVomitEffect, TPW_OFF(ObjectDef, mVomitEffect) == 0x14c);
TPW_CHECK(ObjectDef_mHappinessEffect, TPW_OFF(ObjectDef, mHappinessEffect) == 0x150);
TPW_CHECK(ObjectDef_mLitterEffect, TPW_OFF(ObjectDef, mLitterEffect) == 0x154);
TPW_CHECK(ObjectDef_mSpecialIngredient, TPW_OFF(ObjectDef, mSpecialIngredient) == 0x158);
TPW_CHECK(ObjectDef_mAppearanceEffect, TPW_OFF(ObjectDef, mAppearanceEffect) == 0x15c);
TPW_CHECK(ObjectDef_mInitPrizeValue, TPW_OFF(ObjectDef, mInitPrizeValue) == 0x160);
TPW_CHECK(ObjectDef_mShopType, TPW_OFF(ObjectDef, mShopType) == 0x164);
TPW_CHECK(ObjectDef_mExciteFactor, TPW_OFF(ObjectDef, mExciteFactor) == 0x168);
TPW_CHECK(ObjectDef_mRipOffOK, TPW_OFF(ObjectDef, mRipOffOK) == 0x16c);
TPW_CHECK(ObjectDef_mNumSimultAnims, TPW_OFF(ObjectDef, mNumSimultAnims) == 0x170);
TPW_CHECK(ObjectDef_mCost, TPW_OFF(ObjectDef, mCost) == 0x174);
TPW_CHECK(ObjectDef_mGroup, TPW_OFF(ObjectDef, mGroup) == 0x178);
TPW_CHECK(ObjectDef_mCategory, TPW_OFF(ObjectDef, mCategory) == 0x17c);
TPW_CHECK(ObjectDef_mUpgrades, TPW_OFF(ObjectDef, mUpgrades) == 0x180);
TPW_CHECK(ObjectDef_mSupplementalMeshes, TPW_OFF(ObjectDef, mSupplementalMeshes) == 0x244);
TPW_CHECK(ObjectDef_mAttractionNewBonus, TPW_OFF(ObjectDef, mAttractionNewBonus) == 0x268);
TPW_CHECK(ObjectDef_mSignTextures, TPW_OFF(ObjectDef, mSignTextures) == 0x278);
TPW_CHECK(ObjectDef_mUpgradesId, TPW_OFF(ObjectDef, mUpgradesId) == 0x284);
TPW_CHECK(ObjectDef_mUpgradeType, TPW_OFF(ObjectDef, mUpgradeType) == 0x288);
TPW_CHECK(ObjectDef_mSamPath, TPW_OFF(ObjectDef, mSamPath) == 0x28c);
TPW_CHECK(BalanceLevelPeepType_mPreferredExcitement, TPW_OFF(BalanceLevelPeepType, mPreferredExcitement) == 0x0);
TPW_CHECK(BalanceLevelPeepType_mStartingCash, TPW_OFF(BalanceLevelPeepType, mStartingCash) == 0x4);
TPW_CHECK(BalanceLevelPeepType_mBoredomThreshold, TPW_OFF(BalanceLevelPeepType, mBoredomThreshold) == 0x8);
TPW_CHECK(BalanceLevelPeepType_size, sizeof(BalanceLevelPeepType) == 0xc);
TPW_CHECK(BalanceLevelLoanInfo_mLoanAmount, TPW_OFF(BalanceLevelLoanInfo, mLoanAmount) == 0x0);
TPW_CHECK(BalanceLevelLoanInfo_mAPRInPercent, TPW_OFF(BalanceLevelLoanInfo, mAPRInPercent) == 0x4);
TPW_CHECK(BalanceLevelLoanInfo_mRepaymentPeriodInMonths, TPW_OFF(BalanceLevelLoanInfo, mRepaymentPeriodInMonths) == 0x8);
TPW_CHECK(BalanceLevelLoanInfo_mLendername, TPW_OFF(BalanceLevelLoanInfo, mLendername) == 0xc);
TPW_CHECK(BalanceLevelLoanInfo_size, sizeof(BalanceLevelLoanInfo) == 0x10);
TPW_CHECK(BalanceLevelPerGradeStaffConst_mBaseWage, TPW_OFF(BalanceLevelPerGradeStaffConst, mBaseWage) == 0x0);
TPW_CHECK(BalanceLevelPerGradeStaffConst_mIdleDuration, TPW_OFF(BalanceLevelPerGradeStaffConst, mIdleDuration) == 0x4);
TPW_CHECK(BalanceLevelPerGradeStaffConst_mRecuperationRate, TPW_OFF(BalanceLevelPerGradeStaffConst, mRecuperationRate) == 0x8);
TPW_CHECK(BalanceLevelPerGradeStaffConst_mHappinessRecuperationRate, TPW_OFF(BalanceLevelPerGradeStaffConst, mHappinessRecuperationRate) == 0xc);
TPW_CHECK(BalanceLevelPerGradeStaffConst_size, sizeof(BalanceLevelPerGradeStaffConst) == 0x10);
TPW_CHECK(BalanceLevelEntertainerConstsPerGrade_mWorkDuration, TPW_OFF(BalanceLevelEntertainerConstsPerGrade, mWorkDuration) == 0x0);
TPW_CHECK(BalanceLevelEntertainerConstsPerGrade_mHappinessEffectOnCell, TPW_OFF(BalanceLevelEntertainerConstsPerGrade, mHappinessEffectOnCell) == 0x4);
TPW_CHECK(BalanceLevelEntertainerConstsPerGrade_mActivationDistance, TPW_OFF(BalanceLevelEntertainerConstsPerGrade, mActivationDistance) == 0x8);
TPW_CHECK(BalanceLevelEntertainerConstsPerGrade_mPoundsPerTrainingPoint, TPW_OFF(BalanceLevelEntertainerConstsPerGrade, mPoundsPerTrainingPoint) == 0xc);
TPW_CHECK(BalanceLevelEntertainerConstsPerGrade_size, sizeof(BalanceLevelEntertainerConstsPerGrade) == 0x10);
TPW_CHECK(BalanceLevelHandymanConstsPerGrade_mWorkDuration, TPW_OFF(BalanceLevelHandymanConstsPerGrade, mWorkDuration) == 0x0);
TPW_CHECK(BalanceLevelHandymanConstsPerGrade_mDetectionRange, TPW_OFF(BalanceLevelHandymanConstsPerGrade, mDetectionRange) == 0x4);
TPW_CHECK(BalanceLevelHandymanConstsPerGrade_mPoundsPerTrainingPoint, TPW_OFF(BalanceLevelHandymanConstsPerGrade, mPoundsPerTrainingPoint) == 0x8);
TPW_CHECK(BalanceLevelHandymanConstsPerGrade_size, sizeof(BalanceLevelHandymanConstsPerGrade) == 0xc);
TPW_CHECK(BalanceLevelMechanicConstsPerGrade_mWorkDuration, TPW_OFF(BalanceLevelMechanicConstsPerGrade, mWorkDuration) == 0x0);
TPW_CHECK(BalanceLevelMechanicConstsPerGrade_mPoundsPerTrainingPoint, TPW_OFF(BalanceLevelMechanicConstsPerGrade, mPoundsPerTrainingPoint) == 0x4);
TPW_CHECK(BalanceLevelMechanicConstsPerGrade_size, sizeof(BalanceLevelMechanicConstsPerGrade) == 0x8);
TPW_CHECK(BalanceLevelResearcherConstsPerGrade_mWorkDuration, TPW_OFF(BalanceLevelResearcherConstsPerGrade, mWorkDuration) == 0x0);
TPW_CHECK(BalanceLevelResearcherConstsPerGrade_mResearchAbility, TPW_OFF(BalanceLevelResearcherConstsPerGrade, mResearchAbility) == 0x4);
TPW_CHECK(BalanceLevelResearcherConstsPerGrade_mPoundsPerTrainingPoint, TPW_OFF(BalanceLevelResearcherConstsPerGrade, mPoundsPerTrainingPoint) == 0x8);
TPW_CHECK(BalanceLevelResearcherConstsPerGrade_size, sizeof(BalanceLevelResearcherConstsPerGrade) == 0xc);
TPW_CHECK(BalanceLevelGuardConstsPerGrade_mWorkDuration, TPW_OFF(BalanceLevelGuardConstsPerGrade, mWorkDuration) == 0x0);
TPW_CHECK(BalanceLevelGuardConstsPerGrade_mPoundsPerTrainingPoint, TPW_OFF(BalanceLevelGuardConstsPerGrade, mPoundsPerTrainingPoint) == 0x4);
TPW_CHECK(BalanceLevelGuardConstsPerGrade_size, sizeof(BalanceLevelGuardConstsPerGrade) == 0x8);
TPW_CHECK(BalanceLevelSeason_mAvgWeatherQuality, TPW_OFF(BalanceLevelSeason, mAvgWeatherQuality) == 0x0);
TPW_CHECK(BalanceLevelSeason_mNormalTolerance, TPW_OFF(BalanceLevelSeason, mNormalTolerance) == 0x4);
TPW_CHECK(BalanceLevelSeason_mChanceForExceptionalWeather, TPW_OFF(BalanceLevelSeason, mChanceForExceptionalWeather) == 0x8);
TPW_CHECK(BalanceLevelSeason_size, sizeof(BalanceLevelSeason) == 0xc);
TPW_CHECK(BalanceLevelRegionFX_mRadius, TPW_OFF(BalanceLevelRegionFX, mRadius) == 0x0);
TPW_CHECK(BalanceLevelRegionFX_mHappiness, TPW_OFF(BalanceLevelRegionFX, mHappiness) == 0x4);
TPW_CHECK(BalanceLevelRegionFX_mIllness, TPW_OFF(BalanceLevelRegionFX, mIllness) == 0x8);
TPW_CHECK(BalanceLevelRegionFX_mHunger, TPW_OFF(BalanceLevelRegionFX, mHunger) == 0xc);
TPW_CHECK(BalanceLevelRegionFX_mSecurity, TPW_OFF(BalanceLevelRegionFX, mSecurity) == 0x10);
TPW_CHECK(BalanceLevelRegionFX_mAttraction, TPW_OFF(BalanceLevelRegionFX, mAttraction) == 0x14);
TPW_CHECK(BalanceLevelRegionFX_size, sizeof(BalanceLevelRegionFX) == 0x18);
TPW_CHECK(BalanceLevelThemeAdvisorCostume_mAddOns, TPW_OFF(BalanceLevelThemeAdvisorCostume, mAddOns) == 0x0);
TPW_CHECK(BalanceLevelThemeAdvisorCostume_mSubOffs, TPW_OFF(BalanceLevelThemeAdvisorCostume, mSubOffs) == 0x4);
TPW_CHECK(BalanceLevelThemeAdvisorCostume_size, sizeof(BalanceLevelThemeAdvisorCostume) == 0x8);
TPW_CHECK(BalanceLevelSkyObject_mX, TPW_OFF(BalanceLevelSkyObject, mX) == 0x0);
TPW_CHECK(BalanceLevelSkyObject_mY, TPW_OFF(BalanceLevelSkyObject, mY) == 0x4);
TPW_CHECK(BalanceLevelSkyObject_mZ, TPW_OFF(BalanceLevelSkyObject, mZ) == 0x8);
TPW_CHECK(BalanceLevelSkyObject_mSize, TPW_OFF(BalanceLevelSkyObject, mSize) == 0xc);
TPW_CHECK(BalanceLevelSkyObject_mFrontFlag, TPW_OFF(BalanceLevelSkyObject, mFrontFlag) == 0x10);
TPW_CHECK(BalanceLevelSkyObject_mFilename, TPW_OFF(BalanceLevelSkyObject, mFilename) == 0x14);
TPW_CHECK(BalanceLevelSkyObject_size, sizeof(BalanceLevelSkyObject) == 0x18);
TPW_CHECK(BalanceLevel_vtable, TPW_OFF(BalanceLevel, vtable) == 0x0);
TPW_CHECK(BalanceLevel_mSlotInfo, TPW_OFF(BalanceLevel, mSlotInfo) == 0x4);
TPW_CHECK(BalanceLevel_mExitLevel, TPW_OFF(BalanceLevel, mExitLevel) == 0xc);
TPW_CHECK(BalanceLevel_mExitLevelVar, TPW_OFF(BalanceLevel, mExitLevelVar) == 0x10);
TPW_CHECK(BalanceLevel_mStartingCashVarPc, TPW_OFF(BalanceLevel, mStartingCashVarPc) == 0x14);
TPW_CHECK(BalanceLevel_mSmallHappinessChange, TPW_OFF(BalanceLevel, mSmallHappinessChange) == 0x18);
TPW_CHECK(BalanceLevel_mMediumHappinessChange, TPW_OFF(BalanceLevel, mMediumHappinessChange) == 0x1c);
TPW_CHECK(BalanceLevel_mBigHappinessChange, TPW_OFF(BalanceLevel, mBigHappinessChange) == 0x20);
TPW_CHECK(BalanceLevel_mPerfectRide, TPW_OFF(BalanceLevel, mPerfectRide) == 0x24);
TPW_CHECK(BalanceLevel_mGoodRide, TPW_OFF(BalanceLevel, mGoodRide) == 0x28);
TPW_CHECK(BalanceLevel_mOKRide, TPW_OFF(BalanceLevel, mOKRide) == 0x2c);
TPW_CHECK(BalanceLevel_mRideVomitDivisor, TPW_OFF(BalanceLevel, mRideVomitDivisor) == 0x30);
TPW_CHECK(BalanceLevel_mToiletDesparate, TPW_OFF(BalanceLevel, mToiletDesparate) == 0x34);
TPW_CHECK(BalanceLevel_mVomitCapacity, TPW_OFF(BalanceLevel, mVomitCapacity) == 0x38);
TPW_CHECK(BalanceLevel_mExcitementToCostDivisor, TPW_OFF(BalanceLevel, mExcitementToCostDivisor) == 0x3c);
TPW_CHECK(BalanceLevel_mMinimumEntryFee, TPW_OFF(BalanceLevel, mMinimumEntryFee) == 0x40);
TPW_CHECK(BalanceLevel_mCheapPriceMultiplier, TPW_OFF(BalanceLevel, mCheapPriceMultiplier) == 0x44);
TPW_CHECK(BalanceLevel_mAveragePriceMultiplier, TPW_OFF(BalanceLevel, mAveragePriceMultiplier) == 0x48);
TPW_CHECK(BalanceLevel_mExpensivePriceMultiplier, TPW_OFF(BalanceLevel, mExpensivePriceMultiplier) == 0x4c);
TPW_CHECK(BalanceLevel_mDecisionVariable1, TPW_OFF(BalanceLevel, mDecisionVariable1) == 0x50);
TPW_CHECK(BalanceLevel_mDecisionVariable2, TPW_OFF(BalanceLevel, mDecisionVariable2) == 0x54);
TPW_CHECK(BalanceLevel_mDecisionVariable3, TPW_OFF(BalanceLevel, mDecisionVariable3) == 0x58);
TPW_CHECK(BalanceLevel_mDecisionVariable4, TPW_OFF(BalanceLevel, mDecisionVariable4) == 0x5c);
TPW_CHECK(BalanceLevel_mDecisionVariable5, TPW_OFF(BalanceLevel, mDecisionVariable5) == 0x60);
TPW_CHECK(BalanceLevel_mDecisionVariable6, TPW_OFF(BalanceLevel, mDecisionVariable6) == 0x64);
TPW_CHECK(BalanceLevel_mDecisionVariable7, TPW_OFF(BalanceLevel, mDecisionVariable7) == 0x68);
TPW_CHECK(BalanceLevel_mDecisionVariable8, TPW_OFF(BalanceLevel, mDecisionVariable8) == 0x6c);
TPW_CHECK(BalanceLevel_mDecisionVarDistWeight, TPW_OFF(BalanceLevel, mDecisionVarDistWeight) == 0x70);
TPW_CHECK(BalanceLevel_mDecisionVarQueueWeight, TPW_OFF(BalanceLevel, mDecisionVarQueueWeight) == 0x74);
TPW_CHECK(BalanceLevel_mDecisionVarExcitementWeight, TPW_OFF(BalanceLevel, mDecisionVarExcitementWeight) == 0x78);
TPW_CHECK(BalanceLevel_mDecisionVarThirstWeight, TPW_OFF(BalanceLevel, mDecisionVarThirstWeight) == 0x7c);
TPW_CHECK(BalanceLevel_mDecisionVarHungerWeight, TPW_OFF(BalanceLevel, mDecisionVarHungerWeight) == 0x80);
TPW_CHECK(BalanceLevel_mDecisionVarToiletWeight, TPW_OFF(BalanceLevel, mDecisionVarToiletWeight) == 0x84);
TPW_CHECK(BalanceLevel_mDecisionVarIllnessWeight, TPW_OFF(BalanceLevel, mDecisionVarIllnessWeight) == 0x88);
TPW_CHECK(BalanceLevel_mOpinionForMuchTooHigh, TPW_OFF(BalanceLevel, mOpinionForMuchTooHigh) == 0x8c);
TPW_CHECK(BalanceLevel_mOpinionForTooHigh, TPW_OFF(BalanceLevel, mOpinionForTooHigh) == 0x90);
TPW_CHECK(BalanceLevel_mOpinionForTooLow, TPW_OFF(BalanceLevel, mOpinionForTooLow) == 0x94);
TPW_CHECK(BalanceLevel_mOpinionForAboutRight, TPW_OFF(BalanceLevel, mOpinionForAboutRight) == 0x98);
TPW_CHECK(BalanceLevel_mPrankeryLikelihood, TPW_OFF(BalanceLevel, mPrankeryLikelihood) == 0x9c);
TPW_CHECK(BalanceLevel_mStinkbombLikelihood, TPW_OFF(BalanceLevel, mStinkbombLikelihood) == 0xa0);
TPW_CHECK(BalanceLevel_mPeepTypes, TPW_OFF(BalanceLevel, mPeepTypes) == 0xa4);
TPW_CHECK(BalanceLevel_mInitialCash, TPW_OFF(BalanceLevel, mInitialCash) == 0x198);
TPW_CHECK(BalanceLevel_mInitialAdmissionFee, TPW_OFF(BalanceLevel, mInitialAdmissionFee) == 0x19c);
TPW_CHECK(BalanceLevel_mLoanInfo, TPW_OFF(BalanceLevel, mLoanInfo) == 0x1a0);
TPW_CHECK(BalanceLevel_mChanceToGetGreatHandymen, TPW_OFF(BalanceLevel, mChanceToGetGreatHandymen) == 0x244);
TPW_CHECK(BalanceLevel_mChanceToGetGreatMechanics, TPW_OFF(BalanceLevel, mChanceToGetGreatMechanics) == 0x248);
TPW_CHECK(BalanceLevel_mChanceToGetGreatEntertainers, TPW_OFF(BalanceLevel, mChanceToGetGreatEntertainers) == 0x24c);
TPW_CHECK(BalanceLevel_mChanceToGetGreatGuards, TPW_OFF(BalanceLevel, mChanceToGetGreatGuards) == 0x250);
TPW_CHECK(BalanceLevel_mChanceToGetGreatResearchers, TPW_OFF(BalanceLevel, mChanceToGetGreatResearchers) == 0x254);
TPW_CHECK(BalanceLevel_mAvgGradeOfHandymen, TPW_OFF(BalanceLevel, mAvgGradeOfHandymen) == 0x258);
TPW_CHECK(BalanceLevel_mAvgGradeOfMechanics, TPW_OFF(BalanceLevel, mAvgGradeOfMechanics) == 0x25c);
TPW_CHECK(BalanceLevel_mAvgGradeOfEntertainers, TPW_OFF(BalanceLevel, mAvgGradeOfEntertainers) == 0x260);
TPW_CHECK(BalanceLevel_mAvgGradeOfGuards, TPW_OFF(BalanceLevel, mAvgGradeOfGuards) == 0x264);
TPW_CHECK(BalanceLevel_mAvgGradeOfResearchers, TPW_OFF(BalanceLevel, mAvgGradeOfResearchers) == 0x268);
TPW_CHECK(BalanceLevel_mBeginningNumberOfHandymen, TPW_OFF(BalanceLevel, mBeginningNumberOfHandymen) == 0x26c);
TPW_CHECK(BalanceLevel_mBeginningNumberOfMechanics, TPW_OFF(BalanceLevel, mBeginningNumberOfMechanics) == 0x270);
TPW_CHECK(BalanceLevel_mBeginningNumberOfEntertainers, TPW_OFF(BalanceLevel, mBeginningNumberOfEntertainers) == 0x274);
TPW_CHECK(BalanceLevel_mBeginningNumberOfGuards, TPW_OFF(BalanceLevel, mBeginningNumberOfGuards) == 0x278);
TPW_CHECK(BalanceLevel_mBeginningNumberOfResearchers, TPW_OFF(BalanceLevel, mBeginningNumberOfResearchers) == 0x27c);
TPW_CHECK(BalanceLevel_mMaxHandymen, TPW_OFF(BalanceLevel, mMaxHandymen) == 0x280);
TPW_CHECK(BalanceLevel_mMaxMechanics, TPW_OFF(BalanceLevel, mMaxMechanics) == 0x284);
TPW_CHECK(BalanceLevel_mMaxEntertainers, TPW_OFF(BalanceLevel, mMaxEntertainers) == 0x288);
TPW_CHECK(BalanceLevel_mMaxGuards, TPW_OFF(BalanceLevel, mMaxGuards) == 0x28c);
TPW_CHECK(BalanceLevel_mMaxResearchers, TPW_OFF(BalanceLevel, mMaxResearchers) == 0x290);
TPW_CHECK(BalanceLevel_mMinHandymenInPool, TPW_OFF(BalanceLevel, mMinHandymenInPool) == 0x294);
TPW_CHECK(BalanceLevel_mMinMechanicsInPool, TPW_OFF(BalanceLevel, mMinMechanicsInPool) == 0x298);
TPW_CHECK(BalanceLevel_mMinEntertainersInPool, TPW_OFF(BalanceLevel, mMinEntertainersInPool) == 0x29c);
TPW_CHECK(BalanceLevel_mMinGuardsInPool, TPW_OFF(BalanceLevel, mMinGuardsInPool) == 0x2a0);
TPW_CHECK(BalanceLevel_mMinResearchersInPool, TPW_OFF(BalanceLevel, mMinResearchersInPool) == 0x2a4);
TPW_CHECK(BalanceLevel_mMaxHandymenInPark, TPW_OFF(BalanceLevel, mMaxHandymenInPark) == 0x2a8);
TPW_CHECK(BalanceLevel_mMaxMechanicsInPark, TPW_OFF(BalanceLevel, mMaxMechanicsInPark) == 0x2ac);
TPW_CHECK(BalanceLevel_mMaxEntertainersInPark, TPW_OFF(BalanceLevel, mMaxEntertainersInPark) == 0x2b0);
TPW_CHECK(BalanceLevel_mMaxGuardsInPark, TPW_OFF(BalanceLevel, mMaxGuardsInPark) == 0x2b4);
TPW_CHECK(BalanceLevel_mMaxResearchersInPark, TPW_OFF(BalanceLevel, mMaxResearchersInPark) == 0x2b8);
TPW_CHECK(BalanceLevel_mBaseCostPerStaff, TPW_OFF(BalanceLevel, mBaseCostPerStaff) == 0x2bc);
TPW_CHECK(BalanceLevel_mCostPerQualityLevel, TPW_OFF(BalanceLevel, mCostPerQualityLevel) == 0x2c0);
TPW_CHECK(BalanceLevel_mTimeBetweenStaffUpdates, TPW_OFF(BalanceLevel, mTimeBetweenStaffUpdates) == 0x2c4);
TPW_CHECK(BalanceLevel_mMaxNumberOfStaffPerUpdate, TPW_OFF(BalanceLevel, mMaxNumberOfStaffPerUpdate) == 0x2c8);
TPW_CHECK(BalanceLevel_mStaffTimeoutTime, TPW_OFF(BalanceLevel, mStaffTimeoutTime) == 0x2cc);
TPW_CHECK(BalanceLevel_mMinPeople, TPW_OFF(BalanceLevel, mMinPeople) == 0x2d0);
TPW_CHECK(BalanceLevel_mTimeBetweenArrivals, TPW_OFF(BalanceLevel, mTimeBetweenArrivals) == 0x2d4);
TPW_CHECK(BalanceLevel_mFixedRate, TPW_OFF(BalanceLevel, mFixedRate) == 0x2d8);
TPW_CHECK(BalanceLevel_mNewParkBonus, TPW_OFF(BalanceLevel, mNewParkBonus) == 0x2dc);
TPW_CHECK(BalanceLevel_mPointsPerVisitor, TPW_OFF(BalanceLevel, mPointsPerVisitor) == 0x2e0);
TPW_CHECK(BalanceLevel_mRestLevel, TPW_OFF(BalanceLevel, mRestLevel) == 0x2e4);
TPW_CHECK(BalanceLevel_mHappyHitCosNoRestArea, TPW_OFF(BalanceLevel, mHappyHitCosNoRestArea) == 0x2e8);
TPW_CHECK(BalanceLevel_mPerGradeStaffConsts, TPW_OFF(BalanceLevel, mPerGradeStaffConsts) == 0x2ec);
TPW_CHECK(BalanceLevel_mPerTypeStaffConstsPayMultiplier, TPW_OFF(BalanceLevel, mPerTypeStaffConstsPayMultiplier) == 0x340);
TPW_CHECK(BalanceLevel_mEntertainerConstsPerGrade, TPW_OFF(BalanceLevel, mEntertainerConstsPerGrade) == 0x358);
TPW_CHECK(BalanceLevel_mHandymanConstsPerGrade, TPW_OFF(BalanceLevel, mHandymanConstsPerGrade) == 0x3ac);
TPW_CHECK(BalanceLevel_mMechanicConstsPerGrade, TPW_OFF(BalanceLevel, mMechanicConstsPerGrade) == 0x3ec);
TPW_CHECK(BalanceLevel_mResearcherConstsPerGrade, TPW_OFF(BalanceLevel, mResearcherConstsPerGrade) == 0x418);
TPW_CHECK(BalanceLevel_mGuardConstsPerGrade, TPW_OFF(BalanceLevel, mGuardConstsPerGrade) == 0x458);
TPW_CHECK(BalanceLevel_mSeasons, TPW_OFF(BalanceLevel, mSeasons) == 0x484);
TPW_CHECK(BalanceLevel_mDaysOfWarning, TPW_OFF(BalanceLevel, mDaysOfWarning) == 0x4b8);
TPW_CHECK(BalanceLevel_mDaysBetweenChanges, TPW_OFF(BalanceLevel, mDaysBetweenChanges) == 0x4bc);
TPW_CHECK(BalanceLevel_mSpeedOfChange, TPW_OFF(BalanceLevel, mSpeedOfChange) == 0x4c0);
TPW_CHECK(BalanceLevel_mQualityForSnowLow, TPW_OFF(BalanceLevel, mQualityForSnowLow) == 0x4c4);
TPW_CHECK(BalanceLevel_mQualityForSnowHigh, TPW_OFF(BalanceLevel, mQualityForSnowHigh) == 0x4c8);
TPW_CHECK(BalanceLevel_mQualityForRainLow, TPW_OFF(BalanceLevel, mQualityForRainLow) == 0x4cc);
TPW_CHECK(BalanceLevel_mQualityForRainHigh, TPW_OFF(BalanceLevel, mQualityForRainHigh) == 0x4d0);
TPW_CHECK(BalanceLevel_mQualityForLightningLow, TPW_OFF(BalanceLevel, mQualityForLightningLow) == 0x4d4);
TPW_CHECK(BalanceLevel_mQualityForLightningHigh, TPW_OFF(BalanceLevel, mQualityForLightningHigh) == 0x4d8);
TPW_CHECK(BalanceLevel_mLightningCountdownSpeed, TPW_OFF(BalanceLevel, mLightningCountdownSpeed) == 0x4dc);
TPW_CHECK(BalanceLevel_mLightningCountdownFrom, TPW_OFF(BalanceLevel, mLightningCountdownFrom) == 0x4e0);
TPW_CHECK(BalanceLevel_mLightningTime, TPW_OFF(BalanceLevel, mLightningTime) == 0x4e4);
TPW_CHECK(BalanceLevel_mThunderStartsThisFarAway, TPW_OFF(BalanceLevel, mThunderStartsThisFarAway) == 0x4e8);
TPW_CHECK(BalanceLevel_mThunderRandomiser, TPW_OFF(BalanceLevel, mThunderRandomiser) == 0x4ec);
TPW_CHECK(BalanceLevel_mMaxRaindrops, TPW_OFF(BalanceLevel, mMaxRaindrops) == 0x4f0);
TPW_CHECK(BalanceLevel_mMinTimeAnyMessage, TPW_OFF(BalanceLevel, mMinTimeAnyMessage) == 0x4f4);
TPW_CHECK(BalanceLevel_mMinTimeSameMessage, TPW_OFF(BalanceLevel, mMinTimeSameMessage) == 0x4f8);
TPW_CHECK(BalanceLevel_mResearchTechPercentageForThisTech, TPW_OFF(BalanceLevel, mResearchTechPercentageForThisTech) == 0x4fc);
TPW_CHECK(BalanceLevel_mResearchCategoriesEffort, TPW_OFF(BalanceLevel, mResearchCategoriesEffort) == 0x528);
TPW_CHECK(BalanceLevel_mStartingWorkLoad, TPW_OFF(BalanceLevel, mStartingWorkLoad) == 0x540);
TPW_CHECK(BalanceLevel_mHeightfieldXStart, TPW_OFF(BalanceLevel, mHeightfieldXStart) == 0x544);
TPW_CHECK(BalanceLevel_mHeightfieldYStart, TPW_OFF(BalanceLevel, mHeightfieldYStart) == 0x548);
TPW_CHECK(BalanceLevel_mHeightfieldWidth, TPW_OFF(BalanceLevel, mHeightfieldWidth) == 0x54c);
TPW_CHECK(BalanceLevel_mHeightfieldHeight, TPW_OFF(BalanceLevel, mHeightfieldHeight) == 0x550);
TPW_CHECK(BalanceLevel_mFixedItemOriginX, TPW_OFF(BalanceLevel, mFixedItemOriginX) == 0x554);
TPW_CHECK(BalanceLevel_mFixedItemOriginY, TPW_OFF(BalanceLevel, mFixedItemOriginY) == 0x558);
TPW_CHECK(BalanceLevel_mTicketBoothAPosX, TPW_OFF(BalanceLevel, mTicketBoothAPosX) == 0x55c);
TPW_CHECK(BalanceLevel_mTicketBoothAPosY, TPW_OFF(BalanceLevel, mTicketBoothAPosY) == 0x560);
TPW_CHECK(BalanceLevel_mTicketBoothBPosX, TPW_OFF(BalanceLevel, mTicketBoothBPosX) == 0x564);
TPW_CHECK(BalanceLevel_mTicketBoothBPosY, TPW_OFF(BalanceLevel, mTicketBoothBPosY) == 0x568);
TPW_CHECK(BalanceLevel_mBusStopAPosX, TPW_OFF(BalanceLevel, mBusStopAPosX) == 0x56c);
TPW_CHECK(BalanceLevel_mBusStopAPosY, TPW_OFF(BalanceLevel, mBusStopAPosY) == 0x570);
TPW_CHECK(BalanceLevel_mBusStopBPosX, TPW_OFF(BalanceLevel, mBusStopBPosX) == 0x574);
TPW_CHECK(BalanceLevel_mBusStopBPosY, TPW_OFF(BalanceLevel, mBusStopBPosY) == 0x578);
TPW_CHECK(BalanceLevel_mEntranceAPosX, TPW_OFF(BalanceLevel, mEntranceAPosX) == 0x57c);
TPW_CHECK(BalanceLevel_mEntranceAPosY, TPW_OFF(BalanceLevel, mEntranceAPosY) == 0x580);
TPW_CHECK(BalanceLevel_mEntranceBPosX, TPW_OFF(BalanceLevel, mEntranceBPosX) == 0x584);
TPW_CHECK(BalanceLevel_mEntranceBPosY, TPW_OFF(BalanceLevel, mEntranceBPosY) == 0x588);
TPW_CHECK(BalanceLevel_mCrossingParkSideAPosX, TPW_OFF(BalanceLevel, mCrossingParkSideAPosX) == 0x58c);
TPW_CHECK(BalanceLevel_mCrossingParkSideAPosY, TPW_OFF(BalanceLevel, mCrossingParkSideAPosY) == 0x590);
TPW_CHECK(BalanceLevel_mCrossingParkSideBPosX, TPW_OFF(BalanceLevel, mCrossingParkSideBPosX) == 0x594);
TPW_CHECK(BalanceLevel_mCrossingParkSideBPosY, TPW_OFF(BalanceLevel, mCrossingParkSideBPosY) == 0x598);
TPW_CHECK(BalanceLevel_mCrossingBSSideAPosX, TPW_OFF(BalanceLevel, mCrossingBSSideAPosX) == 0x59c);
TPW_CHECK(BalanceLevel_mCrossingBSSideAPosY, TPW_OFF(BalanceLevel, mCrossingBSSideAPosY) == 0x5a0);
TPW_CHECK(BalanceLevel_mCrossingBSSideBPosX, TPW_OFF(BalanceLevel, mCrossingBSSideBPosX) == 0x5a4);
TPW_CHECK(BalanceLevel_mCrossingBSSideBPosY, TPW_OFF(BalanceLevel, mCrossingBSSideBPosY) == 0x5a8);
TPW_CHECK(BalanceLevel_mStrikeAreaStartX, TPW_OFF(BalanceLevel, mStrikeAreaStartX) == 0x5ac);
TPW_CHECK(BalanceLevel_mStrikeAreaStartY, TPW_OFF(BalanceLevel, mStrikeAreaStartY) == 0x5b0);
TPW_CHECK(BalanceLevel_mStrikeAreaSizeX, TPW_OFF(BalanceLevel, mStrikeAreaSizeX) == 0x5b4);
TPW_CHECK(BalanceLevel_mStrikeAreaSizeY, TPW_OFF(BalanceLevel, mStrikeAreaSizeY) == 0x5b8);
TPW_CHECK(BalanceLevel_mQueueCell, TPW_OFF(BalanceLevel, mQueueCell) == 0x5bc);
TPW_CHECK(BalanceLevel_mPathCell, TPW_OFF(BalanceLevel, mPathCell) == 0x5c0);
TPW_CHECK(BalanceLevel_mKartTrackCell, TPW_OFF(BalanceLevel, mKartTrackCell) == 0x5c4);
TPW_CHECK(BalanceLevel_mWaterTrackCell, TPW_OFF(BalanceLevel, mWaterTrackCell) == 0x5c8);
TPW_CHECK(BalanceLevel_mMapCell, TPW_OFF(BalanceLevel, mMapCell) == 0x5cc);
TPW_CHECK(BalanceLevel_mRegionFX, TPW_OFF(BalanceLevel, mRegionFX) == 0x5d0);
TPW_CHECK(BalanceLevel_mFogColour, TPW_OFF(BalanceLevel, mFogColour) == 0x694);
TPW_CHECK(BalanceLevel_mAmbientLightLevel, TPW_OFF(BalanceLevel, mAmbientLightLevel) == 0x698);
TPW_CHECK(BalanceLevel_mDirectionalLightLevel, TPW_OFF(BalanceLevel, mDirectionalLightLevel) == 0x69c);
TPW_CHECK(BalanceLevel_mProceduralTexturingXOffset, TPW_OFF(BalanceLevel, mProceduralTexturingXOffset) == 0x6a0);
TPW_CHECK(BalanceLevel_mProceduralTexturingYOffset, TPW_OFF(BalanceLevel, mProceduralTexturingYOffset) == 0x6a4);
TPW_CHECK(BalanceLevel_mLightNormalX, TPW_OFF(BalanceLevel, mLightNormalX) == 0x6a8);
TPW_CHECK(BalanceLevel_mLightNormalY, TPW_OFF(BalanceLevel, mLightNormalY) == 0x6ac);
TPW_CHECK(BalanceLevel_mLightNormalZ, TPW_OFF(BalanceLevel, mLightNormalZ) == 0x6b0);
TPW_CHECK(BalanceLevel_mThemeAdvisorCostumes, TPW_OFF(BalanceLevel, mThemeAdvisorCostumes) == 0x6b4);
TPW_CHECK(BalanceLevel_mWaterStartPosX, TPW_OFF(BalanceLevel, mWaterStartPosX) == 0x6d8);
TPW_CHECK(BalanceLevel_mWaterStartPosY, TPW_OFF(BalanceLevel, mWaterStartPosY) == 0x6dc);
TPW_CHECK(BalanceLevel_mWaterStartPosZ, TPW_OFF(BalanceLevel, mWaterStartPosZ) == 0x6e0);
TPW_CHECK(BalanceLevel_mn, TPW_OFF(BalanceLevel, mn) == 0x6e4);
TPW_CHECK(BalanceLevel_mSkyObjects, TPW_OFF(BalanceLevel, mSkyObjects) == 0x6e8);
TPW_CHECK(BalanceLevel_mReverb, TPW_OFF(BalanceLevel, mReverb) == 0x74c);
TPW_CHECK(BalanceLevel_mVisitors, TPW_OFF(BalanceLevel, mVisitors) == 0x750);
TPW_CHECK(BalanceLevel_mPeopleInPark, TPW_OFF(BalanceLevel, mPeopleInPark) == 0x754);
TPW_CHECK(BalanceLevel_mHappiness, TPW_OFF(BalanceLevel, mHappiness) == 0x758);
TPW_CHECK(BalanceLevel_mAtLeastThisManyHappyPeople, TPW_OFF(BalanceLevel, mAtLeastThisManyHappyPeople) == 0x75c);
TPW_CHECK(BalanceLevel_mProfitYear, TPW_OFF(BalanceLevel, mProfitYear) == 0x760);
TPW_CHECK(BalanceLevel_mRecentVisitors, TPW_OFF(BalanceLevel, mRecentVisitors) == 0x764);
TPW_CHECK(BalanceLevel_mRecentVisitorMonths, TPW_OFF(BalanceLevel, mRecentVisitorMonths) == 0x768);
TPW_CHECK(BalanceLevel_mCoasterHeight, TPW_OFF(BalanceLevel, mCoasterHeight) == 0x76c);
TPW_CHECK(BalanceLevel_mGokartExcitement, TPW_OFF(BalanceLevel, mGokartExcitement) == 0x770);
TPW_CHECK(BalanceLevel_mWaterLength, TPW_OFF(BalanceLevel, mWaterLength) == 0x774);
TPW_CHECK(BalanceLevel_mMinCellsOwned, TPW_OFF(BalanceLevel, mMinCellsOwned) == 0x778);
TPW_CHECK(BalanceLevel_mMinCellsCovered, TPW_OFF(BalanceLevel, mMinCellsCovered) == 0x77c);
TPW_CHECK(BalanceLevel_mDaysAfterCompletedChallenge, TPW_OFF(BalanceLevel, mDaysAfterCompletedChallenge) == 0x780);
TPW_CHECK(BalanceLevel_mDaysAfterDeclinedChallenge, TPW_OFF(BalanceLevel, mDaysAfterDeclinedChallenge) == 0x784);
TPW_CHECK(BalanceLevel_mDaysUntilFirstChallenge, TPW_OFF(BalanceLevel, mDaysUntilFirstChallenge) == 0x788);
TPW_CHECK(BalanceLevel_mDeclinesToForfeit, TPW_OFF(BalanceLevel, mDeclinesToForfeit) == 0x78c);
TPW_CHECK(BalanceLevel_mShortTimeLeftWarningAt, TPW_OFF(BalanceLevel, mShortTimeLeftWarningAt) == 0x790);
TPW_CHECK(BalanceLevel_mChallengesInThisLevelChallengeType, TPW_OFF(BalanceLevel, mChallengesInThisLevelChallengeType) == 0x794);
TPW_CHECK(BalanceLevel_size, sizeof(BalanceLevel) == 0x7e8);
TPW_CHECK(BalanceOnline_vtable, TPW_OFF(BalanceOnline, vtable) == 0x0);
TPW_CHECK(BalanceOnline_mSlotInfo, TPW_OFF(BalanceOnline, mSlotInfo) == 0x4);
TPW_CHECK(BalanceOnline_mMailPort, TPW_OFF(BalanceOnline, mMailPort) == 0xc);
TPW_CHECK(BalanceOnline_mChatPort, TPW_OFF(BalanceOnline, mChatPort) == 0x10);
TPW_CHECK(BalanceOnline_mLoginPort, TPW_OFF(BalanceOnline, mLoginPort) == 0x14);
TPW_CHECK(BalanceOnline_mCityPort, TPW_OFF(BalanceOnline, mCityPort) == 0x18);
TPW_CHECK(BalanceOnline_mNewsPort, TPW_OFF(BalanceOnline, mNewsPort) == 0x1c);
TPW_CHECK(BalanceOnline_mVotePort, TPW_OFF(BalanceOnline, mVotePort) == 0x20);
TPW_CHECK(BalanceOnline_mMailServer, TPW_OFF(BalanceOnline, mMailServer) == 0x24);
TPW_CHECK(BalanceOnline_mChatServer, TPW_OFF(BalanceOnline, mChatServer) == 0x28);
TPW_CHECK(BalanceOnline_mLoginServer, TPW_OFF(BalanceOnline, mLoginServer) == 0x2c);
TPW_CHECK(BalanceOnline_mCityServer, TPW_OFF(BalanceOnline, mCityServer) == 0x30);
TPW_CHECK(BalanceOnline_mNewsServer, TPW_OFF(BalanceOnline, mNewsServer) == 0x34);
TPW_CHECK(BalanceOnline_mVoteServer, TPW_OFF(BalanceOnline, mVoteServer) == 0x38);
TPW_CHECK(BalanceOnline_mComplainEmail, TPW_OFF(BalanceOnline, mComplainEmail) == 0x3c);
TPW_CHECK(BalanceOnline_size, sizeof(BalanceOnline) == 0x40);
TPW_CHECK(BalanceGraphics_vtable, TPW_OFF(BalanceGraphics, vtable) == 0x0);
TPW_CHECK(BalanceGraphics_mSlotInfo, TPW_OFF(BalanceGraphics, mSlotInfo) == 0x4);
TPW_CHECK(BalanceGraphics_mTEXTUREQUALITY, TPW_OFF(BalanceGraphics, mTEXTUREQUALITY) == 0xc);
TPW_CHECK(BalanceGraphics_mTEXTUREFILTERING, TPW_OFF(BalanceGraphics, mTEXTUREFILTERING) == 0x10);
TPW_CHECK(BalanceGraphics_mPROCEDURALTEXTURING, TPW_OFF(BalanceGraphics, mPROCEDURALTEXTURING) == 0x14);
TPW_CHECK(BalanceGraphics_mPROCEDURALTEXTURECACHE, TPW_OFF(BalanceGraphics, mPROCEDURALTEXTURECACHE) == 0x18);
TPW_CHECK(BalanceGraphics_mANIMATINGTEXTURES, TPW_OFF(BalanceGraphics, mANIMATINGTEXTURES) == 0x1c);
TPW_CHECK(BalanceGraphics_mSKYQUALITY, TPW_OFF(BalanceGraphics, mSKYQUALITY) == 0x20);
TPW_CHECK(BalanceGraphics_mSKYDETAIL, TPW_OFF(BalanceGraphics, mSKYDETAIL) == 0x24);
TPW_CHECK(BalanceGraphics_mSKYSHADOW, TPW_OFF(BalanceGraphics, mSKYSHADOW) == 0x28);
TPW_CHECK(BalanceGraphics_mMESHSHADOW, TPW_OFF(BalanceGraphics, mMESHSHADOW) == 0x2c);
TPW_CHECK(BalanceGraphics_mSPRITESHADOW, TPW_OFF(BalanceGraphics, mSPRITESHADOW) == 0x30);
TPW_CHECK(BalanceGraphics_mTRIPLEBUFFER, TPW_OFF(BalanceGraphics, mTRIPLEBUFFER) == 0x34);
TPW_CHECK(BalanceGraphics_mFOG, TPW_OFF(BalanceGraphics, mFOG) == 0x38);
TPW_CHECK(BalanceGraphics_mMIPMAP, TPW_OFF(BalanceGraphics, mMIPMAP) == 0x3c);
TPW_CHECK(BalanceGraphics_mRENDER32, TPW_OFF(BalanceGraphics, mRENDER32) == 0x40);
TPW_CHECK(BalanceGraphics_mTEXTURE32, TPW_OFF(BalanceGraphics, mTEXTURE32) == 0x44);
TPW_CHECK(BalanceGraphics_mBUMPMAPPING, TPW_OFF(BalanceGraphics, mBUMPMAPPING) == 0x48);
TPW_CHECK(BalanceGraphics_mFIRSTPERSONVIEWDISTANCE, TPW_OFF(BalanceGraphics, mFIRSTPERSONVIEWDISTANCE) == 0x4c);
TPW_CHECK(BalanceGraphics_mLIGHTING, TPW_OFF(BalanceGraphics, mLIGHTING) == 0x50);
TPW_CHECK(BalanceGraphics_mFORCELOWALPHAREF, TPW_OFF(BalanceGraphics, mFORCELOWALPHAREF) == 0x54);
TPW_CHECK(BalanceGraphics_mCOASTERSMOOTHNESS, TPW_OFF(BalanceGraphics, mCOASTERSMOOTHNESS) == 0x58);
TPW_CHECK(BalanceGraphics_mCOASTERTRACKDETAIL, TPW_OFF(BalanceGraphics, mCOASTERTRACKDETAIL) == 0x5c);
TPW_CHECK(BalanceGraphics_mPARTICLEDENSITY, TPW_OFF(BalanceGraphics, mPARTICLEDENSITY) == 0x60);
TPW_CHECK(BalanceGraphics_mTOTALPARTICLES, TPW_OFF(BalanceGraphics, mTOTALPARTICLES) == 0x64);
TPW_CHECK(BalanceGraphics_mNUMKIDS, TPW_OFF(BalanceGraphics, mNUMKIDS) == 0x68);
TPW_CHECK(BalanceGraphics_mWEATHER, TPW_OFF(BalanceGraphics, mWEATHER) == 0x6c);
TPW_CHECK(BalanceGraphics_mWATER, TPW_OFF(BalanceGraphics, mWATER) == 0x70);
TPW_CHECK(BalanceGraphics_mLOBBYOBJECTS, TPW_OFF(BalanceGraphics, mLOBBYOBJECTS) == 0x74);
TPW_CHECK(BalanceGraphics_size, sizeof(BalanceGraphics) == 0x78);
TPW_CHECK(BalanceResolution_vtable, TPW_OFF(BalanceResolution, vtable) == 0x0);
TPW_CHECK(BalanceResolution_mSlotInfo, TPW_OFF(BalanceResolution, mSlotInfo) == 0x4);
TPW_CHECK(BalanceResolution_mRESOLUTION, TPW_OFF(BalanceResolution, mRESOLUTION) == 0xc);
TPW_CHECK(BalanceResolution_size, sizeof(BalanceResolution) == 0x10);
TPW_CHECK(BalanceSoundChannelInfo_mCHANNELS, TPW_OFF(BalanceSoundChannelInfo, mCHANNELS) == 0x0);
TPW_CHECK(BalanceSoundChannelInfo_mSWITCH, TPW_OFF(BalanceSoundChannelInfo, mSWITCH) == 0x4);
TPW_CHECK(BalanceSoundChannelInfo_size, sizeof(BalanceSoundChannelInfo) == 0x8);
TPW_CHECK(BalanceSoundRadiusInfo_mMINRADIUS, TPW_OFF(BalanceSoundRadiusInfo, mMINRADIUS) == 0x0);
TPW_CHECK(BalanceSoundRadiusInfo_mSWITCH, TPW_OFF(BalanceSoundRadiusInfo, mSWITCH) == 0x4);
TPW_CHECK(BalanceSoundRadiusInfo_size, sizeof(BalanceSoundRadiusInfo) == 0x8);
TPW_CHECK(BalanceSound_vtable, TPW_OFF(BalanceSound, vtable) == 0x0);
TPW_CHECK(BalanceSound_mSlotInfo, TPW_OFF(BalanceSound, mSlotInfo) == 0x4);
TPW_CHECK(BalanceSound_mQSOUND, TPW_OFF(BalanceSound, mQSOUND) == 0xc);
TPW_CHECK(BalanceSound_mREAR, TPW_OFF(BalanceSound, mREAR) == 0x10);
TPW_CHECK(BalanceSound_mCROSSOVER, TPW_OFF(BalanceSound, mCROSSOVER) == 0x14);
TPW_CHECK(BalanceSound_mEAX, TPW_OFF(BalanceSound, mEAX) == 0x18);
TPW_CHECK(BalanceSound_mGEOMETRY, TPW_OFF(BalanceSound, mGEOMETRY) == 0x1c);
TPW_CHECK(BalanceSound_mMPEGRATE, TPW_OFF(BalanceSound, mMPEGRATE) == 0x20);
TPW_CHECK(BalanceSound_mDUCKINGLEVEL, TPW_OFF(BalanceSound, mDUCKINGLEVEL) == 0x24);
TPW_CHECK(BalanceSound_mDROPPABLELEVEL, TPW_OFF(BalanceSound, mDROPPABLELEVEL) == 0x28);
TPW_CHECK(BalanceSound_mDEFAULTQUALITY, TPW_OFF(BalanceSound, mDEFAULTQUALITY) == 0x2c);
TPW_CHECK(BalanceSound_mFADETOGGLE, TPW_OFF(BalanceSound, mFADETOGGLE) == 0x30);
TPW_CHECK(BalanceSound_mSOUNDON, TPW_OFF(BalanceSound, mSOUNDON) == 0x34);
TPW_CHECK(BalanceSound_mSFX, TPW_OFF(BalanceSound, mSFX) == 0x38);
TPW_CHECK(BalanceSound_mMUSIC, TPW_OFF(BalanceSound, mMUSIC) == 0x3c);
TPW_CHECK(BalanceSound_mSPEECH, TPW_OFF(BalanceSound, mSPEECH) == 0x40);
TPW_CHECK(BalanceSound_mMOVIE, TPW_OFF(BalanceSound, mMOVIE) == 0x44);
TPW_CHECK(BalanceSound_mChannelInfo, TPW_OFF(BalanceSound, mChannelInfo) == 0x48);
TPW_CHECK(BalanceSound_mRadiusInfo, TPW_OFF(BalanceSound, mRadiusInfo) == 0x64);
TPW_CHECK(BalanceSound_size, sizeof(BalanceSound) == 0x80);
TPW_CHECK(BalanceHoardingsHoardingClick_mX, TPW_OFF(BalanceHoardingsHoardingClick, mX) == 0x0);
TPW_CHECK(BalanceHoardingsHoardingClick_mY, TPW_OFF(BalanceHoardingsHoardingClick, mY) == 0x4);
TPW_CHECK(BalanceHoardingsHoardingClick_size, sizeof(BalanceHoardingsHoardingClick) == 0x8);
TPW_CHECK(BalanceHoardings_vtable, TPW_OFF(BalanceHoardings, vtable) == 0x0);
TPW_CHECK(BalanceHoardings_mSlotInfo, TPW_OFF(BalanceHoardings, mSlotInfo) == 0x4);
TPW_CHECK(BalanceHoardings_mHoardingClicks, TPW_OFF(BalanceHoardings, mHoardingClicks) == 0xc);
TPW_CHECK(BalanceHoardings_size, sizeof(BalanceHoardings) == 0x210);
TPW_CHECK(BalanceTrackAsTextureData_mpcTextureFilename, TPW_OFF(BalanceTrackAsTextureData, mpcTextureFilename) == 0x0);
TPW_CHECK(BalanceTrackAsTextureData_mbIsSelfIlluminating, TPW_OFF(BalanceTrackAsTextureData, mbIsSelfIlluminating) == 0x4);
TPW_CHECK(BalanceTrackAsTextureData_mfScrollRate, TPW_OFF(BalanceTrackAsTextureData, mfScrollRate) == 0x8);
TPW_CHECK(BalanceTrackAsTextureData_size, sizeof(BalanceTrackAsTextureData) == 0xc);
TPW_CHECK(BalanceTrackAsCrossSectionPoint_mfX, TPW_OFF(BalanceTrackAsCrossSectionPoint, mfX) == 0x0);
TPW_CHECK(BalanceTrackAsCrossSectionPoint_mfY, TPW_OFF(BalanceTrackAsCrossSectionPoint, mfY) == 0x4);
TPW_CHECK(BalanceTrackAsCrossSectionPoint_mfU, TPW_OFF(BalanceTrackAsCrossSectionPoint, mfU) == 0x8);
TPW_CHECK(BalanceTrackAsCrossSectionPoint_mfNx, TPW_OFF(BalanceTrackAsCrossSectionPoint, mfNx) == 0xc);
TPW_CHECK(BalanceTrackAsCrossSectionPoint_mfNy, TPW_OFF(BalanceTrackAsCrossSectionPoint, mfNy) == 0x10);
TPW_CHECK(BalanceTrackAsCrossSectionPoint_size, sizeof(BalanceTrackAsCrossSectionPoint) == 0x14);
TPW_CHECK(BalanceTrackAsCrossSectionEdge_musTexture, TPW_OFF(BalanceTrackAsCrossSectionEdge, musTexture) == 0x0);
TPW_CHECK(BalanceTrackAsCrossSectionEdge_musCSPoint1, TPW_OFF(BalanceTrackAsCrossSectionEdge, musCSPoint1) == 0x4);
TPW_CHECK(BalanceTrackAsCrossSectionEdge_musCSPoint2, TPW_OFF(BalanceTrackAsCrossSectionEdge, musCSPoint2) == 0x8);
TPW_CHECK(BalanceTrackAsCrossSectionEdge_size, sizeof(BalanceTrackAsCrossSectionEdge) == 0xc);
TPW_CHECK(BalanceTrackAsCrossSection_musCSPointBlockIndex, TPW_OFF(BalanceTrackAsCrossSection, musCSPointBlockIndex) == 0x0);
TPW_CHECK(BalanceTrackAsCrossSection_musCSEdgeBlockIndex, TPW_OFF(BalanceTrackAsCrossSection, musCSEdgeBlockIndex) == 0x4);
TPW_CHECK(BalanceTrackAsCrossSection_musTrackEdge, TPW_OFF(BalanceTrackAsCrossSection, musTrackEdge) == 0x8);
TPW_CHECK(BalanceTrackAsCrossSection_musBlendToCSPointBlockIndex, TPW_OFF(BalanceTrackAsCrossSection, musBlendToCSPointBlockIndex) == 0xc);
TPW_CHECK(BalanceTrackAsCrossSection_mfBlendLength, TPW_OFF(BalanceTrackAsCrossSection, mfBlendLength) == 0x10);
TPW_CHECK(BalanceTrackAsCrossSection_size, sizeof(BalanceTrackAsCrossSection) == 0x14);
TPW_CHECK(BalanceTrackAsCrossSectionSelect_mfSpeedRangeMin, TPW_OFF(BalanceTrackAsCrossSectionSelect, mfSpeedRangeMin) == 0x0);
TPW_CHECK(BalanceTrackAsCrossSectionSelect_mfSpeedRangeMax, TPW_OFF(BalanceTrackAsCrossSectionSelect, mfSpeedRangeMax) == 0x4);
TPW_CHECK(BalanceTrackAsCrossSectionSelect_mfDirectionRangeMin, TPW_OFF(BalanceTrackAsCrossSectionSelect, mfDirectionRangeMin) == 0x8);
TPW_CHECK(BalanceTrackAsCrossSectionSelect_mfDirectionRangeMax, TPW_OFF(BalanceTrackAsCrossSectionSelect, mfDirectionRangeMax) == 0xc);
TPW_CHECK(BalanceTrackAsCrossSectionSelect_mfPitchRangeMin, TPW_OFF(BalanceTrackAsCrossSectionSelect, mfPitchRangeMin) == 0x10);
TPW_CHECK(BalanceTrackAsCrossSectionSelect_mfPitchRangeMax, TPW_OFF(BalanceTrackAsCrossSectionSelect, mfPitchRangeMax) == 0x14);
TPW_CHECK(BalanceTrackAsCrossSectionSelect_mfBankRangeMin, TPW_OFF(BalanceTrackAsCrossSectionSelect, mfBankRangeMin) == 0x18);
TPW_CHECK(BalanceTrackAsCrossSectionSelect_mfBankRangeMax, TPW_OFF(BalanceTrackAsCrossSectionSelect, mfBankRangeMax) == 0x1c);
TPW_CHECK(BalanceTrackAsCrossSectionSelect_musCrossSectionIndex, TPW_OFF(BalanceTrackAsCrossSectionSelect, musCrossSectionIndex) == 0x20);
TPW_CHECK(BalanceTrackAsCrossSectionSelect_musBlendFromCrossSectionIndex, TPW_OFF(BalanceTrackAsCrossSectionSelect, musBlendFromCrossSectionIndex) == 0x24);
TPW_CHECK(BalanceTrackAsCrossSectionSelect_musBlendToCrossSectionIndex, TPW_OFF(BalanceTrackAsCrossSectionSelect, musBlendToCrossSectionIndex) == 0x28);
TPW_CHECK(BalanceTrackAsCrossSectionSelect_size, sizeof(BalanceTrackAsCrossSectionSelect) == 0x2c);
TPW_CHECK(BalanceTrackAsPylonControl_mpcMeshFilename, TPW_OFF(BalanceTrackAsPylonControl, mpcMeshFilename) == 0x0);
TPW_CHECK(BalanceTrackAsPylonControl_muiPylonTypeUniqueID, TPW_OFF(BalanceTrackAsPylonControl, muiPylonTypeUniqueID) == 0x4);
TPW_CHECK(BalanceTrackAsPylonControl_mbDontDisableBaseOnStack, TPW_OFF(BalanceTrackAsPylonControl, mbDontDisableBaseOnStack) == 0x8);
TPW_CHECK(BalanceTrackAsPylonControl_mfCollisionPossibleRadius, TPW_OFF(BalanceTrackAsPylonControl, mfCollisionPossibleRadius) == 0xc);
TPW_CHECK(BalanceTrackAsPylonControl_mfPylonStraightDotThreshold, TPW_OFF(BalanceTrackAsPylonControl, mfPylonStraightDotThreshold) == 0x10);
TPW_CHECK(BalanceTrackAsPylonControl_muiPylonWidthCells, TPW_OFF(BalanceTrackAsPylonControl, muiPylonWidthCells) == 0x14);
TPW_CHECK(BalanceTrackAsPylonControl_muiPylonLengthCells, TPW_OFF(BalanceTrackAsPylonControl, muiPylonLengthCells) == 0x18);
TPW_CHECK(BalanceTrackAsPylonControl_muiMaxStackedPylons, TPW_OFF(BalanceTrackAsPylonControl, muiMaxStackedPylons) == 0x1c);
TPW_CHECK(BalanceTrackAsPylonControl_mfMinHeight, TPW_OFF(BalanceTrackAsPylonControl, mfMinHeight) == 0x20);
TPW_CHECK(BalanceTrackAsPylonControl_mfMaxHeight, TPW_OFF(BalanceTrackAsPylonControl, mfMaxHeight) == 0x24);
TPW_CHECK(BalanceTrackAsPylonControl_mfMaxHeightDiffPerGSquareDist, TPW_OFF(BalanceTrackAsPylonControl, mfMaxHeightDiffPerGSquareDist) == 0x28);
TPW_CHECK(BalanceTrackAsPylonControl_mfEntranceMinXZAngleAtMinDist, TPW_OFF(BalanceTrackAsPylonControl, mfEntranceMinXZAngleAtMinDist) == 0x2c);
TPW_CHECK(BalanceTrackAsPylonControl_mfEntranceMaxXZAngleAtMinDist, TPW_OFF(BalanceTrackAsPylonControl, mfEntranceMaxXZAngleAtMinDist) == 0x30);
TPW_CHECK(BalanceTrackAsPylonControl_mfEntranceMinXZAngleAtMaxDist, TPW_OFF(BalanceTrackAsPylonControl, mfEntranceMinXZAngleAtMaxDist) == 0x34);
TPW_CHECK(BalanceTrackAsPylonControl_mfEntranceMaxXZAngleAtMaxDist, TPW_OFF(BalanceTrackAsPylonControl, mfEntranceMaxXZAngleAtMaxDist) == 0x38);
TPW_CHECK(BalanceTrackAsPylonControl_mfMinXZAngleAtMinDist, TPW_OFF(BalanceTrackAsPylonControl, mfMinXZAngleAtMinDist) == 0x3c);
TPW_CHECK(BalanceTrackAsPylonControl_mfMaxXZAngleAtMinDist, TPW_OFF(BalanceTrackAsPylonControl, mfMaxXZAngleAtMinDist) == 0x40);
TPW_CHECK(BalanceTrackAsPylonControl_mfMinXZAngleAtMaxDist, TPW_OFF(BalanceTrackAsPylonControl, mfMinXZAngleAtMaxDist) == 0x44);
TPW_CHECK(BalanceTrackAsPylonControl_mfMaxXZAngleAtMaxDist, TPW_OFF(BalanceTrackAsPylonControl, mfMaxXZAngleAtMaxDist) == 0x48);
TPW_CHECK(BalanceTrackAsPylonControl_mfMaxRotnAngleDeviance, TPW_OFF(BalanceTrackAsPylonControl, mfMaxRotnAngleDeviance) == 0x4c);
TPW_CHECK(BalanceTrackAsPylonControl_mfMaxLoopRotnAngleDeviance, TPW_OFF(BalanceTrackAsPylonControl, mfMaxLoopRotnAngleDeviance) == 0x50);
TPW_CHECK(BalanceTrackAsPylonControl_muiMinPlaceDist, TPW_OFF(BalanceTrackAsPylonControl, muiMinPlaceDist) == 0x54);
TPW_CHECK(BalanceTrackAsPylonControl_muiMaxPlaceDist, TPW_OFF(BalanceTrackAsPylonControl, muiMaxPlaceDist) == 0x58);
TPW_CHECK(BalanceTrackAsPylonControl_mfMaxLoopHeightDifference, TPW_OFF(BalanceTrackAsPylonControl, mfMaxLoopHeightDifference) == 0x5c);
TPW_CHECK(BalanceTrackAsPylonControl_mfCostPerUnit, TPW_OFF(BalanceTrackAsPylonControl, mfCostPerUnit) == 0x60);
TPW_CHECK(BalanceTrackAsPylonControl_mfCostPer2DLength, TPW_OFF(BalanceTrackAsPylonControl, mfCostPer2DLength) == 0x64);
TPW_CHECK(BalanceTrackAsPylonControl_mfCostPerHeight, TPW_OFF(BalanceTrackAsPylonControl, mfCostPerHeight) == 0x68);
TPW_CHECK(BalanceTrackAsPylonControl_mfLoftAnimStartCap, TPW_OFF(BalanceTrackAsPylonControl, mfLoftAnimStartCap) == 0x6c);
TPW_CHECK(BalanceTrackAsPylonControl_mfLoftAnimEndCap, TPW_OFF(BalanceTrackAsPylonControl, mfLoftAnimEndCap) == 0x70);
TPW_CHECK(BalanceTrackAsPylonControl_mfLoftAnimMultiplier, TPW_OFF(BalanceTrackAsPylonControl, mfLoftAnimMultiplier) == 0x74);
TPW_CHECK(BalanceTrackAsPylonControl_mfLoftAnimAdd, TPW_OFF(BalanceTrackAsPylonControl, mfLoftAnimAdd) == 0x78);
TPW_CHECK(BalanceTrackAsPylonControl_mfRotateAnimStartCap, TPW_OFF(BalanceTrackAsPylonControl, mfRotateAnimStartCap) == 0x7c);
TPW_CHECK(BalanceTrackAsPylonControl_mfRotateAnimEndCap, TPW_OFF(BalanceTrackAsPylonControl, mfRotateAnimEndCap) == 0x80);
TPW_CHECK(BalanceTrackAsPylonControl_mfRotateAnimMultiplier, TPW_OFF(BalanceTrackAsPylonControl, mfRotateAnimMultiplier) == 0x84);
TPW_CHECK(BalanceTrackAsPylonControl_mfRotateAnimAdd, TPW_OFF(BalanceTrackAsPylonControl, mfRotateAnimAdd) == 0x88);
TPW_CHECK(BalanceTrackAsPylonControl_mfInclineAnimStartCap, TPW_OFF(BalanceTrackAsPylonControl, mfInclineAnimStartCap) == 0x8c);
TPW_CHECK(BalanceTrackAsPylonControl_mfInclineAnimEndCap, TPW_OFF(BalanceTrackAsPylonControl, mfInclineAnimEndCap) == 0x90);
TPW_CHECK(BalanceTrackAsPylonControl_mfInclineAnimMultiplier, TPW_OFF(BalanceTrackAsPylonControl, mfInclineAnimMultiplier) == 0x94);
TPW_CHECK(BalanceTrackAsPylonControl_mfInclineAnimAdd, TPW_OFF(BalanceTrackAsPylonControl, mfInclineAnimAdd) == 0x98);
TPW_CHECK(BalanceTrackAsPylonControl_mfBankAnimStartCap, TPW_OFF(BalanceTrackAsPylonControl, mfBankAnimStartCap) == 0x9c);
TPW_CHECK(BalanceTrackAsPylonControl_mfBankAnimEndCap, TPW_OFF(BalanceTrackAsPylonControl, mfBankAnimEndCap) == 0xa0);
TPW_CHECK(BalanceTrackAsPylonControl_mfBankAnimMultiplier, TPW_OFF(BalanceTrackAsPylonControl, mfBankAnimMultiplier) == 0xa4);
TPW_CHECK(BalanceTrackAsPylonControl_mfBankAnimAdd, TPW_OFF(BalanceTrackAsPylonControl, mfBankAnimAdd) == 0xa8);
TPW_CHECK(BalanceTrackAsPylonControl_mfLoopRotateAnimStartCap, TPW_OFF(BalanceTrackAsPylonControl, mfLoopRotateAnimStartCap) == 0xac);
TPW_CHECK(BalanceTrackAsPylonControl_mfLoopRotateAnimEndCap, TPW_OFF(BalanceTrackAsPylonControl, mfLoopRotateAnimEndCap) == 0xb0);
TPW_CHECK(BalanceTrackAsPylonControl_mfLoopRotateAnimMultiplier, TPW_OFF(BalanceTrackAsPylonControl, mfLoopRotateAnimMultiplier) == 0xb4);
TPW_CHECK(BalanceTrackAsPylonControl_mfLoopRotateAnimAdd, TPW_OFF(BalanceTrackAsPylonControl, mfLoopRotateAnimAdd) == 0xb8);
TPW_CHECK(BalanceTrackAsPylonControl_size, sizeof(BalanceTrackAsPylonControl) == 0xbc);
TPW_CHECK(BalanceTrackAsCarType_mbBendCar, TPW_OFF(BalanceTrackAsCarType, mbBendCar) == 0x0);
TPW_CHECK(BalanceTrackAsCarType_muiCarTypeUniqueID, TPW_OFF(BalanceTrackAsCarType, muiCarTypeUniqueID) == 0x4);
TPW_CHECK(BalanceTrackAsCarType_mpcMeshFilename, TPW_OFF(BalanceTrackAsCarType, mpcMeshFilename) == 0x8);
TPW_CHECK(BalanceTrackAsCarType_mfFrontDistanceAdd, TPW_OFF(BalanceTrackAsCarType, mfFrontDistanceAdd) == 0xc);
TPW_CHECK(BalanceTrackAsCarType_mfRearDistanceAdd, TPW_OFF(BalanceTrackAsCarType, mfRearDistanceAdd) == 0x10);
TPW_CHECK(BalanceTrackAsCarType_mfYOffset, TPW_OFF(BalanceTrackAsCarType, mfYOffset) == 0x14);
TPW_CHECK(BalanceTrackAsCarType_mfAnimSpeedMultiplier, TPW_OFF(BalanceTrackAsCarType, mfAnimSpeedMultiplier) == 0x18);
TPW_CHECK(BalanceTrackAsCarType_size, sizeof(BalanceTrackAsCarType) == 0x1c);
TPW_CHECK(BalanceTrackAsTrackCollisionPoint_mfCollisionX, TPW_OFF(BalanceTrackAsTrackCollisionPoint, mfCollisionX) == 0x0);
TPW_CHECK(BalanceTrackAsTrackCollisionPoint_mfCollisionY, TPW_OFF(BalanceTrackAsTrackCollisionPoint, mfCollisionY) == 0x4);
TPW_CHECK(BalanceTrackAsTrackCollisionPoint_size, sizeof(BalanceTrackAsTrackCollisionPoint) == 0x8);
TPW_CHECK(BalanceTrack_vtable, TPW_OFF(BalanceTrack, vtable) == 0x0);
TPW_CHECK(BalanceTrack_mSlotInfo, TPW_OFF(BalanceTrack, mSlotInfo) == 0x4);
TPW_CHECK(BalanceTrack_masTextureData, TPW_OFF(BalanceTrack, masTextureData) == 0xc);
TPW_CHECK(BalanceTrack_masCrossSectionPoints1, TPW_OFF(BalanceTrack, masCrossSectionPoints1) == 0x100);
TPW_CHECK(BalanceTrack_masCrossSectionPoints2, TPW_OFF(BalanceTrack, masCrossSectionPoints2) == 0x294);
TPW_CHECK(BalanceTrack_masCrossSectionPoints3, TPW_OFF(BalanceTrack, masCrossSectionPoints3) == 0x428);
TPW_CHECK(BalanceTrack_masCrossSectionPoints4, TPW_OFF(BalanceTrack, masCrossSectionPoints4) == 0x5bc);
TPW_CHECK(BalanceTrack_masCrossSectionPoints5, TPW_OFF(BalanceTrack, masCrossSectionPoints5) == 0x750);
TPW_CHECK(BalanceTrack_masCrossSectionPoints6, TPW_OFF(BalanceTrack, masCrossSectionPoints6) == 0x8e4);
TPW_CHECK(BalanceTrack_masCrossSectionPoints7, TPW_OFF(BalanceTrack, masCrossSectionPoints7) == 0xa78);
TPW_CHECK(BalanceTrack_masCrossSectionPoints8, TPW_OFF(BalanceTrack, masCrossSectionPoints8) == 0xc0c);
TPW_CHECK(BalanceTrack_masCrossSectionPoints9, TPW_OFF(BalanceTrack, masCrossSectionPoints9) == 0xda0);
TPW_CHECK(BalanceTrack_masCrossSectionPoints10, TPW_OFF(BalanceTrack, masCrossSectionPoints10) == 0xf34);
TPW_CHECK(BalanceTrack_masCrossSectionPoints11, TPW_OFF(BalanceTrack, masCrossSectionPoints11) == 0x10c8);
TPW_CHECK(BalanceTrack_masCrossSectionPoints12, TPW_OFF(BalanceTrack, masCrossSectionPoints12) == 0x125c);
TPW_CHECK(BalanceTrack_masCrossSectionPoints13, TPW_OFF(BalanceTrack, masCrossSectionPoints13) == 0x13f0);
TPW_CHECK(BalanceTrack_masCrossSectionPoints14, TPW_OFF(BalanceTrack, masCrossSectionPoints14) == 0x1584);
TPW_CHECK(BalanceTrack_masCrossSectionPoints15, TPW_OFF(BalanceTrack, masCrossSectionPoints15) == 0x1718);
TPW_CHECK(BalanceTrack_masCrossSectionPoints16, TPW_OFF(BalanceTrack, masCrossSectionPoints16) == 0x18ac);
TPW_CHECK(BalanceTrack_masCrossSectionPoints17, TPW_OFF(BalanceTrack, masCrossSectionPoints17) == 0x1a40);
TPW_CHECK(BalanceTrack_masCrossSectionPoints18, TPW_OFF(BalanceTrack, masCrossSectionPoints18) == 0x1bd4);
TPW_CHECK(BalanceTrack_masCrossSectionPoints19, TPW_OFF(BalanceTrack, masCrossSectionPoints19) == 0x1d68);
TPW_CHECK(BalanceTrack_masCrossSectionPoints20, TPW_OFF(BalanceTrack, masCrossSectionPoints20) == 0x1efc);
TPW_CHECK(BalanceTrack_masCrossSectionEdges1, TPW_OFF(BalanceTrack, masCrossSectionEdges1) == 0x2090);
TPW_CHECK(BalanceTrack_masCrossSectionEdges2, TPW_OFF(BalanceTrack, masCrossSectionEdges2) == 0x2184);
TPW_CHECK(BalanceTrack_masCrossSectionEdges3, TPW_OFF(BalanceTrack, masCrossSectionEdges3) == 0x2278);
TPW_CHECK(BalanceTrack_masCrossSectionEdges4, TPW_OFF(BalanceTrack, masCrossSectionEdges4) == 0x236c);
TPW_CHECK(BalanceTrack_masCrossSectionEdges5, TPW_OFF(BalanceTrack, masCrossSectionEdges5) == 0x2460);
TPW_CHECK(BalanceTrack_masCrossSectionEdges6, TPW_OFF(BalanceTrack, masCrossSectionEdges6) == 0x2554);
TPW_CHECK(BalanceTrack_masCrossSectionEdges7, TPW_OFF(BalanceTrack, masCrossSectionEdges7) == 0x2648);
TPW_CHECK(BalanceTrack_masCrossSectionEdges8, TPW_OFF(BalanceTrack, masCrossSectionEdges8) == 0x273c);
TPW_CHECK(BalanceTrack_masCrossSectionEdges9, TPW_OFF(BalanceTrack, masCrossSectionEdges9) == 0x2830);
TPW_CHECK(BalanceTrack_masCrossSectionEdges10, TPW_OFF(BalanceTrack, masCrossSectionEdges10) == 0x2924);
TPW_CHECK(BalanceTrack_masCrossSectionEdges11, TPW_OFF(BalanceTrack, masCrossSectionEdges11) == 0x2a18);
TPW_CHECK(BalanceTrack_masCrossSectionEdges12, TPW_OFF(BalanceTrack, masCrossSectionEdges12) == 0x2b0c);
TPW_CHECK(BalanceTrack_masCrossSectionEdges13, TPW_OFF(BalanceTrack, masCrossSectionEdges13) == 0x2c00);
TPW_CHECK(BalanceTrack_masCrossSectionEdges14, TPW_OFF(BalanceTrack, masCrossSectionEdges14) == 0x2cf4);
TPW_CHECK(BalanceTrack_masCrossSectionEdges15, TPW_OFF(BalanceTrack, masCrossSectionEdges15) == 0x2de8);
TPW_CHECK(BalanceTrack_masCrossSectionEdges16, TPW_OFF(BalanceTrack, masCrossSectionEdges16) == 0x2edc);
TPW_CHECK(BalanceTrack_masCrossSectionEdges17, TPW_OFF(BalanceTrack, masCrossSectionEdges17) == 0x2fd0);
TPW_CHECK(BalanceTrack_masCrossSectionEdges18, TPW_OFF(BalanceTrack, masCrossSectionEdges18) == 0x30c4);
TPW_CHECK(BalanceTrack_masCrossSectionEdges19, TPW_OFF(BalanceTrack, masCrossSectionEdges19) == 0x31b8);
TPW_CHECK(BalanceTrack_masCrossSectionEdges20, TPW_OFF(BalanceTrack, masCrossSectionEdges20) == 0x32ac);
TPW_CHECK(BalanceTrack_masCrossSections, TPW_OFF(BalanceTrack, masCrossSections) == 0x33a0);
TPW_CHECK(BalanceTrack_masCrossSectionSelects, TPW_OFF(BalanceTrack, masCrossSectionSelects) == 0x3534);
TPW_CHECK(BalanceTrack_masPylonControls, TPW_OFF(BalanceTrack, masPylonControls) == 0x38a8);
TPW_CHECK(BalanceTrack_masCarTypes, TPW_OFF(BalanceTrack, masCarTypes) == 0x3a24);
TPW_CHECK(BalanceTrack_muiMaxCars, TPW_OFF(BalanceTrack, muiMaxCars) == 0x3a7c);
TPW_CHECK(BalanceTrack_muiFrontCarTypeIndex, TPW_OFF(BalanceTrack, muiFrontCarTypeIndex) == 0x3a80);
TPW_CHECK(BalanceTrack_muiCentreCarTypeIndex, TPW_OFF(BalanceTrack, muiCentreCarTypeIndex) == 0x3a84);
TPW_CHECK(BalanceTrack_muiRearCarTypeIndex, TPW_OFF(BalanceTrack, muiRearCarTypeIndex) == 0x3a88);
TPW_CHECK(BalanceTrack_mfFrontCarSpacing, TPW_OFF(BalanceTrack, mfFrontCarSpacing) == 0x3a8c);
TPW_CHECK(BalanceTrack_mfStdCarSpacing, TPW_OFF(BalanceTrack, mfStdCarSpacing) == 0x3a90);
TPW_CHECK(BalanceTrack_mfEndCarSpacing, TPW_OFF(BalanceTrack, mfEndCarSpacing) == 0x3a94);
TPW_CHECK(BalanceTrack_masTrackCollisionPoints, TPW_OFF(BalanceTrack, masTrackCollisionPoints) == 0x3a98);
TPW_CHECK(BalanceTrack_mfCurviness, TPW_OFF(BalanceTrack, mfCurviness) == 0x3acc);
TPW_CHECK(BalanceTrack_mfStraightDivisor, TPW_OFF(BalanceTrack, mfStraightDivisor) == 0x3ad0);
TPW_CHECK(BalanceTrack_mfAutoBankDivisor, TPW_OFF(BalanceTrack, mfAutoBankDivisor) == 0x3ad4);
TPW_CHECK(BalanceTrack_mfAccelerationPerHeight, TPW_OFF(BalanceTrack, mfAccelerationPerHeight) == 0x3ad8);
TPW_CHECK(BalanceTrack_mfUphillAccelModifier, TPW_OFF(BalanceTrack, mfUphillAccelModifier) == 0x3adc);
TPW_CHECK(BalanceTrack_mfDownhillAccelModifier, TPW_OFF(BalanceTrack, mfDownhillAccelModifier) == 0x3ae0);
TPW_CHECK(BalanceTrack_mfInitialPylonLoftHeight, TPW_OFF(BalanceTrack, mfInitialPylonLoftHeight) == 0x3ae4);
TPW_CHECK(BalanceTrack_muiMaxTrains, TPW_OFF(BalanceTrack, muiMaxTrains) == 0x3ae8);
TPW_CHECK(BalanceTrack_muiTrainReleaseRate, TPW_OFF(BalanceTrack, muiTrainReleaseRate) == 0x3aec);
TPW_CHECK(BalanceTrack_mbSpecialIsTrough, TPW_OFF(BalanceTrack, mbSpecialIsTrough) == 0x3af0);
TPW_CHECK(BalanceTrack_mbIsSuspendedCoaster, TPW_OFF(BalanceTrack, mbIsSuspendedCoaster) == 0x3af4);
TPW_CHECK(BalanceTrack_mbNoAutoBanking, TPW_OFF(BalanceTrack, mbNoAutoBanking) == 0x3af8);
TPW_CHECK(BalanceTrack_mbVisibleEndPylon, TPW_OFF(BalanceTrack, mbVisibleEndPylon) == 0x3afc);
TPW_CHECK(BalanceTrack_muiInvalidCrossSection, TPW_OFF(BalanceTrack, muiInvalidCrossSection) == 0x3b00);
TPW_CHECK(BalanceTrack_muiWinchCrossSection, TPW_OFF(BalanceTrack, muiWinchCrossSection) == 0x3b04);
TPW_CHECK(BalanceTrack_mfTrackHeightAdjust, TPW_OFF(BalanceTrack, mfTrackHeightAdjust) == 0x3b08);
TPW_CHECK(BalanceTrack_mfWinchSpeed, TPW_OFF(BalanceTrack, mfWinchSpeed) == 0x3b0c);
TPW_CHECK(BalanceTrack_mfMinSpeed, TPW_OFF(BalanceTrack, mfMinSpeed) == 0x3b10);
TPW_CHECK(BalanceTrack_mfMaxSpeedAtMinSetting, TPW_OFF(BalanceTrack, mfMaxSpeedAtMinSetting) == 0x3b14);
TPW_CHECK(BalanceTrack_mfMaxSpeedAtMaxSetting, TPW_OFF(BalanceTrack, mfMaxSpeedAtMaxSetting) == 0x3b18);
TPW_CHECK(BalanceTrack_mfFrictionMultiplier, TPW_OFF(BalanceTrack, mfFrictionMultiplier) == 0x3b1c);
TPW_CHECK(BalanceTrack_mfBendElevationLow, TPW_OFF(BalanceTrack, mfBendElevationLow) == 0x3b20);
TPW_CHECK(BalanceTrack_mfBendElevationHigh, TPW_OFF(BalanceTrack, mfBendElevationHigh) == 0x3b24);
TPW_CHECK(BalanceTrack_mfGravityX, TPW_OFF(BalanceTrack, mfGravityX) == 0x3b28);
TPW_CHECK(BalanceTrack_mfGravityY, TPW_OFF(BalanceTrack, mfGravityY) == 0x3b2c);
TPW_CHECK(BalanceTrack_mfGravityZ, TPW_OFF(BalanceTrack, mfGravityZ) == 0x3b30);
TPW_CHECK(BalanceTrack_mfForceMultiplier, TPW_OFF(BalanceTrack, mfForceMultiplier) == 0x3b34);
TPW_CHECK(BalanceTrack_mfInitialSickness, TPW_OFF(BalanceTrack, mfInitialSickness) == 0x3b38);
TPW_CHECK(BalanceTrack_mfInitialExcitement, TPW_OFF(BalanceTrack, mfInitialExcitement) == 0x3b3c);
TPW_CHECK(BalanceTrack_mfInitialUncomfortableness, TPW_OFF(BalanceTrack, mfInitialUncomfortableness) == 0x3b40);
TPW_CHECK(BalanceTrack_mfGForceSicknessModifier, TPW_OFF(BalanceTrack, mfGForceSicknessModifier) == 0x3b44);
TPW_CHECK(BalanceTrack_mfUpsideDownSicknessModifier, TPW_OFF(BalanceTrack, mfUpsideDownSicknessModifier) == 0x3b48);
TPW_CHECK(BalanceTrack_mfSpeedExcitementModifier, TPW_OFF(BalanceTrack, mfSpeedExcitementModifier) == 0x3b4c);
TPW_CHECK(BalanceTrack_mfGForceExcitementModifier, TPW_OFF(BalanceTrack, mfGForceExcitementModifier) == 0x3b50);
TPW_CHECK(BalanceTrack_mfUpsideDownExcitementModifier, TPW_OFF(BalanceTrack, mfUpsideDownExcitementModifier) == 0x3b54);
TPW_CHECK(BalanceTrack_mfBankExcitementModifier, TPW_OFF(BalanceTrack, mfBankExcitementModifier) == 0x3b58);
TPW_CHECK(BalanceTrack_mfXGForceUncomfortableModifier, TPW_OFF(BalanceTrack, mfXGForceUncomfortableModifier) == 0x3b5c);
TPW_CHECK(BalanceTrack_mfInvertedUncomfortableModifier, TPW_OFF(BalanceTrack, mfInvertedUncomfortableModifier) == 0x3b60);
TPW_CHECK(BalanceTrack_mfNormalScreamStartExciteDelta, TPW_OFF(BalanceTrack, mfNormalScreamStartExciteDelta) == 0x3b64);
TPW_CHECK(BalanceTrack_mfTerrorScreamStartExciteDelta, TPW_OFF(BalanceTrack, mfTerrorScreamStartExciteDelta) == 0x3b68);
TPW_CHECK(BalanceTrack_mfNormalScreamSustainExcitement, TPW_OFF(BalanceTrack, mfNormalScreamSustainExcitement) == 0x3b6c);
TPW_CHECK(BalanceTrack_mfTerrorScreamSustainExcitement, TPW_OFF(BalanceTrack, mfTerrorScreamSustainExcitement) == 0x3b70);
TPW_CHECK(BalanceTrack_mfEntranceSizeInterval, TPW_OFF(BalanceTrack, mfEntranceSizeInterval) == 0x3b74);
TPW_CHECK(BalanceTrack_miTrackSectionMeshVerts, TPW_OFF(BalanceTrack, miTrackSectionMeshVerts) == 0x3b78);
TPW_CHECK(BalanceTrack_miTrackSectionMeshFaces, TPW_OFF(BalanceTrack, miTrackSectionMeshFaces) == 0x3b7c);
TPW_CHECK(BalanceTrack_size, sizeof(BalanceTrack) == 0x3b80);
TPW_CHECK(BalanceChallengesChallenge_mType, TPW_OFF(BalanceChallengesChallenge, mType) == 0x0);
TPW_CHECK(BalanceChallengesChallenge_mFollowupType, TPW_OFF(BalanceChallengesChallenge, mFollowupType) == 0x4);
TPW_CHECK(BalanceChallengesChallenge_mTargetTime, TPW_OFF(BalanceChallengesChallenge, mTargetTime) == 0x8);
TPW_CHECK(BalanceChallengesChallenge_mTargetVal, TPW_OFF(BalanceChallengesChallenge, mTargetVal) == 0xc);
TPW_CHECK(BalanceChallengesChallenge_mTargetObj, TPW_OFF(BalanceChallengesChallenge, mTargetObj) == 0x10);
TPW_CHECK(BalanceChallengesChallenge_mTargetObj2, TPW_OFF(BalanceChallengesChallenge, mTargetObj2) == 0x14);
TPW_CHECK(BalanceChallengesChallenge_mTargetStaffType, TPW_OFF(BalanceChallengesChallenge, mTargetStaffType) == 0x18);
TPW_CHECK(BalanceChallengesChallenge_mPrize, TPW_OFF(BalanceChallengesChallenge, mPrize) == 0x1c);
TPW_CHECK(BalanceChallengesChallenge_mCheckAtEndOnly, TPW_OFF(BalanceChallengesChallenge, mCheckAtEndOnly) == 0x20);
TPW_CHECK(BalanceChallengesChallenge_mIndependent, TPW_OFF(BalanceChallengesChallenge, mIndependent) == 0x24);
TPW_CHECK(BalanceChallengesChallenge_size, sizeof(BalanceChallengesChallenge) == 0x28);
TPW_CHECK(BalanceChallenges_vtable, TPW_OFF(BalanceChallenges, vtable) == 0x0);
TPW_CHECK(BalanceChallenges_mSlotInfo, TPW_OFF(BalanceChallenges, mSlotInfo) == 0x4);
TPW_CHECK(BalanceChallenges_mChallenges, TPW_OFF(BalanceChallenges, mChallenges) == 0xc);
TPW_CHECK(BalanceChallenges_size, sizeof(BalanceChallenges) == 0x970);
TPW_CHECK(BalanceTheme_vtable, TPW_OFF(BalanceTheme, vtable) == 0x0);
TPW_CHECK(BalanceTheme_mSlotInfo, TPW_OFF(BalanceTheme, mSlotInfo) == 0x4);
TPW_CHECK(BalanceTheme_mCanEarnTicketsInTheme, TPW_OFF(BalanceTheme, mCanEarnTicketsInTheme) == 0xc);
TPW_CHECK(BalanceTheme_mCanSpendTicketsInTheme, TPW_OFF(BalanceTheme, mCanSpendTicketsInTheme) == 0x10);
TPW_CHECK(BalanceTheme_mCostToEnter, TPW_OFF(BalanceTheme, mCostToEnter) == 0x14);
TPW_CHECK(BalanceTheme_mGateObjectId, TPW_OFF(BalanceTheme, mGateObjectId) == 0x18);
TPW_CHECK(BalanceTheme_size, sizeof(BalanceTheme) == 0x1c);
TPW_CHECK(AdvisorScoringMessageGroup_mMinTimeSameMessage, TPW_OFF(AdvisorScoringMessageGroup, mMinTimeSameMessage) == 0x0);
TPW_CHECK(AdvisorScoringMessageGroup_mSayOnlyOnce, TPW_OFF(AdvisorScoringMessageGroup, mSayOnlyOnce) == 0x4);
TPW_CHECK(AdvisorScoringMessageGroup_mDiscardAfterSlaps, TPW_OFF(AdvisorScoringMessageGroup, mDiscardAfterSlaps) == 0x8);
TPW_CHECK(AdvisorScoringMessageGroup_size, sizeof(AdvisorScoringMessageGroup) == 0xc);
TPW_CHECK(AdvisorScoring_vtable, TPW_OFF(AdvisorScoring, vtable) == 0xc);
TPW_CHECK(AdvisorScoring_mSlotInfo, TPW_OFF(AdvisorScoring, mSlotInfo) == 0x10);
TPW_CHECK(AdvisorScoring_mMinTimeAnyMessage, TPW_OFF(AdvisorScoring, mMinTimeAnyMessage) == 0x18);
TPW_CHECK(AdvisorScoring_mMinTimeSameMessage, TPW_OFF(AdvisorScoring, mMinTimeSameMessage) == 0x1c);
TPW_CHECK(AdvisorScoring_mMinScoreForConsideration, TPW_OFF(AdvisorScoring, mMinScoreForConsideration) == 0x20);
TPW_CHECK(AdvisorScoring_mMessageGroups, TPW_OFF(AdvisorScoring, mMessageGroups) == 0x24);
TPW_CHECK(AdvisorScoring_mWelcomeScore, TPW_OFF(AdvisorScoring, mWelcomeScore) == 0xa0);
TPW_CHECK(AdvisorScoring_mScorePerWaitingPerson, TPW_OFF(AdvisorScoring, mScorePerWaitingPerson) == 0xa4);
TPW_CHECK(AdvisorScoring_mCloseParkScore, TPW_OFF(AdvisorScoring, mCloseParkScore) == 0xa8);
TPW_CHECK(AdvisorScoring_mScorePerThirstyPerson, TPW_OFF(AdvisorScoring, mScorePerThirstyPerson) == 0xac);
TPW_CHECK(AdvisorScoring_mThirstierThan, TPW_OFF(AdvisorScoring, mThirstierThan) == 0xb0);
TPW_CHECK(AdvisorScoring_mScorePerHungryPerson, TPW_OFF(AdvisorScoring, mScorePerHungryPerson) == 0xb4);
TPW_CHECK(AdvisorScoring_mHungrierThan, TPW_OFF(AdvisorScoring, mHungrierThan) == 0xb8);
TPW_CHECK(AdvisorScoring_mScorePerWornRide, TPW_OFF(AdvisorScoring, mScorePerWornRide) == 0xbc);
TPW_CHECK(AdvisorScoring_mPoorerStateThan, TPW_OFF(AdvisorScoring, mPoorerStateThan) == 0xc0);
TPW_CHECK(AdvisorScoring_mScorePerBrokenRide, TPW_OFF(AdvisorScoring, mScorePerBrokenRide) == 0xc4);
TPW_CHECK(AdvisorScoring_mIdealNumberOfRidesPerMechanic, TPW_OFF(AdvisorScoring, mIdealNumberOfRidesPerMechanic) == 0xc8);
TPW_CHECK(AdvisorScoring_mScorePerExtraRide, TPW_OFF(AdvisorScoring, mScorePerExtraRide) == 0xcc);
TPW_CHECK(AdvisorScoring_mStaffHireHandymen1ScorePerLitterPoint, TPW_OFF(AdvisorScoring, mStaffHireHandymen1ScorePerLitterPoint) == 0xd0);
TPW_CHECK(AdvisorScoring_mStaffHireHandymen1LitterPointCutoffLow, TPW_OFF(AdvisorScoring, mStaffHireHandymen1LitterPointCutoffLow) == 0xd4);
TPW_CHECK(AdvisorScoring_mLitterPointCutoffHigh, TPW_OFF(AdvisorScoring, mLitterPointCutoffHigh) == 0xd8);
TPW_CHECK(AdvisorScoring_mStaffHireHandymen2ScorePerLitterPoint, TPW_OFF(AdvisorScoring, mStaffHireHandymen2ScorePerLitterPoint) == 0xdc);
TPW_CHECK(AdvisorScoring_mStaffHireHandymen2LitterPointCutoffLow, TPW_OFF(AdvisorScoring, mStaffHireHandymen2LitterPointCutoffLow) == 0xe0);
TPW_CHECK(AdvisorScoring_mIdealDevelopedCellsPerHandyman, TPW_OFF(AdvisorScoring, mIdealDevelopedCellsPerHandyman) == 0xe4);
TPW_CHECK(AdvisorScoring_mStaffHireHandymen3ScorePerExtra10Cells, TPW_OFF(AdvisorScoring, mStaffHireHandymen3ScorePerExtra10Cells) == 0xe8);
TPW_CHECK(AdvisorScoring_mStaffHireHandymen3MinCellsForConsideration, TPW_OFF(AdvisorScoring, mStaffHireHandymen3MinCellsForConsideration) == 0xec);
TPW_CHECK(AdvisorScoring_mStaffHireGuards1AtLeastThisBusy, TPW_OFF(AdvisorScoring, mStaffHireGuards1AtLeastThisBusy) == 0xf0);
TPW_CHECK(AdvisorScoring_mStaffHireGuards1ScorePerExtraBusyPoint, TPW_OFF(AdvisorScoring, mStaffHireGuards1ScorePerExtraBusyPoint) == 0xf4);
TPW_CHECK(AdvisorScoring_mStaffHireGuards2Score, TPW_OFF(AdvisorScoring, mStaffHireGuards2Score) == 0xf8);
TPW_CHECK(AdvisorScoring_mAdditionalPerMisbehavingKid, TPW_OFF(AdvisorScoring, mAdditionalPerMisbehavingKid) == 0xfc);
TPW_CHECK(AdvisorScoring_mIdealDevelopedCellsPerGuard, TPW_OFF(AdvisorScoring, mIdealDevelopedCellsPerGuard) == 0x100);
TPW_CHECK(AdvisorScoring_mStaffHireGuards3ScorePerExtra10Cells, TPW_OFF(AdvisorScoring, mStaffHireGuards3ScorePerExtra10Cells) == 0x104);
TPW_CHECK(AdvisorScoring_mStaffHireGuards3MinCellsForConsideration, TPW_OFF(AdvisorScoring, mStaffHireGuards3MinCellsForConsideration) == 0x108);
TPW_CHECK(AdvisorScoring_mStaffHireEntertainers1AtLeastThisBusy, TPW_OFF(AdvisorScoring, mStaffHireEntertainers1AtLeastThisBusy) == 0x10c);
TPW_CHECK(AdvisorScoring_mStaffHireEntertainers1ScorePerExtraBusyPoint, TPW_OFF(AdvisorScoring, mStaffHireEntertainers1ScorePerExtraBusyPoint) == 0x110);
TPW_CHECK(AdvisorScoring_mMost, TPW_OFF(AdvisorScoring, mMost) == 0x114);
TPW_CHECK(AdvisorScoring_mPointsPerExtraPct, TPW_OFF(AdvisorScoring, mPointsPerExtraPct) == 0x118);
TPW_CHECK(AdvisorScoring_mStaffHireResearchers2Score, TPW_OFF(AdvisorScoring, mStaffHireResearchers2Score) == 0x11c);
TPW_CHECK(AdvisorScoring_mStaffHireResearchers3MonthsIntoGameForConsideration, TPW_OFF(AdvisorScoring, mStaffHireResearchers3MonthsIntoGameForConsideration) == 0x120);
TPW_CHECK(AdvisorScoring_mSlowerThan, TPW_OFF(AdvisorScoring, mSlowerThan) == 0x124);
TPW_CHECK(AdvisorScoring_mStaffHireResearchers3Score, TPW_OFF(AdvisorScoring, mStaffHireResearchers3Score) == 0x128);
TPW_CHECK(AdvisorScoring_mStaffSetMechanicAreaAtLeastThisMany, TPW_OFF(AdvisorScoring, mStaffSetMechanicAreaAtLeastThisMany) == 0x12c);
TPW_CHECK(AdvisorScoring_mStaffSetMechanicAreaDontBotherIfPatrolledUp, TPW_OFF(AdvisorScoring, mStaffSetMechanicAreaDontBotherIfPatrolledUp) == 0x130);
TPW_CHECK(AdvisorScoring_mStaffSetMechanicAreaScorePerUnpatrolledPct, TPW_OFF(AdvisorScoring, mStaffSetMechanicAreaScorePerUnpatrolledPct) == 0x134);
TPW_CHECK(AdvisorScoring_mStaffSetMechanicAreaMonthsIntoGameForConsideration, TPW_OFF(AdvisorScoring, mStaffSetMechanicAreaMonthsIntoGameForConsideration) == 0x138);
TPW_CHECK(AdvisorScoring_mStaffMechanicCoverageScore, TPW_OFF(AdvisorScoring, mStaffMechanicCoverageScore) == 0x13c);
TPW_CHECK(AdvisorScoring_mStaffMechanicCoverageScorePerNonCoveredPercent, TPW_OFF(AdvisorScoring, mStaffMechanicCoverageScorePerNonCoveredPercent) == 0x140);
TPW_CHECK(AdvisorScoring_mStaffSetHandymanAreaAtLeastThisMany, TPW_OFF(AdvisorScoring, mStaffSetHandymanAreaAtLeastThisMany) == 0x144);
TPW_CHECK(AdvisorScoring_mStaffSetHandymanAreaDontBotherIfPatrolledUp, TPW_OFF(AdvisorScoring, mStaffSetHandymanAreaDontBotherIfPatrolledUp) == 0x148);
TPW_CHECK(AdvisorScoring_mStaffSetHandymanAreaScorePerUnpatrolledPct, TPW_OFF(AdvisorScoring, mStaffSetHandymanAreaScorePerUnpatrolledPct) == 0x14c);
TPW_CHECK(AdvisorScoring_mStaffSetHandymanAreaMonthsIntoGameForConsideration, TPW_OFF(AdvisorScoring, mStaffSetHandymanAreaMonthsIntoGameForConsideration) == 0x150);
TPW_CHECK(AdvisorScoring_mStaffHandymanCoverageScore, TPW_OFF(AdvisorScoring, mStaffHandymanCoverageScore) == 0x154);
TPW_CHECK(AdvisorScoring_mStaffHandymanCoverageScorePerNonCoveredPercent, TPW_OFF(AdvisorScoring, mStaffHandymanCoverageScorePerNonCoveredPercent) == 0x158);
TPW_CHECK(AdvisorScoring_mAcceptableTrouble, TPW_OFF(AdvisorScoring, mAcceptableTrouble) == 0x15c);
TPW_CHECK(AdvisorScoring_mStaffGuardBuildCameraScore, TPW_OFF(AdvisorScoring, mStaffGuardBuildCameraScore) == 0x160);
TPW_CHECK(AdvisorScoring_mStaffSetGuardAreaAtLeastThisMany, TPW_OFF(AdvisorScoring, mStaffSetGuardAreaAtLeastThisMany) == 0x164);
TPW_CHECK(AdvisorScoring_mStaffSetGuardAreaDontBotherIfPatrolledUp, TPW_OFF(AdvisorScoring, mStaffSetGuardAreaDontBotherIfPatrolledUp) == 0x168);
TPW_CHECK(AdvisorScoring_mStaffSetGuardAreaScorePerUnpatrolledPct, TPW_OFF(AdvisorScoring, mStaffSetGuardAreaScorePerUnpatrolledPct) == 0x16c);
TPW_CHECK(AdvisorScoring_mStaffSetGuardAreaMonthsIntoGameForConsideration, TPW_OFF(AdvisorScoring, mStaffSetGuardAreaMonthsIntoGameForConsideration) == 0x170);
TPW_CHECK(AdvisorScoring_mStaffGuardCoverageScore, TPW_OFF(AdvisorScoring, mStaffGuardCoverageScore) == 0x174);
TPW_CHECK(AdvisorScoring_mStaffGuardCoverageScorePerNonCoveredPercent, TPW_OFF(AdvisorScoring, mStaffGuardCoverageScorePerNonCoveredPercent) == 0x178);
TPW_CHECK(AdvisorScoring_mStaffSetEntertainerAreaAtLeastThisMany, TPW_OFF(AdvisorScoring, mStaffSetEntertainerAreaAtLeastThisMany) == 0x17c);
TPW_CHECK(AdvisorScoring_mStaffSetEntertainerAreaDontBotherIfPatrolledUp, TPW_OFF(AdvisorScoring, mStaffSetEntertainerAreaDontBotherIfPatrolledUp) == 0x180);
TPW_CHECK(AdvisorScoring_mStaffSetEntertainerAreaScorePerUnpatrolledPct, TPW_OFF(AdvisorScoring, mStaffSetEntertainerAreaScorePerUnpatrolledPct) == 0x184);
TPW_CHECK(AdvisorScoring_mStaffSetEntertainerAreaMonthsIntoGameForConsideration, TPW_OFF(AdvisorScoring, mStaffSetEntertainerAreaMonthsIntoGameForConsideration) == 0x188);
TPW_CHECK(AdvisorScoring_mStaffEntertainerCoverageScore, TPW_OFF(AdvisorScoring, mStaffEntertainerCoverageScore) == 0x18c);
TPW_CHECK(AdvisorScoring_mStaffEntertainerCoverageScorePerNonCoveredPercent, TPW_OFF(AdvisorScoring, mStaffEntertainerCoverageScorePerNonCoveredPercent) == 0x190);
TPW_CHECK(AdvisorScoring_mStaffTrainMechanicsWorseThan, TPW_OFF(AdvisorScoring, mStaffTrainMechanicsWorseThan) == 0x194);
TPW_CHECK(AdvisorScoring_mStaffTrainMechanicsScore, TPW_OFF(AdvisorScoring, mStaffTrainMechanicsScore) == 0x198);
TPW_CHECK(AdvisorScoring_mStaffTrainHandymenWorseThan, TPW_OFF(AdvisorScoring, mStaffTrainHandymenWorseThan) == 0x19c);
TPW_CHECK(AdvisorScoring_mStaffTrainHandymenScore, TPW_OFF(AdvisorScoring, mStaffTrainHandymenScore) == 0x1a0);
TPW_CHECK(AdvisorScoring_mStaffTrainGuardsWorseThan, TPW_OFF(AdvisorScoring, mStaffTrainGuardsWorseThan) == 0x1a4);
TPW_CHECK(AdvisorScoring_mStaffTrainGuardsScore, TPW_OFF(AdvisorScoring, mStaffTrainGuardsScore) == 0x1a8);
TPW_CHECK(AdvisorScoring_mStaffTrainEntertainersWorseThan, TPW_OFF(AdvisorScoring, mStaffTrainEntertainersWorseThan) == 0x1ac);
TPW_CHECK(AdvisorScoring_mStaffTrainEntertainersScore, TPW_OFF(AdvisorScoring, mStaffTrainEntertainersScore) == 0x1b0);
TPW_CHECK(AdvisorScoring_mStaffTrainResearchersWorseThan, TPW_OFF(AdvisorScoring, mStaffTrainResearchersWorseThan) == 0x1b4);
TPW_CHECK(AdvisorScoring_mStaffTrainResearchersScore, TPW_OFF(AdvisorScoring, mStaffTrainResearchersScore) == 0x1b8);
TPW_CHECK(AdvisorScoring_mStaffUnhappyMechanicsScore, TPW_OFF(AdvisorScoring, mStaffUnhappyMechanicsScore) == 0x1bc);
TPW_CHECK(AdvisorScoring_mStaffUnhappyHandymenScore, TPW_OFF(AdvisorScoring, mStaffUnhappyHandymenScore) == 0x1c0);
TPW_CHECK(AdvisorScoring_mStaffUnhappyGuardsScore, TPW_OFF(AdvisorScoring, mStaffUnhappyGuardsScore) == 0x1c4);
TPW_CHECK(AdvisorScoring_mStaffUnhappyEntertainersScore, TPW_OFF(AdvisorScoring, mStaffUnhappyEntertainersScore) == 0x1c8);
TPW_CHECK(AdvisorScoring_mStaffUnhappyResearchersScore, TPW_OFF(AdvisorScoring, mStaffUnhappyResearchersScore) == 0x1cc);
TPW_CHECK(AdvisorScoring_mStaffVeryUnhappyMechanicsScore, TPW_OFF(AdvisorScoring, mStaffVeryUnhappyMechanicsScore) == 0x1d0);
TPW_CHECK(AdvisorScoring_mStaffVeryUnhappyHandymenScore, TPW_OFF(AdvisorScoring, mStaffVeryUnhappyHandymenScore) == 0x1d4);
TPW_CHECK(AdvisorScoring_mStaffVeryUnhappyGuardsScore, TPW_OFF(AdvisorScoring, mStaffVeryUnhappyGuardsScore) == 0x1d8);
TPW_CHECK(AdvisorScoring_mStaffVeryUnhappyEntertainersScore, TPW_OFF(AdvisorScoring, mStaffVeryUnhappyEntertainersScore) == 0x1dc);
TPW_CHECK(AdvisorScoring_mStaffVeryUnhappyResearchersScore, TPW_OFF(AdvisorScoring, mStaffVeryUnhappyResearchersScore) == 0x1e0);
TPW_CHECK(AdvisorScoring_mStaffStrikingMechanicsScore, TPW_OFF(AdvisorScoring, mStaffStrikingMechanicsScore) == 0x1e4);
TPW_CHECK(AdvisorScoring_mStaffStrikingHandymenScore, TPW_OFF(AdvisorScoring, mStaffStrikingHandymenScore) == 0x1e8);
TPW_CHECK(AdvisorScoring_mStaffStrikingGuardsScore, TPW_OFF(AdvisorScoring, mStaffStrikingGuardsScore) == 0x1ec);
TPW_CHECK(AdvisorScoring_mStaffStrikingEntertainersScore, TPW_OFF(AdvisorScoring, mStaffStrikingEntertainersScore) == 0x1f0);
TPW_CHECK(AdvisorScoring_mStaffStrikingResearchersScore, TPW_OFF(AdvisorScoring, mStaffStrikingResearchersScore) == 0x1f4);
TPW_CHECK(AdvisorScoring_mStaffHappierMechanicsScore, TPW_OFF(AdvisorScoring, mStaffHappierMechanicsScore) == 0x1f8);
TPW_CHECK(AdvisorScoring_mStaffHappierHandymenScore, TPW_OFF(AdvisorScoring, mStaffHappierHandymenScore) == 0x1fc);
TPW_CHECK(AdvisorScoring_mStaffHappierGuardsScore, TPW_OFF(AdvisorScoring, mStaffHappierGuardsScore) == 0x200);
TPW_CHECK(AdvisorScoring_mStaffHappierEntertainersScore, TPW_OFF(AdvisorScoring, mStaffHappierEntertainersScore) == 0x204);
TPW_CHECK(AdvisorScoring_mStaffHappierResearchersScore, TPW_OFF(AdvisorScoring, mStaffHappierResearchersScore) == 0x208);
TPW_CHECK(AdvisorScoring_mStrikeEndBadMechanicsScore, TPW_OFF(AdvisorScoring, mStrikeEndBadMechanicsScore) == 0x20c);
TPW_CHECK(AdvisorScoring_mStrikeEndBadHandymenScore, TPW_OFF(AdvisorScoring, mStrikeEndBadHandymenScore) == 0x210);
TPW_CHECK(AdvisorScoring_mStrikeEndBadGuardsScore, TPW_OFF(AdvisorScoring, mStrikeEndBadGuardsScore) == 0x214);
TPW_CHECK(AdvisorScoring_mStrikeEndBadEntertainersScore, TPW_OFF(AdvisorScoring, mStrikeEndBadEntertainersScore) == 0x218);
TPW_CHECK(AdvisorScoring_mStrikeEndBadResearchersScore, TPW_OFF(AdvisorScoring, mStrikeEndBadResearchersScore) == 0x21c);
TPW_CHECK(AdvisorScoring_mRidesPoorSelectionPoorerThan, TPW_OFF(AdvisorScoring, mRidesPoorSelectionPoorerThan) == 0x220);
TPW_CHECK(AdvisorScoring_mMoreRidesThan, TPW_OFF(AdvisorScoring, mMoreRidesThan) == 0x224);
TPW_CHECK(AdvisorScoring_mRidesPoorSelectionScore, TPW_OFF(AdvisorScoring, mRidesPoorSelectionScore) == 0x228);
TPW_CHECK(AdvisorScoring_mRidesResearchMoreScore, TPW_OFF(AdvisorScoring, mRidesResearchMoreScore) == 0x22c);
TPW_CHECK(AdvisorScoring_mRidesBuildRidesScore, TPW_OFF(AdvisorScoring, mRidesBuildRidesScore) == 0x230);
TPW_CHECK(AdvisorScoring_mRidesBuildRidesAdditionalPerDay, TPW_OFF(AdvisorScoring, mRidesBuildRidesAdditionalPerDay) == 0x234);
TPW_CHECK(AdvisorScoring_mRidesNoUpgradesMonthsIntoGameForConsideration, TPW_OFF(AdvisorScoring, mRidesNoUpgradesMonthsIntoGameForConsideration) == 0x238);
TPW_CHECK(AdvisorScoring_mRidesNoUpgradesScore, TPW_OFF(AdvisorScoring, mRidesNoUpgradesScore) == 0x23c);
TPW_CHECK(AdvisorScoring_mLessAvailableThan, TPW_OFF(AdvisorScoring, mLessAvailableThan) == 0x240);
TPW_CHECK(AdvisorScoring_mRidesMoreUpgradesScore, TPW_OFF(AdvisorScoring, mRidesMoreUpgradesScore) == 0x244);
TPW_CHECK(AdvisorScoring_mRidesBreakdownImminentScore, TPW_OFF(AdvisorScoring, mRidesBreakdownImminentScore) == 0x248);
TPW_CHECK(AdvisorScoring_mRidesBreakdownNoMechanicsScore, TPW_OFF(AdvisorScoring, mRidesBreakdownNoMechanicsScore) == 0x24c);
TPW_CHECK(AdvisorScoring_mRidesBreakdownBusyMechanicsScore, TPW_OFF(AdvisorScoring, mRidesBreakdownBusyMechanicsScore) == 0x250);
TPW_CHECK(AdvisorScoring_mRidesBreakdownMechanicOnItScore, TPW_OFF(AdvisorScoring, mRidesBreakdownMechanicOnItScore) == 0x254);
TPW_CHECK(AdvisorScoring_mShopsPoorSelectionPoorerThan, TPW_OFF(AdvisorScoring, mShopsPoorSelectionPoorerThan) == 0x258);
TPW_CHECK(AdvisorScoring_mMoreShopsThan, TPW_OFF(AdvisorScoring, mMoreShopsThan) == 0x25c);
TPW_CHECK(AdvisorScoring_mShopsPoorSelectionScore, TPW_OFF(AdvisorScoring, mShopsPoorSelectionScore) == 0x260);
TPW_CHECK(AdvisorScoring_mShopsResearchMoreScore, TPW_OFF(AdvisorScoring, mShopsResearchMoreScore) == 0x264);
TPW_CHECK(AdvisorScoring_mShopsBuildShopsScore, TPW_OFF(AdvisorScoring, mShopsBuildShopsScore) == 0x268);
TPW_CHECK(AdvisorScoring_mShopsBuildShopsAdditionalPerDay, TPW_OFF(AdvisorScoring, mShopsBuildShopsAdditionalPerDay) == 0x26c);
TPW_CHECK(AdvisorScoring_mShopsPerRideRatio, TPW_OFF(AdvisorScoring, mShopsPerRideRatio) == 0x270);
TPW_CHECK(AdvisorScoring_mShopsBuildMoreShopsWealthMoreThan, TPW_OFF(AdvisorScoring, mShopsBuildMoreShopsWealthMoreThan) == 0x274);
TPW_CHECK(AdvisorScoring_mShopsBuildMoreShopsScore, TPW_OFF(AdvisorScoring, mShopsBuildMoreShopsScore) == 0x278);
TPW_CHECK(AdvisorScoring_mSideshowsPoorSelectionPoorerThan, TPW_OFF(AdvisorScoring, mSideshowsPoorSelectionPoorerThan) == 0x27c);
TPW_CHECK(AdvisorScoring_mMoreSideshowsThan, TPW_OFF(AdvisorScoring, mMoreSideshowsThan) == 0x280);
TPW_CHECK(AdvisorScoring_mSideshowsPoorSelectionScore, TPW_OFF(AdvisorScoring, mSideshowsPoorSelectionScore) == 0x284);
TPW_CHECK(AdvisorScoring_mSideshowsResearchMoreScore, TPW_OFF(AdvisorScoring, mSideshowsResearchMoreScore) == 0x288);
TPW_CHECK(AdvisorScoring_mSideshowsBuildSideshowsScore, TPW_OFF(AdvisorScoring, mSideshowsBuildSideshowsScore) == 0x28c);
TPW_CHECK(AdvisorScoring_mSideshowsBuildSideshowsAdditionalPerDay, TPW_OFF(AdvisorScoring, mSideshowsBuildSideshowsAdditionalPerDay) == 0x290);
TPW_CHECK(AdvisorScoring_mSideshowsPerRideRatio, TPW_OFF(AdvisorScoring, mSideshowsPerRideRatio) == 0x294);
TPW_CHECK(AdvisorScoring_mSideshowsBuildMoreSideshowsWealthMoreThan, TPW_OFF(AdvisorScoring, mSideshowsBuildMoreSideshowsWealthMoreThan) == 0x298);
TPW_CHECK(AdvisorScoring_mSideshowsBuildMoreSideshowsScore, TPW_OFF(AdvisorScoring, mSideshowsBuildMoreSideshowsScore) == 0x29c);
TPW_CHECK(AdvisorScoring_mFeaturesPoorSelectionPoorerThan, TPW_OFF(AdvisorScoring, mFeaturesPoorSelectionPoorerThan) == 0x2a0);
TPW_CHECK(AdvisorScoring_mMoreFeaturesThan, TPW_OFF(AdvisorScoring, mMoreFeaturesThan) == 0x2a4);
TPW_CHECK(AdvisorScoring_mFeaturesPoorSelectionScore, TPW_OFF(AdvisorScoring, mFeaturesPoorSelectionScore) == 0x2a8);
TPW_CHECK(AdvisorScoring_mFeaturesResearchMoreScore, TPW_OFF(AdvisorScoring, mFeaturesResearchMoreScore) == 0x2ac);
TPW_CHECK(AdvisorScoring_mFeaturesBuildFeaturesScore, TPW_OFF(AdvisorScoring, mFeaturesBuildFeaturesScore) == 0x2b0);
TPW_CHECK(AdvisorScoring_mFeaturesBuildFeaturesAdditionalPerDay, TPW_OFF(AdvisorScoring, mFeaturesBuildFeaturesAdditionalPerDay) == 0x2b4);
TPW_CHECK(AdvisorScoring_mNeedingMoreThan, TPW_OFF(AdvisorScoring, mNeedingMoreThan) == 0x2b8);
TPW_CHECK(AdvisorScoring_mMorePeopleThanThisNeeding, TPW_OFF(AdvisorScoring, mMorePeopleThanThisNeeding) == 0x2bc);
TPW_CHECK(AdvisorScoring_mFeaturesNoToiletsScore, TPW_OFF(AdvisorScoring, mFeaturesNoToiletsScore) == 0x2c0);
TPW_CHECK(AdvisorScoring_mFeaturesFarToToiletsScore, TPW_OFF(AdvisorScoring, mFeaturesFarToToiletsScore) == 0x2c4);
TPW_CHECK(AdvisorScoring_mAverageQueueLongerThan, TPW_OFF(AdvisorScoring, mAverageQueueLongerThan) == 0x2c8);
TPW_CHECK(AdvisorScoring_mFeaturesNotEnoughToiletsScore, TPW_OFF(AdvisorScoring, mFeaturesNotEnoughToiletsScore) == 0x2cc);
TPW_CHECK(AdvisorScoring_mFeaturesDirtyToilets1FilthierThan, TPW_OFF(AdvisorScoring, mFeaturesDirtyToilets1FilthierThan) == 0x2d0);
TPW_CHECK(AdvisorScoring_mFeaturesDirtyToilets1PointsPerFilthPct, TPW_OFF(AdvisorScoring, mFeaturesDirtyToilets1PointsPerFilthPct) == 0x2d4);
TPW_CHECK(AdvisorScoring_mFeaturesDirtyToilets2FilthierThan, TPW_OFF(AdvisorScoring, mFeaturesDirtyToilets2FilthierThan) == 0x2d8);
TPW_CHECK(AdvisorScoring_mFeaturesDirtyToilets2PointsPerFilthPct, TPW_OFF(AdvisorScoring, mFeaturesDirtyToilets2PointsPerFilthPct) == 0x2dc);
TPW_CHECK(AdvisorScoring_mFeaturesDirtyToilets3FilthierThan, TPW_OFF(AdvisorScoring, mFeaturesDirtyToilets3FilthierThan) == 0x2e0);
TPW_CHECK(AdvisorScoring_mFeaturesDirtyToilets3PointsPerFilthPct, TPW_OFF(AdvisorScoring, mFeaturesDirtyToilets3PointsPerFilthPct) == 0x2e4);
TPW_CHECK(AdvisorScoring_mFeaturesDirtyToilets4FilthierThan, TPW_OFF(AdvisorScoring, mFeaturesDirtyToilets4FilthierThan) == 0x2e8);
TPW_CHECK(AdvisorScoring_mFeaturesDirtyToilets4PointsPerFilthPct, TPW_OFF(AdvisorScoring, mFeaturesDirtyToilets4PointsPerFilthPct) == 0x2ec);
TPW_CHECK(AdvisorScoring_mTicketSatisfactionUnder, TPW_OFF(AdvisorScoring, mTicketSatisfactionUnder) == 0x2f0);
TPW_CHECK(AdvisorScoring_mPricingTicketTooExpensiveScore, TPW_OFF(AdvisorScoring, mPricingTicketTooExpensiveScore) == 0x2f4);
TPW_CHECK(AdvisorScoring_mTicketSatisfactionOver, TPW_OFF(AdvisorScoring, mTicketSatisfactionOver) == 0x2f8);
TPW_CHECK(AdvisorScoring_mPricingTicketTooCheapScore, TPW_OFF(AdvisorScoring, mPricingTicketTooCheapScore) == 0x2fc);
TPW_CHECK(AdvisorScoring_mResearchRideResearchedScore, TPW_OFF(AdvisorScoring, mResearchRideResearchedScore) == 0x300);
TPW_CHECK(AdvisorScoring_mResearchAddonResearchedScore, TPW_OFF(AdvisorScoring, mResearchAddonResearchedScore) == 0x304);
TPW_CHECK(AdvisorScoring_mResearchShopResearchedScore, TPW_OFF(AdvisorScoring, mResearchShopResearchedScore) == 0x308);
TPW_CHECK(AdvisorScoring_mResearchSideshowResearchedScore, TPW_OFF(AdvisorScoring, mResearchSideshowResearchedScore) == 0x30c);
TPW_CHECK(AdvisorScoring_mResearchFeatureResearchedScore, TPW_OFF(AdvisorScoring, mResearchFeatureResearchedScore) == 0x310);
TPW_CHECK(AdvisorScoring_mMoreThanThisGotAway, TPW_OFF(AdvisorScoring, mMoreThanThisGotAway) == 0x314);
TPW_CHECK(AdvisorScoring_mScorePerGetAway, TPW_OFF(AdvisorScoring, mScorePerGetAway) == 0x318);
TPW_CHECK(AdvisorScoring_mGeneralWonGoldenTicketScore, TPW_OFF(AdvisorScoring, mGeneralWonGoldenTicketScore) == 0x31c);
TPW_CHECK(AdvisorScoring_mAwardGivenScore, TPW_OFF(AdvisorScoring, mAwardGivenScore) == 0x320);
TPW_CHECK(AdvisorScoring_mAwardMovedScore, TPW_OFF(AdvisorScoring, mAwardMovedScore) == 0x324);
TPW_CHECK(AdvisorScoring_mHelpMessageScore, TPW_OFF(AdvisorScoring, mHelpMessageScore) == 0x328);
TPW_CHECK(AdvisorScoring_mContextHelpScore, TPW_OFF(AdvisorScoring, mContextHelpScore) == 0x32c);
TPW_CHECK(AdvisorScoring_mMoreMoneyThan, TPW_OFF(AdvisorScoring, mMoreMoneyThan) == 0x330);
TPW_CHECK(AdvisorScoring_mWealthHighScore, TPW_OFF(AdvisorScoring, mWealthHighScore) == 0x334);
TPW_CHECK(AdvisorScoring_mMargin, TPW_OFF(AdvisorScoring, mMargin) == 0x338);
TPW_CHECK(AdvisorScoring_mWealthHighRepayLoanScore, TPW_OFF(AdvisorScoring, mWealthHighRepayLoanScore) == 0x33c);
TPW_CHECK(AdvisorScoring_mTakeLoanScore, TPW_OFF(AdvisorScoring, mTakeLoanScore) == 0x340);
TPW_CHECK(AdvisorScoring_mInTheRedSixMonthsScore, TPW_OFF(AdvisorScoring, mInTheRedSixMonthsScore) == 0x344);
TPW_CHECK(AdvisorScoring_mInTheRedThreeMonthsScore, TPW_OFF(AdvisorScoring, mInTheRedThreeMonthsScore) == 0x348);
TPW_CHECK(AdvisorScoring_mInTheRedMonthLeftScore, TPW_OFF(AdvisorScoring, mInTheRedMonthLeftScore) == 0x34c);
TPW_CHECK(AdvisorScoring_mBankruptedScore, TPW_OFF(AdvisorScoring, mBankruptedScore) == 0x350);
TPW_CHECK(AdvisorScoring_mWagesTimesHigherThanIncome, TPW_OFF(AdvisorScoring, mWagesTimesHigherThanIncome) == 0x354);
TPW_CHECK(AdvisorScoring_mAverageOverMonths, TPW_OFF(AdvisorScoring, mAverageOverMonths) == 0x358);
TPW_CHECK(AdvisorScoring_mWagesHighScore, TPW_OFF(AdvisorScoring, mWagesHighScore) == 0x35c);
TPW_CHECK(AdvisorScoring_mLotsHandymanFudgeFactor, TPW_OFF(AdvisorScoring, mLotsHandymanFudgeFactor) == 0x360);
TPW_CHECK(AdvisorScoring_mLotsHandymanScore, TPW_OFF(AdvisorScoring, mLotsHandymanScore) == 0x364);
TPW_CHECK(AdvisorScoring_mLotsEntertainersFudgeFactor, TPW_OFF(AdvisorScoring, mLotsEntertainersFudgeFactor) == 0x368);
TPW_CHECK(AdvisorScoring_mLotsEntertainersScore, TPW_OFF(AdvisorScoring, mLotsEntertainersScore) == 0x36c);
TPW_CHECK(AdvisorScoring_mLotsMechanicsFudgeFactor, TPW_OFF(AdvisorScoring, mLotsMechanicsFudgeFactor) == 0x370);
TPW_CHECK(AdvisorScoring_mLotsMechanicsScore, TPW_OFF(AdvisorScoring, mLotsMechanicsScore) == 0x374);
TPW_CHECK(AdvisorScoring_mLotsSecurityFudgeFactor, TPW_OFF(AdvisorScoring, mLotsSecurityFudgeFactor) == 0x378);
TPW_CHECK(AdvisorScoring_mLotsSecurityScore, TPW_OFF(AdvisorScoring, mLotsSecurityScore) == 0x37c);
TPW_CHECK(AdvisorScoring_mLotsResearchersFudgeFactor, TPW_OFF(AdvisorScoring, mLotsResearchersFudgeFactor) == 0x380);
TPW_CHECK(AdvisorScoring_mLotsResearchersScore, TPW_OFF(AdvisorScoring, mLotsResearchersScore) == 0x384);
TPW_CHECK(AdvisorScoring_mQueueUnconnectedScore, TPW_OFF(AdvisorScoring, mQueueUnconnectedScore) == 0x388);
TPW_CHECK(AdvisorScoring_mMaxHandymenMoreThan, TPW_OFF(AdvisorScoring, mMaxHandymenMoreThan) == 0x38c);
TPW_CHECK(AdvisorScoring_mMaxHandymenScore, TPW_OFF(AdvisorScoring, mMaxHandymenScore) == 0x390);
TPW_CHECK(AdvisorScoring_mMaxHandymenV2Score, TPW_OFF(AdvisorScoring, mMaxHandymenV2Score) == 0x394);
TPW_CHECK(AdvisorScoring_mMaxEntertainersMoreThan, TPW_OFF(AdvisorScoring, mMaxEntertainersMoreThan) == 0x398);
TPW_CHECK(AdvisorScoring_mMaxEntertainersScore, TPW_OFF(AdvisorScoring, mMaxEntertainersScore) == 0x39c);
TPW_CHECK(AdvisorScoring_mMaxEntertainersV2Score, TPW_OFF(AdvisorScoring, mMaxEntertainersV2Score) == 0x3a0);
TPW_CHECK(AdvisorScoring_mMaxMechanicsMoreThan, TPW_OFF(AdvisorScoring, mMaxMechanicsMoreThan) == 0x3a4);
TPW_CHECK(AdvisorScoring_mMaxMechanicsScore, TPW_OFF(AdvisorScoring, mMaxMechanicsScore) == 0x3a8);
TPW_CHECK(AdvisorScoring_mMaxMechanicsV2Score, TPW_OFF(AdvisorScoring, mMaxMechanicsV2Score) == 0x3ac);
TPW_CHECK(AdvisorScoring_mMaxGuardsMoreThan, TPW_OFF(AdvisorScoring, mMaxGuardsMoreThan) == 0x3b0);
TPW_CHECK(AdvisorScoring_mMaxGuardsScore, TPW_OFF(AdvisorScoring, mMaxGuardsScore) == 0x3b4);
TPW_CHECK(AdvisorScoring_mMaxGuardsV2Score, TPW_OFF(AdvisorScoring, mMaxGuardsV2Score) == 0x3b8);
TPW_CHECK(AdvisorScoring_mMaxResearchersMoreThan, TPW_OFF(AdvisorScoring, mMaxResearchersMoreThan) == 0x3bc);
TPW_CHECK(AdvisorScoring_mMaxResearchersScore, TPW_OFF(AdvisorScoring, mMaxResearchersScore) == 0x3c0);
TPW_CHECK(AdvisorScoring_mMaxResearchersV2Score, TPW_OFF(AdvisorScoring, mMaxResearchersV2Score) == 0x3c4);
TPW_CHECK(AdvisorScoring_mMaxMonths, TPW_OFF(AdvisorScoring, mMaxMonths) == 0x3c8);
TPW_CHECK(AdvisorScoring_mDroppedPercent, TPW_OFF(AdvisorScoring, mDroppedPercent) == 0x3cc);
TPW_CHECK(AdvisorScoring_mHypeNewparkScore, TPW_OFF(AdvisorScoring, mHypeNewparkScore) == 0x3d0);
TPW_CHECK(AdvisorScoring_mPrankSbombScore, TPW_OFF(AdvisorScoring, mPrankSbombScore) == 0x3d4);
TPW_CHECK(AdvisorScoring_mPrankLitterScore, TPW_OFF(AdvisorScoring, mPrankLitterScore) == 0x3d8);
TPW_CHECK(AdvisorScoring_mPrankBalloonScore, TPW_OFF(AdvisorScoring, mPrankBalloonScore) == 0x3dc);
TPW_CHECK(AdvisorScoring_mParkNowOpenScore, TPW_OFF(AdvisorScoring, mParkNowOpenScore) == 0x3e0);
TPW_CHECK(AdvisorScoring_mParkNowClosedScore, TPW_OFF(AdvisorScoring, mParkNowClosedScore) == 0x3e4);
TPW_CHECK(AdvisorScoring_mUpgradeHireMechanicScore, TPW_OFF(AdvisorScoring, mUpgradeHireMechanicScore) == 0x3e8);
TPW_CHECK(AdvisorScoring_mUpgradeStikingMechanicsScore, TPW_OFF(AdvisorScoring, mUpgradeStikingMechanicsScore) == 0x3ec);
TPW_CHECK(AdvisorScoring_mNewUpgradeNoMechanicScore, TPW_OFF(AdvisorScoring, mNewUpgradeNoMechanicScore) == 0x3f0);
TPW_CHECK(AdvisorScoring_mNewResearchGroupRideScore, TPW_OFF(AdvisorScoring, mNewResearchGroupRideScore) == 0x3f4);
TPW_CHECK(AdvisorScoring_mNewResearchGroupShopScore, TPW_OFF(AdvisorScoring, mNewResearchGroupShopScore) == 0x3f8);
TPW_CHECK(AdvisorScoring_mNewResearchGroupSideshowScore, TPW_OFF(AdvisorScoring, mNewResearchGroupSideshowScore) == 0x3fc);
TPW_CHECK(AdvisorScoring_mNewResearchGroupFeatureScore, TPW_OFF(AdvisorScoring, mNewResearchGroupFeatureScore) == 0x400);
TPW_CHECK(AdvisorScoring_mBusArrivedScore, TPW_OFF(AdvisorScoring, mBusArrivedScore) == 0x404);
TPW_CHECK(AdvisorScoring_mPlaneArrivedScore, TPW_OFF(AdvisorScoring, mPlaneArrivedScore) == 0x408);
TPW_CHECK(AdvisorScoring_mBoatArrivedScore, TPW_OFF(AdvisorScoring, mBoatArrivedScore) == 0x40c);
TPW_CHECK(AdvisorScoring_mHowBigIsBig, TPW_OFF(AdvisorScoring, mHowBigIsBig) == 0x410);
TPW_CHECK(AdvisorScoring_mBigBusloadArrivedScore, TPW_OFF(AdvisorScoring, mBigBusloadArrivedScore) == 0x414);
TPW_CHECK(AdvisorScoring_mAverageScarierThan, TPW_OFF(AdvisorScoring, mAverageScarierThan) == 0x418);
TPW_CHECK(AdvisorScoring_mMinimumRides, TPW_OFF(AdvisorScoring, mMinimumRides) == 0x41c);
TPW_CHECK(AdvisorScoring_mVisitorsTooScaredScore, TPW_OFF(AdvisorScoring, mVisitorsTooScaredScore) == 0x420);
TPW_CHECK(AdvisorScoring_mOnlineNewItemScore, TPW_OFF(AdvisorScoring, mOnlineNewItemScore) == 0x424);
TPW_CHECK(AdvisorScoring_mOnlineMessageScore, TPW_OFF(AdvisorScoring, mOnlineMessageScore) == 0x428);
TPW_CHECK(AdvisorScoring_mMinimumStaff, TPW_OFF(AdvisorScoring, mMinimumStaff) == 0x42c);
TPW_CHECK(AdvisorScoring_mCongratStaffHappyHappierThan, TPW_OFF(AdvisorScoring, mCongratStaffHappyHappierThan) == 0x430);
TPW_CHECK(AdvisorScoring_mCongratStaffHappyScore, TPW_OFF(AdvisorScoring, mCongratStaffHappyScore) == 0x434);
TPW_CHECK(AdvisorScoring_mMinimumVisitors, TPW_OFF(AdvisorScoring, mMinimumVisitors) == 0x438);
TPW_CHECK(AdvisorScoring_mCongratVisitorsHappyHappierThan, TPW_OFF(AdvisorScoring, mCongratVisitorsHappyHappierThan) == 0x43c);
TPW_CHECK(AdvisorScoring_mCongratVisitorsHappyScore, TPW_OFF(AdvisorScoring, mCongratVisitorsHappyScore) == 0x440);
TPW_CHECK(AdvisorScoring_mMinimumGuards, TPW_OFF(AdvisorScoring, mMinimumGuards) == 0x444);
TPW_CHECK(AdvisorScoring_mCongratSecurityTrainedBetterThan, TPW_OFF(AdvisorScoring, mCongratSecurityTrainedBetterThan) == 0x448);
TPW_CHECK(AdvisorScoring_mCongratSecurityTrainedScore, TPW_OFF(AdvisorScoring, mCongratSecurityTrainedScore) == 0x44c);
TPW_CHECK(AdvisorScoring_mMinimumResearchers, TPW_OFF(AdvisorScoring, mMinimumResearchers) == 0x450);
TPW_CHECK(AdvisorScoring_mCongratResearchersTrainedBetterThan, TPW_OFF(AdvisorScoring, mCongratResearchersTrainedBetterThan) == 0x454);
TPW_CHECK(AdvisorScoring_mCongratResearchersTrainedScore, TPW_OFF(AdvisorScoring, mCongratResearchersTrainedScore) == 0x458);
TPW_CHECK(AdvisorScoring_mMinimumHandymen, TPW_OFF(AdvisorScoring, mMinimumHandymen) == 0x45c);
TPW_CHECK(AdvisorScoring_mCongratHandymenTrainedBetterThan, TPW_OFF(AdvisorScoring, mCongratHandymenTrainedBetterThan) == 0x460);
TPW_CHECK(AdvisorScoring_mCongratHandymenTrainedScore, TPW_OFF(AdvisorScoring, mCongratHandymenTrainedScore) == 0x464);
TPW_CHECK(AdvisorScoring_mMinimumEntertainers, TPW_OFF(AdvisorScoring, mMinimumEntertainers) == 0x468);
TPW_CHECK(AdvisorScoring_mCongratEntertainersTrainedBetterThan, TPW_OFF(AdvisorScoring, mCongratEntertainersTrainedBetterThan) == 0x46c);
TPW_CHECK(AdvisorScoring_mCongratEntertainersTrainedScore, TPW_OFF(AdvisorScoring, mCongratEntertainersTrainedScore) == 0x470);
TPW_CHECK(AdvisorScoring_mMinimumMechanics, TPW_OFF(AdvisorScoring, mMinimumMechanics) == 0x474);
TPW_CHECK(AdvisorScoring_mCongratMechanicsTrainedBetterThan, TPW_OFF(AdvisorScoring, mCongratMechanicsTrainedBetterThan) == 0x478);
TPW_CHECK(AdvisorScoring_mCongratMechanicsTrainedScore, TPW_OFF(AdvisorScoring, mCongratMechanicsTrainedScore) == 0x47c);
TPW_CHECK(AdvisorScoring_mCongratPublishParkValueBetterThan, TPW_OFF(AdvisorScoring, mCongratPublishParkValueBetterThan) == 0x480);
TPW_CHECK(AdvisorScoring_mCongratPublishParkHappinessGreaterThan, TPW_OFF(AdvisorScoring, mCongratPublishParkHappinessGreaterThan) == 0x484);
TPW_CHECK(AdvisorScoring_mCongratPublishParkVisitorsMoreThan, TPW_OFF(AdvisorScoring, mCongratPublishParkVisitorsMoreThan) == 0x488);
TPW_CHECK(AdvisorScoring_mCongratPublishParkScore, TPW_OFF(AdvisorScoring, mCongratPublishParkScore) == 0x48c);
TPW_CHECK(AdvisorScoring_mCongratSendPostcardValueBetterThan, TPW_OFF(AdvisorScoring, mCongratSendPostcardValueBetterThan) == 0x490);
TPW_CHECK(AdvisorScoring_mCongratSendPostcardHappinessGreaterThan, TPW_OFF(AdvisorScoring, mCongratSendPostcardHappinessGreaterThan) == 0x494);
TPW_CHECK(AdvisorScoring_mCongratSendPostcardVisitorsMoreThan, TPW_OFF(AdvisorScoring, mCongratSendPostcardVisitorsMoreThan) == 0x498);
TPW_CHECK(AdvisorScoring_mCongratSendPostcardScore, TPW_OFF(AdvisorScoring, mCongratSendPostcardScore) == 0x49c);
TPW_CHECK(AdvisorScoring_mCongratBalloonsMoreThan, TPW_OFF(AdvisorScoring, mCongratBalloonsMoreThan) == 0x4a0);
TPW_CHECK(AdvisorScoring_mCongratBalloonsScore, TPW_OFF(AdvisorScoring, mCongratBalloonsScore) == 0x4a4);
TPW_CHECK(AdvisorScoring_mCongratCostumesMoreThan, TPW_OFF(AdvisorScoring, mCongratCostumesMoreThan) == 0x4a8);
TPW_CHECK(AdvisorScoring_mCongratCostumesScore, TPW_OFF(AdvisorScoring, mCongratCostumesScore) == 0x4ac);
TPW_CHECK(AdvisorScoring_mCongratAllCompleteScore, TPW_OFF(AdvisorScoring, mCongratAllCompleteScore) == 0x4b0);
TPW_CHECK(AdvisorScoring_mCongratAllCompleteLeavingScore, TPW_OFF(AdvisorScoring, mCongratAllCompleteLeavingScore) == 0x4b4);
TPW_CHECK(AdvisorScoring_mPctOfStaffTired, TPW_OFF(AdvisorScoring, mPctOfStaffTired) == 0x4b8);
TPW_CHECK(AdvisorScoring_mNeedStaffRoomScore, TPW_OFF(AdvisorScoring, mNeedStaffRoomScore) == 0x4bc);
TPW_CHECK(AdvisorScoring_mHireStaffGeneralScore, TPW_OFF(AdvisorScoring, mHireStaffGeneralScore) == 0x4c0);
TPW_CHECK(AdvisorScoring_mMinVisitorsEver, TPW_OFF(AdvisorScoring, mMinVisitorsEver) == 0x4c4);
TPW_CHECK(AdvisorScoring_mSetPatrolsGeneralMinStaffForMessage, TPW_OFF(AdvisorScoring, mSetPatrolsGeneralMinStaffForMessage) == 0x4c8);
TPW_CHECK(AdvisorScoring_mSetPatrolsGeneralScore, TPW_OFF(AdvisorScoring, mSetPatrolsGeneralScore) == 0x4cc);
TPW_CHECK(AdvisorScoring_mTrainStaffGeneralMinStaffForMessage, TPW_OFF(AdvisorScoring, mTrainStaffGeneralMinStaffForMessage) == 0x4d0);
TPW_CHECK(AdvisorScoring_mTrainStaffGeneralScore, TPW_OFF(AdvisorScoring, mTrainStaffGeneralScore) == 0x4d4);
TPW_CHECK(AdvisorScoring_mFoodPoorValueValueWorseThan, TPW_OFF(AdvisorScoring, mFoodPoorValueValueWorseThan) == 0x4d8);
TPW_CHECK(AdvisorScoring_mFoodPoorValueScorePerPoint, TPW_OFF(AdvisorScoring, mFoodPoorValueScorePerPoint) == 0x4dc);
TPW_CHECK(AdvisorScoring_mShopsPoorValueValueWorseThan, TPW_OFF(AdvisorScoring, mShopsPoorValueValueWorseThan) == 0x4e0);
TPW_CHECK(AdvisorScoring_mShopsPoorValueScorePerPoint, TPW_OFF(AdvisorScoring, mShopsPoorValueScorePerPoint) == 0x4e4);
TPW_CHECK(AdvisorScoring_mRestaurantPoorValueValueWorseThan, TPW_OFF(AdvisorScoring, mRestaurantPoorValueValueWorseThan) == 0x4e8);
TPW_CHECK(AdvisorScoring_mRestaurantPoorValueScorePerPoint, TPW_OFF(AdvisorScoring, mRestaurantPoorValueScorePerPoint) == 0x4ec);
TPW_CHECK(AdvisorScoring_mDrinksPoorValueValueWorseThan, TPW_OFF(AdvisorScoring, mDrinksPoorValueValueWorseThan) == 0x4f0);
TPW_CHECK(AdvisorScoring_mDrinksPoorValueScorePerPoint, TPW_OFF(AdvisorScoring, mDrinksPoorValueScorePerPoint) == 0x4f4);
TPW_CHECK(AdvisorScoring_mSideshowsPoorValueValueWorseThan, TPW_OFF(AdvisorScoring, mSideshowsPoorValueValueWorseThan) == 0x4f8);
TPW_CHECK(AdvisorScoring_mSideshowsPoorValueScorePerPoint, TPW_OFF(AdvisorScoring, mSideshowsPoorValueScorePerPoint) == 0x4fc);
TPW_CHECK(AdvisorScoring_mFoodPoorSatisfactionSatisfactionWorseThan, TPW_OFF(AdvisorScoring, mFoodPoorSatisfactionSatisfactionWorseThan) == 0x500);
TPW_CHECK(AdvisorScoring_mFoodPoorSatisfactionScorePerPoint, TPW_OFF(AdvisorScoring, mFoodPoorSatisfactionScorePerPoint) == 0x504);
TPW_CHECK(AdvisorScoring_mShopsPoorSatisfactionSatisfactionWorseThan, TPW_OFF(AdvisorScoring, mShopsPoorSatisfactionSatisfactionWorseThan) == 0x508);
TPW_CHECK(AdvisorScoring_mShopsPoorSatisfactionScorePerPoint, TPW_OFF(AdvisorScoring, mShopsPoorSatisfactionScorePerPoint) == 0x50c);
TPW_CHECK(AdvisorScoring_mRestaurantPoorSatisfactionSatisfactionWorseThan, TPW_OFF(AdvisorScoring, mRestaurantPoorSatisfactionSatisfactionWorseThan) == 0x510);
TPW_CHECK(AdvisorScoring_mRestaurantPoorSatisfactionScorePerPoint, TPW_OFF(AdvisorScoring, mRestaurantPoorSatisfactionScorePerPoint) == 0x514);
TPW_CHECK(AdvisorScoring_mDrinksPoorSatisfactionSatisfactionWorseThan, TPW_OFF(AdvisorScoring, mDrinksPoorSatisfactionSatisfactionWorseThan) == 0x518);
TPW_CHECK(AdvisorScoring_mDrinksPoorSatisfactionScorePerPoint, TPW_OFF(AdvisorScoring, mDrinksPoorSatisfactionScorePerPoint) == 0x51c);
TPW_CHECK(AdvisorScoring_mSideshowsPoorSatisfactionSatisfactionWorseThan, TPW_OFF(AdvisorScoring, mSideshowsPoorSatisfactionSatisfactionWorseThan) == 0x520);
TPW_CHECK(AdvisorScoring_mSideshowsPoorSatisfactionScorePerPoint, TPW_OFF(AdvisorScoring, mSideshowsPoorSatisfactionScorePerPoint) == 0x524);
TPW_CHECK(AdvisorScoring_mFoodGreatValueValueBetterThan, TPW_OFF(AdvisorScoring, mFoodGreatValueValueBetterThan) == 0x528);
TPW_CHECK(AdvisorScoring_mFoodGreatValueScorePerPoint, TPW_OFF(AdvisorScoring, mFoodGreatValueScorePerPoint) == 0x52c);
TPW_CHECK(AdvisorScoring_mShopsGreatValueValueBetterThan, TPW_OFF(AdvisorScoring, mShopsGreatValueValueBetterThan) == 0x530);
TPW_CHECK(AdvisorScoring_mShopsGreatValueScorePerPoint, TPW_OFF(AdvisorScoring, mShopsGreatValueScorePerPoint) == 0x534);
TPW_CHECK(AdvisorScoring_mRestaurantGreatValueValueBetterThan, TPW_OFF(AdvisorScoring, mRestaurantGreatValueValueBetterThan) == 0x538);
TPW_CHECK(AdvisorScoring_mRestaurantGreatValueScorePerPoint, TPW_OFF(AdvisorScoring, mRestaurantGreatValueScorePerPoint) == 0x53c);
TPW_CHECK(AdvisorScoring_mDrinksGreatValueValueBetterThan, TPW_OFF(AdvisorScoring, mDrinksGreatValueValueBetterThan) == 0x540);
TPW_CHECK(AdvisorScoring_mDrinksGreatValueScorePerPoint, TPW_OFF(AdvisorScoring, mDrinksGreatValueScorePerPoint) == 0x544);
TPW_CHECK(AdvisorScoring_mSideshowsGreatValueValueBetterThan, TPW_OFF(AdvisorScoring, mSideshowsGreatValueValueBetterThan) == 0x548);
TPW_CHECK(AdvisorScoring_mSideshowsGreatValueScorePerPoint, TPW_OFF(AdvisorScoring, mSideshowsGreatValueScorePerPoint) == 0x54c);
TPW_CHECK(AdvisorScoring_mFoodGreatSatisfactionSatisfactionBetterThan, TPW_OFF(AdvisorScoring, mFoodGreatSatisfactionSatisfactionBetterThan) == 0x550);
TPW_CHECK(AdvisorScoring_mFoodGreatSatisfactionScorePerPoint, TPW_OFF(AdvisorScoring, mFoodGreatSatisfactionScorePerPoint) == 0x554);
TPW_CHECK(AdvisorScoring_mShopsGreatSatisfactionSatisfactionBetterThan, TPW_OFF(AdvisorScoring, mShopsGreatSatisfactionSatisfactionBetterThan) == 0x558);
TPW_CHECK(AdvisorScoring_mShopsGreatSatisfactionScorePerPoint, TPW_OFF(AdvisorScoring, mShopsGreatSatisfactionScorePerPoint) == 0x55c);
TPW_CHECK(AdvisorScoring_mRestaurantGreatSatisfactionSatisfactionBetterThan, TPW_OFF(AdvisorScoring, mRestaurantGreatSatisfactionSatisfactionBetterThan) == 0x560);
TPW_CHECK(AdvisorScoring_mRestaurantGreatSatisfactionScorePerPoint, TPW_OFF(AdvisorScoring, mRestaurantGreatSatisfactionScorePerPoint) == 0x564);
TPW_CHECK(AdvisorScoring_mDrinksGreatSatisfactionSatisfactionBetterThan, TPW_OFF(AdvisorScoring, mDrinksGreatSatisfactionSatisfactionBetterThan) == 0x568);
TPW_CHECK(AdvisorScoring_mDrinksGreatSatisfactionScorePerPoint, TPW_OFF(AdvisorScoring, mDrinksGreatSatisfactionScorePerPoint) == 0x56c);
TPW_CHECK(AdvisorScoring_mSideshowsGreatSatisfactionSatisfactionBetterThan, TPW_OFF(AdvisorScoring, mSideshowsGreatSatisfactionSatisfactionBetterThan) == 0x570);
TPW_CHECK(AdvisorScoring_mSideshowsGreatSatisfactionScorePerPoint, TPW_OFF(AdvisorScoring, mSideshowsGreatSatisfactionScorePerPoint) == 0x574);
TPW_CHECK(AdvisorScoring_mRideCondemnedScore, TPW_OFF(AdvisorScoring, mRideCondemnedScore) == 0x578);
TPW_CHECK(AdvisorScoring_mAtLeastThisManyRides, TPW_OFF(AdvisorScoring, mAtLeastThisManyRides) == 0x57c);
TPW_CHECK(AdvisorScoring_mRidesClosedMoreThanThisPercentClosed, TPW_OFF(AdvisorScoring, mRidesClosedMoreThanThisPercentClosed) == 0x580);
TPW_CHECK(AdvisorScoring_mRidesClosedScore, TPW_OFF(AdvisorScoring, mRidesClosedScore) == 0x584);
TPW_CHECK(AdvisorScoring_mShopsClosedMoreThanThisPercentClosed, TPW_OFF(AdvisorScoring, mShopsClosedMoreThanThisPercentClosed) == 0x588);
TPW_CHECK(AdvisorScoring_mShopsClosedScore, TPW_OFF(AdvisorScoring, mShopsClosedScore) == 0x58c);
TPW_CHECK(AdvisorScoring_mSideshowsClosedMoreThanThisPercentClosed, TPW_OFF(AdvisorScoring, mSideshowsClosedMoreThanThisPercentClosed) == 0x590);
TPW_CHECK(AdvisorScoring_mSideshowsClosedScore, TPW_OFF(AdvisorScoring, mSideshowsClosedScore) == 0x594);
TPW_CHECK(AdvisorScoring_mToiletsClosedMoreThanThisPercentClosed, TPW_OFF(AdvisorScoring, mToiletsClosedMoreThanThisPercentClosed) == 0x598);
TPW_CHECK(AdvisorScoring_mToiletsClosedScore, TPW_OFF(AdvisorScoring, mToiletsClosedScore) == 0x59c);
TPW_CHECK(AdvisorScoring_mStaffroomClosedMoreThanThisPercentClosed, TPW_OFF(AdvisorScoring, mStaffroomClosedMoreThanThisPercentClosed) == 0x5a0);
TPW_CHECK(AdvisorScoring_mStaffroomClosedScore, TPW_OFF(AdvisorScoring, mStaffroomClosedScore) == 0x5a4);
TPW_CHECK(AdvisorScoring_mMorePrankstersThan, TPW_OFF(AdvisorScoring, mMorePrankstersThan) == 0x5a8);
TPW_CHECK(AdvisorScoring_mLotsOfPranksScore, TPW_OFF(AdvisorScoring, mLotsOfPranksScore) == 0x5ac);
TPW_CHECK(AdvisorScoring_mRideExitBlockedScore, TPW_OFF(AdvisorScoring, mRideExitBlockedScore) == 0x5b0);
TPW_CHECK(AdvisorScoring_mWonFirstGoldenTicketScore, TPW_OFF(AdvisorScoring, mWonFirstGoldenTicketScore) == 0x5b4);
TPW_CHECK(AdvisorScoring_mGoldTicketNearToFirstOnePeopleMultiplier, TPW_OFF(AdvisorScoring, mGoldTicketNearToFirstOnePeopleMultiplier) == 0x5b8);
TPW_CHECK(AdvisorScoring_mGoldTicketNearToFirstOneScore, TPW_OFF(AdvisorScoring, mGoldTicketNearToFirstOneScore) == 0x5bc);
TPW_CHECK(AdvisorScoring_mGoldTicketCluesForRemainderScore, TPW_OFF(AdvisorScoring, mGoldTicketCluesForRemainderScore) == 0x5c0);
TPW_CHECK(AdvisorScoring_mGoldTicketNearToXPeepsPeopleMultiplier, TPW_OFF(AdvisorScoring, mGoldTicketNearToXPeepsPeopleMultiplier) == 0x5c4);
TPW_CHECK(AdvisorScoring_mGoldTicketNearToXPeepsScore, TPW_OFF(AdvisorScoring, mGoldTicketNearToXPeepsScore) == 0x5c8);
TPW_CHECK(AdvisorScoring_mHappinessMultiplier, TPW_OFF(AdvisorScoring, mHappinessMultiplier) == 0x5cc);
TPW_CHECK(AdvisorScoring_mGoldTicketNearToXHappinessScore, TPW_OFF(AdvisorScoring, mGoldTicketNearToXHappinessScore) == 0x5d0);
TPW_CHECK(AdvisorScoring_mProfitMultiplier, TPW_OFF(AdvisorScoring, mProfitMultiplier) == 0x5d4);
TPW_CHECK(AdvisorScoring_mGoldTicketNearToXProfitScore, TPW_OFF(AdvisorScoring, mGoldTicketNearToXProfitScore) == 0x5d8);
TPW_CHECK(AdvisorScoring_mTutorialUIPanelAfterTime, TPW_OFF(AdvisorScoring, mTutorialUIPanelAfterTime) == 0x5dc);
TPW_CHECK(AdvisorScoring_mTutorialUIPanelCutoffTime, TPW_OFF(AdvisorScoring, mTutorialUIPanelCutoffTime) == 0x5e0);
TPW_CHECK(AdvisorScoring_mTutorialUIPanelScore, TPW_OFF(AdvisorScoring, mTutorialUIPanelScore) == 0x5e4);
TPW_CHECK(AdvisorScoring_mTutorialExplainTooltipsAfterTime, TPW_OFF(AdvisorScoring, mTutorialExplainTooltipsAfterTime) == 0x5e8);
TPW_CHECK(AdvisorScoring_mTutorialExplainTooltipsCutoffTime, TPW_OFF(AdvisorScoring, mTutorialExplainTooltipsCutoffTime) == 0x5ec);
TPW_CHECK(AdvisorScoring_mTutorialExplainTooltipsScore, TPW_OFF(AdvisorScoring, mTutorialExplainTooltipsScore) == 0x5f0);
TPW_CHECK(AdvisorScoring_mGoldTicketExplanationScore, TPW_OFF(AdvisorScoring, mGoldTicketExplanationScore) == 0x5f4);
TPW_CHECK(AdvisorScoring_mPrebuiltParkScore, TPW_OFF(AdvisorScoring, mPrebuiltParkScore) == 0x5f8);
TPW_CHECK(AdvisorScoring_mGTAllResearchedAHigherPctThanThis, TPW_OFF(AdvisorScoring, mGTAllResearchedAHigherPctThanThis) == 0x5fc);
TPW_CHECK(AdvisorScoring_mGTAllResearchedAScore, TPW_OFF(AdvisorScoring, mGTAllResearchedAScore) == 0x600);
TPW_CHECK(AdvisorScoring_mGTAllResearchedBHigherPctThanThis, TPW_OFF(AdvisorScoring, mGTAllResearchedBHigherPctThanThis) == 0x604);
TPW_CHECK(AdvisorScoring_mGTAllResearchedBScore, TPW_OFF(AdvisorScoring, mGTAllResearchedBScore) == 0x608);
TPW_CHECK(AdvisorScoring_mGTAllResearchedCHigherPctThanThis, TPW_OFF(AdvisorScoring, mGTAllResearchedCHigherPctThanThis) == 0x60c);
TPW_CHECK(AdvisorScoring_mGTAllResearchedCScore, TPW_OFF(AdvisorScoring, mGTAllResearchedCScore) == 0x610);
TPW_CHECK(AdvisorScoring_mGTAmountVisitorsAHigherPctThanThis, TPW_OFF(AdvisorScoring, mGTAmountVisitorsAHigherPctThanThis) == 0x614);
TPW_CHECK(AdvisorScoring_mGTAmountVisitorsAScore, TPW_OFF(AdvisorScoring, mGTAmountVisitorsAScore) == 0x618);
TPW_CHECK(AdvisorScoring_mGTAmountVisitorsBHigherPctThanThis, TPW_OFF(AdvisorScoring, mGTAmountVisitorsBHigherPctThanThis) == 0x61c);
TPW_CHECK(AdvisorScoring_mGTAmountVisitorsBScore, TPW_OFF(AdvisorScoring, mGTAmountVisitorsBScore) == 0x620);
TPW_CHECK(AdvisorScoring_mGTAmountVisitorsCHigherPctThanThis, TPW_OFF(AdvisorScoring, mGTAmountVisitorsCHigherPctThanThis) == 0x624);
TPW_CHECK(AdvisorScoring_mGTAmountVisitorsCScore, TPW_OFF(AdvisorScoring, mGTAmountVisitorsCScore) == 0x628);
TPW_CHECK(AdvisorScoring_mChallengeScore, TPW_OFF(AdvisorScoring, mChallengeScore) == 0x62c);
TPW_CHECK(AdvisorScoring_mDaysForStaff, TPW_OFF(AdvisorScoring, mDaysForStaff) == 0x630);
TPW_CHECK(AdvisorScoring_mDaysForObjects, TPW_OFF(AdvisorScoring, mDaysForObjects) == 0x634);
TPW_CHECK(AdvisorScoring_size, sizeof(AdvisorScoring) == 0x638);
#endif
/* END GENERATED */

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
