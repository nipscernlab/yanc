// Generic sort + binary search (algorithms). Exercises function pointers (a
// comparator passed and called indirectly, qsort-style), pointers and arrays.
#pragma yanc prname tst40
#pragma yanc nubits 32
#pragma yanc nbmant 23
#pragma yanc nbexpo 8

int asc(int a, int b)  { return a - b; }   // <0 when a should come first
int desc(int a, int b) { return b - a; }

void isort(int *arr, int n, int (*cmp)(int, int)) {
    for (int i = 1; i < n; i = i + 1) {
        int key = arr[i];
        int j = i - 1;
        while (j >= 0 && cmp(arr[j], key) > 0) {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}

int bsearch_int(int *arr, int n, int target) {
    int lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (arr[mid] == target) return mid;
        if (arr[mid] < target)  lo = mid + 1;
        else                    hi = mid - 1;
    }
    return -1;
}

void main(void) {
    int a[8] = { 5, 2, 8, 1, 9, 3, 7, 4 };

    isort(a, 8, asc);          // 1 2 3 4 5 7 8 9
    out(0, a[0]);              // 1
    out(0, a[7]);              // 9
    out(0, bsearch_int(a, 8, 7));   // 5 (index of 7)
    out(0, bsearch_int(a, 8, 6));   // -1 (absent)

    isort(a, 8, desc);         // 9 8 7 5 4 3 2 1
    out(0, a[0]);              // 9
    out(0, a[7]);              // 1
}
