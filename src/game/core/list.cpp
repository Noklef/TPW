// The Exec-style list: see list.h.
#include "list.h"

namespace {

ListNode *tailNode(const List *l) { return reinterpret_cast<ListNode *>(const_cast<ListNode **>(&l->tail)); }

}  // namespace

extern "C" {

List *__cdecl List_Init(List *l) {
    l->tailPred = reinterpret_cast<ListNode *>(l);
    l->head = tailNode(l);
    l->tail = nullptr;
    return l;
}

ListNode *__cdecl List_InsertAfter(List *, ListNode *after, ListNode *node) {
    ListNode *next = after->next;
    node->prev = after;
    node->next = next;
    after->next->prev = node;  // read again, as the original does
    after->next = node;
    return after;
}

ListNode *__cdecl List_First(const List *l) { return l->head != tailNode(l) ? l->head : nullptr; }

}  // extern "C"
