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

/* Base of every world object (0x50b090). unk_00 is probably the vtable. */
typedef struct Thing {
    u32 unk_00;                          /* 0x00 */
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

/* Thing model 1 (0x4fb530). The pv_ fields are floats saved as 0..255 under a placeholder
 * name; they are probably the needs (hunger, thirst, ...).
 * Only the saved fields are known: the real object may be larger. */
typedef struct Visitor {
    Sprite base;                         /* 0x00 */
    float pv_19c;                        /* 0x19c */
    s32 mCash;                           /* 0x1a0 */
    float pv_1a4;                        /* 0x1a4 */
    float pv_1a8;                        /* 0x1a8 */
    float pv_1ac;                        /* 0x1ac */
    float pv_1b0;                        /* 0x1b0 */
    float pv_1b4;                        /* 0x1b4 */
    float pv_1b8;                        /* 0x1b8 */
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
    u8 unk_1f8[0x4];
    float pv_1fc;                        /* 0x1fc */
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

#ifndef __GHIDRA__
#define TPW_CHECK(name, cond) typedef char tpw_check_##name[(cond) ? 1 : -1]
#define TPW_OFF(t, f) ((u32)(unsigned long)&((t *)0)->f)
TPW_CHECK(Thing_unk_00, TPW_OFF(Thing, unk_00) == 0x0);
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
TPW_CHECK(Visitor_pv_19c, TPW_OFF(Visitor, pv_19c) == 0x19c);
TPW_CHECK(Visitor_mCash, TPW_OFF(Visitor, mCash) == 0x1a0);
TPW_CHECK(Visitor_pv_1a4, TPW_OFF(Visitor, pv_1a4) == 0x1a4);
TPW_CHECK(Visitor_pv_1a8, TPW_OFF(Visitor, pv_1a8) == 0x1a8);
TPW_CHECK(Visitor_pv_1ac, TPW_OFF(Visitor, pv_1ac) == 0x1ac);
TPW_CHECK(Visitor_pv_1b0, TPW_OFF(Visitor, pv_1b0) == 0x1b0);
TPW_CHECK(Visitor_pv_1b4, TPW_OFF(Visitor, pv_1b4) == 0x1b4);
TPW_CHECK(Visitor_pv_1b8, TPW_OFF(Visitor, pv_1b8) == 0x1b8);
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
TPW_CHECK(Staff_pv_1fc, TPW_OFF(Staff, pv_1fc) == 0x1fc);
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
