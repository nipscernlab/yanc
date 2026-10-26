#!/usr/bin/env python3
# proc_rls whole, today's unrolled Dirac code against the same program with
# every Dirac stretch replaced by the pointer + RPT loops of the prototype
# (dirac-expressions.md section 6), written by hand as bench.py writes them.
#   python3 rls_bench.py N [N ...]
import os, re, shutil, subprocess, sys
sys.path.insert(0, 'C:/nipscern/yanc-dloops/.smoke/dirac-proto/proto')
import bench
HERE = os.path.dirname(os.path.abspath(__file__))
bench.S    = 'C:/nipscern/yanc-dloops/.smoke/dirac-proto'
bench.PROT = bench.S + '/proto/SAPHO'
bench.BIN  = 'C:/nipscern/yanc/bin'
ORIG = 'C:/nipscern/yanc/SAPHO'
TESTS = 'C:/nipscern/yanc/Compilers/CMMComp/Tests/proc_rls'
run = bench.run

def gen(N, path):
    src = open(TESTS + '/proc_rls.cmm', encoding='utf-8').read().replace('[4]', '[%d]' % N)
    line = "        x # fin(0)*0.001 -> |x⟩;\n"
    assert src.count(line * 4) == 1
    src = src.replace(line * 4, (line + "        q = 90;\n") * N)
    marks = ["float e = d - ⟨w|x⟩;", "float Px[%d] # |P|x⟩;" % N,
             "float g     = 1.0/(0.99 + ⟨x|Px⟩);", "float K [%d] # g|Px⟩;" % N,
             "w # |w⟩ + e|K⟩;", "P # |P| - |K⟩⟨Px|;", "out(0,1000.0|w⟩);"]
    for k, s in enumerate(marks, 1):
        assert src.count(s) == 1, s
        src = src.replace(s, s + " q = %d;" % k)
    src = src.replace("float P[%d][%d];\n" % (N, N), "float P[%d][%d];\nint q; int rr; float tt;\n" % (N, N))
    src = src.replace("    int j = 0;\n", "    rr = 0; tt = 0.0;\n    int j = 0;\n")
    open(path, 'w', encoding='utf-8').write(src)

def marker(asm, i):
    return asm[i].strip() == 'SET q' and re.match(r'^LOD \d+$', asm[i - 1].strip())

def loops(N):
    """the pointer code of each stretch, in the order of the markers"""
    mv = lambda M, v, y, L: ["SETP 0 %s 0" % M, "SETP 2 %s 0" % y, "NEG_M %d" % N, "SET rr",
        "@%sr SETP 1 %s 0" % (L, v), "LOD @0+", "F_MLT @1+",
        "RPT 3 %d" % (N - 1), "P_LOD @0+", "F_MLT @1+", "SF_ADD",
        "SET @2+", "LOD rr", "ADD 1", "SET rr", "JIZ %se" % L, "JMP %sr" % L, "@%se NOP" % L]
    dot = lambda a, b: ["SETP 0 %s 1" % a, "SETP 1 %s 1" % b, "LOD %s" % a, "F_MLT %s" % b,
        "RPT 3 %d" % (N - 1), "P_LOD @0+", "F_MLT @1+", "SF_ADD"]
    K, Px, g, e = 'rls_update_K', 'rls_update_Px', 'rls_update_g', 'rls_update_e'
    body = ["LOD @0+ >2+", "F_MLT tt", "F_SU2 @1+"]
    return [
        # 1: e = d - <w|x>
        dot('w', 'x') + ["F_SU2 rls_update_d", "SET %s" % e],
        # 2: Px # |P|x>
        mv('P', 'x', Px, '__X2'),
        # 3: g = 1.0/(0.99 + <x|Px>)
        dot('x', Px) + ["F_ADD 0.99", "F_DIV 1.0", "NOP", "NOP", "F_QUO", "SET %s" % g],
        # 4: K # g|Px>
        ["SETP 0 %s 0" % Px, "SETP 1 %s 0" % K, "LOD @0+", "F_MLT %s" % g,
         "RPT 2 %d" % (N - 1), "LOD @0+ >1+", "F_MLT %s" % g, "SET @1+"],
        # 5: w # |w> + e|K>, in place
        ["SETP 0 %s 0" % K, "SETP 1 w 0", "SETP 2 w 0", "LOD @0+", "F_MLT %s" % e, "F_ADD @1+",
         "RPT 3 %d" % (N - 1), "LOD @0+ >2+", "F_MLT %s" % e, "F_ADD @1+", "SET @2+"],
        # 6: P # |P| - |K><Px|, today's element: P[r][c] = -(K[c]*Px[r]) + P[r][c]
        ["SETP 1 P 0", "SETP 2 P 0", "SETP 3 %s 0" % Px, "NEG_M %d" % N, "SET rr",
         "@__X6r SETP 0 %s 0" % K, "LOD @3+", "SET tt", "LOD @0+", "F_MLT tt", "F_SU2 @1+",
         "RPT 3 %d" % (N - 1)] + body +
        ["SET @2+", "LOD rr", "ADD 1", "SET rr", "JIZ __X6e", "JMP __X6r", "@__X6e NOP"],
        # 7: P # 1.010101|P| (up to RET)
        ["SETP 0 P 0", "SETP 1 P 0", "LOD @0+", "F_MLT 1.010101",
         "RPT 2 %d" % (N * N - 1), "LOD @0+ >1+", "F_MLT 1.010101", "SET @1+"],
    ]

def shift(N):   # after SET __aux_var: x # aux -> |x>
    return ["SETP 0 x %d" % (N - 2), "SETP 1 x %d" % (N - 1), "LOD @0-",
            "RPT 1 %d" % (N - 2), "LOD @0- >1-", "SET @1-", "LOD __aux_var", "SET x"]

def out(N):
    return ["SETP 0 w 0", "RPT 4 %d" % N, "LOD @0+", "F_MLT 1000.0", "F2I", "OUT 0"]

def build(N, d):
    asm = open('%s/c/r.asm' % d).read().split('\n')
    mk = [i for i in range(len(asm)) if marker(asm, i)]
    ref, ptr, last = [], [], 0
    keep = lambda seg: [l for l in seg if l.startswith('#')]
    L = loops(N)
    # rls_update: from the label to the 6 markers, then marker 6 to RET
    f0 = next(i for i, l in enumerate(asm) if l.startswith('@rls_update'))
    ptr += asm[:f0] + [asm[f0]]
    starts = [f0 + 1] + [m + 1 for m in mk[:6]]
    ends = [m - 1 for m in mk[:6]] + [next(i for i, l in enumerate(asm) if l.strip() == 'RET')]
    for s, e, code in zip(starts, ends, L):
        ptr += keep(asm[s:e]) + code
    pos = ends[-1]
    # main: the N shifts, then the out
    for m in mk[6:6 + N]:
        a = max(i for i in range(pos, m) if asm[i].strip() == 'SET __aux_var')
        ptr += asm[pos:a + 1] + shift(N)
        pos = m - 1
    o = mk[6 + N]
    a = max(i for i in range(pos, o) if asm[i].strip() == 'CAL rls_update')
    ptr += asm[pos:a + 1] + out(N)
    ptr += asm[o - 1:]
    strip = lambda lines: [l for i, l in enumerate(lines)
                           if not (l.strip() == 'SET q' or (i + 1 < len(lines) and lines[i + 1].strip() == 'SET q'
                                                          and re.match(r'^LOD \d+$', l.strip())))]
    # rr and tt are used only by the pointer code: no sharing for them
    ptr = [l for l in ptr if not (l.startswith('#SHARE') and set(l.split()[1:]) & {'rr', 'tt'})]
    return strip(asm), strip(ptr)

def simulate(name, d, hdl):
    P = '%s/%s/' % (d, name)
    for f in ('input_0.txt', 'input_1.txt'): shutil.copy(os.path.dirname(d) + '/' + f, P + f)
    trad = [l.split(' ', 1) for l in open(P + 'trad_opcode.txt').read().split('\n') if l.strip()]
    i = next(n for n, (_, t) in enumerate(trad) if t.split() == ['JIZ', '__Lwh1end'])
    a = int(trad[i + 1][0])
    b = next(int(x) for x, t in trad if t.split() == ['JMP', '__Lwh1'])
    probe = open(bench.S + '/proto/region_probe.v').read().replace('TOP.proc.PROC', 'r_tb.proc.p_r')
    open(d + '/probe.v', 'w').write(probe)
    run(['iverilog', '-s', 'r_tb', '-s', 'region_probe', '-Pregion_probe.A=%d' % a, '-Pregion_probe.B=%d' % b,
         '-o', d + '/s.vvp', P + 'r_tb.v', P + 'r.v', d + '/probe.v'] +
        ['%s/%s.v' % (hdl, m) for m in ('addr_dec', 'instr_dec', 'processor', 'core', 'ula')])
    log = run(['vvp', '-n', d + '/s.vvp'], cwd=P)
    turns = [int(l.split()[1]) for l in log.split('\n') if l.startswith('REGION')]
    words = int(re.search(r'^n_ins\s+(\d+)', open(P + 'app_log.txt').read(), re.M).group(1))
    return open(P + 'output_0.txt').read(), turns, words

def one(N):
    d = '%s/n%d' % (HERE, N)
    shutil.rmtree(d, ignore_errors=True); os.makedirs(d)
    gen(N, d + '/r.cmm')
    ins = open(TESTS + '/input_0.txt').read().split()
    open(d + '/input_0.txt', 'w').write('\n'.join((ins * (1 + 20 * N // len(ins)))[:max(400, 20 * N)]) + '\n')
    shutil.copy(TESTS + '/input_1.txt', d + '/input_1.txt')
    run([bench.BIN + '/cmmcomp.exe', '-o', d + '/c', d + '/r.cmm'])
    ref, ptr = build(N, d)
    clocks = 60 * 20 * N * N + 200000
    for nm, lines, proto in (('ref', ref, False), ('ptr', ptr, True)):
        os.makedirs(d + '/' + nm, exist_ok=True)
        bench.assemble(lines, 'r', d + '/' + nm, clocks, proto)
    o_ref, t_ref, w_ref = simulate('r', d + '/ref', ORIG)
    o_ptr, t_ptr, w_ptr = simulate('r', d + '/ptr', bench.PROT)
    gold = ''
    if N == 4:
        gold = 'golden %s' % ('SAME' if o_ref.split() == open(TESTS + '/golden_sim/output_0.txt').read().split() else 'DIFFER')
    same = o_ref.split() == o_ptr.split() and len(o_ref.split()) == 20 * N
    print('N=%-3d bits %s %s  words %d -> %d  clocks/turn %s -> %s' % (
        N, 'SAME' if same else 'DIFFER', gold, w_ref, w_ptr,
        sorted(set(t_ref)), sorted(set(t_ptr))), flush=True)

if __name__ == '__main__':
    for n in sys.argv[1:] or ['4']:
        one(int(n))
