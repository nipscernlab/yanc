#pragma yanc prname test23
// Realistic embedded example: a byte-stream packet framer.
// A ring buffer feeds a state machine that frames packets of the form:
//   STX | LEN | payload[LEN] | ETX
// On a complete, well-formed frame it emits the payload checksum.
// Exercises implicit-this method calls (full()) and implicit-this member-array
// subscript (buf[tail]), plus switch over an enum class.

enum class State { Idle, Len, Data, End };

const int STX = 2;
const int ETX = 3;

class RingBuffer {
    int buf[16];
    int head, tail, count;
public:
    RingBuffer() : head(0), tail(0), count(0) {}
    bool empty() { return count == 0; }
    bool full()  { return count == 16; }
    void push(int v) {
        if (full()) return;
        buf[tail] = v;
        tail = (tail + 1) % 16;
        count = count + 1;
    }
    int pop() {
        int v = buf[head];
        head = (head + 1) % 16;
        count = count - 1;
        return v;
    }
};

class Framer {
    State st;
    int remaining;
    int sum;
public:
    Framer() : st(State::Idle), remaining(0), sum(0) {}

    // Returns the payload checksum when a frame completes, else -1.
    int feed(int byte) {
        switch (st) {
            case State::Idle:
                if (byte == STX) { st = State::Len; }
                return -1;
            case State::Len:
                remaining = byte;
                sum = 0;
                st = (remaining > 0) ? State::Data : State::End;
                return -1;
            case State::Data:
                sum = sum + byte;
                remaining = remaining - 1;
                if (remaining == 0) st = State::End;
                return -1;
            case State::End:
                st = State::Idle;
                if (byte == ETX) return sum;
                return -1;
        }
        return -1;
    }
};

void main(void) {
    RingBuffer rb;
    // Two back-to-back frames: STX,3,{10,20,30},ETX  then  STX,2,{5,7},ETX
    int stream[12];
    stream[0]=2; stream[1]=3; stream[2]=10; stream[3]=20; stream[4]=30; stream[5]=3;
    stream[6]=2; stream[7]=2; stream[8]=5;  stream[9]=7;  stream[10]=3; stream[11]=99;
    for (int i = 0; i < 12; i = i + 1) rb.push(stream[i]);

    Framer fr;
    while (!rb.empty()) {
        int b = rb.pop();
        int r = fr.feed(b);
        if (r >= 0) out(0, r);   // emit checksum per completed frame
    }
}
