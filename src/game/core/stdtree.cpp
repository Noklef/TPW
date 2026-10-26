#include "stdtree.h"

#include <cstdint>

#include "crt.h"
#include "gameheap.h"
#include "stdwstring.h"
#include "string.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

constexpr u32 kRed = 0, kBlack = 1;
enum Value { kPlain, kStdString, kString, kKeyString, kKeyMapI };   // (kKeyString: {a dword key, a String}; kKeyMapI: {a dword key, a tree I object})

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
        return Insert(tree, out, x, y, value, less, [](u8 *to, const V *v) { *reinterpret_cast<V *>(to) = *v; });
    }

    // The same with the value copied into the node (at +0xc) by `copy` (a copy constructor).
    template <typename V, typename Less, typename Copy> static u8 **Insert(u8 *tree, u8 **out, u8 *x, u8 *y, const V *value, Less less, Copy copy) {
        u8 *z = static_cast<u8 *>(GameHeap_Alloc(nodeSize));
        P(z) = y;
        C(z) = kRed;
        L(z) = Nil();
        R(z) = Nil();
        copy(z + 0xc, value);
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
        else if (value == kKeyString)
            String_Dtor(reinterpret_cast<String *>(y + 0x10));
        else if (value == kKeyMapI)  // (the tree's destructor inlined in the originals)
            StdTree_DtorI(y + 0x10);
        GameHeap_Free(y);
        at<u32>(tree, 0xc) -= 1;
        *out = next;
        return out;
    }

    // _Erase: the nodes under n freed, the right subtrees by recursion and down the left by the loop (plain values, or a std::string released as erase does).
    static void EraseSubtree(u8 *n) {
        for (u8 *x = n; x != Nil();) {
            EraseSubtree(R(x));
            u8 *const left = L(x);
            if (value == kStdString) {  // (freed 2 bytes before the characters, sic, as in Erase)
                if (u8 *chars = at<u8 *>(x, 0x10)) {
                    const u8 count = chars[-1];
                    if (count == 0 || count == 0xff)
                        GameHeap_Free(chars - 2);
                    else
                        chars[-1] = count - 1;
                }
                at<u32>(x, 0x10) = 0;
                at<u32>(x, 0x14) = 0;
                at<u32>(x, 0x18) = 0;
            } else if (value == kKeyString) {
                String_Dtor(reinterpret_cast<String *>(x + 0x10));
            } else if (value == kKeyMapI) {
                StdTree_DtorI(x + 0x10);
            }
            GameHeap_Free(x);
            x = left;
        }
    }

    // erase(first, last): a range that is the whole of a tree with nodes (first the leftmost, last the head) is cleared at once (EraseSubtree from the root, then the head made
    // empty again); any other range is erased node by node. *out = last.
    static u8 **EraseRange(u8 *tree, u8 **out, u8 *first, u8 *last) {
        u8 *const head = at<u8 *>(tree, 4);
        if (at<u32>(tree, 0xc) != 0 && first == L(head) && last == head) {
            EraseSubtree(P(head));
            P(head) = Nil();
            at<u32>(tree, 0xc) = 0;
            L(head) = head;
            R(head) = head;
            *out = L(head);
            return out;
        }
        while (first != last) {
            u8 *const node = first;
            Inc(&first);
            u8 *next;
            Erase(tree, &next, node);
        }
        *out = first;
        return out;
    }

    // The tree object's destructor: every node erased, the head node freed and the head and size zeroed; the last tree out (the counter down to 0) frees the nil node.
    static void Dtor(u8 *tree) {
        u8 *const head = at<u8 *>(tree, 4);
        u8 *out;
        EraseRange(tree, &out, L(head), head);
        GameHeap_Free(at<u8 *>(tree, 4));
        at<u32>(tree, 4) = 0;
        at<u32>(tree, 0xc) = 0;
        u32 &count = at<u32>(reinterpret_cast<void *>((uintptr_t)counterAt), 0);
        if (--count == 0) {
            u8 *&nil = at<u8 *>(reinterpret_cast<void *>((uintptr_t)nilAt), 0);
            GameHeap_Free(nil);
            nil = nullptr;
        }
    }
};

using TreeA = Tree<0x007b4a54, 0x18, kPlain, 0x007b4a50, 0x1c>;
using TreeB = Tree<0x00785de4, 0x10, kPlain, 0x00785de0, 0x14>;
using TreeC = Tree<0x007ced2c, 0x20, kStdString, 0x007ced30, 0x24>;
using TreeD = Tree<0x007ced24, 0x1c, kStdString, 0x007ced28, 0x20>;
using TreeE = Tree<0x007ced34, 0x1c, kStdString, 0x007ced38, 0x20>;
using TreeF = Tree<0x00f7c968, 0x24, kString, 0x00f7c964, 0x28>;  // (its users counted at 0xf7c964: 0x5af07e, 0x5af325, 0x5af470)
using TreeG = Tree<0x00f7c970, 0x10, kPlain, 0x00f7c96c, 0x14>;
using TreeH = Tree<0x00fb2008, 0x24, kKeyString, 0x00fb200c, 0x28>;
using TreeI = Tree<0x00fb2010, 0x14, kPlain, 0x00fb2014, 0x18>;
using TreeJ = Tree<0x00fb2000, 0x20, kKeyMapI, 0x00fb2004, 0x24>;

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// The lower bound in a tree whose nodes {left, parent, right, key} end at the sentinel: the last node passed on the way down whose key is not below the key.
template <typename K> u8 *LowerBound(u8 *tree, K key, u32 sentinelAt) {
    u8 *head = at<u8 *>(tree, 4);
    u8 *result = head;
    u8 *sentinel = global<u8 *>(sentinelAt);
    u8 *node = at<u8 *>(head, 4);
    while (node != sentinel) {
        if (at<K>(node, 0xc) < key) {
            node = at<u8 *>(node, 8);
        } else {
            result = node;
            node = at<u8 *>(node, 0);
        }
    }
    return result;
}

// The insert over a tree: Dec is the instantiation's iterator decrement and Insert its node insert; keyBelow(node) is whether the value's key is below the
// node's, nodeBelow(node) whether the node's is below the value's.
template <typename Value, u32 SentinelAt, void(__fastcall *Dec)(u8 **), u8 **(__fastcall *Insert)(u8 *, void *, u8 **, u8 *, u8 *, const Value *), typename KeyBelow,
          typename NodeBelow>
void MapInsertBy(u8 *tree, u8 **result, const Value *value, KeyBelow keyBelow, NodeBelow nodeBelow) {
    u8 *const head = at<u8 *>(tree, 4);
    u8 *const nil = global<u8 *>(SentinelAt);
    u8 *y = head;
    u8 *x = at<u8 *>(head, 4);
    bool below = true;
    while (x != nil) {
        y = x;
        below = keyBelow(x);
        x = at<u8 *>(x, below ? 0 : 8);
    }
    bool insert = at<u8>(tree, 8) != 0;
    u8 *found = y;
    if (!insert) {
        if (below) {
            if (y == at<u8 *>(at<u8 *>(tree, 4), 0))
                insert = true;
            else
                Dec(&found);
        }
        if (!insert && nodeBelow(found))
            insert = true;
    }
    if (insert) {
        u8 *iterator = nullptr;
        u8 **made = Insert(tree, nullptr, &iterator, x, y, value);
        at<u8 *>(result, 0) = *made;
        at<u8>(result, 4) = 1;
        return;
    }
    at<u8 *>(result, 0) = found;
    at<u8>(result, 4) = 0;
}

// The same over a tree whose keys are Key (the first member of the value, at +0xc of a node), compared with <.
template <typename Key, typename Value, u32 SentinelAt, void(__fastcall *Dec)(u8 **), u8 **(__fastcall *Insert)(u8 *, void *, u8 **, u8 *, u8 *, const Value *)>
void MapInsert(u8 *tree, u8 **result, const Value *value) {
    const Key key = *reinterpret_cast<const Key *>(value);
    MapInsertBy<Value, SentinelAt, Dec, Insert>(
        tree, result, value, [key](u8 *node) { return key < at<Key>(node, 0xc); }, [key](u8 *node) { return at<Key>(node, 0xc) < key; });
}

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
void __fastcall StdTree_DecB(u8 **it) { TreeB::Dec(it); }
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

u8 **__fastcall StdTree_InsertB(u8 *tree, void *, u8 **out, u8 *x, u8 *y, const u16 *value) {
    return TreeB::Insert<u16>(tree, out, x, y, value, [](const u16 *v, u8 *node) { return StdTreeB_KeyLess(v, nullptr, reinterpret_cast<const u16 *>(node + 0xc)) != 0; });
}

u8 **__fastcall StdTree_InsertD(u8 *tree, void *, u8 **out, u8 *x, u8 *y, const StdWString *value) {
    return TreeD::Insert<StdWString>(
        tree, out, x, y, value, [](const StdWString *v, u8 *node) { return WString_Compare(reinterpret_cast<const u8 *>(v), nullptr, node + 0xc) < 0; },
        [](u8 *to, const StdWString *v) { StdWString_Construct(reinterpret_cast<StdWString *>(to), v); });
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

void __fastcall StdTree_EraseSubtreeB(u8 *, void *, u8 *node) { TreeB::EraseSubtree(node); }
void __fastcall StdTree_EraseSubtreeC(u8 *, void *, u8 *node) { TreeC::EraseSubtree(node); }
void __fastcall StdTree_EraseSubtreeD(u8 *, void *, u8 *node) { TreeD::EraseSubtree(node); }
void __fastcall StdTree_EraseSubtreeE(u8 *, void *, u8 *node) { TreeE::EraseSubtree(node); }
u8 **__fastcall StdTree_EraseRangeB(u8 *tree, void *, u8 **out, u8 *first, u8 *last) { return TreeB::EraseRange(tree, out, first, last); }
u8 **__fastcall StdTree_EraseRangeC(u8 *tree, void *, u8 **out, u8 *first, u8 *last) { return TreeC::EraseRange(tree, out, first, last); }
u8 **__fastcall StdTree_EraseRangeD(u8 *tree, void *, u8 **out, u8 *first, u8 *last) { return TreeD::EraseRange(tree, out, first, last); }
u8 **__fastcall StdTree_EraseRangeE(u8 *tree, void *, u8 **out, u8 *first, u8 *last) { return TreeE::EraseRange(tree, out, first, last); }
void StdTree_DtorB(u8 *tree) { TreeB::Dtor(tree); }
void StdTree_CtorB(u8 *tree) { TreeB::Ctor(tree); }
void __fastcall StdTree_DtorC(u8 *tree) { TreeC::Dtor(tree); }
void __fastcall StdTree_DtorD(u8 *tree) { TreeD::Dtor(tree); }
void __fastcall StdTree_DtorE(u8 *tree) { TreeE::Dtor(tree); }

u8 *__fastcall Tree16a_LowerBound(u8 *tree, void *, const u16 *key) { return LowerBound<u16>(tree, *key, 0x00785de4); }

u8 *__fastcall Tree32_LowerBound(u8 *tree, void *, const u32 *key) { return LowerBound<u32>(tree, *key, 0x007b4a54); }

u8 *__fastcall Tree16b_LowerBound(u8 *tree, void *, const u16 *key) { return LowerBound<u16>(tree, *key, 0x00f7c970); }

u8 **__fastcall StdTreeC_PostInc(u8 **it, void *, u8 **out, s32) {
    u8 *old = *it;
    StdTree_IncC(it);
    *out = old;
    return out;
}

u8 **__fastcall StdTreeE_PostInc(u8 **it, void *, u8 **out, s32) {
    u8 *old = *it;
    StdTree_IncE(it);
    *out = old;
    return out;
}

u8 **__fastcall Tree16a_Find(u8 *tree, void *, u8 **out, const u16 *key) {
    u8 *found = Tree16a_LowerBound(tree, nullptr, key);
    u8 *end = at<u8 *>(tree, 4);
    *out = (found == end || *key < at<u16>(found, 0xc)) ? end : found;
    return out;
}

u8 **__fastcall Tree32_Find(u8 *tree, void *, u8 **out, const u32 *key) {
    u8 *found = Tree32_LowerBound(tree, nullptr, key);
    u8 *end = at<u8 *>(tree, 4);
    *out = (found == end || *key < at<u32>(found, 0xc)) ? end : found;
    return out;
}

u8 **__fastcall Tree16b_Find(u8 *tree, void *, u8 **out, const u16 *key) {
    u8 *found = Tree16b_LowerBound(tree, nullptr, key);
    u8 *end = at<u8 *>(tree, 4);
    *out = (found == end || *key < at<u16>(found, 0xc)) ? end : found;
    return out;
}

u32 __fastcall Obj_HasKey5af780(u8 *object, void *, u32 key) {
    u16 word = (u16)key;
    u8 *tree = object + 0x28;
    u8 *found = Tree16b_LowerBound(tree, nullptr, &word);
    u8 *end = at<u8 *>(tree, 4);
    u8 *it = (found == end || word < at<u16>(found, 0xc)) ? end : found;
    return it != at<u8 *>(object, 0x2c);
}

void __fastcall StdMapA_Insert(u8 *tree, void *, u8 **result, const u32 *value) {
    MapInsert<u32, u32, 0x007b4a54, StdTree_DecA, StdTree_InsertA>(tree, result, value);
}

void __fastcall StdMapG_Insert(u8 *tree, void *, u8 **result, const u16 *value) {
    MapInsert<u16, u16, 0x00f7c970, StdTree_DecG, StdTree_InsertG>(tree, result, value);
}

u8 **__fastcall StdMapB_Insert(u8 *tree, void *, u8 **result, const u16 *value) {
    MapInsert<u16, u16, 0x00785de4, StdTree_DecB, StdTree_InsertB>(tree, result, value);
    return result;
}

u8 **__fastcall StdMapD_Insert(u8 *tree, void *, u8 **result, const StdWString *value) {
    const u8 *key = reinterpret_cast<const u8 *>(value);
    MapInsertBy<StdWString, 0x007ced24, StdTree_DecD, StdTree_InsertD>(
        tree, result, value,
        [key](u8 *node) {
            const u16 *chars = at<const u16 *>(node, 0x10);
            if (!chars) chars = reinterpret_cast<const u16 *>(0x006fd6c4);
            return WString_CompareSub(key, nullptr, 0, at<u32>(key, 8), chars, at<u32>(node, 0x14)) < 0;
        },
        [key](u8 *node) { return WString_Compare(node + 0xc, nullptr, key) < 0; });
    return result;
}

u8 *__fastcall StdTreeF_LowerBound(u8 *tree, void *, const u8 *key) {
    u8 *result = at<u8 *>(tree, 4);
    u8 *const nil = global<u8 *>(0x00f7c968);
    u8 *node = at<u8 *>(result, 4);
    while (node != nil) {
        if (String_Less(reinterpret_cast<const String *>(node + 0xc), nullptr, reinterpret_cast<const StringView *>(key))) {
            node = at<u8 *>(node, 8);
        } else {
            result = node;
            node = at<u8 *>(node, 0);
        }
    }
    return result;
}

u8 __fastcall StdTreeF_AllFlagged(u8 *object) {
    u8 *const head = at<u8 *>(object, 0x3c);
    u8 *node = at<u8 *>(head, 0);
    if (node == head) return 1;
    do {
        if (at<u8>(at<u8 *>(node, 0x20), 0xb9) == 0) return 0;
        StdTree_IncF(&node);
    } while (node != at<u8 *>(object, 0x3c));
    return 1;
}

u8 **__fastcall StdTreeF_FindLower(u8 *tree, void *, u8 **out, const u8 *key) {
    *out = StdTreeF_LowerBound(tree, nullptr, key);
    return out;
}

u32 *__fastcall StdMapG_InsertCopy(u8 *tree, void *, u32 *out, const u16 *value) {
    u32 result[2];
    StdMapG_Insert(tree, nullptr, reinterpret_cast<u8 **>(result), value);
    at<u32>(out, 0) = result[0];
    at<u8>(out, 4) = (u8)result[1];
    return out;
}

u32 *__fastcall StdMapD_InsertCopy(u8 *tree, void *, u32 *out, const StdWString *value) {
    u32 result[2];
    StdMapD_Insert(tree, nullptr, reinterpret_cast<u8 **>(result), value);
    at<u32>(out, 0) = result[0];
    at<u8>(out, 4) = (u8)result[1];
    return out;
}

u8 __fastcall Object_AddKeyed(u8 *object, void *, u32 key, s32 amount) {
    u32 result[2];
    u16 value = (u16)key;
    StdMapG_Insert(object + 0x28, nullptr, reinterpret_cast<u8 **>(result), &value);
    if ((u8)result[1] == 0) return 0;
    at<s32>(object, 0x1c) += amount;
    return 1;
}

u32 __fastcall StdTreeB_KeyLess(const u16 *key, void *, const u16 *other) { return *key < *other ? 1 : 0; }

u8 *__fastcall StdMapA_Ctor(u8 *map, void *, const u8 *comp, const u8 *alloc) {
    map[0] = *alloc;
    map[1] = *comp;
    map[8] = 0;
    StdTree_CtorA(map);
    return map;
}

u32 *__fastcall StdMapA_Index(u8 *map, void *, const u32 *key) {
    const u32 value[3] = {*key, 0, 0};
    u8 *result[2];
    StdMapA_Insert(map, nullptr, result, value);
    return reinterpret_cast<u32 *>(result[0] + 0x10);
}

void __fastcall StdTree_IncH(u8 **it) { TreeH::Inc(it); }
void __fastcall StdTree_DecH(u8 **it) { TreeH::Dec(it); }

u8 **__fastcall StdTree_InsertH(u8 *tree, void *, u8 **out, u8 *x, u8 *y, const u8 *value) {
    struct Pair { u8 bytes[0x18]; };
    return TreeH::Insert<Pair>(
        tree, out, x, y, reinterpret_cast<const Pair *>(value), [](const Pair *v, u8 *node) { return at<u32>(v, 0) < at<u32>(node, 0xc); },
        [](u8 *to, const Pair *v) { StdPairH_Construct(to, v->bytes); });
}

u8 **__fastcall StdTree_EraseH(u8 *tree, void *, u8 **out, u8 *node) { return TreeH::Erase(tree, out, node); }
void __fastcall StdTree_EraseSubtreeH(u8 *, void *, u8 *node) { TreeH::EraseSubtree(node); }
u8 **__fastcall StdTree_EraseRangeH(u8 *tree, void *, u8 **out, u8 *first, u8 *last) { return TreeH::EraseRange(tree, out, first, last); }
void __fastcall StdTree_DtorH(u8 *tree) { TreeH::Dtor(tree); }

void __fastcall StdTree_IncI(u8 **it) { TreeI::Inc(it); }
u8 **__fastcall StdTree_EraseI(u8 *tree, void *, u8 **out, u8 *node) { return TreeI::Erase(tree, out, node); }
void __fastcall StdTree_EraseSubtreeI(u8 *, void *, u8 *node) { TreeI::EraseSubtree(node); }
u8 **__fastcall StdTree_EraseRangeI(u8 *tree, void *, u8 **out, u8 *first, u8 *last) { return TreeI::EraseRange(tree, out, first, last); }
void __fastcall StdTree_DtorI(u8 *tree) { TreeI::Dtor(tree); }

void __fastcall StdTree_IncJ(u8 **it) { TreeJ::Inc(it); }
void __fastcall StdTree_CtorJ(u8 *tree) { TreeJ::Ctor(tree); }
u8 **__fastcall StdTree_EraseJ(u8 *tree, void *, u8 **out, u8 *node) { return TreeJ::Erase(tree, out, node); }
void __fastcall StdTree_EraseSubtreeJ(u8 *, void *, u8 *node) { TreeJ::EraseSubtree(node); }
u8 **__fastcall StdTree_EraseRangeJ(u8 *tree, void *, u8 **out, u8 *first, u8 *last) { return TreeJ::EraseRange(tree, out, first, last); }

void __fastcall StdMapH_Insert(u8 *tree, void *, u8 **result, const u8 *value) {
    MapInsert<u32, u8, 0x00fb2008, StdTree_DecH, StdTree_InsertH>(tree, result, value);
}

u8 *__fastcall StdMapH_Index(u8 *map, void *, const u32 *key) {
    String empty;
    String_Ctor(&empty);
    struct {
        u32 key;
        String text;
    } value;
    value.key = *key;
    String_CtorCopy(&value.text, nullptr, reinterpret_cast<const StringView *>(&empty));
    u8 *result[2];
    StdMapH_Insert(map, nullptr, result, reinterpret_cast<const u8 *>(&value));
    String_Dtor(&value.text);
    String_Dtor(&empty);
    return result[0] + 0x10;
}

void __cdecl StdPairH_Construct(u8 *where, const u8 *value) {
    if (where)
        StdPairH_Copy(where, nullptr, value);
}

u8 *__fastcall StdPairH_Copy(u8 *pair, void *, const u8 *value) {
    at<u32>(pair, 0) = at<u32>(value, 0);
    String_CtorCopy(reinterpret_cast<String *>(pair + 4), nullptr, reinterpret_cast<const StringView *>(value + 4));
    return pair;
}

u8 *__fastcall StdMapH_AssignWide(u8 *map, void *, const u8 *from, u32 key) {
    String *const text = reinterpret_cast<String *>(StdMapH_Index(map, nullptr, &key));
    return reinterpret_cast<u8 *>(String_AssignWide(text, nullptr, reinterpret_cast<const WString *>(from)));
}

u32 __fastcall StdMapH_FindValue(u8 *map, void *, const u8 *text, u32 *index) {
    for (u32 i = 0; i < at<u32>(map, 0xc); i++) {
        const String *const s = reinterpret_cast<const String *>(StdMapH_Index(map, nullptr, &i));
        if (String_Equals(s, nullptr, reinterpret_cast<const StringView *>(text))) {
            *index = i;
            return 1;
        }
    }
    return 0;
}

void __cdecl StdTree_InitUnusedOne() { *reinterpret_cast<float *>(0x007899b4) = 1.0f; }

void __cdecl StdTree_InitUnusedEpsilon() { *reinterpret_cast<float *>(0x0078a0bc) = 1.0f / 8388608.0f; }
}  // extern "C"
