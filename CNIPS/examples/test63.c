// Recursive merge sort on a singly-linked list (divide & conquer on a linked
// structure). Nodes live in a static pool and `next` is an index (NIL = end),
// so there's no malloc. `merge` and `msort` recurse; the interesting stress is
// `pool[a].next = merge(...)` — an indexed store whose base index is a frame
// local of a recursive function AND whose RHS is a recursive call, so the
// store address must survive across the call. `split` is iterative (fast/slow
// pointers) and stays non-recursive with fast fixed-address locals.
#pragma yanc prname tst63
#pragma yanc sdepth 128
#pragma yanc ndstac 128

#define NIL (-1)

struct node { int val; int next; };
struct node pool[16];
int npool;

int new_node(int v, int nx) {
    int i = npool;
    npool = npool + 1;
    pool[i].val = v;
    pool[i].next = nx;
    return i;
}

int split(int h) {                 // returns head of the second half
    int slow, fast, mid;
    if (h == NIL || pool[h].next == NIL) return NIL;
    slow = h;
    fast = pool[h].next;
    while (fast != NIL && pool[fast].next != NIL) {
        slow = pool[slow].next;
        fast = pool[pool[fast].next].next;
    }
    mid = pool[slow].next;
    pool[slow].next = NIL;         // cut the list in two
    return mid;
}

int merge(int a, int b) {          // merge two sorted index-lists
    if (a == NIL) return b;
    if (b == NIL) return a;
    if (pool[a].val <= pool[b].val) {
        pool[a].next = merge(pool[a].next, b);
        return a;
    }
    pool[b].next = merge(a, pool[b].next);
    return b;
}

int msort(int h) {
    int mid;
    if (h == NIL || pool[h].next == NIL) return h;
    mid = split(h);
    return merge(msort(h), msort(mid));
}

void main(void) {
    int h, p;
    npool = 0;
    h = NIL;                       // build 5,2,8,1,9,3,7,4 (prepend in reverse)
    h = new_node(4, h);
    h = new_node(7, h);
    h = new_node(3, h);
    h = new_node(9, h);
    h = new_node(1, h);
    h = new_node(8, h);
    h = new_node(2, h);
    h = new_node(5, h);
    h = msort(h);
    for (p = h; p != NIL; p = pool[p].next) out(0, pool[p].val);  // 1 2 3 4 5 7 8 9
}
