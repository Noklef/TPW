"""Read Theme Park World saved games (.sav / level .TPWI). Pure Python, no dependencies.

Container (SaveGame_Save 0x414920, SaveGame_WriteHeader 0x415f50, reader 0x416240):
  0x000 u32  version (400; SaveGame_Load refuses > 500)
  0x004 u8   language (0 English, 1 Spanish, 2 Italian, 3 Swedish, 4 German, 5 French, 6 Japanese)
  0x005 0x500 bytes  UTF-16 legal notice (the loader rejects the file if it was edited)
  0x505 0x100 bytes  first 32 bytes copied to/from DAT_00802080
  0x605 u32  big-endian magic 0x01221985
  0x609 u32  1 if a preview header follows (FUN_00418da0), else 0
  ....  'BILZ' blob (28-byte header + zlib) holding the module stream

Module stream (SaveGame_SaveModules 0x4164c0): GActionRec, then 16 modules each followed by a
4-byte delimiter tag (written as reversed ASCII, e.g. 'DLRW' for WRLD).

The world module (World_Save 0x516c80) is decoded field by field. Every serializer announces
each field through a debug stub, FUN_00416e10(size, "mName", isLoad), so the names below
are the game's own. Fields called 'pv' are raw pointers saved by accident (meaningless).

Usage:
  python -I tools/tpsave.py <file> [--json out.json] [--modules out_dir]
"""
import json
import os
import struct
import sys
import zlib

MAGIC = 0x01221985
LANGUAGES = ['English', 'Spanish', 'Italian', 'Swedish', 'German', 'French', 'Japanese']
MODULE_TAGS = ['WRLD', 'SPSC', 'PART', 'MESS', 'CLOK', 'VANT', 'GSYS', 'RSYS', 'TRAK', 'FLYR',
               'RSSE', 'KAME', 'COAS', 'ADVS', 'SOUN', 'CHTS', 'ADSC']
MODULE_NAMES = ['World', 'SpriteScripts', 'Particles', 'MessageCentre', 'Clock', 'VanillaTime',
                'GameSystem', 'RideSystem', 'TrackRides', 'FlyingRides', 'RSSE', 'Camera',
                'Coasters', 'Advisor', 'Sound', 'Cheats', 'AdvisorScoring', 'UI']


# --- schema DSL --------------------------------------------------------------------------
# A schema is a list of items:
#   (name, 'u8'|'u16'|'u32'|'i32'|'f32'|'u64'|'v2'|'v3'|'raw:N')   scalar
#   (name, ('arr', count, item))                                   fixed array
#   (name, ('circ', elem))   circular buffer: mCurrentEntry u32, mNumEntries u32,
#                            mWrappedAround u8, mTemp elem, mData elem[mNumEntries]
#   (None, ('inc', schema))  inline another schema (C++ base class / sub-object)

def arr(n, t):
    return ('arr', n, t)


def circ(t):
    return ('circ', t)


def inc(s):
    return (None, ('inc', s))


# CThing base (0x50b090)
# mX/mY are 8.8 fixed-point cell coordinates; cell ids are 1-based y*128 + x + 1.
THING = [('mX', 'u16'), ('mY', 'u16'), ('mMapChild', 'u16'), ('mMapParent', 'u16')]

# Map cells (FUN_004d7ea0, 128x128, each cell guarded by a status byte)
CELL_BASE = [('mDirection', 'u8'), ('mFlags', 'u16'), ('mMeshInstance', 'u32'),
             ('mNeighbours', 'u8'), ('mOverlapCounter', 'u16'), ('mParentID', 'u16'),
             ('mTileData', 'raw:12'), ('mType', 'u32'), ('mHoardingNeighbours', 'u8')]  # 0x4d0b30
MAP_CELL = [inc(CELL_BASE), ('mLitter', 'u32'), ('mLitterCollector', 'u16'),
            ('mLitterScript', 'u32'), ('mLitterScript2', 'u32'), ('mPylonIndex', 'u16'),
            ('mStatusFlags', 'u8'), ('mTimeMarkedForLitterCollection', 'u32'),
            ('mWho', 'u16')]  # 0x4d8dd0
TRACK_CELL = [inc(CELL_BASE), ('mSegmentNumber', 'u16')]  # 0x50c200
RE_CELL = [('mpEffect', arr(5, 'u16'))]  # 0x502490

TIME_SIG = [('mTimeSig', 'u32')]  # 0x41a860

# Steering / navigation (0x50d150)
NAV = [('force', 'v2'), ('formation_pos', 'v2'), ('local_xaxis', 'v2'), ('local_yaxis', 'v2'),
       ('mass', 'f32'), ('max_force', 'f32'), ('max_speed', 'f32'), ('mCantReachDest', 'u32'),
       ('nav_mode', 'u32'), ('path_buffer_count', 'u32'), ('path_count', 'u32'),
       ('path_finished', 'u8'), ('path_last_progress', 'u32'), ('path_stuck_buffer', 'u32'),
       ('path_subpath_dist', 'f32'), ('path_tail_dist', 'f32'), ('path_target_pos', 'v2'),
       ('path_timestamp', 'u32'), ('path_total_count', 'u32'), ('path_total_dist', 'f32'),
       ('pos', 'v2'), ('radius', 'f32'),
       ('subpath', arr(5, [('subpath_buffer_p', 'v2'), ('subpath_dist_p', 'f32')])),
       ('vel', 'v2')]
THOUGHTS = [('mActionHistIndex', 'u32'), ('mEventHistory', arr(0x20, 'u32')),
            ('mLastThought', 'u32'), ('mThoughtScript', 'u32'), ('mTimeBubbleShown', 'u32')]  # 0x50bad0
ANIM = [('mSpriteScript', 'u32'), ('mNextAnim', 'u32'), ('mNextServiceInterval', 'u32')]  # 0x4d3ee0

# CSprite (0x4f8b10): moving people
SPRITE = [inc(THING), inc(ANIM),
          ('mAccurateDestX', 'u16'), ('mAccurateDestY', 'u16'), ('mAdjustorSpeed', 'u16'),
          ('mBaseSpeed', 'u16'), ('mCount', 'u8'), ('mESPSprite', 'u32'),
          ('mLastRecordedMapId', 'u16'), ('nav', ('inc', NAV)),
          ('mPreviousSpeed', 'u32'), ('mPreviousX', 'u32'), ('mPreviousY', 'u32'),
          ('mStrandedTime', 'u32'), ('mPurposeSpeed', 'u16'), ('mSetDestSuccessfully', 'u32'),
          ('mSpriteAngle', 'u32'), ('mSpriteID', 'u32'), ('mSpriteUnderRideCtrl', 'u32'),
          inc(THOUGHTS)]

VISITOR = [inc(SPRITE),
           ('mArrivalDate', 'u32'), ('mArrivalIndex', 'u32'), ('mBalloonScript', 'u32'),
           ('mBeenAdmitted', 'u32'), ('mCash', 'i32'), ('mExitLevel', 'u32'), ('pv1', 'u32'),
           ('pv2', 'u32'), ('mLastPosX', 'u32'), ('mLastPosY', 'u32'), ('pv3', 'u32'),
           ('mMajorDest', 'u16'), ('mNumRides', 'u32'), ('mNumShops', 'u32'),
           ('mNumSideshows', 'u32'), ('mNumSideshowsWon', 'u32'), ('mPaidAdmission', 'u32'),
           ('mParkOpeningWaitingTime', 'u32'), ('mPersonType', 'u8'), ('mPrankeryIndex', 'u8'),
           ('mPreviousRides', arr(4, [('ride', 'u16'), ('temporary', 'u16')])),
           ('mQNext', 'u16'), ('mQPrev', 'u16'), ('mQueueMoveDelay', 'u32'), ('mQueuePos', 'u8'),
           ('mRemainingBalloonLife', 'u32'), ('mSavedMajorDest', 'u16'), ('mSavedState', 'u32'),
           ('mState', 'u32'), ('pv4', 'u32'), ('mTimeOfLastSpotAnim', 'u32'),
           ('mTimeStartedIdling', 'u32'), ('pv5', 'u32'), ('pv6', 'u32'), ('pv7', 'u32')]  # 0x4fb530

STAFF = [inc(SPRITE),
         ('mCurrentPayGrade', 'u32'), ('pv1', 'u32'), ('mJobsDone', 'u32'),
         ('mName', arr(0x21, 'u16')), ('mPatrolRegionBL', 'u16'), ('mPatrolRegionTR', 'u16'),
         ('mPercentageThroughGrade', 'u8'), ('mRestArea', 'u16'), ('mState', 'u32'),
         ('mTimeStartedIdling', 'u32'), ('mTimeHired', 'u64'), ('pv2', 'u32')]  # 0x504de0
MECHANIC = [inc(STAFF), ('mDurationOfRepair', 'u32'), ('mObjectToRepair', 'u16'), ('mNext', 'u16')]
HANDYMAN = [inc(STAFF), ('mTargetLitterCell', 'u16'), ('mTimeStartedCleaning', 'u32'),
            ('mToiletToClean', 'u16'), ('mNext', 'u16')]
ENTERTAINER = [inc(STAFF), ('mTimeStartedEntertaining', 'u32'), ('mNext', 'u16')]
GUARD = [inc(STAFF), ('mPerp', 'u16'), ('mProsecutionTimestamp', 'u32'), ('mNext', 'u16')]
RESEARCHER = [inc(STAFF), ('mTimeStartedResearching', 'u32'), ('mNext', 'u16')]

OBJECT = [inc(THING),
          ('mAngle', 'u32'), ('mId', 'u16'), ('tv_t', arr(8, 'u32')), ('MeshInstanceID', 'u32'),
          ('mFlags', 'u16'), ('mName', arr(0x21, [('a', 'u16'), ('b', 'u16')])),  # mNameA[i], mNameB[i] interleaved
          ('mRideScriptHandle', 'u32'), ('mTrackRideHandle', 'u32'), ('mState', 'u32'),
          ('mTopLeft', 'u16'), ('mEntryPos', 'u16'), ('mNext', 'u16'),
          ('mAssignedStaffMember', 'u16'), ('mBackOfQueue', 'u16'), ('mCanLoad', 'u32'),
          ('mExitPos', 'u16'), ('mFirstInQ', 'u16'), ('mIsTrackRideValid', 'u32'),
          ('mUpgradeParent', 'u16'),
          ('hist0', circ('u32')), ('hist1', circ('u32')), ('mNumCustomers', 'u32'),
          ('hist2', circ('u32')), ('mNumWalkAways', 'u32'), ('hist3', circ('u32')),
          ('hist4', circ('u32')), ('hist5', circ('u32')),
          ('mOperatingCapacity', 'u8'), ('mOperatingDuration', 'u8'), ('mOperatingSpeed', 'u32'),
          ('mPersonBeingLoaded', 'u16'), ('mCostOfGoods', 'u32'), ('mQualityOfGoods', 'u32'),
          ('mChanceOfWinning', 'u32'), ('mPricePerUse', 'u32'),
          ('mAmountOfSpecialIngredient', 'u32'), ('mQueueSizeInCells', 'u32'),
          ('pv1', 'u32'), ('pv2', 'u32'), ('pv3', 'u32'), ('mRequestedService', 'u32'),
          ('mTimeMarkedForMaintenance', 'u32'), ('mTotalCosts', 'u32'), ('mTotalTakings', 'u32'),
          ('mUpgradeBalloonSprite', 'u32'), ('mUpgradeLevel', 'u8')]  # 0x4db7d0

STAFF_HQ = [inc(THING), ('mForceStrike', 'u32'), ('mStaffMemberPickedUp', 'u16'),
            ('mStrikeLevel', arr(5, 'raw:12')),
            ('mBudget', arr(5, 'u32')), ('mHaveEverTrained', 'u8')]  # 0x508bb0 + 0x50c640
MECHANIC_HQ = [inc(THING), ('mNextObject', 'u16')]  # 0x4da960
ADVISOR_MSG = [('mThingId', 'u16'), ('mSecondary', 'u32'), ('mMessage', 'u32'), ('mScore', 'u32'),
               ('mValid', 'u8'), ('mResponseNo', 'u32'), ('mOverrideOnlyOnce', 'u8'),
               ('mScoreOverridden', 'u8')]  # 0x59ba30
ADVISOR = [inc(THING), inc(TIME_SIG), ('mSoundId', 'u32'), ('mQueue', arr(8, ADVISOR_MSG)),
                   ('mNextAvailableQueuePos', 'u32'), ('mLastActionStarted', 'u32'),
                   ('mLastActionDuration', 'u32'), ('mHistory', arr(0x15f, 'raw:16')),
                   ('mCurrent', ('inc', ADVISOR_MSG)), ('mRequestedLastTutorial', 'u8'),
                   ('mMessagePlaying', 'u32')]  # 0x599f60
VAL = [('mVal', 'u32'), ('mWorkedOut', 'u8')]  # 0x4cd9c0
ANALYSER_CELL = [('mStartX', 'u32'), ('mStartY', 'u32'), ('mSizeX', 'u32'), ('mSizeY', 'u32'),
                 ('vals', arr(9, VAL))]  # 0x4c85b0
PARK_ANALYSER = [inc(THING),
                 ('mMapSquaresPerCell', 'u32'), ('mGridSizeX', 'u32'), ('mGridSizeY', 'u32'),
                 ('mMapSizeX', 'u32'), ('mMapSizeY', 'u32'),
                 ('cells', arr(0x400, ANALYSER_CELL)),
                 ('hist16', arr(4, circ('u16'))), ('vals', arr(6, VAL)),
                 ('hist32', arr(9, circ('u32'))), ('hist32b', arr(9, circ('u32'))),
                 ('hist8', arr(0x14, circ('u8'))),
                 ('mMisbehavingKids', 'u32'), ('mMisbehavingKidsThatGotAwayWithIt', 'u32'),
                 ('mMisbehavingKidsEver', 'u32'), ('mLifetimeVisitors', 'u32'),
                 ('mLastStrike', 'u64'), ('mLastPuke', 'u64'), ('mLastPrank', 'u64'),
                 ('mParkLastOpened', 'u64'), ('mLastObjectBuilt', 'u64'), ('mLastStaffHired', 'u64'),
                 ('mMostPaidForTicket', 'u32'), ('mLargestBusLoad', 'u32'), ('mLongestStay', 'u32')]  # 0x4c5d70
RESEARCH_PROJECT = [('mObjId', 'u16'), ('mCat', 'u32'), ('mResearchGroup', 'u8'),
                    ('mUpgradeLevel', 'u8'), ('mCost', 'f32'), ('mResearchDone', 'f32')]  # 0x504210
RESEARCH_GROUP = [('effort', 'u32'), ('output', 'u32'), ('currentProject', 'u32'),
                  ('highestGroupAvailable', 'u8')]  # 0x504760
RESEARCH_LAB = [inc(THING), ('mProjects', arr(0x100, RESEARCH_PROJECT)),
                ('mGroups', arr(5, RESEARCH_GROUP)), ('mNumProjects', 'u32'), ('mWorkLoad', 'u32'),
                ('mMostExpensiveProject', 'u32'), ('mLabActive', 'u8')]  # 0x502dc0
FORECAST = [('mTime', 'u64'), ('mQuality', 'u32'), ('mValid', 'u8')]  # 0x5132a0
WEATHER = [inc(THING), ('mQuality', 'u32'), ('mLastChanged', 'u64'), ('mLastForecasted', 'u64'),
           ('mForecast', ('inc', FORECAST)), ('mCurrentDrops', 'u32'), ('mTargetDrops', 'u32'),
           ('mLightning', 'u8'), ('mLightningCountdown', 'u32'), ('mLightningNumThisPeriod', 'u32'),
           ('mThunderCountdown', 'u32'), ('mLastLightningLocation', 'v3'),
           ('mRainSoundHandle', 'u32'), ('mPType', 'u32'), ('mWindDir', 'f32'),
           ('mWindForce', 'f32'), ('mOverridden', 'u8')]  # 0x512010
BANK = [inc(THING), ('mAdmissionFee', 'u32'), ('mBalance', 'i32'), ('mBatchBalance', 'i32'),
        ('mWithdrawalsEnabled', 'u32'), ('mLastBalance', 'i32'), ('mTurnEnteredRed', 'u32'),
        ('mProfitThisYear', 'i32'),
        ('mLoans', arr(8, [('loan_available', 'u32'), ('amount_available', 'u32'),
                           ('APR_in_percent', 'u32'), ('repayment_period_in_months', 'u32'),
                           ('monthly_repayment', 'u32'), ('loan_bought', 'u32'),
                           ('months_repaid', 'u32'), ('lenderNameIndex', 'u32')]))]  # 0x4cf920
CHALLENGE = [('mType', 'u32'), ('mTargetTime', 'u32'), ('mTargetVal', 'u32'), ('mTargetObj', 'u16'),
             ('mTargetObj2', 'u16'), ('mTargetStaffType', 'u32'), ('mPrize', 'u32'),
             ('mDoubleOrNothingFollowup', 'u32'), ('mCheckAtEndOnly', 'u8'),
             ('mChallengeOffered', 'u32'), ('mChallengeLost', 'u32'), ('mChallengeDeclined', 'u32'),
             ('mHaveWon', 'u8'), ('mIndependent', 'u8'), ('mThingIdForFollowup', 'u16')]  # 0x4d0f90
CHALLENGES = [inc(THING), ('mChallenges', arr(0x14, CHALLENGE)), ('mCurrentChallenge', 'u32'),
              ('mNextChallengeEventTime', 'u64'), ('mSystemSwitchedOn', 'u8'),
              ('mChallengeOn', 'u8'), ('mWaitingForReply', 'u8'), ('mValue', 'u32'),
              ('mActualThing', 'u16')]  # 0x4d2320

THING_MODELS = {
    1: ('Visitor', VISITOR), 3: ('Object', OBJECT), 4: ('Mechanic', MECHANIC),
    5: ('Handyman', HANDYMAN), 6: ('Entertainer', ENTERTAINER), 7: ('Guard', GUARD),
    8: ('Researcher', RESEARCHER), 9: ('StaffHQ', STAFF_HQ), 10: ('MechanicHQ', MECHANIC_HQ),
    11: ('Advisor', ADVISOR), 12: ('TagSystem', THING), 13: ('ParkAnalyser', PARK_ANALYSER),
    14: ('ResearchLab', RESEARCH_LAB), 15: ('Weather', WEATHER), 16: ('BankAccount', BANK),
    17: ('UIMsgReceiver', THING), 19: ('Challenges', CHALLENGES),
}

WORLD_HEADER = [
    ('version', 'u32'), ('mArrivalVehicle_Size1', 'u16'), ('mArrivalVehicle_Size2', 'u16'),
    ('mArrivalVehicle_Size3', 'u16'), ('mBankAccount', 'u16'), ('mCurrentArrivalVehicle', 'u16'),
    ('mGameTick', 'u32'), ('mMechanicHQ', 'u16'), ('mParkAnalyser', 'u16'), ('mParkClosed', 'u32'),
    ('mNumberOfVisitorsToDate', 'u32'), ('mParkGates', 'u16'), ('mTrafficLights', 'u16'),
    ('mRandomSeed', 'u32'), ('mResearchLab', 'u16'), ('mStaffHQ', 'u16'), ('mTagSystem', 'u16'),
    ('mUIMsgReceiver', 'u16'), ('mWeather', 'u16'), ('mWorldState', 'u32'),
    ('mFirstHandyman', 'u16'), ('mFirstMechanic', 'u16'), ('mFirstEntertainer', 'u16'),
    ('mFirstGuard', 'u16'), ('mFirstResearcher', 'u16'), ('mFirstObject', 'u16'),
    # 0x4d3aa0
    ('mObjectControls', arr(0x96, 'raw:32')), ('mNumObjectControls', 'u32'),
    ('mPreviousSearchKey', 'u16'),
    # 0x4d7a70 = staff pool 0x507850, clock 0x4f7f30, park gates 0x4cf050
    ('mStaffPool', arr(0x20, [('mType', 'u32'), ('mName', 'u32'), ('mPayGrade', 'u8'),
                              ('mSubType', 'u8'), ('mValid', 'u8'), ('mOnPointer', 'u8'),
                              ('mTimeSig', 'u32'), ('mTimeoutTime', 'u32')])),
    ('mStaffCategories', arr(5, [('mPeopleInCat', 'u32'), ('mStopProducing', 'u8')])),
    ('mStaffPoolTimeSig', 'u32'), ('mOpeningStaffPoolGenerated', 'u8'),
    ('mFunnyTimeStart', 'u64'), ('mSessionStart', 'u64'), ('mMonthAtLastUpdate', 'u32'),
    ('mDayAtLastUpdate', 'u32'), ('mFunnySecsPerRealSec', 'u32'),
    ('mArrivalRate', 'u32'), ('mGatesTimeSig', 'u32'), ('mTargetVehicleCapacity', 'u32'),
    ('mPeopleOnBus', 'u32'), ('mOffloading', 'u8'), ('mGatesOpen', 'u8'),
]


class Reader:
    def __init__(self, data, pos=0):
        self.d, self.p = data, pos

    def take(self, n):
        if self.p + n > len(self.d):
            raise EOFError(f'read past end at {self.p:#x}')
        v = self.d[self.p:self.p + n]
        self.p += n
        return v

    def scalar(self, t):
        if t.startswith('raw:'):
            return self.take(int(t[4:])).hex()
        fmt = {'u8': '<B', 'u16': '<H', 'u32': '<I', 'i32': '<i', 'f32': '<f', 'u64': '<Q',
               'v2': '<2f', 'v3': '<3f'}[t]
        v = struct.unpack(fmt, self.take(struct.calcsize(fmt)))
        return list(v) if len(v) > 1 else v[0]

    def item(self, t):
        if isinstance(t, str):
            return self.scalar(t)
        if isinstance(t, list):
            return self.schema(t)
        kind = t[0]
        if kind == 'arr':
            return [self.item(t[2]) for _ in range(t[1])]
        if kind == 'circ':
            out = {'mCurrentEntry': self.scalar('u32'), 'mNumEntries': self.scalar('u32'),
                   'mWrappedAround': self.scalar('u8'), 'mTemp': self.scalar(t[1])}
            out['mData'] = [self.scalar(t[1]) for _ in range(out['mNumEntries'])]
            return out
        if kind == 'inc':
            return self.schema(t[1])
        raise ValueError(t)

    def schema(self, sch, out=None):
        out = {} if out is None else out
        for name, t in sch:
            if name is None:  # inline base
                self.schema(t[1], out)
            else:
                out[name] = self.item(t)
        return out


def read_container(data):
    version, lang = struct.unpack_from('<IB', data, 0)
    legal = data[5:0x505].decode('utf-16-le').split('\0', 1)[0]
    magic = struct.unpack_from('>I', data, 0x605)[0]
    has_preview = struct.unpack_from('<I', data, 0x609)[0]
    if magic != MAGIC:
        raise ValueError(f'bad magic {magic:#x}')
    if has_preview:
        raise NotImplementedError('preview header (FUN_00418da0) not decoded yet')
    blob = data[0x60d:]
    if blob[:4] != b'BILZ':
        raise ValueError('module stream is not BILZ-compressed')
    size = struct.unpack_from('<I', blob, 4)[0]
    payload = zlib.decompress(blob[28:])
    if len(payload) != size:
        raise ValueError(f'BILZ size mismatch {len(payload)} != {size}')
    return {'version': version, 'language': LANGUAGES[lang] if lang < len(LANGUAGES) else lang,
            'legal': legal, 'header_block': data[0x505:0x525].hex()}, payload


def split_modules(payload):
    """Return [(name, start, end)] using the delimiter tags (each appears exactly once)."""
    pub, rec_size = struct.unpack_from('<II', payload, 0)
    bounds = [('ActionRec', 0, 8 + rec_size)]
    start = 8 + rec_size
    for tag, name in zip(MODULE_TAGS, MODULE_NAMES):
        i = payload.find(tag[::-1].encode('ascii'), start)
        if i < 0:
            raise ValueError(f'delimiter {tag} not found')
        bounds.append((name, start, i))
        start = i + 4
    bounds.append((MODULE_NAMES[-1], start, len(payload)))
    return bounds


def read_world(payload, start, end):
    r = Reader(payload, start)
    world = r.schema(WORLD_HEADER)
    counts = [0, 0, 0]
    cells = []
    for i in range(0x4000):
        status = r.scalar('u8')
        cell = {'index': i}
        if status & 1:
            cell['map'] = r.schema(MAP_CELL); counts[0] += 1
        if status & 2:
            cell['track'] = r.schema(TRACK_CELL); counts[1] += 1
        if status & 4:
            cell['re'] = r.schema(RE_CELL); counts[2] += 1
        if status & ~7:
            raise ValueError(f'bad cell status {status:#x} at {r.p - 1:#x}')
        cells.append(cell)
    world['cell_counts'] = {'map': counts[0], 'track': counts[1], 're': counts[2]}
    world['cells'] = cells
    head = r.scalar('u32')
    things = []
    while True:
        at = r.p
        nxt, model = r.scalar('u32'), r.scalar('u32')
        if model not in THING_MODELS:
            raise ValueError(f'unknown thing model {model} at {at:#x}')
        name, sch = THING_MODELS[model]
        thing = {'slot': None, 'next': nxt, 'model': model, 'kind': name, 'offset': at}
        thing.update(r.schema(sch))
        things.append(thing)
        if nxt == 0:
            break
    # slots: the list starts at 'head' and follows 'next'
    slot = head
    for t in things:
        t['slot'] = slot
        slot = t['next']
    world['used_thing_head'] = head
    world['things'] = things
    if r.p != end:
        raise ValueError(f'world module ends at {end:#x} but parsing stopped at {r.p:#x}')
    return world


def load(path):
    data = open(path, 'rb').read()
    info, payload = read_container(data)
    mods = split_modules(payload)
    info['modules'] = [{'name': n, 'offset': s, 'size': e - s} for n, s, e in mods]
    wn, ws, we = mods[1]
    info['world'] = read_world(payload, ws, we)
    return info, payload, mods


def summary(info):
    w = info['world']
    print(f"save version {info['version']}, language {info['language']}")
    for m in info['modules']:
        print(f"  {m['name']:<15} {m['size']:>9,} bytes")
    print(f"world v{w['version']}: tick {w['mGameTick']}, visitors to date {w['mNumberOfVisitorsToDate']}, "
          f"seed {w['mRandomSeed']:#x}, cells {w['cell_counts']}")
    kinds = {}
    for t in w['things']:
        kinds[t['kind']] = kinds.get(t['kind'], 0) + 1
    print('things:', ', '.join(f'{k} x{v}' for k, v in sorted(kinds.items())))
    for t in w['things']:
        if t['kind'] == 'BankAccount':
            print(f"bank: balance {t['mBalance']}, admission fee {t['mAdmissionFee']}, "
                  f"profit this year {t['mProfitThisYear']}")


if __name__ == '__main__':
    args = sys.argv[1:]
    if not args:
        sys.exit(__doc__)
    info, payload, mods = load(args[0])
    summary(info)
    if '--json' in args:
        with open(args[args.index('--json') + 1], 'w', encoding='utf-8') as f:
            json.dump(info, f, indent=1)
    if '--modules' in args:
        out = args[args.index('--modules') + 1]
        os.makedirs(out, exist_ok=True)
        for n, s, e in mods:
            with open(os.path.join(out, f'{n}.bin'), 'wb') as f:
                f.write(payload[s:e])
