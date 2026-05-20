// Calendar math (date / time). Zeller's congruence for day-of-week and a
// leap-year test. Exercises integer division/modulo and boolean logic.
#pragma yanc prname tst48

// returns 0=Saturday, 1=Sunday, 2=Monday, ... 6=Friday
int zeller(int day, int month, int year) {
    if (month < 3) { month = month + 12; year = year - 1; }
    int K = year % 100;
    int J = year / 100;
    return (day + 13 * (month + 1) / 5 + K + K / 4 + J / 4 + 5 * J) % 7;
}

int is_leap(int y) {
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

void main(void) {
    out(0, zeller(1, 1, 2000));     // 2000-01-01 = Saturday -> 0
    out(0, zeller(13, 10, 2023));   // 2023-10-13 = Friday   -> 6
    out(0, zeller(20, 7, 1969));    // 1969-07-20 = Sunday   -> 1

    out(0, is_leap(2000));          // 1
    out(0, is_leap(1900));          // 0
    out(0, is_leap(2024));          // 1
    out(0, is_leap(2023));          // 0
}
