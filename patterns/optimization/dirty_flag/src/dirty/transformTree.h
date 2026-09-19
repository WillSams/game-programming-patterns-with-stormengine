#pragma once

#include <cstddef>
#include <vector>

// ── Dirty Flag pattern: do the work when someone asks, not when it changes ───
//
// The chapter's problem: a parent moves, and every descendant's world position is
// now wrong. Recomputing all of them every frame walks the whole tree for the sake
// of one node; recomputing eagerly when a node changes walks it every time a node
// changes. The flag DEFERS it -- mark the subtree out of date, and recompute a node
// only when something actually reads it.
//
// ⚠️ AND THE PATTERN'S REAL DANGER IS NOT PERFORMANCE. It is that the flag is easy
// to forget to set, and the consequence is a SILENT WRONG ANSWER -- no crash, no
// warning, just a stale number. So a missed `MarkSubtree` here would not fail
// anything loudly: it would draw a box in the wrong place forever. That is why the
// specs count recomputations (to prove the deferral is real) AND check the lazy
// result against a hand-walked ground truth (to prove it is not stale).
//
// Two things this implementation owes the reader:
//
//   * A node reads as dirty from the moment it exists. A value that has never been
//     computed is not clean, and a flag that starts `false` is the classic bug.
//   * The cached world value is PRIVATE to `WorldX`. Nothing else may read it,
//     because anything else could read it while it is stale.
namespace dirty {

class DirtyFlag {
public:
    bool IsDirty() const { return dirty_; }
    void MarkDirty() { dirty_ = true; }
    void Clear() { dirty_ = false; }

private:
    // ⚠️ STARTS SET. "Never computed" is not "up to date".
    bool dirty_ = true;
};

class TransformTree {
public:
    // ── Building. A root has no parent; a child's local offset is relative to its
    // parent, which is the whole reason a parent's move invalidates a child.
    int AddRoot(float localX, float localY) {
        Node n;
        n.parent = -1;
        n.localX = localX;
        n.localY = localY;
        nodes_.push_back(n);
        children_.push_back({});
        return static_cast<int>(nodes_.size()) - 1;
    }

    int AddChild(int parent, float localX, float localY) {
        if (parent < 0 || parent >= static_cast<int>(nodes_.size()))
            return -1;                       // an invalid parent is refused, not stored
        Node n;
        n.parent = parent;
        n.localX = localX;
        n.localY = localY;
        nodes_.push_back(n);
        children_.push_back({});
        const int index = static_cast<int>(nodes_.size()) - 1;
        children_[static_cast<std::size_t>(parent)].push_back(index);
        return index;
    }

    std::size_t size() const { return nodes_.size(); }

    // ── A WRITE. It does no arithmetic at all -- it marks this node and everything
    // under it out of date and returns. That is the pattern: the cost of a change no
    // longer scales with how much depends on it.
    void SetLocal(int i, float localX, float localY) {
        if (!Valid(i))
            return;
        nodes_[static_cast<std::size_t>(i)].localX = localX;
        nodes_[static_cast<std::size_t>(i)].localY = localY;
        ++sets_;
        MarkSubtree(i);
    }

    // ── A READ, and the only way the cached world value is ever touched. A stale
    // node is brought up to date HERE, which is what makes a missed MarkDirty the
    // only way to get a wrong answer.
    float WorldX(int i) {
        ++reads_;
        if (!Valid(i))
            return 0.0f;
        Resolve(i);
        return nodes_[static_cast<std::size_t>(i)].worldX;
    }

    float WorldY(int i) {
        ++reads_;
        if (!Valid(i))
            return 0.0f;
        Resolve(i);
        return nodes_[static_cast<std::size_t>(i)].worldY;
    }

    // ── The counters. A demo and a spec both need to see the DEFERRAL rather than
    // infer it: recomputations are the work the flag avoided, and reads are the work
    // that could have caused it.
    std::size_t Recomputes() const { return recomputes_; }
    std::size_t Reads() const { return reads_; }
    std::size_t Sets() const { return sets_; }
    void ResetCounters() { recomputes_ = reads_ = sets_ = 0; }

    bool IsDirty(int i) const {
        return Valid(i) && nodes_[static_cast<std::size_t>(i)].flag.IsDirty();
    }

    std::size_t DirtyCount() const {
        std::size_t n = 0;
        for (const Node &node : nodes_)
            if (node.flag.IsDirty())
                ++n;
        return n;
    }

    int Parent(int i) const {
        return Valid(i) ? nodes_[static_cast<std::size_t>(i)].parent : -1;
    }

    // The LOCAL offset, not the resolved one -- what was set, not what came out.
    float LocalX(int i) const {
        return Valid(i) ? nodes_[static_cast<std::size_t>(i)].localX : 0.0f;
    }
    float LocalY(int i) const {
        return Valid(i) ? nodes_[static_cast<std::size_t>(i)].localY : 0.0f;
    }

private:
    struct Node {
        int       parent = -1;
        float     localX = 0.0f;
        float     localY = 0.0f;
        float     worldX = 0.0f;   // ⚠️ CACHE. Valid only when flag is clean.
        float     worldY = 0.0f;
        DirtyFlag flag;            // starts dirty, so the cache starts INVALID
    };

    bool Valid(int i) const {
        return i >= 0 && i < static_cast<int>(nodes_.size());
    }

    // Mark a node and every descendant. Marking TWICE is harmless, which is what
    // makes the flag idempotent -- and a spec checks the work is not doubled.
    void MarkSubtree(int i) {
        nodes_[static_cast<std::size_t>(i)].flag.MarkDirty();
        for (int child : children_[static_cast<std::size_t>(i)])
            MarkSubtree(child);
    }

    // Bring a node up to date, resolving its parent first. The parent must be clean
    // BEFORE the child's sum is taken, or the child inherits a stale ancestor.
    //
    // ⚠️ RECURSIVE, AND THAT IS THE ONE PLACE THIS PATTERN CAN BITE: a tree deeper
    // than the stack is a crash, and this demo's trees are shallow by construction.
    // An iterative walk (resolve down from the root, then up) is what a deep tree
    // would need.
    void Resolve(int i) {
        const std::size_t index = static_cast<std::size_t>(i);
        if (!nodes_[index].flag.IsDirty())
            return;                      // already clean: no work, which is the point
        const int parent = nodes_[index].parent;
        if (parent >= 0) {
            Resolve(parent);             // ⚠️ parent first
            nodes_[index].worldX = nodes_[static_cast<std::size_t>(parent)].worldX
                                 + nodes_[index].localX;
            nodes_[index].worldY = nodes_[static_cast<std::size_t>(parent)].worldY
                                 + nodes_[index].localY;
        } else {
            nodes_[index].worldX = nodes_[index].localX;
            nodes_[index].worldY = nodes_[index].localY;
        }
        nodes_[index].flag.Clear();
        ++recomputes_;
    }

    std::vector<Node>              nodes_;
    std::vector<std::vector<int>>  children_;
    std::size_t                    recomputes_ = 0;
    std::size_t                    reads_ = 0;
    std::size_t                    sets_ = 0;
};

} // namespace dirty
