// Dynamic allocation on the 4K heap: new / delete (desugar to malloc/free),
// new[] arrays, free-list reuse, and a heap-built singly linked list. (Uses
// `struct Node` spelling for now; the bare class-name shorthand arrives with
// tier-1 classes.)
#pragma yanc prname test3

struct Node { int val; struct Node* next; };

void main(void) {
    int* p = new int;
    *p = 42;
    out(0, *p);                  // 42
    delete p;

    int* arr = new int[5];
    for (int i = 0; i < 5; i = i + 1) arr[i] = i * i;
    out(0, arr[4]);              // 16
    out(0, arr[2] + arr[3]);     // 13

    struct Node* head = 0;
    for (int i = 4; i >= 1; i = i - 1) {
        struct Node* n = new struct Node;
        n->val = i;
        n->next = head;
        head = n;
    }
    int sum = 0;
    for (struct Node* q = head; q != 0; q = q->next) sum = sum + q->val;
    out(0, sum);                 // 10  (1+2+3+4)

    delete[] arr;
    out(0, 7);                   // 7  (reached the end)
}
