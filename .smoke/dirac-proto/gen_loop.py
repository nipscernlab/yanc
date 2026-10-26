# proc_rls written with explicit for loops instead of Dirac forms, in the
# operand order each exec_* generator uses, so that the float results can be
# compared bit for bit with the Dirac fixture.
# usage: gen_loop.py N variant out.cmm
#   variant "same": P update with the index exec_Mmvvt uses today (P[r][c] -= K[c] Px[r])
#   variant "fix" : the outer product in the right order       (P[r][c] -= K[r] Px[c])
import sys
N, var, out = int(sys.argv[1]), sys.argv[2], sys.argv[3]
kc, pr = ("K[c]", "Px[r]") if var in ("same", "sub") else ("K[r]", "Px[c]")
# the element expressions; "nat" is the order a direct lowering of the Dirac
# expression writes them (coefficient first, target operand first)
nat = var == "nat"
K_e   = "g*Px[r]" if nat else "Px[r]*g"
w_e   = "w[r] + e*K[r]" if nat else "K[r]*e + w[r]"
P_e   = (f"P[r][c] - {kc}*{pr}" if var in ("sub", "nat") else f"-({kc}*{pr}) + P[r][c]")
Pc_e  = "1.010101*P[r][c]" if nat else "P[r][c]*1.010101"
o_e   = "1000.0*w[r]" if nat else "w[r]*1000.0"
reads = "".join(
    "        for (r = %d; r > 0; r = r - 1) x[r] = x[r-1];\n" % (N - 1)
    + "        x[0] = fin(0)*0.001;\n" for _ in range(N))
src = f"""#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 2
#NUIOOU 2

float x[{N}];
float w[{N}];
float P[{N}][{N}];
float Px[{N}];
float K[{N}];

void rls_update(float d)
{{
    int r; int c;
    float s;

    s = w[0]*x[0];
    for (r = 1; r < {N}; r++) s = s + w[r]*x[r];
    float e = d - s;

    for (r = 0; r < {N}; r++)
    {{
        s = P[r][0]*x[0];
        for (c = 1; c < {N}; c++) s = s + P[r][c]*x[c];
        Px[r] = s;
    }}

    s = x[0]*Px[0];
    for (r = 1; r < {N}; r++) s = s + x[r]*Px[r];
    float g = 1.0/(0.99 + s);

    for (r = 0; r < {N}; r++) K[r] = {K_e};

    for (r = 0; r < {N}; r++) w[r] = {w_e};

    for (r = 0; r < {N}; r++)
        for (c = 0; c < {N}; c++)
            P[r][c] = {P_e};

    for (r = 0; r < {N}; r++)
        for (c = 0; c < {N}; c++)
            P[r][c] = {Pc_e};
}}

void main()
{{
    int r; int c;

    for (r = 0; r < {N}; r++)
        for (c = 0; c < {N}; c++)
            P[r][c] = 0.0;
    for (r = 0; r < {N}; r++) P[r][r] = 1000.0;
    for (r = 0; r < {N}; r++) w[r] = 0.0;

    int j = 0;
    while (j < 20)
    {{
{reads}
        rls_update(in(1)*0.001);

        for (r = 0; r < {N}; r++) out(0, {o_e});

        j++;
    }}

    int ib;
    for (r = 0; r < {N}; r++)
        for (c = 0; c < {N}; c++) {{ copy(P[r][c], ib); out(1, ib); }}
    for (r = 0; r < {N}; r++) {{ copy(w[r], ib); out(1, ib); }}
}}
"""
open(out, "w", encoding="utf-8").write(src)
