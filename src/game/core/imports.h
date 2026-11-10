// The exe's import address table (0x6fd000, 0x610 bytes): a slot for each imported function, in the exe's order, each DLL's slots ended by a null one. Members are
// named DLL_function (the DLL's name, then the import's: a C++ member of the EA Online DLLs as Class_Method, its constructor Class_Ctor, its destructor
// Class_Dtor; an import by ordinal by its documented name, WinSock 1.1's and DSOUND's 1, the ordinal in the comment). Made from tp.exe's import directory
// (.docs/09-globals.md, "Imports").
//
// The ports read a slot by its address, IMPORT(member), wherever they read 0x6fdXXX before. The hook DLL, the tests and the comparison driver bind gImports
// to 0x6fd000, so they call through the import table Windows (or a bench) filled; in tpw.exe gImports is an object of its own, which the loader fills
// from the real DLLs (src/loader/loader.cpp, FillImports).
#pragma once

#include <cstdint>

#include "../../../re/types.h"
#include "gamedata.h"

struct ImportTable {
    // ADVAPI32.dll
    void *Advapi32_RegCloseKey;   // 0x6fd000
    void *Advapi32_RegOpenKeyA;   // 0x6fd004
    void *Advapi32_RegQueryValueExA;   // 0x6fd008
    void *Advapi32_RegCreateKeyExA;   // 0x6fd00c
    void *Advapi32_null;   // 0x6fd010
    // DDRAW.dll
    void *Ddraw_DirectDrawEnumerateA;   // 0x6fd014
    void *Ddraw_DirectDrawCreate;   // 0x6fd018
    void *Ddraw_null;   // 0x6fd01c
    // DINPUT.dll
    void *Dinput_DirectInputCreateA;   // 0x6fd020
    void *Dinput_null;   // 0x6fd024
    // DSOUND.dll
    void *Dsound_DirectSoundCreate;   // 0x6fd028 #1
    void *Dsound_null;   // 0x6fd02c
    // GDI32.dll
    void *Gdi32_AddFontResourceA;   // 0x6fd030
    void *Gdi32_SetBkColor;   // 0x6fd034
    void *Gdi32_DeleteObject;   // 0x6fd038
    void *Gdi32_SetTextColor;   // 0x6fd03c
    void *Gdi32_SetMapMode;   // 0x6fd040
    void *Gdi32_SelectObject;   // 0x6fd044
    void *Gdi32_CreateFontIndirectA;   // 0x6fd048
    void *Gdi32_CreateDIBSection;   // 0x6fd04c
    void *Gdi32_CreateCompatibleDC;   // 0x6fd050
    void *Gdi32_GetStockObject;   // 0x6fd054
    void *Gdi32_RemoveFontResourceA;   // 0x6fd058
    void *Gdi32_DeleteDC;   // 0x6fd05c
    void *Gdi32_null;   // 0x6fd060
    // IMM32.dll
    void *Imm32_ImmGetCompositionStringA;   // 0x6fd064
    void *Imm32_ImmNotifyIME;   // 0x6fd068
    void *Imm32_ImmGetOpenStatus;   // 0x6fd06c
    void *Imm32_ImmSetOpenStatus;   // 0x6fd070
    void *Imm32_ImmSetCompositionWindow;   // 0x6fd074
    void *Imm32_ImmGetCandidateListA;   // 0x6fd078
    void *Imm32_ImmDestroyContext;   // 0x6fd07c
    void *Imm32_ImmReleaseContext;   // 0x6fd080
    void *Imm32_ImmGetContext;   // 0x6fd084
    void *Imm32_ImmGetDescriptionA;   // 0x6fd088
    void *Imm32_ImmGetProperty;   // 0x6fd08c
    void *Imm32_ImmCreateContext;   // 0x6fd090
    void *Imm32_ImmSetCompositionStringA;   // 0x6fd094
    void *Imm32_null;   // 0x6fd098
    // KERNEL32.dll
    void *Kernel32_GetDriveTypeA;   // 0x6fd09c
    void *Kernel32_GetLogicalDriveStringsA;   // 0x6fd0a0
    void *Kernel32_GetVolumeInformationA;   // 0x6fd0a4
    void *Kernel32_FreeLibrary;   // 0x6fd0a8
    void *Kernel32_QueryPerformanceFrequency;   // 0x6fd0ac
    void *Kernel32_GetProcAddress;   // 0x6fd0b0
    void *Kernel32_LoadLibraryA;   // 0x6fd0b4
    void *Kernel32_GetExitCodeThread;   // 0x6fd0b8
    void *Kernel32_FileTimeToSystemTime;   // 0x6fd0bc
    void *Kernel32_InterlockedIncrement;   // 0x6fd0c0
    void *Kernel32_DeleteFileA;   // 0x6fd0c4
    void *Kernel32_FindNextFileA;   // 0x6fd0c8
    void *Kernel32_QueryPerformanceCounter;   // 0x6fd0cc
    void *Kernel32_DeleteCriticalSection;   // 0x6fd0d0
    void *Kernel32_CreateEventA;   // 0x6fd0d4
    void *Kernel32_CreateFileA;   // 0x6fd0d8
    void *Kernel32_ReadFile;   // 0x6fd0dc
    void *Kernel32_WriteFile;   // 0x6fd0e0
    void *Kernel32_SetFilePointer;   // 0x6fd0e4
    void *Kernel32_GetFileSize;   // 0x6fd0e8
    void *Kernel32_GetSystemInfo;   // 0x6fd0ec
    void *Kernel32_MapViewOfFileEx;   // 0x6fd0f0
    void *Kernel32_CreateFileMappingA;   // 0x6fd0f4
    void *Kernel32_UnmapViewOfFile;   // 0x6fd0f8
    void *Kernel32_FlushFileBuffers;   // 0x6fd0fc
    void *Kernel32_FindFirstFileA;   // 0x6fd100
    void *Kernel32_FindClose;   // 0x6fd104
    void *Kernel32_SetFileAttributesA;   // 0x6fd108
    void *Kernel32_GetFileAttributesA;   // 0x6fd10c
    void *Kernel32_CompareFileTime;   // 0x6fd110
    void *Kernel32_RemoveDirectoryA;   // 0x6fd114
    void *Kernel32_GetCurrentDirectoryA;   // 0x6fd118
    void *Kernel32_MoveFileA;   // 0x6fd11c
    void *Kernel32_CreateDirectoryA;   // 0x6fd120
    void *Kernel32_SetEvent;   // 0x6fd124
    void *Kernel32_WaitForSingleObject;   // 0x6fd128
    void *Kernel32_InterlockedDecrement;   // 0x6fd12c
    void *Kernel32_LeaveCriticalSection;   // 0x6fd130
    void *Kernel32_CreateMutexA;   // 0x6fd134
    void *Kernel32_GetLastError;   // 0x6fd138
    void *Kernel32_CloseHandle;   // 0x6fd13c
    void *Kernel32_EnterCriticalSection;   // 0x6fd140
    void *Kernel32_InitializeCriticalSection;   // 0x6fd144
    void *Kernel32_GlobalMemoryStatus;   // 0x6fd148
    void *Kernel32_GetTempPathA;   // 0x6fd14c
    void *Kernel32_SystemTimeToFileTime;   // 0x6fd150
    void *Kernel32_IsDBCSLeadByte;   // 0x6fd154
    void *Kernel32_CopyFileA;   // 0x6fd158
    void *Kernel32_FileTimeToLocalFileTime;   // 0x6fd15c
    void *Kernel32_WaitForMultipleObjects;   // 0x6fd160
    void *Kernel32_GetLocalTime;   // 0x6fd164
    void *Kernel32_ResetEvent;   // 0x6fd168
    void *Kernel32_CreateSemaphoreA;   // 0x6fd16c
    void *Kernel32_ReleaseSemaphore;   // 0x6fd170
    void *Kernel32_MultiByteToWideChar;   // 0x6fd174
    void *Kernel32_TlsAlloc;   // 0x6fd178
    void *Kernel32_GetACP;   // 0x6fd17c
    void *Kernel32_GetCPInfo;   // 0x6fd180
    void *Kernel32_GetEnvironmentStringsW;   // 0x6fd184
    void *Kernel32_GetEnvironmentStrings;   // 0x6fd188
    void *Kernel32_FreeEnvironmentStringsW;   // 0x6fd18c
    void *Kernel32_FreeEnvironmentStringsA;   // 0x6fd190
    void *Kernel32_GetModuleFileNameA;   // 0x6fd194
    void *Kernel32_LCMapStringW;   // 0x6fd198
    void *Kernel32_LCMapStringA;   // 0x6fd19c
    void *Kernel32_UnhandledExceptionFilter;   // 0x6fd1a0
    void *Kernel32_SetUnhandledExceptionFilter;   // 0x6fd1a4
    void *Kernel32_GetFileType;   // 0x6fd1a8
    void *Kernel32_GetStdHandle;   // 0x6fd1ac
    void *Kernel32_SetHandleCount;   // 0x6fd1b0
    void *Kernel32_VirtualAlloc;   // 0x6fd1b4
    void *Kernel32_VirtualFree;   // 0x6fd1b8
    void *Kernel32_HeapCreate;   // 0x6fd1bc
    void *Kernel32_HeapDestroy;   // 0x6fd1c0
    void *Kernel32_TlsGetValue;   // 0x6fd1c4
    void *Kernel32_SetLastError;   // 0x6fd1c8
    void *Kernel32_GlobalAlloc;   // 0x6fd1cc
    void *Kernel32_GetTickCount;   // 0x6fd1d0
    void *Kernel32_Sleep;   // 0x6fd1d4
    void *Kernel32_IsBadReadPtr;   // 0x6fd1d8
    void *Kernel32_WideCharToMultiByte;   // 0x6fd1dc
    void *Kernel32_OutputDebugStringA;   // 0x6fd1e0
    void *Kernel32_GetCommandLineA;   // 0x6fd1e4
    void *Kernel32_GetCurrentProcessId;   // 0x6fd1e8
    void *Kernel32_GetStringTypeW;   // 0x6fd1ec
    void *Kernel32_GetStringTypeA;   // 0x6fd1f0
    void *Kernel32_GetOEMCP;   // 0x6fd1f4
    void *Kernel32_EnumSystemLocalesA;   // 0x6fd1f8
    void *Kernel32_SetThreadPriority;   // 0x6fd1fc
    void *Kernel32_InterlockedExchange;   // 0x6fd200
    void *Kernel32_GetLocaleInfoW;   // 0x6fd204
    void *Kernel32_HeapSize;   // 0x6fd208
    void *Kernel32_RtlUnwind;   // 0x6fd20c
    void *Kernel32_HeapFree;   // 0x6fd210
    void *Kernel32_GetFullPathNameA;   // 0x6fd214
    void *Kernel32_RaiseException;   // 0x6fd218
    void *Kernel32_GetTimeZoneInformation;   // 0x6fd21c
    void *Kernel32_GetSystemTime;   // 0x6fd220
    void *Kernel32_HeapAlloc;   // 0x6fd224
    void *Kernel32_ResumeThread;   // 0x6fd228
    void *Kernel32_CreateThread;   // 0x6fd22c
    void *Kernel32_TlsSetValue;   // 0x6fd230
    void *Kernel32_ExitThread;   // 0x6fd234
    void *Kernel32_GetModuleHandleA;   // 0x6fd238
    void *Kernel32_GetStartupInfoA;   // 0x6fd23c
    void *Kernel32_GetVersion;   // 0x6fd240
    void *Kernel32_ExitProcess;   // 0x6fd244
    void *Kernel32_GetCurrentThreadId;   // 0x6fd248
    void *Kernel32_TerminateProcess;   // 0x6fd24c
    void *Kernel32_GetCurrentProcess;   // 0x6fd250
    void *Kernel32_GetCurrentThread;   // 0x6fd254
    void *Kernel32_HeapReAlloc;   // 0x6fd258
    void *Kernel32_IsBadWritePtr;   // 0x6fd25c
    void *Kernel32_GetUserDefaultLCID;   // 0x6fd260
    void *Kernel32_GetVersionExA;   // 0x6fd264
    void *Kernel32_SetStdHandle;   // 0x6fd268
    void *Kernel32_GlobalFree;   // 0x6fd26c
    void *Kernel32_CompareStringA;   // 0x6fd270
    void *Kernel32_CompareStringW;   // 0x6fd274
    void *Kernel32_SetEnvironmentVariableA;   // 0x6fd278
    void *Kernel32_SetEndOfFile;   // 0x6fd27c
    void *Kernel32_IsBadCodePtr;   // 0x6fd280
    void *Kernel32_IsValidLocale;   // 0x6fd284
    void *Kernel32_IsValidCodePage;   // 0x6fd288
    void *Kernel32_GetLocaleInfoA;   // 0x6fd28c
    void *Kernel32_null;   // 0x6fd290
    // QMIXER.dll
    void *Qmixer_QSWaveMixSetSpeakerPlacement;   // 0x6fd294
    void *Qmixer_QSWaveMixSetListenerOrientation;   // 0x6fd298
    void *Qmixer_QSWaveMixSetSpeedOfSound;   // 0x6fd29c
    void *Qmixer_QSWaveMixSetVolume;   // 0x6fd2a0
    void *Qmixer_QSWaveMixSetDistanceMapping;   // 0x6fd2a4
    void *Qmixer_QSWaveMixSetSourceCone;   // 0x6fd2a8
    void *Qmixer_QSWaveMixSetFrequency;   // 0x6fd2ac
    void *Qmixer_QSWaveMixSetListenerVelocity;   // 0x6fd2b0
    void *Qmixer_QSWaveMixSetSourcePosition;   // 0x6fd2b4
    void *Qmixer_QSWaveMixSetPosition;   // 0x6fd2b8
    void *Qmixer_QSWaveMixGetSourcePosition;   // 0x6fd2bc
    void *Qmixer_QSWaveMixRestartChannel;   // 0x6fd2c0
    void *Qmixer_QSWaveMixPauseChannel;   // 0x6fd2c4
    void *Qmixer_QSWaveMixStopChannel;   // 0x6fd2c8
    void *Qmixer_QSWaveMixConfigureChannel;   // 0x6fd2cc
    void *Qmixer_QSWaveMixOpenWaveEx;   // 0x6fd2d0
    void *Qmixer_QSWaveMixFreeWave;   // 0x6fd2d4
    void *Qmixer_QSWaveMixPlayEx;   // 0x6fd2d8
    void *Qmixer_QSWaveMixSetPanRate;   // 0x6fd2dc
    void *Qmixer_QSWaveMixEnableChannel;   // 0x6fd2e0
    void *Qmixer_QSWaveMixCloseSession;   // 0x6fd2e4
    void *Qmixer_QSWaveMixGetDirectSoundBuffer;   // 0x6fd2e8
    void *Qmixer_QSWaveMixInitEx;   // 0x6fd2ec
    void *Qmixer_QSWaveMixActivate;   // 0x6fd2f0
    void *Qmixer_QSWaveMixSetOptions;   // 0x6fd2f4
    void *Qmixer_QSWaveMixSetListenerPosition;   // 0x6fd2f8
    void *Qmixer_QSWaveMixOpenChannel;   // 0x6fd2fc
    void *Qmixer_null;   // 0x6fd300
    // USER32.dll
    void *User32_GetKeyboardLayout;   // 0x6fd304
    void *User32_PostQuitMessage;   // 0x6fd308
    void *User32_ShowCursor;   // 0x6fd30c
    void *User32_GetKeyboardLayoutList;   // 0x6fd310
    void *User32_LoadCursorFromFileA;   // 0x6fd314
    void *User32_ScreenToClient;   // 0x6fd318
    void *User32_MoveWindow;   // 0x6fd31c
    void *User32_GetAsyncKeyState;   // 0x6fd320
    void *User32_GetKeyState;   // 0x6fd324
    void *User32_GetFocus;   // 0x6fd328
    void *User32_GetDoubleClickTime;   // 0x6fd32c
    void *User32_SwapMouseButton;   // 0x6fd330
    void *User32_SetCursor;   // 0x6fd334
    void *User32_GetSystemMetrics;   // 0x6fd338
    void *User32_DestroyWindow;   // 0x6fd33c
    void *User32_DefWindowProcA;   // 0x6fd340
    void *User32_MapVirtualKeyA;   // 0x6fd344
    void *User32_ReleaseDC;   // 0x6fd348
    void *User32_GetDC;   // 0x6fd34c
    void *User32_RegisterWindowMessageA;   // 0x6fd350
    void *User32_PeekMessageA;   // 0x6fd354
    void *User32_TranslateMessage;   // 0x6fd358
    void *User32_DispatchMessageA;   // 0x6fd35c
    void *User32_GetMessageA;   // 0x6fd360
    void *User32_LoadIconA;   // 0x6fd364
    void *User32_ShowWindow;   // 0x6fd368
    void *User32_UpdateWindow;   // 0x6fd36c
    void *User32_GetForegroundWindow;   // 0x6fd370
    void *User32_SystemParametersInfoA;   // 0x6fd374
    void *User32_MessageBoxW;   // 0x6fd378
    void *User32_MessageBoxA;   // 0x6fd37c
    void *User32_SetWindowPos;   // 0x6fd380
    void *User32_AdjustWindowRect;   // 0x6fd384
    void *User32_SetWindowLongA;   // 0x6fd388
    void *User32_GetWindowRect;   // 0x6fd38c
    void *User32_GetClientRect;   // 0x6fd390
    void *User32_RegisterClassA;   // 0x6fd394
    void *User32_CreateWindowExA;   // 0x6fd398
    void *User32_null;   // 0x6fd39c
    // USP10.dll
    void *Usp10_ScriptStringFree;   // 0x6fd3a0
    void *Usp10_ScriptStringGetLogicalWidths;   // 0x6fd3a4
    void *Usp10_ScriptStringAnalyse;   // 0x6fd3a8
    void *Usp10_ScriptStringOut;   // 0x6fd3ac
    void *Usp10_ScriptString_pSize;   // 0x6fd3b0
    void *Usp10_null;   // 0x6fd3b4
    // WINMM.dll
    void *Winmm_midiOutPrepareHeader;   // 0x6fd3b8
    void *Winmm_midiOutClose;   // 0x6fd3bc
    void *Winmm_midiOutOpen;   // 0x6fd3c0
    void *Winmm_midiOutGetDevCapsA;   // 0x6fd3c4
    void *Winmm_midiOutGetNumDevs;   // 0x6fd3c8
    void *Winmm_midiOutShortMsg;   // 0x6fd3cc
    void *Winmm_midiOutLongMsg;   // 0x6fd3d0
    void *Winmm_mmioFlush;   // 0x6fd3d4
    void *Winmm_mmioSeek;   // 0x6fd3d8
    void *Winmm_mmioWrite;   // 0x6fd3dc
    void *Winmm_mmioRead;   // 0x6fd3e0
    void *Winmm_mmioOpenA;   // 0x6fd3e4
    void *Winmm_mmioSetBuffer;   // 0x6fd3e8
    void *Winmm_timeGetTime;   // 0x6fd3ec
    void *Winmm_mmioClose;   // 0x6fd3f0
    void *Winmm_null;   // 0x6fd3f4
    // WSOCK32.dll
    void *Wsock32_gethostname;   // 0x6fd3f8 #57
    void *Wsock32_htonl;   // 0x6fd3fc #8
    void *Wsock32_ntohl;   // 0x6fd400 #14
    void *Wsock32_connect;   // 0x6fd404 #4
    void *Wsock32_htons;   // 0x6fd408 #9
    void *Wsock32_gethostbyname;   // 0x6fd40c #52
    void *Wsock32_inet_addr;   // 0x6fd410 #10
    void *Wsock32_socket;   // 0x6fd414 #23
    void *Wsock32_recv;   // 0x6fd418 #16
    void *Wsock32_closesocket;   // 0x6fd41c #3
    void *Wsock32_getsockname;   // 0x6fd420 #6
    void *Wsock32_WSAGetLastError;   // 0x6fd424 #111
    void *Wsock32_send;   // 0x6fd428 #19
    void *Wsock32_WSAStartup;   // 0x6fd42c #115
    void *Wsock32_WSACleanup;   // 0x6fd430 #116
    void *Wsock32_accept;   // 0x6fd434 #1
    void *Wsock32_ntohs;   // 0x6fd438 #15
    void *Wsock32_listen;   // 0x6fd43c #13
    void *Wsock32_bind;   // 0x6fd440 #2
    void *Wsock32_shutdown;   // 0x6fd444 #22
    void *Wsock32_null;   // 0x6fd448
    // ole32.dll
    void *Ole32_CoInitialize;   // 0x6fd44c
    void *Ole32_CoUninitialize;   // 0x6fd450
    void *Ole32_CoCreateInstance;   // 0x6fd454
    void *Ole32_null;   // 0x6fd458
    // weaauthr.dll
    void *Weaauthr_Auth_Release;   // 0x6fd45c ?Release@Auth@@QAEHXZ
    void *Weaauthr_Auth_Dtor;   // 0x6fd460 ??1Auth@@QAE@XZ
    void *Weaauthr_Auth_Connect;   // 0x6fd464 ?Connect@Auth@@QAEHPAD0GP6GXHHPAX1@Z@Z
    void *Weaauthr_Auth_Login;   // 0x6fd468 ?Login@Auth@@QAEKGPAG0PAX@Z
    void *Weaauthr_Auth_Ctor;   // 0x6fd46c ??0Auth@@QAE@XZ
    void *Weaauthr_null;   // 0x6fd470
    // weachatr.dll
    void *Weachatr_Chat_Ctor;   // 0x6fd474 ??0Chat@@QAE@XZ
    void *Weachatr_Chat_RoomInfo;   // 0x6fd478 ?RoomInfo@Chat@@QAEKABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@PAX@Z
    void *Weachatr_Chat_Release;   // 0x6fd47c ?Release@Chat@@QAEHXZ
    void *Weachatr_Chat_SetPlayerData;   // 0x6fd480 ?SetPlayerData@Chat@@QAEKHPAX@Z
    void *Weachatr_Chat_Ignore;   // 0x6fd484 ?Ignore@Chat@@QAEKABV?$basic_string@GU?$char_traits@G@std@@V?$allocator@G@2@@std@@PAX@Z
    void *Weachatr_Chat_AddBuddy;   // 0x6fd488 ?AddBuddy@Chat@@QAEKABV?$basic_string@GU?$char_traits@G@std@@V?$allocator@G@2@@std@@PAX@Z
    void *Weachatr_Chat_Locate;   // 0x6fd48c ?Locate@Chat@@QAEKABV?$basic_string@GU?$char_traits@G@std@@V?$allocator@G@2@@std@@PAX@Z
    void *Weachatr_Chat_Goto;   // 0x6fd490 ?Goto@Chat@@QAEKABV?$basic_string@GU?$char_traits@G@std@@V?$allocator@G@2@@std@@PAX@Z
    void *Weachatr_Chat_CreatePlayer;   // 0x6fd494 ?CreatePlayer@Chat@@QAEKABV?$basic_string@GU?$char_traits@G@std@@V?$allocator@G@2@@std@@HH0PAX@Z
    void *Weachatr_Chat_Connect;   // 0x6fd498 ?Connect@Chat@@QAEHPADGP6GXHPAXAAV?$vector@VArgument@@V?$allocator@VArgument@@@std@@@std@@@Z0@Z
    void *Weachatr_Chat_RemoveBuddy;   // 0x6fd49c ?RemoveBuddy@Chat@@QAEKABV?$basic_string@GU?$char_traits@G@std@@V?$allocator@G@2@@std@@PAX@Z
    void *Weachatr_Chat_Blackmark;   // 0x6fd4a0 ?Blackmark@Chat@@QAEKABV?$basic_string@GU?$char_traits@G@std@@V?$allocator@G@2@@std@@PAX@Z
    void *Weachatr_Chat_UnIgnore;   // 0x6fd4a4 ?UnIgnore@Chat@@QAEKABV?$basic_string@GU?$char_traits@G@std@@V?$allocator@G@2@@std@@PAX@Z
    void *Weachatr_Chat_EarmuffsOn;   // 0x6fd4a8 ?EarmuffsOn@Chat@@QAEKPAX@Z
    void *Weachatr_Chat_EarmuffsOff;   // 0x6fd4ac ?EarmuffsOff@Chat@@QAEKPAX@Z
    void *Weachatr_Chat_RoomShout;   // 0x6fd4b0 ?RoomShout@Chat@@QAEKABV?$basic_string@GU?$char_traits@G@std@@V?$allocator@G@2@@std@@_NPAX@Z
    void *Weachatr_Chat_Tell;   // 0x6fd4b4 ?Tell@Chat@@QAEKABV?$basic_string@GU?$char_traits@G@std@@V?$allocator@G@2@@std@@0_NPAX@Z
    void *Weachatr_Chat_Say;   // 0x6fd4b8 ?Say@Chat@@QAEKABV?$basic_string@GU?$char_traits@G@std@@V?$allocator@G@2@@std@@_NPAX@Z
    void *Weachatr_Chat_NotAFK;   // 0x6fd4bc ?NotAFK@Chat@@QAEKPAX@Z
    void *Weachatr_Chat_EnumPlayers;   // 0x6fd4c0 ?EnumPlayers@Chat@@QAEKPAX@Z
    void *Weachatr_Chat_CreateRoom;   // 0x6fd4c4 ?CreateRoom@Chat@@QAEKABV?$basic_string@GU?$char_traits@G@std@@V?$allocator@G@2@@std@@00ABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@3@HPAX@Z
    void *Weachatr_Chat_AFK;   // 0x6fd4c8 ?AFK@Chat@@QAEKPAX@Z
    void *Weachatr_Chat_MovePlayer;   // 0x6fd4cc ?MovePlayer@Chat@@QAEKHHHPAX@Z
    void *Weachatr_Chat_Page;   // 0x6fd4d0 ?Page@Chat@@QAEKABV?$basic_string@GU?$char_traits@G@std@@V?$allocator@G@2@@std@@PAX@Z
    void *Weachatr_Chat_SetHearingRange;   // 0x6fd4d4 ?SetHearingRange@Chat@@QAEKIPAX@Z
    void *Weachatr_Chat_Dtor;   // 0x6fd4d8 ??1Chat@@QAE@XZ
    void *Weachatr_null;   // 0x6fd4dc
    // weacityr.dll
    void *Weacityr_City_MonitorMessage;   // 0x6fd4e0 ?MonitorMessage@City@@QAEKKPAH0@Z
    void *Weacityr_City_Ctor;   // 0x6fd4e4 ??0City@@QAE@XZ
    void *Weacityr_ccParkList_Release;   // 0x6fd4e8 ?Release@ccParkList@@QAEXXZ
    void *Weacityr_ccCityList_Dtor;   // 0x6fd4ec ??1ccCityList@@QAE@XZ
    void *Weacityr_ccParkList_Dtor;   // 0x6fd4f0 ??1ccParkList@@QAE@XZ
    void *Weacityr_City_Release;   // 0x6fd4f4 ?Release@City@@QAEHXZ
    void *Weacityr_City_GetCurrentMessage;   // 0x6fd4f8 ?GetCurrentMessage@City@@QAEKH@Z
    void *Weacityr_City_UnpublishPark;   // 0x6fd4fc ?UnpublishPark@City@@QAEKPAXHH@Z
    void *Weacityr_ccCityInfo_Dtor;   // 0x6fd500 ??1ccCityInfo@@QAE@XZ
    void *Weacityr_ccParkInfo_Dtor;   // 0x6fd504 ??1ccParkInfo@@QAE@XZ
    void *Weacityr_ccServerInfo_Dtor;   // 0x6fd508 ??1ccServerInfo@@QAE@XZ
    void *Weacityr_ccThemeInfo_Dtor;   // 0x6fd50c ??1ccThemeInfo@@QAE@XZ
    void *Weacityr_ccRideInfo_Dtor;   // 0x6fd510 ??1ccRideInfo@@QAE@XZ
    void *Weacityr_ccRideList_Dtor;   // 0x6fd514 ??1ccRideList@@QAE@XZ
    void *Weacityr_ccRideList_Release;   // 0x6fd518 ?Release@ccRideList@@QAEXXZ
    void *Weacityr_ccServerList_Dtor;   // 0x6fd51c ??1ccServerList@@QAE@XZ
    void *Weacityr_ccServerList_Release;   // 0x6fd520 ?Release@ccServerList@@QAEXXZ
    void *Weacityr_ccThemeList_Dtor;   // 0x6fd524 ??1ccThemeList@@QAE@XZ
    void *Weacityr_ccThemeList_Release;   // 0x6fd528 ?Release@ccThemeList@@QAEXXZ
    void *Weacityr_City_MarkParkVisited;   // 0x6fd52c ?MarkParkVisited@City@@QAEKPAXHH@Z
    void *Weacityr_City_GetLogicalServerList;   // 0x6fd530 ?GetLogicalServerList@City@@QAEKPAXHKH@Z
    void *Weacityr_City_Connect;   // 0x6fd534 ?Connect@City@@QAEHPAD0GP6GXHHPAX1@Z@Z
    void *Weacityr_ccCityList_Release;   // 0x6fd538 ?Release@ccCityList@@QAEXXZ
    void *Weacityr_City_SetSessionInfo;   // 0x6fd53c ?SetSessionInfo@City@@QAEXGHHPAG@Z
    void *Weacityr_City_Dtor;   // 0x6fd540 ??1City@@QAE@XZ
    void *Weacityr_City_GetRideList;   // 0x6fd544 ?GetRideList@City@@QAEKPAXKH@Z
    void *Weacityr_City_GetThemeList;   // 0x6fd548 ?GetThemeList@City@@QAEKPAXKH@Z
    void *Weacityr_City_GetParkByID;   // 0x6fd54c ?GetParkByID@City@@QAEKPAXHKH@Z
    void *Weacityr_City_GetParkInWorldByName;   // 0x6fd550 ?GetParkInWorldByName@City@@QAEKPAXPAGKH@Z
    void *Weacityr_City_GetParkInCityListTop10;   // 0x6fd554 ?GetParkInCityListTop10@City@@QAEKPAXHKH@Z
    void *Weacityr_City_GetParkInCityList;   // 0x6fd558 ?GetParkInCityList@City@@QAEKPAXHKH@Z
    void *Weacityr_City_GetCityList;   // 0x6fd55c ?GetCityList@City@@QAEKPAXKH@Z
    void *Weacityr_null;   // 0x6fd560
    // weamailr.dll
    void *Weamailr_Mail_DoReset;   // 0x6fd564 ?DoReset@Mail@@QAEKXZ
    void *Weamailr_Mail_Quit;   // 0x6fd568 ?Quit@Mail@@QAEKXZ
    void *Weamailr_Mail_DoData;   // 0x6fd56c ?DoData@Mail@@QAEKPAD@Z
    void *Weamailr_Mail_DoTo;   // 0x6fd570 ?DoTo@Mail@@QAEKPAD@Z
    void *Weamailr_Mail_Connect;   // 0x6fd574 ?Connect@Mail@@QAEHPADGP6GXHHPAEPAX@Z20@Z
    void *Weamailr_Mail_DoFrom;   // 0x6fd578 ?DoFrom@Mail@@QAEKPAD@Z
    void *Weamailr_Mail_Ctor;   // 0x6fd57c ??0Mail@@QAE@XZ
    void *Weamailr_null;   // 0x6fd580
    // weanewsr.dll
    void *Weanewsr_News_MonitorMessage;   // 0x6fd584 ?MonitorMessage@News@@QAEKKPAH0@Z
    void *Weanewsr_News_Dtor;   // 0x6fd588 ??1News@@QAE@XZ
    void *Weanewsr_News_Release;   // 0x6fd58c ?Release@News@@QAEHXZ
    void *Weanewsr_News_GetServerNews;   // 0x6fd590 ?GetServerNews@News@@QAEKGPAX@Z
    void *Weanewsr_News_GetGameNews;   // 0x6fd594 ?GetGameNews@News@@QAEKGPAU_GUID@@PAX@Z
    void *Weanewsr_News_GetGameServerDetails;   // 0x6fd598 ?GetGameServerDetails@News@@QAEKGPAU_GUID@@PAX@Z
    void *Weanewsr_News_Connect;   // 0x6fd59c ?Connect@News@@QAEHPAD0GP6GXHHPAX1@Z@Z
    void *Weanewsr_News_Ctor;   // 0x6fd5a0 ??0News@@QAE@XZ
    void *Weanewsr_News_GetCurrentMessage;   // 0x6fd5a4 ?GetCurrentMessage@News@@QAEKH@Z
    void *Weanewsr_null;   // 0x6fd5a8
    // wearasr.dll
    void *Wearasr_Dialup_Initialise;   // 0x6fd5ac ?Initialise@Dialup@@QAEHXZ
    void *Wearasr_Dialup_Release;   // 0x6fd5b0 ?Release@Dialup@@QAEXXZ
    void *Wearasr_Dialup_Connect;   // 0x6fd5b4 ?Connect@Dialup@@QAEHHP6GXHPAX@ZPAG20@Z
    void *Wearasr_Dialup_Disconnect;   // 0x6fd5b8 ?Disconnect@Dialup@@QAEXH@Z
    void *Wearasr_Dialup_GetDialupProperties;   // 0x6fd5bc ?GetDialupProperties@Dialup@@QAEHHPAUDIALUP_PROPERTIES@@@Z
    void *Wearasr_Dialup_GetNoDialUpConnections;   // 0x6fd5c0 ?GetNoDialUpConnections@Dialup@@QAEKXZ
    void *Wearasr_Dialup_Dtor;   // 0x6fd5c4 ??1Dialup@@QAE@XZ
    void *Wearasr_Dialup_Ctor;   // 0x6fd5c8 ??0Dialup@@QAE@XZ
    void *Wearasr_null;   // 0x6fd5cc
    // weauploadr.dll
    void *Weauploadr_Upload_UploadParkRequest;   // 0x6fd5d0 ?UploadParkRequest@Upload@@QAEKPADPAVccParkFileInfo@@PAX@Z
    void *Weauploadr_Upload_GetCurrentMessage;   // 0x6fd5d4 ?GetCurrentMessage@Upload@@QAEKH@Z
    void *Weauploadr_Upload_MonitorMessage;   // 0x6fd5d8 ?MonitorMessage@Upload@@QAEKKPAH0@Z
    void *Weauploadr_Upload_Ctor;   // 0x6fd5dc ??0Upload@@QAE@XZ
    void *Weauploadr_Upload_Connect;   // 0x6fd5e0 ?Connect@Upload@@QAEHPAD0GP6GXHHPAX1@Z@Z
    void *Weauploadr_Upload_SetSessionInfo;   // 0x6fd5e4 ?SetSessionInfo@Upload@@QAEXGHHPAG@Z
    void *Weauploadr_Upload_UploadParkFile;   // 0x6fd5e8 ?UploadParkFile@Upload@@QAEKPADPAX@Z
    void *Weauploadr_Upload_Release;   // 0x6fd5ec ?Release@Upload@@QAEHXZ
    void *Weauploadr_null;   // 0x6fd5f0
    // weavoter.dll
    void *Weavoter_Vote_Ctor;   // 0x6fd5f4 ??0Vote@@QAE@XZ
    void *Weavoter_Vote_Connect;   // 0x6fd5f8 ?Connect@Vote@@QAEHPAD0GP6GXHHPAX1@Z@Z
    void *Weavoter_Vote_SetSessionInfo;   // 0x6fd5fc ?SetSessionInfo@Vote@@QAEXGHHPAG@Z
    void *Weavoter_Vote_SendVote;   // 0x6fd600 ?SendVote@Vote@@QAEKKPAU_GUID@@KPAX@Z
    void *Weavoter_Vote_Release;   // 0x6fd604 ?Release@Vote@@QAEHXZ
    void *Weavoter_Vote_Dtor;   // 0x6fd608 ??1Vote@@QAE@XZ
    void *Weavoter_null;   // 0x6fd60c
};

GAME_DATA(ImportTable, gImports, 0x006fd000, 0x610);

// A slot's address, as the ports' import helpers take it (a u32).
#define IMPORT(member) static_cast<u32>(reinterpret_cast<uintptr_t>(&gImports.member))
