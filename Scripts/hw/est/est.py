"""Chip-agnostic delay table, experiment: sapho_all variants (the base plus one
group of heavy operators) synthesized by Yosys for several FPGA families, the
worst register-to-register arrival read with `sta` (cell delays only, no
routing). Each delay is then divided by the base variant's on the same family.
"""
import concurrent.futures as cf, os, re, shutil, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
YOSYS = r'C:\packs\msys64\mingw64\bin\yosys.exe'
SRC = ['processor.v', 'core.v', 'ula.v', 'instr_dec.v', 'addr_dec.v']

GROUPS = {
    'mlt':   ['MLT', 'S_MLT'],
    'div':   ['DIV', 'S_DIV'],
    'mod':   ['MOD', 'S_MOD'],
    'fadd':  ['F_ADD', 'SF_ADD', 'F_SU1', 'F_SU2', 'SF_SU1', 'SF_SU2'],
    'fmlt':  ['F_MLT', 'SF_MLT'],
    'fdiv':  ['F_DIV', 'SF_DIV'],
    'i2f':   ['I2F', 'I2F_M', 'P_I2F_M'],
    'f2i':   ['F2I', 'F2I_M', 'P_F2I_M'],
    'shift': ['SHL', 'S_SHL', 'SHR', 'S_SHR', 'SRS', 'S_SRS'],
    'cmp':   ['LES', 'S_LES', 'GRE', 'S_GRE', 'EQU', 'S_EQU'],
    'fcmp':  ['F_LES', 'SF_LES', 'F_GRE', 'SF_GRE'],
    'nrm':   ['NRM', 'NRM_M', 'P_NRM_M'],
    'sgn':   ['SGN', 'S_SGN', 'F_SGN', 'SF_SGN'],
    'frot':  ['F_ROT', 'F_SCL', 'XPO'],
}
HEAVY = sorted({p for g in GROUPS.values() for p in g})

FAMILIES = {
    'ice40':  ('synth_ice40 -top sapho_all -abc9 -nobram',
               'read_verilog -lib -specify -DICE40_HX +/ice40/cells_sim.v'),
    'xc7':    ('synth_xilinx -top sapho_all -family xc7 -flatten -abc9 -nobram -nodsp -nolutram',
               'read_verilog -lib -specify +/xilinx/cells_sim.v'),
    'gowin':  ('synth_gowin -top sapho_all -nobram -nolutram',
               'read_verilog -lib -specify +/gowin/cells_sim.v'),
}

def variant_top(keep):
    s = open(os.path.join(HERE, 'sapho_all.v')).read()
    for p in HEAVY:
        if p not in keep:
            s = re.sub(r'\.%s\(1\)' % p, '.%s(0)' % p, s)
    return s

def run(fam, var):
    d = os.path.join(HERE, 'runs', '%s_%s' % (fam, var))
    shutil.rmtree(d, ignore_errors=True); os.makedirs(d)
    keep = set(HEAVY) if var == 'full' else set(GROUPS.get(var, []))
    open(os.path.join(d, 'sapho_all.v'), 'w').write(variant_top(keep))
    for f in SRC: shutil.copy(os.path.join(HERE, f), d)
    syn, cells = FAMILIES[fam]
    open(os.path.join(d, 'r.ys'), 'w').write(
        'read_verilog -defer sapho_all.v %s\nhierarchy -top sapho_all\n%s\n%s\nsta\n' % (' '.join(SRC), syn, cells))
    subprocess.run([YOSYS, '-q', '-l', 'r.log', 'r.ys'], cwd=d, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    t = None
    try:
        m = re.search(r"Latest arrival time in 'sapho_all' is (\d+)", open(os.path.join(d, 'r.log'), errors='replace').read())
        t = int(m.group(1)) if m else None
    except OSError:
        pass
    return fam, var, t

if __name__ == '__main__':
    fams = sys.argv[1].split(',') if len(sys.argv) > 1 else list(FAMILIES)
    vars_ = ['base', 'full'] + list(GROUPS)
    jobs = [(f, v) for f in fams for v in vars_]
    res = {}
    with cf.ThreadPoolExecutor(max_workers=int(os.environ.get('J', '6'))) as ex:
        for fam, var, t in ex.map(lambda a: run(*a), jobs):
            res[(fam, var)] = t
            print(fam, var, t, flush=True)
    with open(os.path.join(HERE, 'est_results.txt'), 'w') as o:
        for (fam, var), t in sorted(res.items()):
            o.write('%s %s %s\n' % (fam, var, t))
