#pragma yanc prname test58
// Realistic embedded example: telemetry command link with CRC-16/CCITT (XModem).
// Frames arrive as words [cmd, arg, crc] where crc = CRC16 over (cmd, arg).
// Valid frames are dispatched to command handlers; corrupted frames are counted.
// Self-check: CRC-16/XMODEM("123456789") must be 0x31C3 (published check value).

typedef unsigned int u32;

u32 crc16_update(u32 crc, u32 data) {
    crc = (crc ^ (data << 8)) & 0xFFFF;
    for (int k = 0; k < 8; k = k + 1) {
        if (crc & 0x8000) crc = ((crc << 1) ^ 0x1021) & 0xFFFF;
        else              crc = (crc << 1) & 0xFFFF;
    }
    return crc;
}

enum Cmd { CMD_NOP, CMD_PING, CMD_SET_GAIN, CMD_READ_TEMP, CMD_STATS };

class Link {
    int gain;
    int temp;
    int good;
    int bad;
public:
    Link() : gain(1), temp(25), good(0), bad(0) {}

    int process(u32 cmd, u32 arg, u32 crc) {
        u32 c = crc16_update(0, cmd);
        c = crc16_update(c, arg);
        if (c != crc) { bad = bad + 1; return -1; }
        good = good + 1;
        switch (cmd) {
            case CMD_PING:      return (int)arg;
            case CMD_SET_GAIN:  gain = (int)arg; return gain;
            case CMD_READ_TEMP: return temp * gain;
            case CMD_STATS:     return good * 100 + bad;
        }
        return -2;
    }
};

void main(void) {
    // CRC algorithm self-check against the published XModem check value.
    u32 c = 0;
    for (int k = 0; k < 9; k = k + 1) c = crc16_update(c, (u32)(0x31 + k));
    out(0, (int)c);                              // expect 12739 (0x31C3)

    Link lk;
    u32 cmds[5] = { CMD_PING, CMD_SET_GAIN, CMD_READ_TEMP, CMD_PING, CMD_STATS };
    u32 args[5] = { 1234,     3,            0,             77,       0 };
    for (int k = 0; k < 5; k = k + 1) {
        u32 crc = crc16_update(crc16_update(0, cmds[k]), args[k]);
        if (k == 3) crc = crc ^ 1;               // frame corrupted in transit
        out(0, lk.process(cmds[k], args[k], crc));
    }
    // expect: 1234 (ping echo), 3 (gain set), 75 (temp 25 * gain 3),
    //         -1 (bad CRC), 401 (good=4, bad=1)
}
