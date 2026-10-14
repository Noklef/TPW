// The Amiga Exec-style doubly linked list (0x62f9e0-0x62fa33, cdecl): the header doubles as the
// first and last nodes. head is the first node, tail is always 0 and tailPred the last node, so
// &head is the node before the first and &tail the node after the last; an empty list's head
// points at its own tail.
#pragma once

#include "../../../re/types.h"

struct ListNode {
    ListNode *next;
    ListNode *prev;
};
struct List {
    ListNode *head;
    ListNode *tail;  // always 0
    ListNode *tailPred;
};
static_assert(sizeof(List) == 0xc, "List is 0xc bytes");

extern "C" {
List *__cdecl List_Init(List *l);  // 0x62f9e0: empty; returns l
// 0x62fa00 (list, after, node): links node in after `after` (the list is not used); returns after.
ListNode *__cdecl List_InsertAfter(List *l, ListNode *after, ListNode *node);
ListNode *__cdecl List_First(const List *l);  // 0x62fa20: the first node, 0 when empty
}
