-- Modelo do núcleo do bug: arredondamento da mantissa de 23 bits.
-- m = significando normalizado (líder explícito), r = bit de arredondamento (0/1).
def TWO22 : Nat := 4194304   -- 2^22  (menor mantissa normalizada)
def TWO23 : Nat := 8388608   -- 2^23  (tamanho do campo de mantissa)

-- BUGADO: o "+1" do arredondamento estoura e o campo perde o overflow.
def mbug (m r : Nat) : Nat := (m + r) % TWO23

-- CORRIGIDO: ao estourar, re-normaliza (recoloca o líder; o expoente sobe).
def mfix (m r : Nat) : Nat := if m + r = TWO23 then TWO22 else m + r

#eval mbug 8388607 1   -- esperado: 0        ← O BUG (mantissa cheia → 0)
#eval mfix 8388607 1   -- esperado: 4194304  ← a correção

-- A PROVA: para TODA mantissa normalizada, a versão corrigida nunca zera.
theorem mfix_nunca_zera (m r : Nat) (h1 : 4194304 ≤ m) (h2 : m < 8388608) :
    mfix m r ≠ 0 := by
  unfold mfix TWO22 TWO23
  split <;> omega
