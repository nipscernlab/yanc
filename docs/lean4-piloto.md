# Piloto Lean 4 no YANC — runbook + estudo de caso (bug real encontrado)

> Companheiro prático do [lean4-viabilidade.md](lean4-viabilidade.md).
> Aqui está o **passo a passo** para usar métodos formais num trecho do YANC, e
> o **estudo de caso real**: um bug de miscompilação que encontramos e
> consertamos hoje (2026-06-14) seguindo exatamente este método.

---

## 0. Resposta rápida às suas perguntas

**"O Lean só diz 'deu certo / deu errado'? Não diz o que nem onde?"**
Mito parcial. O Lean tem dois modos:

- **Modo prova:** quando a prova fica incompleta, ele te mostra o **objetivo
  restante** (o que falta provar, com todas as hipóteses) — ou seja, *onde* você
  empacou. Mas um "não provei" sozinho não aponta o bug.
- **Modo caça-contraexemplo** (o que usamos para achar bug): com `#eval` ou a
  tática `plausible`, o Lean **te entrega o input exato que quebra a
  propriedade**. Isso é literalmente "o quê e onde", no nível da matemática.
  Você mapeia o input de volta pro C (é o mesmo número).

**"Faço tudo em Lean ou misturo ferramentas? Não quero complicar."**
Para um único kernel: **descoberta toda em Lean; confirmação no próprio
YANC/SAPHO.** Nada além. (No estudo de caso abaixo, a "descoberta" foi até feita
com um harness em C de 20 linhas — ainda mais simples — e o Lean entra como a
prova de que a correção cobre **todos** os casos.)

---

## 1. O passo a passo (o método)

```
[1] Escolher um trecho pequeno, puro e de alta densidade de bug
        (kernel numérico / codificador / folding — NÃO a gramática)
              │
[2] Modelar o trecho em Lean  (model = o que o código REALMENTE faz)
              │
[3] Escrever a "verdade" em Lean  (spec = qual deveria ser a resposta)
              │
[4] Rodar a busca:  #eval/plausible procura input onde model ≠ spec
              │
        ┌─────┴─────────────────────────┐
   achou contraexemplo            não achou nada
        │                                │
[5] Abrir o SAPHO: reproduzir       (ganho menor mas real:
   o input na toolchain real,        evidência de correção;
   ver o valor errado                 vire isso numa PROVA com `by decide`)
        │
[6] Consertar o C  →  re-rodar Lean (sem contraexemplo)
                      re-rodar SAPHO (correto)
        │
[7] (opcional) trocar a busca por uma PROVA  →  garantia "para todo input"
```

Regra de ouro: o passo [3] (escrever o *spec*) é a metade difícil e onde mora o
risco — se o spec estiver errado, você "acha" um bug que não existe. Por isso no
passo [5] **sempre** confirmamos no hardware real antes de declarar bug.

---

## 2. Estudo de caso — bug real encontrado hoje

### 2.1 O alvo

O codificador de float `f2mf` em
[`Compilers/ASMComp/Sources/t2t.c:39`](../Compilers/ASMComp/Sources/t2t.c#L39).
É ele que transforma um literal float em bits e escreve a imagem de memória
(`.mif`) via `itob(f2mf(...))` — **ou seja, é o que o SAPHO carrega de verdade.**
Pequeno, puro, e mexe com arredondamento (onde bugs adoram morar).

> Nota: o `const_eval` (constant folding, `ast.c`) foi o primeiro candidato, mas
> ao ler o código ele se mostrou **correto** (o guard de range bate com o
> wrap-around do hardware mod 2^NUBITS). Bom alvo para *provar correção*, ruim
> para *achar bug*. Trocamos para o `f2mf`, que rendeu na hora.

### 2.2 O bug

No arredondamento, quando a mantissa está cheia (todos os bits 1 = `0x7FFFFF`) e
soma 1, ela **estoura** para `0x800000`. O código **não renormaliza**: esse bit
sobe para o campo do expoente (incrementa certo, ×2) mas deixa a mantissa em
**0**. Como o decodificador `mf2f` usa o "1" líder **explícito**
(`f = m * 2^e`, sem somar 1 implícito), mantissa 0 decodifica como **0.0**.

**Resultado: um número diferente de zero é compilado como ZERO** — na config
padrão 32/23/8 (`NUBITS/NBMANT/NBEXPO`).

### 2.3 A prova (round-trip `float → my-float → float`)

Harness com o código *literal* do `t2t.c`: [`Scripts/f2mf_probe.c`](../Scripts/f2mf_probe.c).
Os bits impressos **são** a linha que iria pro `.mif`.

**Antes da correção:**
```
in=1.99999988079071044921875   bits=01110101100000000000000000000000  out=0   relerr=1  <<< ERRO
in=3.99999976158142089843750   bits=01110110000000000000000000000000  out=0   relerr=1  <<< ERRO
in=0.99999994039535522460937500 bits=01110101000000000000000000000000 out=0   relerr=1  <<< ERRO
```
(A mantissa — os 23 bits finais — saiu toda zero. O hardware carrega 0.0.)

### 2.4 A correção

[`t2t.c:82`](../Compilers/ASMComp/Sources/t2t.c#L82), logo após o arredondamento:
```c
// renormalize: a round-up can overflow the mantissa field (all-ones + 1 ->
// 2^nbmant). The leading 1 is stored explicitly, so letting that bit carry
// into the exponent and leaving the mantissa at 0 would decode as 0.0.
// Shift the mantissa back into range and bump the exponent instead.
if (m >> nbmant) { m = m >> 1; e = e + 1; }
```
Racional: `m` estourou para `2^nbmant`. O valor pretendido é
`2^nbmant · 2^e = 2^(nbmant-1) · 2^(e+1)`, então recolocamos o "1" líder
(`m = 2^(nbmant-1)`) e somamos 1 ao expoente.

**Depois da correção:**
```
in=1.99999988079071044921875   bits=01110101110000000000000000000000  out=2   relerr=5.96e-08  ✓
in=3.99999976158142089843750   bits=01110110010000000000000000000000  out=4   relerr=5.96e-08  ✓
in=0.99999994039535522460937500 bits=01110101010000000000000000000000 out=1   relerr=5.96e-08  ✓
```
Os casos que já passavam (1.0, 1.5, 2.0, 100, π) ficaram **idênticos** — sem
regressão. Loop completo: achado → visto → consertado → verificado.

### 2.5 Reproduzir na toolchain completa (quando o build estiver pronto)

O `make`/`bison`/`flex` (MSYS2) não estavam no PATH na sessão de hoje, então a
prova foi feita com o harness (= código exato do `t2t.c`). Para fechar 100% no
SAPHO, depois de `Scripts/setup.bat`:
```bat
:: 1. um C± de uma linha com o literal problemático
::    float x = 1.99999988079071;
:: 2. cmmcomp -> appcomp -> asmcomp  (gera o .mif)
:: 3. abrir Hardware/<proc>.mif e ver a linha do x = 0...0  (mantissa zerada)
:: 4. rebuildar asmcomp com a correção -> a linha do .mif agora tem a mantissa certa
```

### 2.6 Achados relacionados (leads honestos, a confirmar)

Mesma família de função, **ainda não confirmados em execução** — candidatos para
o próximo ciclo:

1. **`t2f`/`f2mf` ramo `nbmant == 23`, denormais:** o deslocamento `sh` contado no
   laço de denormal **não é aplicado** à mantissa nesse ramo (só no `else`).
   Floats minúsculos (≈ 2⁻¹⁰⁷…2⁻¹²⁶ na 32/23/8) sairiam grandes demais por 2^sh.
   *Teste:* rodar o harness com `1e-35`, `1e-37`.
2. **`f2mf` do `cmmcomp`** ([`variaveis.c:164`](../Compilers/CMMComp/Sources/variaveis.c#L164)):
   o `if (f == 0.0) {*m=0;*e=0;}` **não tem `return`** e é sobrescrito. Aqui é
   mascarado (só alimenta um aviso de aproximação, suprimido quando `num==0`),
   mas é a mesma classe de bug — vale alinhar com o `t2t.c`.

---

## 3. Onde o Lean entra (o backstop formal)

O harness em C achou o bug em **3 exemplos**. Isso não prova que a correção
cobre **todos** os inputs. É exatamente para isso que serve o Lean: provar a
propriedade para o espaço inteiro.

### 3.1 Instalar (uma vez, ~15 min)
```powershell
winget install Lean.Elan        # gerenciador de versões do Lean
elan default stable             # baixa o Lean 4 estável
lake new bughunt                # projeto mínimo (sem Mathlib, leve)
```
Instale a extensão **"Lean 4"** no VS Code (publisher `leanprover`) — ela mostra
a saída do `#eval` e os objetivos da prova no painel *Infoview*.

### 3.2 O experimento (esqueleto — `bughunt/Bug.lean`)

Modela a essência: dado um significando de 23 bits e um bit de arredondamento,
o encode bugado pode zerar a mantissa; o corrigido renormaliza. A propriedade:
**o valor decodificado nunca é zero para entrada não-zero.**

```lean
-- significando de entrada: [2^22, 2^23-1] (com "1" líder explícito) + bit de round
def nbmant : Nat := 23

-- encode BUGADO: arredonda e deixa estourar (mantissa pode virar 0 no campo)
def encBug (m : Nat) (round : Bool) : Nat :=
  let m := if round then m + 1 else m
  m % (2 ^ nbmant)            -- o campo de mantissa "esquece" o overflow

-- encode CORRIGIDO: renormaliza
def encFix (m : Nat) (round : Bool) : (Nat × Nat) :=  -- (campo_mantissa, bump_expo)
  let m := if round then m + 1 else m
  if m >>> nbmant != 0 then (m >>> 1 % (2^nbmant), 1) else (m % (2^nbmant), 0)

-- propriedade: para significando cheio que arredonda, o BUG zera a mantissa...
#eval encBug (2^nbmant - 1) true            -- => 0   (o bug!)
-- ...e a correção não:
#eval encFix (2^nbmant - 1) true            -- => (4194304, 1)  (mantissa = 2^22, expo +1)

-- versão "busca de contraexemplo" sobre todos os significandos do range:
#eval (List.range (2^nbmant)).filter (fun m =>
        m >= 2^(nbmant-1) && encBug m true == 0)   -- lista todos os m que o bug zera
```

No editor, o `#eval` que lista os `m` problemáticos é o "Lean te dizendo o quê e
onde". Depois da correção, a versão com `encFix` retorna lista vazia.

### 3.3 Virar prova (a garantia "para todo input")
```lean
-- com a correção, mantissa não-zero de entrada nunca decodifica como zero:
theorem fix_nao_zera (m : Nat) (h : 2^(nbmant-1) ≤ m) (h2 : m < 2^nbmant) (r : Bool) :
    (encFix m r).1 ≠ 0 := by decide   -- (ou prova por casos; `decide` exige domínio finito)
```
> Modelar o `f2mf` **fielmente** (extraindo bits do IEEE via `Float.toBits`/
> `UInt32`) é o passo seguinte — eu te ajudo a escrever e depurar isso ao vivo
> assim que o Lean estiver instalado.

---

## 4. Resumo — o que ganhamos hoje

- ✅ **Bug real, vivo, na config padrão**, no codificador que vira bits do SAPHO.
- ✅ **Demonstrado** (round-trip → 0) e **consertado** (renormalização) **sem regressão**.
- ✅ Método validado de ponta a ponta: ler → modelar → buscar → ver → consertar.
- ▶️ **Próximo passo Lean:** instalar e transformar a verificação dos 3 exemplos
  numa **prova para todos os inputs**, e checar os 2 leads relacionados (§2.6).

> O Lean não *substitui* o harness/teste (que acha bugs barato); ele *garante*
> que a correção não deixou nenhum caso de fora. São complementares — exatamente
> a tese do [estudo de viabilidade](lean4-viabilidade.md).
