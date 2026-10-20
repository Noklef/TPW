// The event queue: see events.h.
#include "events.h"

#include <cstdint>

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
const EventQueueEnv kGameEnv = {
    at<EventQueue *>(0x00fa7cb0),
    0x00703648,
    0x0070364c,
    at<void *(__cdecl *)(u32)>(0x0067acc0),
    at<void(__cdecl *)(void *)>(0x0067acb0),
    at<s32(__cdecl *)(void(__cdecl *)())>(0x0067a7d0),
    at<void(__cdecl *)()>(0x005fbe10),
    at<void(__stdcall *const *)(void *)>(0x006fd144),
    at<void(__stdcall *const *)(void *)>(0x006fd0d0),
    at<void(__stdcall *const *)(void *)>(0x006fd140),
    at<void(__stdcall *const *)(void *)>(0x006fd130),
};
const EventQueueEnv *g_env = &kGameEnv;

void leave(void *lock) {
    if (lock) (*g_env->leave)(lock);
}

// The base list's destructor, inlined into both destructors.
void destroyList(EventList *l) {
    l->vtable = g_env->baseVtable;
    EventNode *n = l->head;
    l->tail = nullptr;
    l->head = nullptr;
    while (n) {
        EventNode *const next = n->next;
        g_env->free(n);
        n = next;
    }
    l->count = 0;
}

}  // namespace

void EventQueue_SetEnv(const EventQueueEnv *e) { g_env = e ? e : &kGameEnv; }
const EventQueueEnv *EventQueue_GetEnv() { return g_env; }

extern "C" {

void __cdecl EventQueue_StaticInit() {
    EventQueue_Construct();
    EventQueue_RegisterExit();
}

EventQueue *__cdecl EventQueue_Construct() { return EventQueue_Ctor(g_env->instance); }

void __cdecl EventQueue_RegisterExit() { g_env->atExit(g_env->onExit); }

void __cdecl EventQueue_AtExit() { EventQueue_Dtor(g_env->instance); }

EventQueue *__fastcall EventQueue_Ctor(EventQueue *q) {
    q->lock = g_env->alloc(0x18);
    for (EventList &l : q->lists) EventList_Ctor(&l);  // the eh vector constructor iterator
    (*g_env->initialize)(q->lock);
    return q;
}

EventList *__fastcall EventList_Ctor(EventList *l) {
    l->head = nullptr;
    l->tail = nullptr;
    l->count = 0;
    l->vtable = g_env->listVtable;
    return l;
}

void __fastcall EventList_Dtor(EventList *l) { destroyList(l); }

EventList *__fastcall EventList_DeletingDtor(EventList *l, void *, u32 flags) {
    destroyList(l);
    if (flags & 1) g_env->free(l);
    return l;
}

void __fastcall EventQueue_Dtor(EventQueue *q) {
    EventQueue_Clear(q);
    (*g_env->remove)(q->lock);
    g_env->free(q->lock);
    q->lock = nullptr;
    for (int i = kEventChannels - 1; i >= 0; --i) EventList_Dtor(&q->lists[i]);  // the eh vector destructor iterator
}

void __fastcall EventQueue_Post(EventQueue *q, void *, u32 channel, const void *event) {
    void *const lock = q->lock;
    (*g_env->enter)(lock);
    if (channel != kEventNone) {
        for (EventNode *n = q->lists[channel].head; n;) {
            const EventListener *const l = n->listener;
            n = n->next;
            if (l->callback) l->callback(channel, event, l->context);
        }
    }
    leave(lock);
}

void __fastcall EventQueue_GuardRelease(void **guard) {
    leave(*guard);
    *guard = nullptr;
}

void __fastcall EventQueue_Listen(EventQueue *q, void *, u32 channel, void *callback, void *context) {
    void *const lock = q->lock;
    (*g_env->enter)(lock);
    if (channel != kEventNone) {
        EventListener *const l = static_cast<EventListener *>(g_env->alloc(sizeof(EventListener)));
        if (l) {
            l->callback = reinterpret_cast<void(__stdcall *)(u32, const void *, void *)>(callback);
            l->context = context;
        }
        EventList &list = q->lists[channel];
        EventNode *const n = static_cast<EventNode *>(g_env->alloc(sizeof(EventNode)));
        if (n) {
            n->listener = l;
            n->next = nullptr;
            n->prev = nullptr;
        }
        n->prev = list.tail;  // through null when the allocation failed
        if (!list.tail)
            list.head = n;
        else
            list.tail->next = n;
        list.tail = n;
        ++list.count;
    }
    leave(lock);
}

void __fastcall EventQueue_Unlisten(EventQueue *q, void *, u32 channel, void *callback, void *context) {
    void *const lock = q->lock;
    if (!lock) return;
    (*g_env->enter)(lock);
    if (channel != kEventNone) {
        EventList &list = q->lists[channel];
        for (EventNode *n = list.head; n;) {
            EventListener *const l = n->listener;
            EventNode *const next = n->next;
            const bool match = reinterpret_cast<void *>(l->callback) == callback && (!context || l->context == context);
            if (match) {
                if (n == list.head)
                    list.head = next;
                else
                    n->prev->next = n->next;
                if (n == list.tail)
                    list.tail = n->prev;
                else
                    n->next->prev = n->prev;
                --list.count;
                g_env->free(n);
                g_env->free(l);
            }
            n = next;
        }
    }
    leave(lock);
}

void __fastcall EventQueue_Clear(EventQueue *q) {
    void *const lock = q->lock;
    (*g_env->enter)(lock);
    for (EventList &list : q->lists) {
        while (list.count) {
            EventNode *const n = list.head;
            void *listener = nullptr;
            if (n) {
                list.head = n->next;
                if (!list.head)
                    list.tail = nullptr;
                else
                    list.head->prev = nullptr;
                listener = n->listener;
                --list.count;
                g_env->free(n);
            }
            g_env->free(listener);
        }
    }
    leave(lock);
}

EventList *__fastcall EventListBase_DeletingDtor(EventList *l, void *, u32 flags) {
    destroyList(l);
    if (flags & 1) g_env->free(l);
    return l;
}

void __cdecl EventQueue_PostKind4(u32 value) {
    u32 event[5];  // (0x14 bytes, the rest unset as the original's)
    event[0] = 4;
    event[1] = value;
    EventQueue_Post(g_env->instance, nullptr, 0, event);
}

}  // extern "C"
