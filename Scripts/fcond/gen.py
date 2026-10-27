#!/usr/bin/env python3
"""gen.py: the float-as-a-condition test.

Writes, into the folder given (default .smoke/fcond):
  fcond.cpp               one program, compiled by the host g++ (the
                          reference, with host.h giving out() as printf)
  fcond_<f>.cpp           the same file with the format's #pragma yanc, for
                          cppcomp, one per float format <f> (FORMATS)
  fcond_<f>_l<n>.cmm      the same cases in C+- (no ?: and no bool in C+-),
                          one per format and #FROUND level n = 0, 1, 2
  cases.txt   one line per printed number: index, value, form, and whether
              the C+- program prints it too

Every case prints 0 or 1 (the do-while prints 1 or 2: how many turns), so
the outputs compare line by line whatever the float format. The loops stop
after one turn too many (break), so a compiler that takes a zero for true
prints 2 (3 for the do-while) instead of hanging.
"""
import os
import sys

# the float formats the test runs at, (NUBITS, NBMANT, NBEXPO): the ones the
# C+- fixtures use. The widths are a per-program choice (#NBMANT/#NBEXPO,
# #pragma yanc), so every approach must hold at each of them.
FORMATS = [(32, 23, 8), (16, 10, 5), (23, 16, 6)]

# (name, C++ initialiser, C+- initialiser, value it should have in C)
VALUES = [
    ('zero',   '0.0f',   '0.0',   '0'),
    ('mzero',  '-0.0f',  '-0.0',  '-0'),
    ('half',   '0.5f',   '0.5',   '0.5'),
    ('m03',    '-0.3f',  '-0.3',  '-0.3'),
    ('p999',   '0.999f', '0.999', '0.999'),
    ('m999',   '-0.999f', '-0.999', '-0.999'),
    ('one',    '1.0f',   '1.0',   '1'),
    ('mone',   '-1.0f',  '-1.0',  '-1'),
    ('p25',    '2.5f',   '2.5',   '2.5'),
    ('m7',     '-7.0f',  '-7.0',  '-7'),
    ('milli',  '0.001f', '0.001', '0.001'),
]
# computed values: (name, C++ statements, C+- statements, what it should be)
COMPUTED = [
    ('amina',  'k_a = 2.5f; amina = k_a - k_a;',          'k_a = 2.5; amina = k_a - k_a;',          'a - a = 0'),
    ('atimz',  'k_a = 2.5f; k_z = 0.0f; atimz = k_a * k_z;', 'k_a = 2.5; k_z = 0.0; atimz = k_a * k_z;', 'a * 0 = 0'),
    ('third',  'k_a = 1.0f; k_b = 3.0f; third = k_a / k_b;', 'k_a = 1.0; k_b = 3.0; third = k_a / k_b;', '1/3'),
    ('loop0',  'loop0 = 1.0f; for (k_n = 0; k_n < 4; k_n++) loop0 = loop0 - 0.25f;',
               'loop0 = 1.0; for (k_n = 0; k_n < 4; k_n++) loop0 = loop0 - 0.25;', '1 - 4 * 0.25 = 0'),
    ('tiny',   'k_a = 0.001f; k_b = 0.5f; tiny = k_a * k_b;', 'k_a = 0.001; k_b = 0.5; tiny = k_a * k_b;', '0.0005'),
]

# the magnitude of each value, for compare.py to leave out, per format, the
# nonzero ones below its smallest normal (they become zero there: a format
# difference, not the compiler's)
MAGNITUDE = {'zero': 0, 'mzero': 0, 'half': 0.5, 'm03': 0.3, 'p999': 0.999, 'm999': 0.999,
             'one': 1, 'mone': 1, 'p25': 2.5, 'm7': 7, 'milli': 0.001,
             'amina': 0, 'atimz': 0, 'third': 0.333, 'loop0': 0, 'tiny': 0.0005}

# forms: (tag, in C+- too, C++ body, C+- body); X is the value's variable.
# (Before the change this test checks, C+- refused a float in && and ||,
# "logical operation, integers only!".)
FORMS = [
    ('if',      True,  'if (X) out(0, 1); else out(0, 0);',
                       'if (X) out(0, 1); else out(0, 0);'),
    ('not',     True,  'if (!X) out(0, 1); else out(0, 0);',
                       'if (!X) out(0, 1); else out(0, 0);'),
    ('while',   True,  'n = 0; t = X; while (t) { n = n + 1; t = 0.0f; if (n > 1) break; } out(0, n);',
                       'n = 0; t = X; while (t) { n = n + 1; t = 0.0; if (n > 1) break; } out(0, n);'),
    ('for',     True,  'n = 0; t = X; for (; t; ) { n = n + 1; t = 0.0f; if (n > 1) break; } out(0, n);',
                       'n = 0; t = X; for (; t; ) { n = n + 1; t = 0.0; if (n > 1) break; } out(0, n);'),
    ('do',      True,  'n = 0; t = X; do { n = n + 1; if (n > 1) t = 0.0f; if (n > 2) break; } while (t); out(0, n);',
                       'n = 0; t = X; do { n = n + 1; if (n > 1) t = 0.0; if (n > 2) break; } while (t); out(0, n);'),
    ('andT',    True,  'if (X && tru) out(0, 1); else out(0, 0);',
                       'if (X && tru) out(0, 1); else out(0, 0);'),
    ('Tand',    True,  'if (tru && X) out(0, 1); else out(0, 0);',
                       'if (tru && X) out(0, 1); else out(0, 0);'),
    ('orF',     True,  'if (X || fal) out(0, 1); else out(0, 0);',
                       'if (X || fal) out(0, 1); else out(0, 0);'),
    ('For',     True,  'if (fal || X) out(0, 1); else out(0, 0);',
                       'if (fal || X) out(0, 1); else out(0, 0);'),
    # && and || with a computed operand (the result in the accumulator, or
    # on the stack when both are), and with an int operand
    ('andE',    True,  'if ((X * k1) && tru) out(0, 1); else out(0, 0);',
                       'if ((X * k1) && tru) out(0, 1); else out(0, 0);'),
    ('Eand',    True,  'if (tru && (X * k1)) out(0, 1); else out(0, 0);',
                       'if (tru && (X * k1)) out(0, 1); else out(0, 0);'),
    ('EandE',   True,  'if ((X * k1) && (tru * k1)) out(0, 1); else out(0, 0);',
                       'if ((X * k1) && (tru * k1)) out(0, 1); else out(0, 0);'),
    ('EorE',    True,  'if ((X * k1) || (fal * k1)) out(0, 1); else out(0, 0);',
                       'if ((X * k1) || (fal * k1)) out(0, 1); else out(0, 0);'),
    ('EorEb',   True,  'if ((fal * k1) || (X * k1)) out(0, 1); else out(0, 0);',
                       'if ((fal * k1) || (X * k1)) out(0, 1); else out(0, 0);'),
    ('andI',    True,  'if (X && i1) out(0, 1); else out(0, 0);',
                       'if (X && i1) out(0, 1); else out(0, 0);'),
    ('Ior',     True,  'if (i0 || X) out(0, 1); else out(0, 0);',
                       'if (i0 || X) out(0, 1); else out(0, 0);'),
    ('tern',    False, 'out(0, X ? 1 : 0);', None),
    ('bool',    False, 'b = X; out(0, b);', None),
]


# complex conditions (C+- only; C++'s std::complex has no bool conversion):
# true when the real or the imaginary part is not zero, as in C99, whose host
# gcc build of fcomp.c is the reference. (name, real, imaginary, magnitude)
COMP_VALUES = [
    ('z00', '0.0', '0.0', 0), ('z02', '0.0', '2.0', 2), ('z20', '2.0', '0.0', 2),
    ('zh0', '0.5', '0.0', 0.5), ('z0m', '0.0', '-0.3', 0.3), ('zmz', '-0.0', '0.0', 0),
    ('z11', '1.0', '1.0', 1), ('z0h', '0.0', '0.5', 0.5),
]
COMP_COMPUTED = [('zdz', 0)]          # z11 - z11, computed: zero
# (tag, C body, C+- body); Z is the value's variable
COMP_FORMS = [
    ('if',    'if (Z) out(1); else out(0);',          'if (Z) out(0, 1); else out(0, 0);'),
    ('not',   'if (!Z) out(1); else out(0);',         'if (!Z) out(0, 1); else out(0, 0);'),
    ('while', 'n = 0; w = Z; while (w) { n = n + 1; w = 0; if (n > 1) break; } out(n);',
              'n = 0; w = Z; while (w) { n = n + 1; w = complex(0.0, 0.0); if (n > 1) break; } out(0, n);'),
    ('do',    'n = 0; w = Z; do { n = n + 1; if (n > 1) w = 0; if (n > 2) break; } while (w); out(n);',
              'n = 0; w = Z; do { n = n + 1; if (n > 1) w = complex(0.0, 0.0); if (n > 2) break; } while (w); out(0, n);'),
    ('ifE',   'if (Z * k1) out(1); else out(0);',     'if (Z * k1) out(0, 1); else out(0, 0);'),
    ('notE',  'if (!(Z * k1)) out(1); else out(0);',  'if (!(Z * k1)) out(0, 1); else out(0, 0);'),
]


def write_comp(out):
    names = [v[0] for v in COMP_VALUES] + [c[0] for c in COMP_COMPUTED]
    mag = dict([(v[0], v[3]) for v in COMP_VALUES] + COMP_COMPUTED)
    c = ['// generated by Scripts/fcond/gen.py: complex as a condition',
         '#include <stdio.h>', '#include <complex.h>',
         'static void out(int v) { printf("%d\\n", v); }',
         'int main(void) {',
         '    float complex ' + ', '.join(names) + ', w;',
         '    float k1 = 1.0f; int n;']
    c += ['    %s = %sf + %sf * I;' % (v[0], v[1], v[2]) for v in COMP_VALUES]
    c += ['    zdz = z11 - z11;']
    m = ['    k1 = 1.0;']
    m += ['    %s = complex(%s, %s);' % (v[0], v[1], v[2]) for v in COMP_VALUES]
    m += ['    zdz = z11 - z11;']
    cases = []
    for name in names:
        for tag, cbody, mbody in COMP_FORMS:
            c.append('    ' + cbody.replace('Z', name))
            m.append('    ' + mbody.replace('Z', name))
            cases.append((len(cases), name, tag, 1, mag[name]))
    c += ['    return 0;', '}']
    open(os.path.join(out, 'fcomp.c'), 'w', newline='\n').write('\n'.join(c) + '\n')
    for (nu, nm, ne), lvl in [(f, l) for f in FORMATS for l in (0, 1, 2)]:
        cmm = ['#NUBITS %d' % nu, '#NUIOIN 1', '#NUIOOU 1', '#NBMANT %d' % nm, '#NBEXPO %d' % ne, '#NUGAIN 128',
               '#FROUND %d' % lvl,
               '// generated by Scripts/fcond/gen.py: complex as a condition',
               'void main()', '{',
               '    comp ' + ', '.join(names) + ', w;',
               '    float k1;', '    int n;'] + m + ['}']
        open(os.path.join(out, 'fcomp_%d_%d_%d_l%d.cmm' % (nu, nm, ne, lvl)), 'w', newline='\n').write('\n'.join(cmm) + '\n')
    with open(os.path.join(out, 'ccases.txt'), 'w', newline='\n') as f:
        for i, name, tag, in_cmm, mg in cases:
            f.write('%d %s %s %d %s\n' % (i, name, tag, in_cmm, mg))
    return len(cases)


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else os.path.join('.smoke', 'fcond')
    os.makedirs(out, exist_ok=True)
    names = [v[0] for v in VALUES] + [c[0] for c in COMPUTED]

    cpp = ['// generated by Scripts/fcond/gen.py: float as a condition',
           'void main(void) {',
           '    float ' + ', '.join(names) + ';',
           '    float k_a, k_b, k_z, t, tru, fal, k1;',
           '    int k_n, n, i0, i1;',
           '    bool b;',
           '    tru = 2.5f; fal = 0.0f; k1 = 1.0f; i0 = 0; i1 = 1;']
    cpp += ['    %s = %s;' % (v[0], v[1]) for v in VALUES]
    cpp += ['    ' + c[1] for c in COMPUTED]

    cmm_body = ['    tru = 2.5; fal = 0.0; k1 = 1.0; i0 = 0; i1 = 1;']
    cmm_body += ['    %s = %s;' % (v[0], v[2]) for v in VALUES]
    cmm_body += ['    ' + c[2] for c in COMPUTED]

    cases = []
    for name in names:
        for tag, in_cmm, cbody, mbody in FORMS:
            cpp.append('    ' + cbody.replace('X', name))
            if in_cmm:
                cmm_body.append('    ' + mbody.replace('X', name))
            cases.append((len(cases), name, tag, in_cmm))
    cpp.append('}')
    open(os.path.join(out, 'fcond.cpp'), 'w', newline='\n').write('\n'.join(cpp) + '\n')
    for nu, nm, ne in FORMATS:
        prag = ['#pragma yanc nubits %d' % nu, '#pragma yanc nbmant %d' % nm, '#pragma yanc nbexpo %d' % ne]
        open(os.path.join(out, 'fcond_%d_%d_%d.cpp' % (nu, nm, ne)), 'w', newline='\n').write('\n'.join(prag + cpp) + '\n')

    open(os.path.join(out, 'host.h'), 'w', newline='\n').write(
        '// the host g++ build of fcond.cpp: out() prints, main() calls it\n'
        '#include <cstdio>\n'
        'static void out(int, int v) { std::printf("%d\\n", v); }\n'
        'void fcond_main(void);\n'
        'int main() { fcond_main(); return 0; }\n'
        '#define main fcond_main\n')

    for (nu, nm, ne), lvl in [(f, l) for f in FORMATS for l in (0, 1, 2)]:
        cmm = ['#NUBITS %d' % nu, '#NUIOIN 1', '#NUIOOU 1', '#NBMANT %d' % nm, '#NBEXPO %d' % ne, '#NUGAIN 128',
               '#FROUND %d' % lvl,
               '// generated by Scripts/fcond/gen.py: float as a condition',
               'void main()', '{',
               '    float ' + ', '.join(names) + ';',
               '    float k_a, k_b, k_z, t, tru, fal, k1;',
               '    int k_n, n, i0, i1;'] + cmm_body + ['}']
        open(os.path.join(out, 'fcond_%d_%d_%d_l%d.cmm' % (nu, nm, ne, lvl)), 'w', newline='\n').write('\n'.join(cmm) + '\n')

    with open(os.path.join(out, 'cases.txt'), 'w', newline='\n') as f:
        for i, name, tag, in_cmm in cases:
            f.write('%d %s %s %d %s\n' % (i, name, tag, 1 if in_cmm else 0, MAGNITUDE[name]))
    nc = write_comp(out)
    print('gen.py: %d cases (%d values x %d forms), %d complex cases, in %s'
          % (len(cases), len(names), len(FORMS), nc, out))


if __name__ == '__main__':
    main()
