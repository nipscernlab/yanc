#!/usr/bin/env python3
# Prototype bench (not for the repository): each Dirac form as today's
# unrolled code (cmmcomp) against a hand-written version with pointers, the
# parallel store and RPT, assembled through placeholders and patched.
#
#   python3 bench.py [kernel ...]
#
# Extended assembly understood here (everything else goes to asmcomp as is):
#   SETP k arr [off]       pointer k <- address of arr + off   (opcode 110)
#   RPT L n                the next L instructions, n times      (opcode 111)
#   OP @k[+|-] [>j[+|-]]   OP through pointer k (then k +-1); with >j also
#                          store the previous result through pointer j
import os, re, random, shutil, subprocess, sys

S    = 'C:/nipscern/yanc-dirac/.smoke/dirac-proto'
BIN  = 'C:/nipscern/yanc-dirac/bin'
ORIG = 'C:/nipscern/yanc-dirac/SAPHO'
PROT = S + '/proto/SAPHO'
WORK = S + '/proto/bench'
PROBE = S + '/out_probe60.v'
N = 16

def run(cmd, cwd=None):
    r = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True)
    if r.returncode != 0:
        raise SystemExit("failed: %s\n%s%s" % (' '.join(cmd), r.stdout[-2000:], r.stderr[-2000:]))
    return r.stdout

# ---------------------------------------------------------------- kernels ----
# each: arrays (name, rows, cols), the C+- statement, the arrays it changes
# (output after it), the pointer code, the region end marker kept
def k_dot():
    return dict(arrays=[('a',N,0),('b',N,0)], stmt='s = ⟨a|b⟩;', out=[], outs='s',
        ptr=["SETP 0 a 1", "SETP 1 b 1", "LOD_V a 0", "F_MLT_V b 0",
             "RPT 3 %d" % (N-1), "P_LOD @0+", "F_MLT @1+", "SF_ADD", "SET main_s"])

def k_mv():
    return dict(arrays=[('a',N,0),('M',N,N),('v',N,0),('y',N,0)], stmt='y # |M|v⟩;', out=['y'],
        ptr=["SETP 0 M 0", "SETP 2 y 0", "NEG_M %d" % N, "SET main_rr",
             "@__Xr SETP 1 v 0", "LOD @0+", "F_MLT @1+",
             "RPT 3 %d" % (N-1), "P_LOD @0+", "F_MLT @1+", "SF_ADD",
             "SET @2+", "LOD main_rr", "ADD 1", "SET main_rr", "JIZ __Xe", "JMP __Xr", "@__Xe NOP"])

def k_cv():
    return dict(arrays=[('a',N,0),('y',N,0)], stmt='y # c|a⟩;', out=['y'],
        ptr=["SETP 0 a 0", "SETP 1 y 0", "LOD @0+", "F_MLT main_c",
             "RPT 2 %d" % (N-1), "LOD @0+ >1+", "F_MLT main_c", "SET @1+"])

def k_apcb():   # w # |w> + e|K>, in place as in proc_rls
    return dict(arrays=[('a',N,0),('w',N,0),('K',N,0)], stmt='w # |w⟩ + c|K⟩;', out=['w'],
        ptr=["SETP 0 K 0", "SETP 1 w 0", "SETP 2 w 0", "LOD @0+", "F_MLT main_c", "F_ADD @1+",
             "RPT 3 %d" % (N-1), "LOD @0+ >2+", "F_MLT main_c", "F_ADD @1+", "SET @2+"])

def k_mmvvt(sub):   # P # |P| - |K><Px|, today's element order: (row j, col i) = -(K[i]*Px[j]) + P[j][i]
    body = ["LOD @0+ >2+", "F_MLT main_tt"] + (["F_SU2 @1+"] if sub else ["F_NEG", "F_ADD @1+"])
    first = ["LOD @0+", "F_MLT main_tt"] + (["F_SU2 @1+"] if sub else ["F_NEG", "F_ADD @1+"])
    return dict(arrays=[('a',N,0),('P',N,N),('K',N,0),('Px',N,0)], stmt='P # |P| - |K⟩⟨Px|;', out=['P'],
        ptr=["SETP 1 P 0", "SETP 2 P 0", "SETP 3 Px 0", "NEG_M %d" % N, "SET main_rr",
             "@__Xr SETP 0 K 0", "LOD @3+", "SET main_tt"] + first +
            ["RPT %d %d" % (len(body), N-1)] + body +
            ["SET @2+", "LOD main_rr", "ADD 1", "SET main_rr", "JIZ __Xe", "JMP __Xr", "@__Xe NOP"])

def k_cm():
    return dict(arrays=[('a',N,0),('P',N,N)], stmt='P # c|P|;', out=['P'],
        ptr=["SETP 0 P 0", "SETP 1 P 0", "LOD @0+", "F_MLT main_c",
             "RPT 2 %d" % (N*N-1), "LOD @0+ >1+", "F_MLT main_c", "SET @1+"])

def k_vvt():   # A # |a><b|, today's element order: (row j, col i) = a[i]*b[j]
    return dict(arrays=[('a',N,0),('b',N,0),('A',N,N)], stmt='A # |a⟩⟨b|;', out=['A'],
        ptr=["SETP 2 A 0", "SETP 3 b 0", "NEG_M %d" % N, "SET main_rr",
             "@__Xr SETP 0 a 0", "LOD @3+", "SET main_tt", "LOD @0+", "F_MLT main_tt",
             "RPT 2 %d" % (N-1), "LOD @0+ >2+", "F_MLT main_tt",
             "SET @2+", "LOD main_rr", "ADD 1", "SET main_rr", "JIZ __Xe", "JMP __Xr", "@__Xe NOP"])

def k_v0():
    return dict(arrays=[('a',N,0),('y',N,0)], stmt='y # |0⟩;', out=['y'],
        ptr=["SETP 0 y 0", "LOD 0.0", "RPT 1 %d" % N, "SET @0+"])

def k_shift():   # a # c -> |a>, c a float in memory
    return dict(arrays=[('a',N,0)], stmt='a # c -> |a⟩;', out=['a'],
        ptr=["SETP 0 a %d" % (N-2), "SETP 1 a %d" % (N-1), "LOD @0-",
             "RPT 1 %d" % (N-2), "LOD @0- >1-", "SET @1-", "LOD main_c", "SET_V a 0"])

def k_out():
    return dict(arrays=[('a',N,0)], stmt='out(0, c|a⟩);', out=[], outs=None,
        ptr=["SETP 0 a 0", "RPT 4 %d" % N, "LOD @0+", "F_MLT main_c", "F2I", "OUT 0"])

KERNELS = {'dot': k_dot, 'mv': k_mv, 'cv': k_cv, 'apcb': k_apcb,
           'mmvvt_neg': lambda: k_mmvvt(False), 'mmvvt_su2': lambda: k_mmvvt(True),
           'cm': k_cm, 'vvt': k_vvt, 'v0': k_v0, 'shift': k_shift, 'out': k_out}

# ------------------------------------------------------------ the program ----
def cmm(k):
    decl = ''.join('float %s[%d];\n' % (n, r) if not c else 'float %s[%d][%d];\n' % (n, r, c)
                   for n, r, c in k['arrays'])
    init = ''
    for n, r, c in k['arrays']:
        if c: init += '    for (k = 0; k < %d; k++) for (j = 0; j < %d; j++) %s[k][j] = fin(0)*0.001;\n' % (r, c, n)
        else: init += '    for (k = 0; k < %d; k++) %s[k] = fin(0)*0.001;\n' % (r, n)
    outs = ''
    for n in k['out']:
        r, c = next((r, c) for nn, r, c in k['arrays'] if nn == n)
        if c: outs += '        for (k = 0; k < %d; k++) for (j = 0; j < %d; j++) {copy(%s[k][j], ib); out(0, ib);}\n' % (r, c, n)
        else: outs += '        for (k = 0; k < %d; k++) {copy(%s[k], ib); out(0, ib);}\n' % (r, n)
    if k.get('outs'): outs += '        copy(%s, ib); out(0, ib);\n' % k['outs']
    return """#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1

%s
void main()
{
    int k; int j; int ib; int q;
    float c; float s;
%s    c = fin(0)*0.001;
    int t = 0;
    while (t < 2)
    {
        %s
        q = 0;
%s        out(0, 7);
        c = c + 0.25;
        a[t] = a[t] + 0.125;
        t++;
    }
}
""" % (decl, init, k['stmt'], outs)

# ------------------------------------------------------- extended assembly ----
PTR = re.compile(r'^@(\d)([+-]?)$')
STO = re.compile(r'^>(\d)([+-]?)$')

def split_label(line):
    labs, rest = [], line.strip()
    while rest.startswith('@'):
        lab, rest = rest.split(' ', 1)
        labs.append(lab); rest = rest.strip()
    return (' '.join(labs) if labs else None), rest

def to_base(lines):
    """placeholders for the extended forms; returns base lines and the patch list"""
    base, patches = [], []
    for line in lines:
        if not line.strip() or line.startswith('#'):
            base.append(line); continue
        lab, ins = split_label(line)
        tok = ins.split()
        kind = None
        if tok[0] == 'SETP':
            k, arr = int(tok[1]), tok[2]; off = tok[3] if len(tok) > 3 else '0'
            ph = 'LOD_V %s %s' % (arr, off); kind = ('SETP', k)
        elif tok[0] == 'RPT':
            ph = 'NOP'; kind = ('RPT', int(tok[1]), int(tok[2]))
        elif len(tok) > 1 and PTR.match(tok[1]):
            m = PTR.match(tok[1]); k, mk = int(m.group(1)), m.group(2)
            st = None
            if len(tok) > 2:
                s2 = STO.match(tok[2]); st = (int(s2.group(1)), s2.group(2))
            ph = '%s __pd' % tok[0]; kind = ('PTR', k, mk, st)
        else:
            ph = ins
        base.append((lab + ' ' if lab else '') + ph)
        if kind: patches.append((len([b for b in base if b.strip() and not b.startswith('#')]) - 1, kind))
    return base, patches

def mod(c):
    return {'': 0, '+': 1, '-': 2}[c]

def assemble(asm_lines, name, d, clocks, proto):
    os.makedirs(d + '/' + name, exist_ok=True)
    base, patches = to_base(asm_lines) if proto else (asm_lines, [])
    open('%s/%s/%s.asm' % (d, name, name), 'w', newline='\n').write('\n'.join(base) + '\n')
    run([BIN + '/asmcomp.exe', '-f', '100', '-c', str(clocks), '%s/%s/%s.asm' % (d, name, name)])
    if not proto: return
    P = '%s/%s/' % (d, name)
    trad = [l.split(' ', 1) for l in open(P + 'trad_opcode.txt').read().split('\n') if l.strip()]
    ins = [split_label(b)[1] for b in base if b.strip() and not b.startswith('#')]
    # align the listing with the instructions (asmcomp may insert NOPs)
    where, i = {}, 0
    for idx, txt in trad:
        t = txt.split()
        if i < len(ins) and t == ins[i].split():
            where[i] = int(idx); i += 1
        elif t == ['NOP']:
            continue
        else:
            raise SystemExit('cannot align %r with %r' % (txt, ins[i] if i < len(ins) else None))
    assert i == len(ins), (i, len(ins))
    v = open(P + name + '.v').read()
    mdatas = int(re.search(r'\.MDATAS\((\d+)\)', v).group(1))
    mdataw = max(1, (mdatas - 1).bit_length())
    old = int(re.search(r'\.NBOPER\((\d+)\)', v).group(1))
    maxcnt = max([p[1][2] for p in patches if p[1][0] == 'RPT'] + [1])
    nboper = max(old + 1, mdataw + 3, 10, maxcnt.bit_length() + 4)
    words = open(P + name + '_inst.mif').read().split()
    new = []
    for w in words:
        assert len(w) == 7 + old, (len(w), old)
        new.append(w[:7] + '0' * (nboper - old) + w[7:])
    nptr = 0
    for bi, kind in patches:
        wi = where[bi]
        opc, opr = new[wi][:7], int(new[wi][7:], 2)
        if kind[0] == 'SETP':
            opc = format(110, '07b'); opr = (kind[1] << mdataw) | (opr & ((1 << mdataw) - 1)); nptr = max(nptr, kind[1] + 1)
        elif kind[0] == 'RPT':
            opc = format(111, '07b'); L, n = kind[1], kind[2]
            assert L < 8 and n < (1 << (nboper - 4))
            opr = (L << (nboper - 4)) | n
        else:
            _, k, mk, st = kind
            opr = (1 << (nboper - 1)) | k | (mod(mk) << 2)
            nptr = max(nptr, k + 1)
            if st: opr |= (1 << 4) | (st[0] << 5) | (mod(st[1]) << 7); nptr = max(nptr, st[0] + 1)
        new[wi] = opc + format(opr, '0%db' % nboper)
    open(P + name + '_inst.mif', 'w', newline='\n').write('\n'.join(new) + '\n')
    rpt = int(any(p[1][0] == 'RPT' for p in patches))
    v = re.sub(r'\.NBOPER\(\d+\)', '.NBOPER(%d),\n.NPTR(%d),\n.RPT(%d)' % (nboper, nptr, rpt), v)
    open(P + name + '.v', 'w', newline='\n').write(v)
    return nboper, nptr, rpt

def simulate(name, d, hdl):
    P = '%s/%s/' % (d, name)
    probe = open(S + '/proto/region_probe.v').read().replace('TOP.proc.PROC', '%s_tb.proc.p_%s' % (name, name))
    open(d + '/probe.v', 'w').write(probe)
    shutil.copy(d + '/input_0.txt', P + 'input_0.txt')
    # the first instruction after the main while test, and the marker q = 0
    trad = [l.split(' ', 1) for l in open(P + 'trad_opcode.txt').read().split('\n') if l.strip()]
    i = next(n for n, (_, t) in enumerate(trad) if t.split() == ['LES', 'main_t'])
    a = int(trad[i + 2][0])
    b = next(int(x) for x, t in trad if t.split() == ['SET', 'main_q']) - 1
    run(['iverilog', '-s', name + '_tb', '-s', 'region_probe', '-Pregion_probe.A=%d' % a, '-Pregion_probe.B=%d' % b,
         '-o', d + '/s.vvp', P + name + '_tb.v', P + name + '.v',
         d + '/probe.v'] + ['%s/%s.v' % (hdl, m) for m in ('addr_dec', 'instr_dec', 'processor', 'core', 'ula')])
    log = run(['vvp', '-n', d + '/s.vvp'], cwd=P)
    turns = sorted(set(int(l.split()[1]) for l in log.split('\n') if l.startswith('REGION')))
    return open(P + 'output_0.txt').read(), turns

def bench(kname):
    k = KERNELS[kname]()
    d = WORK + '/' + kname
    shutil.rmtree(d, ignore_errors=True); os.makedirs(d)
    random.seed(11)
    open(d + '/input_0.txt', 'w').write(''.join('%d\n' % random.randint(-3000, 3000) for _ in range(2000)))
    open(d + '/%s.cmm' % kname, 'w', encoding='utf-8').write(cmm(k))
    run([BIN + '/cmmcomp.exe', '-o', d + '/c', d + '/%s.cmm' % kname])
    asm = open(d + '/c/%s.asm' % kname).read().split('\n')
    # the Dirac code: after the main while test, up to the marker q = 0
    t0 = next(i for i, l in enumerate(asm) if l.startswith('LES main_t')) + 2
    t1 = next(i for i, l in enumerate(asm) if l.startswith('SET main_q')) - 1
    region = asm[t0:t1]
    ref = asm
    ext = asm[:t0] + k['ptr'] + asm[t1:]
    assemble(ref, 'pref', d, 400000, False)
    info = assemble(ext, 'pptr', d, 400000, True)
    o_ref, c_ref = simulate('pref', d, ORIG)
    o_ptr, c_ptr = simulate('pptr', d, PROT)
    same = (o_ref == o_ptr) and len(o_ref) > 0
    nref = len([l for l in region if l.strip()])
    body = [l for l in k['ptr'] if l.strip()]
    print('%-10s bits %s  words: %4d -> %3d   clocks: %s -> %s   (NBOPER %d, NPTR %d, RPT %d)' % (
        kname, 'SAME' if same else 'DIFFER', nref, len(body), c_ref, c_ptr, *info))
    if not same:
        a, b = o_ref.split(), o_ptr.split()
        print('   first difference at word', next((i for i in range(min(len(a), len(b))) if a[i] != b[i]), None), len(a), len(b))

if __name__ == '__main__':
    for kn in (sys.argv[1:] or list(KERNELS)):
        bench(kn)
