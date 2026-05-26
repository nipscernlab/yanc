// Take the user's y signal (given as IEEE bit-patterns in input40.txt), decode to
// float, and emit it as integers scaled by GAIN (truncated/rounded) — the format
// the YANC port and the matching gcc reference both consume. No IEEE on the
// consumer side; this generator only decodes the originally-supplied values.
#include <stdio.h>
#include <string.h>
#include <math.h>
#define GAIN 30000.0f
int main(int argc, char** argv) {
    FILE* in = fopen(argv[1], "r");
    FILE* out = fopen(argv[2], "w");
    int N; fscanf(in, "%d", &N);                    // N comes from input40.txt's header
    // emit only the N scaled samples - no leading count word (the YANC port and
    // the gcc reference both fix N internally).
    for (int i = 0; i < N; i++) {
        int b; fscanf(in, "%d", &b);
        float f; memcpy(&f, &b, 4);                 // decode the supplied IEEE value
        int scaled = (int)lrintf(f * GAIN);         // y * GAIN, rounded
        fprintf(out, "%d\n", scaled);
    }
    fclose(in); fclose(out);
    return 0;
}
