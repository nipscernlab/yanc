# Negative fixtures of the Dirac expressions (step a) and their manifest lines.
D = 'C:/nipscern/yanc-dirac/Compilers/CMMComp/NegTests/'
HEAD = """#NUBITS 32
#NBMANT 23
#NBEXPO 8
#NUIOIN 1
#NUIOOU 1

"""
DECL = """float a[2];
float b[3];
float M[2][2];
float N[3][2];
float v[2];
"""
cases = [
 ("dirac_sum_shape", "a vector and a matrix added: the shapes differ",
  "    v # |a⟩ + |M|;", "the shapes must be the same"),
 ("dirac_product_shape", "a 3x2 matrix times a vector of 3: the inner sizes differ",
  "    v # |N|b⟩;", "do not match the rows of the second"),
 ("dirac_target_shape", "a vector of 3 into a vector of 2",
  "    v # 2.0|b⟩;", "and the expression gives"),
 ("dirac_not_scalar", "a vector where a number is needed",
  "    float x;\n    x = |a⟩;", "where a number is needed"),
 ("dirac_out_matrix", "out() of a matrix",
  "    out(0, 2.0|M|);", "writes a number or a vector"),
 ("dirac_not_matrix", "a 1D array between bars, in the place of a matrix",
  "    v # |a|a⟩;", "ain't even a matrix"),
 ("dirac_not_vector", "a 2D array in the place of a ket",
  "    v # 2.0|M⟩;", "ain't even a vector"),
 ("dirac_scalar_ket", "a number written as a ket",
  "    float x;\n    v # 2.0|x⟩;", "ain't even a vector"),
 ("dirac_braket_one", "one name between angles, with no bar",
  "    float x;\n    x = ⟨a⟩;", "Syntax error"),
 ("dirac_not_yet", "a valid form that step a does not compute yet (docs/design/dirac-expressions.md)",
  "    v # |a⟩ - |a⟩;", "not implemented yet"),
]
lines = []
for name, why, body, expect in cases:
    src = HEAD + "// Negative fixture: " + why + ".\n" + DECL + "\nvoid main()\n{\n" + body + "\n}\n"
    open(D + name + ".cmm", "w", encoding="utf-8", newline="").write(src.replace("\n", "\r\n"))
    lines.append(f"{name}.cmm|{expect}")

ok = HEAD + """// Valid fixture: a bar before an indexed array element is still the
// bitwise or (c | a[0]), not a Dirac bar.
int a[4];

void main()
{
    int c = 1; int d = 2;
    a[0] = 4;
    out(0, c|d|a[0]);
    out(0, c | a[1]);
}
"""
open(D + "dirac_or_index.cmm", "w", encoding="utf-8", newline="").write(ok.replace("\n", "\r\n"))

def append(p, text):
    s = open(p, encoding="utf-8", newline="").read()
    if not s.endswith("\r\n"): s += "\r\n"
    s += text.replace("\n", "\r\n")
    open(p, "w", encoding="utf-8", newline="").write(s)

append(D + "manifest.txt", "\n# Dirac notation as expressions: shapes and the terms of a chain\n" + "\n".join(lines) + "\n")
append(D + "nowarn.txt", "dirac_or_index.cmm|Syntax error\n")
print("\n".join(lines))
