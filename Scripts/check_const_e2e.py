"""Float constants end to end, from the C± source to the data .mif (TODO.md item 5, step 5).

Usage: python3 Scripts/check_const_e2e.py <cmmcomp> <asmcomp> <work dir>

For each format below, writes a C± program whose only data is a float array
initialized between braces (its values go into the data memory through
#arrayd, at address 0, with no instruction), compiles it with cmmcomp and
asmcomp, and compares every word of the array in the .mif with the exact
model of Scripts/check_yanc_num.py, bit for bit. The constants are written in
every form the C± lexer reads (.5, 5., 007.5, 1e-8, 2.5E+3, a suffix), and
include ties, values next to the smallest normal and the largest number, and
random decimal text. Constants the format cannot hold (cmmcomp stops on them)
are left out. check_yanc_num.py tests the encoder alone; this tests the whole
path: lexer, the text cmmcomp writes, appcomp, asmcomp and the .mif.
"""
import os, random, subprocess, sys
from fractions import Fraction as Fr

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from check_yanc_num import model, dec

FORMATS = [(23, 8, 0), (25, 6, 0), (10, 5, 0), (23, 8, 1)]   # (nbmant, nbexpo, fround): 32/23/8, 32/25/6, 16/10/5

FIXED = ['0.0', '1.0', '-1.0', '.5', '5.', '007.5', '0.1', '-0.1', '1e-8', '2.5E+3', '1.5f', '2.25F',
         '3.0l', '1.570796327', '3.14159265358979323846', '0.000001', '-2.5e-3', '6.25e-2', '1E1',
         '65504.0', '65505.0', '4194304.0', '0.3', '0.7', '2.718281828459045', '1e-4', '123.456e-1']

def as_cmm(t):                        # a model text as a C± float literal (always with a point or an e)
    return t if ('.' in t or 'e' in t or 'E' in t) else t + '.0'

def constants(nm, ne, fr):
    emin, emax = -(1 << (ne - 1)), (1 << (ne - 1)) - 1
    out = list(FIXED)
    for k in range(40):                                       # exact ties between two words
        m = random.randrange(1 << nm, 1 << (nm + 1)) | 1
        e = random.randint(max(emin, -60), min(emax, 60))
        out.append(dec(Fr(m) * Fr(2) ** (e - 1)))
    for k in range(40):                                       # around the smallest normal and the largest number
        e = random.choice([emin - 1, emin, emin + 1, emax - 1, emax])
        if e < -60: continue                                  # keeps the exact decimal text short
        m = random.randrange(1, 1 << (nm + 1))
        out.append(dec(Fr(m) * Fr(2) ** (e - 1)))
    for k in range(80):                                       # random decimal text
        mant = str(random.randrange(1, 10 ** random.randint(1, 20)))
        dot = random.randint(0, len(mant))
        t = mant[:dot] + '.' + mant[dot:]
        if random.random() < 0.5: t += 'e%d' % random.randint(-12, 12)
        if random.random() < 0.3: t = '-' + t
        out.append(t)
    keep = []
    for t in out:
        txt = t[:-1] if t[-1] in 'fFlL' else t                # the model reads no suffix
        if len(txt) > 120: continue
        bits, flags = model(txt, nm, ne, fr)
        if 'o' in flags or (flags.startswith('u') and 'f' not in flags): continue   # cmmcomp refuses these
        keep.append((as_cmm(t), bits))
    return keep

def main():
    cmm, asm, work = (os.path.abspath(a) for a in sys.argv[1:4])
    random.seed(20261021)
    os.makedirs(work, exist_ok=True)
    bad = total = 0
    for nm, ne, fr in FORMATS:
        nb = nm + ne + 1
        name = 'flt_%d_%d_%d' % (nb, nm, fr)
        cs = constants(nm, ne, fr)
        src = os.path.join(work, name + '.cmm')
        with open(src, 'w') as f:
            f.write('#NUBITS %d\n#NBMANT %d\n#NBEXPO %d\n#FROUND %d\n#NUIOIN 1\n#NUIOOU 1\n' % (nb, nm, ne, fr))
            f.write('float v[%d] = {%s};\n' % (len(cs), ', '.join(c[0] for c in cs)))
            f.write('void main()\n{\n    fout(0, v[0]);\n}\n')
        out = os.path.join(work, name)
        r = subprocess.run([cmm, '-o', out, src], capture_output=True, text=True)
        if r.returncode != 0:
            print('%s: cmmcomp failed: %s' % (name, (r.stderr or r.stdout).strip()[:300])); bad += 1; continue
        r = subprocess.run([asm, os.path.join(out, name + '.asm')], capture_output=True, text=True)
        if r.returncode != 0:
            print('%s: asmcomp failed: %s' % (name, (r.stderr or r.stdout).strip()[:300])); bad += 1; continue
        words = open(os.path.join(out, name + '_data.mif')).read().split()
        for k, (lit, want) in enumerate(cs):
            total += 1
            got = words[k] if k < len(words) else '(missing)'
            if got != want:
                bad += 1
                if bad <= 10: print('MISMATCH %d/%d/%d fround %d, v[%d] = %s: got %s want %s' % (nb, nm, ne, fr, k, lit, got, want))
    print('check_const_e2e: %d constants in %d formats, %d mismatches' % (total, len(FORMATS), bad))
    sys.exit(1 if bad else 0)

if __name__ == '__main__':
    main()
