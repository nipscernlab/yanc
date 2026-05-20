// Binary search tree in a static node pool (data structures without malloc or
// recursion). "Pointers" are pool indices (-1 = null). Iterative insert/search
// and an explicit-stack in-order traversal.
#pragma yanc prname tst55
#define NODES 32

struct node { int key; int left; int right; };
struct node pool[NODES];
int npool = 0;
int root  = -1;

int bst_new(int key) {
    int id = npool; npool = npool + 1;
    pool[id].key = key; pool[id].left = -1; pool[id].right = -1;
    return id;
}

void bst_insert(int key) {
    int id = bst_new(key);
    if (root == -1) { root = id; return; }
    int cur = root;
    for (;;) {
        if (key < pool[cur].key) {
            if (pool[cur].left == -1) { pool[cur].left = id; return; }
            cur = pool[cur].left;
        } else {
            if (pool[cur].right == -1) { pool[cur].right = id; return; }
            cur = pool[cur].right;
        }
    }
}

int bst_search(int key) {
    int cur = root;
    while (cur != -1) {
        if (key == pool[cur].key) return 1;
        cur = (key < pool[cur].key) ? pool[cur].left : pool[cur].right;
    }
    return 0;
}

void main(void) {
    int keys[7] = { 5, 3, 8, 1, 4, 7, 9 };
    for (int i = 0; i < 7; i = i + 1) bst_insert(keys[i]);

    out(0, npool);            // 7
    out(0, bst_search(7));    // 1
    out(0, bst_search(6));    // 0

    int cur = root;
    while (pool[cur].left != -1) cur = pool[cur].left;
    out(0, pool[cur].key);    // 1 (min)

    cur = root;
    while (pool[cur].right != -1) cur = pool[cur].right;
    out(0, pool[cur].key);    // 9 (max)

    // in-order traversal with an explicit stack -> sum of keys
    int stk[32]; int sp = 0; int sum = 0;
    cur = root;
    while (cur != -1 || sp > 0) {
        while (cur != -1) { stk[sp] = cur; sp = sp + 1; cur = pool[cur].left; }
        sp = sp - 1; cur = stk[sp];
        sum = sum + pool[cur].key;
        cur = pool[cur].right;
    }
    out(0, sum);              // 37
}
