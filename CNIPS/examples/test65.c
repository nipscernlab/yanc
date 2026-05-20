// Selection-sort an array of strings through a char** (pointer-to-pointer /
// double indirection). Exercises `char **arr` params, swapping char* slots,
// indexing a char* out of a char** (arr[j]) and then a char out of that
// (words[i][0]), plus a hand-rolled strcmp over const char*.
#pragma yanc prname tst65

int str_cmp(const char *a, const char *b) {
    while (*a != '\0' && *a == *b) { a = a + 1; b = b + 1; }
    return *a - *b;
}

void sort_strings(char **arr, int n) {
    int i, j, min;
    char *tmp;
    for (i = 0; i < n - 1; i = i + 1) {
        min = i;
        for (j = i + 1; j < n; j = j + 1)
            if (str_cmp(arr[j], arr[min]) < 0) min = j;
        tmp = arr[i]; arr[i] = arr[min]; arr[min] = tmp;
    }
}

void main(void) {
    char *words[5];
    int i;
    words[0] = "pear";
    words[1] = "apple";
    words[2] = "fig";
    words[3] = "cherry";
    words[4] = "banana";
    sort_strings(words, 5);
    // sorted: apple banana cherry fig pear -> first chars a b c f p
    for (i = 0; i < 5; i = i + 1) out(0, words[i][0]);   // 97 98 99 102 112
}
