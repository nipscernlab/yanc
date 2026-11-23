#!/usr/bin/env python3
"""A random SAPHO program for the instruction-set simulator's comparison (#32).

    python Scripts/iss/rand_asm.py <seed> [--size N] [--itr] [--fmt NU/MAN/EXP]
                                          [--fround L] > rnd.asm

Writes a .asm that asmcomp accepts and that runs to a halt loop (a JMP to
itself) on any input. The seed fixes everything: the format, #FROUND, the
instructions and their operands. --size is the number of blocks of the main
program (default 24); a block is a run of straight-line instructions, an
if, an if/else, a counted loop, a call or a division.

What it keeps to, so that the ISS and the RTL are compared only where the
processor is defined (Scripts/iss/sapho_iss.c says what it does not model):

  - every value has a type, int or float, and an operation only takes the
    type it is for: a float at #FROUND 1 or 2 is then always normalized or the
    canonical zero, as every float operation leaves it;
  - an F_DIV at #FROUND 0 divides by a normalized constant;
  - array indices stay inside the arrays, the data stack below #NDSTAC and
    the calls below #SDEPTH, with no recursion;
  - a label starts with a load (no type is assumed across a jump), and every
    block leaves the data stack as it found it;
  - each division is written as the compilers write it, the division, its
    slots and the read (docs/design/pipeln-and-division.md), with random
    single instructions in the slots;
  - INC and P_INC are followed by a NOP (#203: the next word must not write
    the data memory nor read the word INC writes);
  - with --itr the main program starts at the interrupt address (#ITRAD) with
    a load, so a restart finds no stale accumulator.

TOM and CAD are never written: they need a partner processor.
"""

import argparse
import os
import random

FORMATS = [(32, 23, 8), (23, 16, 6), (16, 10, 5)]
NVARS = 6                 # iv0.., fv0..
ASIZE = 8                 # ia, fa: 2^FFTSIZ words, so ILI/ISI stay inside
MAXSTK = 6                # data-stack words a block may hold
NSUBS = 4


class Gen:
    def __init__(self, seed, size, itr, fmt, fround):
        self.r = random.Random(seed)
        r = self.r
        self.nu, self.man, self.exp = fmt if fmt else r.choice(FORMATS)
        self.fround = fround if fround is not None else r.randrange(3)
        self.itr = itr
        self.size = size
        self.gain = r.choice([2, 16, 128])
        self.idiv = r.random() < 0.6          # a program with DIV/MOD: F_DIV takes 5 words
        # from 32 words the data stack reads its top from a register (#170)
        self.ndstac = r.choice([MAXSTK + 4, 32])
        self.lines = []
        self.pend = []                        # labels for the next instruction
        self.nlab = 0
        self.acc = None                       # 'i', 'f' or None (unknown)
        self.stk = []                         # the types on the data stack
        self.used = set()
        self.loopdepth = 0
        self.base = 0                         # stack words the current block may not pop

    # ----------------------------------------------------------- output ---

    def emit(self, ins):
        lab = ''.join('@%s ' % l for l in self.pend)
        self.pend = []
        self.lines.append(lab + ins)
        self.used.add(ins.split()[0])

    def label(self):
        self.nlab += 1
        return 'L%d' % self.nlab

    def place(self, lab):
        self.pend.append(lab)
        self.acc = None

    # -------------------------------------------------------- constants ---

    def iconst(self):
        r, nu = self.r, self.nu
        k = r.randrange(8)
        if k == 0: return '0'
        if k == 1: return '1'
        if k == 2: return '-1'
        if k == 3: return str(r.choice([-(1 << (nu - 1)), (1 << (nu - 1)) - 1]))
        if k < 6:  return str(r.randrange(-40, 41))
        return str(r.randrange(-(1 << (nu - 1)), 1 << (nu - 1)))

    def fconst(self, nonzero=False):
        r = self.r
        if not nonzero and r.random() < 0.1:
            return '0.0'
        v = r.randrange(1, 1000) / r.choice([1, 2, 4, 8, 10, 100])
        if r.random() < 0.5:
            v = -v
        return '%.4f' % v

    def ivar(self): return 'iv%d' % self.r.randrange(NVARS)
    def fvar(self): return 'fv%d' % self.r.randrange(NVARS)
    def var(self, t): return self.ivar() if t == 'i' else self.fvar()
    def const(self, t): return self.iconst() if t == 'i' else self.fconst()

    # a data operand of type t: a variable, a constant or an array word
    def opnd(self, t):
        r = self.r.random()
        if r < 0.55: return self.var(t)
        if r < 0.8:  return self.const(t)
        return None                           # the caller writes an _V form

    # ------------------------------------------------------------ loads ---

    def load(self, t=None):
        r = self.r
        if t is None:
            t = r.choice('if')
        arr = 'ia' if t == 'i' else 'fa'
        k = r.randrange(9)
        if k < 3:
            self.emit('LOD %s' % self.var(t))
        elif k < 5:
            self.emit('LOD %s' % self.const(t))
        elif k == 5:
            self.emit('LOD_V %s %d' % (arr, r.randrange(ASIZE)))
        elif k == 6:
            off = r.randrange(ASIZE)
            self.emit('LOD %d' % r.randrange(ASIZE - off))
            self.emit('LDI_V %s %d' % (arr, off) if off else 'LDI %s' % arr)
        elif k == 7:
            self.emit('LOD %d' % r.randrange(ASIZE))
            self.emit('ILI %s' % arr)
        else:
            if t == 'i': self.emit('INN %d' % r.randrange(2))
            else:        self.emit('F_INN %d' % r.randrange(2))
        self.acc = t

    def ensure(self, t):
        if self.acc != t:
            self.load(t)

    def ensure_any(self):
        if self.acc is None:
            self.load()

    # ------------------------------------------------- one instruction ---
    # Each returns True when it wrote one instruction (the division slots
    # take only these), or False when it could not with the state as it is.

    INT2 = ['ADD', 'MLT', 'SGN', 'AND', 'ORR', 'XOR', 'LAN', 'LOR', 'LES', 'GRE', 'EQU', 'SHL', 'SHR', 'SRS']
    FLT2 = ['F_ADD', 'F_SU1', 'F_SU2', 'F_MLT', 'F_SGN']
    FCMP = ['F_LES', 'F_GRE']
    INT1 = ['NEG', 'ABS', 'PST', 'NRM', 'INV', 'LIN']
    FLT1 = ['F_NEG', 'F_ABS', 'F_PST', 'F_ROT']

    def one(self):
        r = self.r
        a = self.acc
        # a block pops only what it pushed: the two paths of an if meet with
        # the same stack
        top = self.stk[-1] if len(self.stk) > self.base else None
        cands = []
        if a == 'i':
            cands += ['int2', 'int1', 'i2f', 'shift_s' if top == 'i' else 'int2']
        if a == 'f':
            cands += ['flt2', 'flt1', 'fcmp', 'f2i', 'fscl']
        if a is not None:
            cands += ['unm', 'set', 'set', 'out']
            if len(self.stk) < MAXSTK:
                cands += ['psh', 'punm', 'plod']
        if top is not None and top == a:
            cands += ['s2', 's2']
        if top == 'i' and a == 'f':
            cands += ['sfscl']
        if top is not None and a is not None:
            cands += ['pop', 'setp']
        cands += ['lod', 'inn']
        if a is not None and len(self.stk) < MAXSTK:
            cands += ['pinn']
        c = r.choice(cands)

        if c == 'int2':
            o, m = r.choice(self.INT2), self.opnd('i')
            if m is None: self.emit('%s_V ia %d' % (o, r.randrange(ASIZE)) if o in ('ADD', 'MLT') else '%s %s' % (o, self.ivar()))
            else: self.emit('%s %s' % (o, m))
            self.acc = 'i'
        elif c == 'flt2':
            o, m = r.choice(self.FLT2), self.opnd('f')
            if m is None: self.emit('%s_V fa %d' % (o, r.randrange(ASIZE)) if o in ('F_ADD', 'F_MLT') else '%s %s' % (o, self.fvar()))
            else: self.emit('%s %s' % (o, m))
            self.acc = 'f'
        elif c == 'fcmp':
            self.emit('%s %s' % (r.choice(self.FCMP), self.opnd('f') or self.fvar()))
            self.acc = 'i'
        elif c == 'fscl':
            self.emit('F_SCL %s' % (self.ivar() if r.random() < 0.3 else str(r.randrange(-12, 13))))
            self.acc = 'f'
        elif c == 'int1':
            self.emit(r.choice(self.INT1)); self.acc = 'i'
        elif c == 'flt1':
            self.emit(r.choice(self.FLT1)); self.acc = 'f'
        elif c == 'i2f':
            self.emit('I2F'); self.acc = 'f'
        elif c == 'f2i':
            self.emit(r.choice(['F2I', 'XPO'])); self.acc = 'i'
        elif c in ('unm', 'punm'):
            p = (c == 'punm')
            if p: self.stk.append(self.acc)
            t = r.choice('if')
            if t == 'i':
                o = r.choice(['NEG_M', 'ABS_M', 'PST_M', 'NRM_M', 'INV_M', 'LIN_M', 'I2F_M'])
                self.emit('%s%s %s' % ('P_' if p else '', o, self.opnd('i') or self.ivar()))
                self.acc = 'f' if o == 'I2F_M' else 'i'
            else:
                # XPO_M has no P_ form
                o = r.choice(['F_NEG_M', 'F_ABS_M', 'F_PST_M', 'F2I_M'] + ([] if p else ['XPO_M']))
                if p: o = ('PF_' + o[2:]) if o.startswith('F_') else 'P_' + o
                self.emit('%s %s' % (o, self.opnd('f') or self.fvar()))
                self.acc = 'i' if o.endswith(('XPO_M', 'F2I_M')) else 'f'
        elif c == 'set':
            if r.random() < 0.7: self.emit('SET %s' % self.var(self.acc))
            else: self.emit('SET_V %s %d' % ('ia' if self.acc == 'i' else 'fa', r.randrange(ASIZE)))
        elif c == 'out':
            self.emit('OUT %d' % r.randrange(2))
        elif c == 'psh':
            self.emit('PSH'); self.stk.append(self.acc)
        elif c == 'plod':
            self.stk.append(self.acc)
            t = r.choice('if')
            k = r.randrange(3)
            if k == 0: self.emit('P_LOD %s' % self.var(t))
            elif k == 1: self.emit('P_LOD %s' % self.const(t))
            else: self.emit('P_LOD_V %s %d' % ('ia' if t == 'i' else 'fa', r.randrange(ASIZE)))
            self.acc = t
        elif c == 's2':
            self.stk.pop()
            if a == 'i': self.emit('S_' + r.choice(self.INT2[:11] + ['SHL', 'SHR', 'SRS'])); self.acc = 'i'
            else:
                o = r.choice(['SF_ADD', 'SF_SU1', 'SF_SU2', 'SF_MLT', 'SF_SGN', 'SF_LES', 'SF_GRE'])
                self.emit(o); self.acc = 'i' if o in ('SF_LES', 'SF_GRE') else 'f'
        elif c == 'shift_s':
            self.stk.pop(); self.emit(r.choice(['S_SHL', 'S_SHR', 'S_SRS'])); self.acc = 'i'
        elif c == 'sfscl':
            self.stk.pop(); self.emit('SF_SCL'); self.acc = 'f'
        elif c == 'pop':
            self.emit('POP'); self.acc = self.stk.pop()
        elif c == 'setp':
            self.emit('SET_P %s' % self.var(self.acc)); self.acc = self.stk.pop()
        elif c == 'lod':
            t = r.choice('if')
            self.emit('LOD %s' % (self.var(t) if r.random() < 0.6 else self.const(t)))
            self.acc = t
        elif c == 'inn':
            t = r.choice('if')
            self.emit('%s %d' % ('INN' if t == 'i' else 'F_INN', r.randrange(2)))
            self.acc = t
        elif c == 'pinn':
            self.stk.append(self.acc)
            t = r.choice('if')
            self.emit('%s %d' % ('P_INN' if t == 'i' else 'PF_INN', r.randrange(2)))
            self.acc = t
        return True

    # ------------------------------------------------------------- blocks ---

    def straight(self):
        r = self.r
        for _ in range(r.randrange(2, 9)):
            k = r.randrange(20)
            if k == 0:
                self.store_indirect()
            elif k == 1:
                self.inc()
            elif k == 2:
                self.load()
            elif k == 3:
                self.emit('LEA %s' % self.var(r.choice('if'))); self.acc = 'i'
            else:
                self.one()

    def store_indirect(self):
        r = self.r
        if len(self.stk) + 1 > MAXSTK:
            return
        t = r.choice('if')
        arr = 'ia' if t == 'i' else 'fa'
        kind = r.randrange(3)
        off = r.randrange(ASIZE) if kind == 1 else 0
        self.emit('LOD %d' % r.randrange(ASIZE - off))
        self.emit('PSH')
        self.stk.append('i')
        self.load(t)
        self.stk.pop()
        if kind == 0:   self.emit('STI %s' % arr)
        elif kind == 1: self.emit('STI_V %s %d' % (arr, off) if off else 'STI %s' % arr)
        else:           self.emit('ISI %s' % arr)

    def inc(self):
        if self.r.random() < 0.5 or len(self.stk) >= MAXSTK or self.acc is None:
            self.emit('INC %s' % self.ivar())
        else:
            self.stk.append(self.acc)
            self.emit('P_INC %s' % self.ivar())
        self.acc = 'i'
        self.emit('NOP')

    def balance(self):
        # leave the data stack as the block found it
        while len(self.stk) > self.base:
            t = self.stk[-1]
            k = self.r.randrange(3)
            if k == 0 or self.acc is None:
                self.emit('POP'); self.acc = self.stk.pop()
            elif k == 1:
                self.emit('SET_P %s' % self.var(self.acc)); self.acc = self.stk.pop()
            else:
                self.ensure(t)
                self.stk.pop()
                if t == 'i': self.emit('S_' + self.r.choice(['ADD', 'MLT', 'AND', 'XOR', 'LES', 'EQU']))
                else:        self.emit(self.r.choice(['SF_ADD', 'SF_SU1', 'SF_SU2', 'SF_MLT']))
                self.acc = t

    def division(self):
        r = self.r
        kind = r.randrange(6) if self.idiv else r.choice([4, 5])
        if kind < 4:                          # integer: DIV/MOD, memory or stack
            stack = kind >= 2 and len(self.stk) < MAXSTK
            if stack:
                self.ensure('i'); self.emit('PSH'); self.stk.append('i')
            self.ensure('i')
            name = 'DIV' if kind % 2 == 0 else 'MOD'
            if stack:
                self.emit('S_' + name); self.stk.pop()
            else:
                self.emit('%s %s' % (name, self.ivar() if r.random() < 0.7 else self.iconst()))
            nslot, read, res = 3, ('QUO' if name == 'DIV' else 'REM'), 'i'
        else:                                 # float: F_DIV or SF_DIV
            stack = kind == 5 and len(self.stk) < MAXSTK
            if stack:
                self.ensure('f'); self.emit('PSH'); self.stk.append('f')
            if self.fround == 0:
                self.emit('LOD %s' % self.fconst(nonzero=True)); self.acc = 'f'
            else:
                self.ensure('f')
            if stack:
                self.emit('SF_DIV'); self.stk.pop()
            else:
                self.emit('F_DIV %s' % (self.fvar() if r.random() < 0.7 else self.fconst()))
            nslot, read, res = (3 if self.idiv else 2), 'F_QUO', 'f'
        depth = len(self.stk)
        for _ in range(nslot):
            if r.random() < 0.3:
                self.emit('NOP')
            else:
                save = list(self.stk)
                self.one()
                # a slot may push or pop, but the block keeps its stack in bounds
                if len(self.stk) > MAXSTK:
                    self.stk = save
        self.emit(read)
        self.acc = res
        _ = depth

    def block(self, depth, in_sub):
        r = self.r
        self.base = len(self.stk)
        base = self.base
        k = r.randrange(12)
        if k < 4 or depth <= 0:
            self.straight()
        elif k < 6:
            self.division()
        elif k == 6:                          # if
            self.ensure_any()
            self.base = base; self.balance()
            self.ensure_any()
            end = self.label()
            self.emit('%s %s' % (r.choice(['JIZ', 'JNZ']), end))
            self.body(depth - 1, in_sub)
            self.place(end)
        elif k == 7:                          # if / else
            self.ensure_any()
            self.base = base; self.balance()
            self.ensure_any()
            els, end = self.label(), self.label()
            self.emit('%s %s' % (r.choice(['JIZ', 'JNZ']), els))
            self.body(depth - 1, in_sub)
            self.emit('JMP %s' % end)
            self.place(els)
            self.load()
            self.body(depth - 1, in_sub)
            self.place(end)
        elif k == 8 and self.loopdepth < 2:   # a counted loop
            self.base = base; self.balance()
            lc = 'lc%d_%d' % (in_sub + 1, self.loopdepth)   # a subroutine has its own
            top = self.label()
            self.emit('LOD %d' % r.randrange(1, 4))
            self.emit('SET %s' % lc)
            self.place(top)
            self.load()
            self.loopdepth += 1
            self.body(depth - 1, in_sub)
            self.loopdepth -= 1
            self.emit('LOD %s' % lc)
            self.emit('ADD -1')
            self.emit('SET %s' % lc)
            self.emit('JNZ %s' % top)
            self.acc = 'i'
        elif k == 9 and len(self.stk) == 0 and in_sub < NSUBS - 1:
            self.emit('CAL sub%d' % r.randrange(in_sub + 1, NSUBS))
            self.acc = None
        else:
            self.straight()
        self.base = base
        self.balance()
        # what the block computed is stored, or the load that starts the next
        # one would hide a wrong result
        if self.acc is not None and r.random() < 0.85:
            self.emit('SET %s' % self.var(self.acc))

    def body(self, depth, in_sub):
        for _ in range(self.r.randrange(1, 4)):
            self.block(depth, in_sub)

    # ------------------------------------------------------------ program ---

    def program(self):
        r = self.r
        hdr = ['NOP',
               '#NUBITS %d' % self.nu, '#NBMANT %d' % self.man, '#NBEXPO %d' % self.exp,
               '#NDSTAC %d' % self.ndstac, '#SDEPTH %d' % (NSUBS + 2),
               '#NUIOIN 2', '#NUIOOU 2', '#NUGAIN %d' % self.gain, '#FFTSIZ 3',
               '#FROUND %d' % self.fround,
               '#array ia 1 %d' % ASIZE, '#array fa 2 %d' % ASIZE]
        if self.itr:
            self.lines.append('@main #ITRAD')
        else:
            self.pend = ['main']
        # every variable starts with a value of its type
        for v in range(NVARS):
            self.emit('LOD %s' % self.iconst()); self.emit('SET iv%d' % v)
            self.emit('LOD %s' % self.fconst()); self.emit('SET fv%d' % v)
        for k in range(ASIZE):
            self.emit('LOD %s' % self.iconst()); self.emit('SET_V ia %d' % k)
            self.emit('LOD %s' % self.fconst()); self.emit('SET_V fa %d' % k)
        self.acc = 'f'
        if self.idiv:
            self.base = 0
            self.division()
        for _ in range(self.size):
            self.block(2, -1)
        self.base = 0
        self.balance()
        self.ensure_any()
        self.emit('OUT 0')
        self.place('halt')
        self.emit('JMP halt')
        # the subroutines: sub k calls only subroutines after it
        for s in range(NSUBS):
            self.place('sub%d' % s)
            self.load()
            self.stk = []
            self.body(2, s)
            self.base = 0
            self.balance()
            self.emit('RET')
        return '\n'.join(hdr + self.lines) + '\n'


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('seed', type=int)
    ap.add_argument('--size', type=int, default=24)
    ap.add_argument('--itr', action='store_true')
    ap.add_argument('--fmt')
    ap.add_argument('--fround', type=int)
    a = ap.parse_args()
    fmt = tuple(int(x) for x in a.fmt.split('/')) if a.fmt else None
    g = Gen(a.seed, a.size, a.itr, fmt, a.fround)
    text = g.program()
    first, rest = text.split(chr(10), 1)
    print(first)
    print('// rand_asm.py %d --size %d%s: %d/%d/%d, #FROUND %d%s'
          % (a.seed, a.size, ' --itr' if a.itr else '', g.nu, g.man, g.exp, g.fround,
             ', with DIV/MOD' if g.idiv else ''))
    # every mnemonic written must be in the instruction set: asmcomp drops a
    # line it does not know without a word
    isa = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'Compilers', 'common', 'isa.tsv')
    known = {l.split('	')[0] for l in open(isa, encoding='utf8') if l.strip() and not l.startswith('#')}
    bad = sorted(g.used - known)
    if bad:
        raise SystemExit('rand_asm.py: not in isa.tsv: ' + ' '.join(bad))
    print('// mnemonics: ' + ' '.join(sorted(g.used)))
    print(rest, end='')


if __name__ == '__main__':
    main()
