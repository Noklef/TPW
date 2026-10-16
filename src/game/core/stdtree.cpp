#include "stdtree.h"

#include <cstdint>

#include "gameheap.h"
#include "string.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

constexpr u32 kRed = 0, kBlack = 1;
enum Value { kPlain, kStdString, kString };

// A tree whose nil node is the global pointer at `nilAt` and whose nodes keep their colour at +`colourAt`.
template <u32 nilAt, u32 colourAt, Value value, u32 counterAt, u32 nodeSize> struct Tree {
    static u8 *Nil() { return at<u8 *>(reinterpret_cast<void *>((uintptr_t)nilAt), 0); }
    static u8 *&L(u8 *n) { return at<u8 *>(n, 0); }
    static u8 *&P(u8 *n) { return at<u8 *>(n, 4); }
    static u8 *&R(u8 *n) { return at<u8 *>(n, 8); }
    static u32 &C(u8 *n) { return at<u32>(n, colourAt); }

    // The next node in order (++iterator).
    static void Inc(u8 **it) {
        u8 *right = R(*it);
        if (right == Nil()) {
            u8 *parent = P(*it);
            if (*it == R(parent)) {
                do {
                    *it = parent;
                    parent = P(parent);
                } while (*it == R(parent));
            }
            if (R(*it) != parent)
                *it = parent;
        } else {
            for (u8 *n = L(right); n != Nil(); n = L(n))
                right = n;
            *it = right;
        }
    }

    // The previous node in order (--iterator); the end (the head node, which is red and its own grandparent) goes to the rightmost.
    static void Dec(u8 **it) {
        u8 *node = *it;
        if (C(node) == kRed && P(P(node)) == node) {
            *it = R(node);
        } else if (L(node) == Nil()) {
            u8 *parent = P(node);
            if (*it == L(parent)) {
                do {
                    *it = parent;
                    parent = P(parent);
                } while (*it == L(parent));
            }
            *it = parent;
        } else {
            u8 *n = L(node);
            for (u8 *m = R(n); m != Nil(); m = R(m))
                n = m;
            *it = n;
        }
    }

    // The tree object's constructor: the nil node on first use, then the head node.
    static void Ctor(u8 *tree) {
        u8 *&nil = at<u8 *>(reinterpret_cast<void *>((uintptr_t)nilAt), 0);
        if (!nil) {
            u8 *node = static_cast<u8 *>(GameHeap_Alloc(nodeSize));
            P(node) = nullptr;
            C(node) = kBlack;
            nil = node;
            L(node) = nullptr;
            R(node) = nullptr;
        }
        u8 *theNil = nil;
        at<u32>(reinterpret_cast<void *>((uintptr_t)counterAt), 0) += 1;
        u8 *head = static_cast<u8 *>(GameHeap_Alloc(nodeSize));
        P(head) = theNil;
        C(head) = kRed;
        at<u8 *>(tree, 4) = head;
        at<u32>(tree, 0xc) = 0;
        L(head) = head;
        R(head) = head;
    }

    // Inserts a new node for `value` under y (the node made by NewNode, which copies the value and makes the node's links nil): x is not nil for an insertion to the left.
    template <typename V, typename Less> static u8 **Insert(u8 *tree, u8 **out, u8 *x, u8 *y, const V *value, Less less) {
        u8 *z = static_cast<u8 *>(GameHeap_Alloc(nodeSize));
        P(z) = y;
        C(z) = kRed;
        L(z) = Nil();
        R(z) = Nil();
        *reinterpret_cast<V *>(z + 0xc) = *value;
        at<u32>(tree, 0xc) += 1;
        u8 *head = at<u8 *>(tree, 4);
        if (y == head || x != Nil() || less(value, y)) {
            L(y) = z;
            if (y == head) {
                P(head) = z;
                R(head) = z;
            } else if (y == L(head)) {
                L(head) = z;
            }
        } else {
            R(y) = z;
            if (y == R(head))
                R(head) = z;
        }
        for (x = z; x != P(head) && C(P(x)) == kRed;) {
            u8 *parent = P(x);
            if (parent == L(P(parent))) {
                u8 *uncle = R(P(parent));
                if (C(uncle) == kRed) {
                    C(parent) = kBlack;
                    C(uncle) = kBlack;
                    C(P(P(x))) = kRed;
                    x = P(P(x));
                } else {
                    if (x == R(parent)) {
                        x = parent;
                        Lrotate(head, x);
                    }
                    C(P(x)) = kBlack;
                    C(P(P(x))) = kRed;
                    Rrotate(head, P(P(x)));
                }
            } else {
                u8 *uncle = L(P(parent));
                if (C(uncle) == kRed) {
                    C(parent) = kBlack;
                    C(uncle) = kBlack;
                    C(P(P(x))) = kRed;
                    x = P(P(x));
                } else {
                    if (x == L(parent)) {
                        x = parent;
                        Rrotate(head, x);
                    }
                    C(P(x)) = kBlack;
                    C(P(P(x))) = kRed;
                    Lrotate(head, P(P(x)));
                }
            }
        }
        C(P(head)) = kBlack;
        *out = z;
        return out;
    }

    static void Lrotate(u8 *head, u8 *x) {
        u8 *y = R(x);
        R(x) = L(y);
        if (L(y) != Nil())
            P(L(y)) = x;
        P(y) = P(x);
        if (x == P(head))
            P(head) = y;
        else if (x == L(P(x)))
            L(P(x)) = y;
        else
            R(P(x)) = y;
        L(y) = x;
        P(x) = y;
    }

    static void Rrotate(u8 *head, u8 *x) {
        u8 *y = L(x);
        L(x) = R(y);
        if (R(y) != Nil())
            P(R(y)) = x;
        P(y) = P(x);
        if (x == P(head))
            P(head) = y;
        else if (x == R(P(x)))
            R(P(x)) = y;
        else
            L(P(x)) = y;
        R(y) = x;
        P(x) = y;
    }

    static u8 **Erase(u8 *tree, u8 **out, u8 *erased) {
        u8 *head = at<u8 *>(tree, 4);
        u8 *next = erased;
        Inc(&next);
        u8 *x = erased;
        u8 *y = x;
        u8 *z;
        u8 **yRightField = &R(x);
        if (L(x) == Nil()) {
            z = R(x);
        } else {
            z = L(x);
            if (R(x) != Nil()) {
                y = R(x);
                for (u8 *n = L(y); n != Nil(); n = L(n))
                    y = n;
                z = R(y);
                yRightField = &R(y);
            }
        }
        u8 *oldY = y;
        if (y == x) {
            P(z) = P(x);
            if (P(head) == x) {
                P(head) = z;
            } else {
                u8 *parent = P(x);
                if (L(parent) == x)
                    L(parent) = z;
                else
                    R(parent) = z;
            }
            if (L(head) == x) {
                if (R(x) == Nil()) {
                    L(head) = P(x);
                } else {
                    u8 *low = z;
                    for (u8 *n = L(z); n != Nil(); n = L(n))
                        low = n;
                    L(head) = low;
                }
            }
            if (R(head) == x) {
                if (L(x) == Nil()) {
                    R(head) = P(x);
                } else {
                    u8 *high = z;
                    for (u8 *n = R(z); n != Nil(); n = R(n))
                        high = n;
                    R(head) = high;
                }
            }
        } else {
            P(L(x)) = y;
            L(y) = L(x);
            if (y == R(x)) {
                P(z) = y;
            } else {
                P(z) = P(y);
                L(P(y)) = z;
                *yRightField = R(x);
                P(R(x)) = y;
            }
            if (P(head) == x) {
                P(head) = y;
            } else {
                u8 *parent = P(x);
                if (L(parent) == x)
                    L(parent) = y;
                else
                    R(parent) = y;
            }
            y = x;
            P(oldY) = P(x);
            const u32 colour = C(oldY);
            C(oldY) = C(x);
            C(x) = colour;
        }
        if (C(y) == kBlack) {
            while (z != P(head) && C(z) == kBlack) {
                u8 *w;
                if (z == L(P(z))) {
                    w = R(P(z));
                    if (C(w) == kRed) {
                        C(w) = kBlack;
                        C(P(z)) = kRed;
                        Lrotate(head, P(z));
                        w = R(P(z));
                    }
                    if (C(L(w)) != kBlack || C(R(w)) != kBlack) {
                        if (C(R(w)) == kBlack) {
                            C(L(w)) = kBlack;
                            C(w) = kRed;
                            Rrotate(head, w);
                            w = R(P(z));
                        }
                        C(w) = C(P(z));
                        C(P(z)) = kBlack;
                        C(R(w)) = kBlack;
                        Lrotate(head, P(z));
                        break;
                    }
                } else {
                    w = L(P(z));
                    if (C(w) == kRed) {
                        C(w) = kBlack;
                        C(P(z)) = kRed;
                        Rrotate(head, P(z));
                        w = L(P(z));
                    }
                    if (C(R(w)) != kBlack || C(L(w)) != kBlack) {
                        if (C(L(w)) == kBlack) {
                            C(R(w)) = kBlack;
                            C(w) = kRed;
                            Lrotate(head, w);
                            w = L(P(z));
                        }
                        C(w) = C(P(z));
                        C(P(z)) = kBlack;
                        C(L(w)) = kBlack;
                        Rrotate(head, P(z));
                        break;
                    }
                }
                C(w) = kRed;
                z = P(z);
            }
            C(z) = kBlack;
        }
        if (value == kStdString) {  // (the string's block, freed 2 bytes before its characters, sic: its count byte is 1 before them)
            if (u8 *chars = at<u8 *>(y, 0x10)) {
                const u8 count = chars[-1];
                if (count == 0 || count == 0xff)
                    GameHeap_Free(chars - 2);
                else
                    chars[-1] = count - 1;
            }
            at<u32>(y, 0x10) = 0;
            at<u32>(y, 0x14) = 0;
            at<u32>(y, 0x18) = 0;
        }
        else if (value == kString)
            String_Dtor(reinterpret_cast<String *>(y + 0xc));
        GameHeap_Free(y);
        at<u32>(tree, 0xc) -= 1;
        *out = next;
        return out;
    }
};

using TreeA = Tree<0x007b4a54, 0x18, kPlain, 0x007b4a50, 0x1c>;
using TreeB = Tree<0x00785de4, 0x10, kPlain, 0x00785de0, 0x14>;
using TreeC = Tree<0x007ced2c, 0x20, kStdString, 0x007ced30, 0x24>;
using TreeD = Tree<0x007ced24, 0x1c, kStdString, 0x007ced28, 0x20>;
using TreeE = Tree<0x007ced34, 0x1c, kStdString, 0x007ced38, 0x20>;
using TreeF = Tree<0x00f7c968, 0x24, kString, 0x00f7c96c, 0x28>;
using TreeG = Tree<0x00f7c970, 0x10, kPlain, 0x00f7c96c, 0x14>;

}  // namespace

extern "C" {

void __fastcall StdTree_IncA(u8 **it) { TreeA::Inc(it); }
void __fastcall StdTree_IncB(u8 **it) { TreeB::Inc(it); }
void __fastcall StdTree_IncC(u8 **it) { TreeC::Inc(it); }
void __fastcall StdTree_IncD(u8 **it) { TreeD::Inc(it); }
void __fastcall StdTree_IncE(u8 **it) { TreeE::Inc(it); }
void __fastcall StdTree_IncF(u8 **it) { TreeF::Inc(it); }
void __fastcall StdTree_IncG(u8 **it) { TreeG::Inc(it); }

void __fastcall StdTree_DecA(u8 **it) { TreeA::Dec(it); }
void __fastcall StdTree_DecC(u8 **it) { TreeC::Dec(it); }
void __fastcall StdTree_DecD(u8 **it) { TreeD::Dec(it); }
void __fastcall StdTree_DecE(u8 **it) { TreeE::Dec(it); }
void __fastcall StdTree_DecF(u8 **it) { TreeF::Dec(it); }
void __fastcall StdTree_DecG(u8 **it) { TreeG::Dec(it); }

void __fastcall StdTree_CtorA(u8 *tree) { TreeA::Ctor(tree); }
void __fastcall StdTree_CtorC(u8 *tree) { TreeC::Ctor(tree); }
void __fastcall StdTree_CtorD(u8 *tree) { TreeD::Ctor(tree); }
void __fastcall StdTree_CtorE(u8 *tree) { TreeE::Ctor(tree); }
void __fastcall StdTree_CtorG(u8 *tree) { TreeG::Ctor(tree); }

u8 **__fastcall StdTree_InsertA(u8 *tree, void *, u8 **out, u8 *x, u8 *y, const u32 *value) {
    struct Node3 { u32 v[3]; };
    return TreeA::Insert<Node3>(tree, out, x, y, reinterpret_cast<const Node3 *>(value), [](const Node3 *v, u8 *node) { return v->v[0] < at<u32>(node, 0xc); });
}

u8 **__fastcall StdTree_InsertG(u8 *tree, void *, u8 **out, u8 *x, u8 *y, const u16 *value) {
    return TreeG::Insert<u16>(tree, out, x, y, value, [](const u16 *v, u8 *node) { return *v < at<u16>(node, 0xc); });
}

u8 **__fastcall StdTree_EraseA(u8 *tree, void *, u8 **out, u8 *node) { return TreeA::Erase(tree, out, node); }
u8 **__fastcall StdTree_EraseB(u8 *tree, void *, u8 **out, u8 *node) { return TreeB::Erase(tree, out, node); }
u8 **__fastcall StdTree_EraseC(u8 *tree, void *, u8 **out, u8 *node) { return TreeC::Erase(tree, out, node); }
u8 **__fastcall StdTree_EraseD(u8 *tree, void *, u8 **out, u8 *node) { return TreeD::Erase(tree, out, node); }
u8 **__fastcall StdTree_EraseE(u8 *tree, void *, u8 **out, u8 *node) { return TreeE::Erase(tree, out, node); }
u8 **__fastcall StdTree_EraseF(u8 *tree, void *, u8 **out, u8 *node) { return TreeF::Erase(tree, out, node); }
u8 **__fastcall StdTree_EraseG(u8 *tree, void *, u8 **out, u8 *node) { return TreeG::Erase(tree, out, node); }

}  // extern "C"
