// The money record's changes (the object at 0x7cdb30 is one): reimplementation of 0x4d0190, 0x4d01f0, 0x4d0260 and others. Named by what they do until
// the class is known.
#pragma once

#include "../../../re/types.h"

struct FileTime;

extern "C" {

// 0x4d0190 (thiscall; amount, pops 4): the balance at +0xc and the total at +0x124 gain amount, and so does the dword at +0x1fc90 of the
// thing the game object's (the dword at 0x7cf83c) handle at +0x1da720 names (the debug print of an amount over 1000000 is not
// reproduced).
void __fastcall Money_Credit(u8 *money, void *, s32 amount);

// 0x4d01f0 (thiscall; amount, pops 4): nothing unless the dword at +0x114 is set; then the balance at +0xc loses amount and is copied to +0x11c; going below 0
// while the dword there was not below 0 records the time (the dword at +0x1da70c of the object [0x80239c]) at +0x120; the dword at +0x1f5a0 of that thing gains amount and the total at +0x124 loses it.
void __fastcall Money_Debit(u8 *money, void *, s32 amount);

// 0x4d0750 (thiscall; index, pops 4): taking out the loan the 0x20-byte record `index` (from the money record) describes: 0 if it is taken already (+0x28 set); else its
// amount (+0x18) is added to the balance at +0xc and to the dword at +0x1fc90 of the game object's thing (as Money_Credit), the total at +0x124 is left as it was, the loan
// is marked taken (+0x28 = 1) and 1 is returned. (The debug messages are not reproduced.)
u32 __fastcall Money_TakeLoan(u8 *money, void *, u32 index);

// 0x4d0850 (thiscall; index, pops 4): paying a loan off: 0 if it was not taken or the balance (+0xc, unsigned) is below what is owed, (the first dword of the next record -
// the months paid at +0x2c) * the rate at +0x24; else, when the dword at +0x114 is set, that is debited as Money_Debit; the loan is unmarked (+0x28 and +0x2c cleared) and
// the total at +0x124 gains what is owed less ((term * rate - amount) / term, unsigned) * term, where term is the next record's first dword (none when it is 0). Returns 1.
u32 __fastcall Money_PayOffLoan(u8 *money, void *, u32 index);

// 0x4d09a0 (thiscall; nothing calls it): the loan owing least. Over the 8 loan entries of 0x20 bytes from +0x20 (owed = (term +0 -
// months paid +0xc) * rate +4 when taken, +8, is set, else 0), the index of the smallest non-zero amount owed (unsigned; the first of
// equals), or -1 for none. (Entry i is the loan Money_TakeLoan's index i names: its +0x24, +0x28 and +0x2c, and the next record's
// first dword as the term, as Money_PayOffLoan reads them.)
s32 __fastcall Obj_SmallestEntry(const u8 *object);

// 0x4d0970 (thiscall (loan), pops 4; called by 0x59e975; name invented): the amount the loan owes, as Obj_SmallestEntry reckons it:
// (term - months paid) * rate when taken, else 0.
u32 __fastcall Money_LoanOwed(const u8 *money, void *, u32 loan);

// 0x4d0a00 (thiscall; called by 0x59e93b; name invented): the eight loans' amounts owed added up (Money_LoanOwed's, wrapping).
u32 __fastcall Money_TotalOwed(const u8 *money);

// 0x4d06b0 (thiscall (out, index), pops 8; name invented): the loan offer `index` (the 0x20-byte record at +0x14 + 0x20 * index, whose +4
// is its amount) checked against the park's credit: the analyser's score (the table 0x7cfb90's entry for the world's [0x80239c] word
// +0x1da720; ParkAnalyser_ParkScore, at most 0x14 quarters old) times the float [0x700370] (x87, __ftol's low dword) less what the loans
// owe (Money_TotalOwed, inlined in the original). The record's first dword becomes 1 when its amount is within that (signed), else 0, and
// the record is copied to out, which is returned.
u32 *__fastcall Money_LoanOffer(u8 *money, void *, u32 *out, u32 index);

// 0x4d0810 (thiscall; name invented): 1 when one of the eight offers (Money_LoanOffer, in turn) is not taken (+0x14 0) and within the
// credit (+0 set), else 0.
u32 __fastcall Money_AnyLoanOffer(u8 *money);

// 0x4d09f0 (thiscall): the total at +0x124.
u32 __fastcall Money_Total(const u8 *money);

// 0x4d0260 (thiscall; out, pops 4): how long the balance has been in the red: 0 unless the balance copy at +0x11c is below 0, else the
// world's funny time span since the time it went below (+0x120, the world's ticks then; FunnyTime_SpanSince). Returns out.
FileTime *__fastcall Money_TimeInRed(const u8 *money, void *edx, FileTime *out);

// 0x4d0370 (thiscall; name invented): the month's end. The income pending at +0x10 is credited (as Money_Credit) and cleared; each of the
// eight loans taken (as Money_LoanOwed reads them: +0x28 set) pays its rate +0x24 (as Money_Debit) and a month (+0x2c), the total +0x124
// gaining rate - (rate * term - amount +0x18) / term (unsigned; not when the term, the next record's first dword, is 0), and a loan paid up
// (months == term) is cleared (+0x28 and +0x2c = 0). Then, the balance +0xc below 0, when it has been in the red (Money_TimeInRed) for 6
// or more months of 30 days (the quotient's low dword, signed), message 0x13 (2) is posted (Object_PostMessage). (The original has those
// three inline, and its debug prints are not reproduced.)
void __fastcall Money_MonthEnd(u8 *money);

// BankAccount_HandleMessage's call into agent A's unported 0x5168f0 (thiscall on the game object [0x7cf83c]: the game lost; mode
// +0x1da738 = 4). The game's by default; the fncmp driver points it at a recorder.
struct BankAccountEnv {
    void(__fastcall *gameOver)(u8 *game);
};
void BankAccount_SetEnv(const BankAccountEnv *env);

// 0x4d02d0 (thiscall (message), pops 4): the money record's messages, by the message's type (its vtable's first function): 0xc the month's
// end (Money_MonthEnd); 0xd the total +0x124 zeroed; 0x13 with +8 2 the game lost (BankAccountEnv) unless the game object's mode
// +0x1da738 is 4 already or the word 0x7cf4f4 is set, with +8 0xa the balance +0xc = the starting money 0x7851d8 and +0x10 = 0.
void __fastcall BankAccount_HandleMessage(u8 *money, void *, u8 *message);

}  // extern "C"
