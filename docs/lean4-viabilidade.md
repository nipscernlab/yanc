# Plano de Viabilidade — Lean 4 para validação dos compiladores do YANC

> Estudo de viabilidade da adoção de **Lean 4** (e métodos formais em geral) para
> validar a toolchain de compilação **YANC**, parte do ecossistema **SAPHO**
> (*Scalable Architecture for Hardware Optimization*), ao lado da GUI **Aurora**.
>
> Documento técnico de decisão — escrito para um laboratório acadêmico pequeno.
> Data: 2026-06-14. Status: **recomendação fechada (ver §1)**.

---

## 1. Sumário executivo (a resposta curta)

**Vale a pena verificação formal *completa* (estilo CompCert) do YANC? Não.**
Para uma equipe acadêmica pequena, provar formalmente toda a toolchain
(C±/C++ → assembly → Verilog) é **inviável**: o esforço de referência é da ordem
de **6 pessoas-ano e ~100 mil linhas de prova** só para um compilador C
(CompCert), e o YANC tem *dois* front-ends, um back-end que gera **Verilog**, e
um alvo com **float não-IEEE de largura configurável**. O custo não fecha.

**Vale a pena usar Lean 4 / métodos formais *pontualmente*? Sim — em alvos
pequenos, estáveis e de alta densidade de bugs.** A análise do histórico de bugs
do próprio YANC mostra que **~40% dos bugs reais são semânticos/numéricos**
(exatamente o que uma prova pega), e existem "ilhas" verificáveis de altíssimo
valor e baixa superfície: os **kernels numéricos** (`atan2`/`fase`, conversão de
float `f2mf`, *constant folding*, transcendentais complexos) e a **consistência
da tabela de opcodes** entre montador e hardware.

**Recomendação (faseada, com gates de go/no-go — detalhe em §9):**

1. **Agora, sem Lean (semanas):** fortalecer o que já existe — *fuzzing* estilo
   CSmith + sanitizers (ASan/UBSan) para a classe *memory-safety*, e um
   **checador de consistência da tabela de opcodes** (decidível, barato).
   Isso ataca ~60% dos bugs com baixíssimo custo.
2. **Piloto Lean limitado (1–2 meses, 1 pessoa):** formalizar **um** kernel de
   alto valor — recomendo `atan2`/`fase` (quebrou **duas vezes**) ou a conversão
   `f2mf` IEEE→float-SAPHO. Objetivo duplo: pegar bugs reais **e** construir
   capacidade na equipe.
3. **Gate de decisão:** só expandir o uso de Lean se o piloto (a) encontrar/
   prevenir um bug real e (b) couber no orçamento de tempo da equipe. Caso
   contrário, manter Lean restrito aos kernels e investir o resto em
   teste/fuzzing.

**Uma IA substituiria o Lean 4? Não — elas respondem perguntas diferentes**
(detalhe em §7). IA *aumenta a probabilidade de você achar bugs barato*; Lean
*dá uma prova checada por máquina de que uma classe de bugs não existe* (dentro
de uma especificação). São **complementares**, não substitutos.

---

## 2. O que é o YANC hoje (linha de base honesta)

YANC ("Yet Another Compiler") é a espinha dorsal de compilação do SAPHO. Pega um
programa em **C±** (C-like com ponto-fixo, ponto-flutuante, complexos e operadores
em notação de Dirac de primeira classe) ou em **C++** e o leva até um core SAPHO
sintetizável + imagens de memória + testbench.

### 2.1 Componentes e tamanho

| Binário | Diretório | Construído com | Papel | ~LoC |
| --- | --- | --- | --- | --- |
| `cmmcomp` | `Compilers/CMMComp/` | Flex + Bison + GCC | Front-end C± → assembly | ~16.000 |
| `cppcomp` | `Compilers/CPPComp/` | Flex + Bison + GCC | Front-end C++ → assembly | ~7.800 |
| `asmcomp` | `Compilers/ASMComp/` | Flex + GCC | Back-end: assembly → Verilog + `.mif` + testbench | ~4.100 |
| `appcomp` | `Compilers/APPComp/` | Flex + GCC | 1ª passada: resolve endereços/params | ~660 |
| `cpppp` | `Compilers/CPPComp/` | GCC | Pré-processador C++ | ~730 |
| `comp2gtkw`, `gen_gtkw` | `Scripts/` | GCC | Visualização de forma de onda | — |

Alvo: **core SAPHO**, ~3.460 linhas de Verilog (`HDL/`), das quais a ULA
(`HDL/ula.v`) sozinha tem ~1.450. Maiores fontes C: `stdlib.c` (3.508),
`oper.c` (3.310), `codegen.c` do C++ (3.280), `array_index.c` (2.340).

### 2.2 O que já existe de garantia de qualidade (a régua que o Lean tem de bater)

O YANC **já tem um harness de teste diferencial sério** (`Scripts/regress.sh`):

- **Golden `.asm`**: compila cada teste e compara o assembly gerado contra um
  *golden* versionado.
- **Diferencial de simulação end-to-end**: `appcomp + asmcomp + iverilog/verilator`,
  comparando `output_*.txt` contra `golden_sim/`.
- **Testes negativos** (`CMMComp/NegTests/manifest.txt`): garante que programas
  malformados são **rejeitados de forma limpa** (saída não-zero, nunca *crash*,
  nunca aceitação silenciosa).
- **CI** (`.github/workflows/`) em Windows + Linux.

**Conclusão importante:** o YANC não está partindo do zero em qualidade. Qualquer
proposta de Lean precisa entregar valor *acima* dessa linha de base já boa — e é
isso que torna a verificação total ainda menos atraente, e os alvos pontuais mais
atraentes.

---

## 3. Para que serve o Lean 4? (sem hype)

Lean 4 é **duas coisas no mesmo artefato**:

1. **Um provador interativo de teoremas (ITP)** baseado em teoria de tipos
   dependentes (Cálculo de Construções Indutivas). Você escreve uma
   *especificação* (o que "correto" significa) e uma *prova*; o **núcleo (kernel)**
   do Lean checa a prova. Se passa, é um **certificado dedutivo** verificado por
   máquina.
2. **Uma linguagem de programação funcional** (compila para C). O próprio Lean 4 é
   escrito em Lean.

A base é o isomorfismo **Curry-Howard** ("provas como programas"): uma proposição
é um tipo, uma prova é um termo desse tipo, e **checar a prova é type-checking**.
A confiança se concentra no **kernel pequeno** (critério de de Bruijn) +
`Mathlib` (biblioteca de matemática).

### 3.1 O que uma prova em Lean **garante**

- Que **uma propriedade vale para TODOS os casos** (não uma amostra), *dentro da
  especificação escrita*. Ex.: "para todo `x` representável, `f2mf(x)` produz o
  float-SAPHO corretamente arredondado mais próximo de `x`".
- Ausência de uma **classe inteira de bugs** — é o oposto de teste. Lembre de
  Dijkstra: *"testar pode mostrar a presença de bugs, nunca a ausência"*. Lean é
  uma máquina de **ausência** (dentro do spec).

### 3.2 O que uma prova em Lean **NÃO garante** (limites honestos)

- **Não garante que sua especificação está certa.** Se você especifica a coisa
  errada, prova a coisa errada. Escrever o *spec* é metade do trabalho e é onde
  mora o risco.
- **Não cobre o que está fora da base confiável**: o kernel do Lean, os axiomas,
  o *parser*/elaborador, o hardware real, e — crucial para o YANC — **a semântica
  do Verilog gerado e do core SAPHO**, que teria de ser formalizada à parte.
- **Não é grátis de manter**: a prova quebra quando o código muda (ver §6).
- Comparado a Coq/Rocq, HOL4, Isabelle/HOL: Lean é excelente para matemática
  (Mathlib), mas **seu track record específico em verificação de compiladores é
  fino** — não existe um "CompCert do Lean" (ver §5).

---

## 4. O que significaria, concretamente, "validar o YANC com Lean"

"Validar o compilador" formalmente quer dizer provar **preservação semântica**:
*para todo programa fonte S, se o compilador produz C sem erro, então o
comportamento observável de C é um dos comportamentos permitidos de S.*

Para o YANC isso exigiria, em cascata:

1. Uma **semântica formal de C±** (e/ou do subconjunto C++).
2. Uma **semântica formal do assembly SAPHO**.
3. Uma **semântica formal da ISA SAPHO** — incluindo a ULA com **float
   configurável não-IEEE** (`NUBITS`/`NBMANT`/`NBEXPO` são parâmetros de build:
   nos testes aparecem 32/23/8, 16/10/5, 23/16/6). Sem NaN/Inf, com tratamento
   especial de zero e denormais que descartam bits.
4. A prova de que cada passo do compilador **preserva** essa semântica.
5. Para ir até o silício: uma **semântica formal de Verilog** + prova do core.

Os itens 3 e 5 são **custos ocultos enormes** e independentes do compilador.
É por isso que o caminho realista (§9) **evita** a prova end-to-end e mira nas
ilhas verificáveis.

### 4.1 As "ilhas verificáveis" reais no YANC (mapa de alvos)

A análise do código identificou alvos concretos, ordenados por custo-benefício:

**Alvos de ALTO valor / BAIXA superfície (bons para Lean ou SMT):**

- **`atan2`/`fase`** (`CMMComp/Sources/stdlib.c`): o alvo mais rico — quebrou
  **duas vezes** (computação de fase e depois `F_LES` invertido dando quadrante
  errado + divisão por zero em `real==0`). Spec pequeno e total: `atan2(b,a)` em
  `(-π, π]` com os 4 quadrantes + 4 eixos + origem.
- **`f2mf`** (`variaveis.c:164-198`): conversão IEEE-754-32 → float-SAPHO. Função
  bit-level pura; provar "arredondamento correto para o representável mais
  próximo" dado `nbmant`/`nbexpo`.
- **Constant folding** (`ast.c:391-431`): provar que dobrar uma subexpressão só
  ocorre quando é observacionalmente equivalente a não dobrar (sem divergência de
  *wraparound*). Teorema-texto de *constant folding soundness*.
- **Folding por identidade algébrica** (`ast.c` `is_const_value`): cada regra
  (`x+0→x`, `x*1→x`, …) preserva valor e tipo. (O bug `77b9005` dobrou
  `(1+2i)*(3+4i)` indevidamente.)
- **Transcendentais complexos** (`stdlib.c`): `exp/sin/cos/tan/log/sqrt/conj`
  sobre `comp`, compostos de identidades fechadas → provavelmente equivalentes à
  composição das ops reais.

**Alvo BARATO e decidível (nem precisa de Lean — dá com SMT/BMC ou script):**

- **Consistência da tabela de opcodes**: o mesmo número de opcode é mantido **à
  mão em 3 lugares** — montador (`ASMComp.l`), decodificador (`instr_dec.v`) e mux
  da ULA (`ula.v`). Uma checagem de equivalência finita (decidível) elimina uma
  classe inteira de erros silenciosos. **Win quase de graça.**
- **Decodificação `b5..b0`** (`instr_dec.v:381-402`) vs. a tabela dourada: prova
  por SAT/BMC exaustivo sobre o opcode de 7 bits.

**Alvos INVIÁVEIS (não tente):**

- **A gramática C++ de 2310 linhas** (`CPPComp.y`): conflitos shift/reduce
  resolvidos **silenciosamente** pelo default do Bison (sem `%expect`, sem GLR) +
  feedback léxico dependente da tabela de símbolos. Não há gramática declarativa
  para verificar contra. **Obstáculo dominante.**
- **Prova source-to-silicon completa**: 5 binários + Verilog + float não-IEEE.

---

## 5. O panorama de compiladores verificados (onde o Lean se encaixa)

| Projeto | Ferramenta | O que garante | Escala/esforço |
| --- | --- | --- | --- |
| **CompCert** | Coq/Rocq | Preservação semântica C → Asm | ~100k linhas Coq, **~6 pessoas-ano**; ~86% é prova |
| **CakeML** | HOL4 | Compilador ML verificado **até código de máquina** | Múltiplos pessoas-ano |
| **seL4** | Isabelle/HOL | Microkernel verificado | ~20+ pessoas-ano |
| **Vellvm** | Coq | Semântica formal do LLVM IR | Projeto de pesquisa grande |
| **Sail / riscv-coq** | Coq/HOL/Isabelle | Semântica formal de ISA | Referência para hardware |

**Dois fatos que definem a decisão:**

1. **CompCert funciona**: o CSmith encontrou **79 bugs no GCC e 202 no LLVM**, mas
   **zero bugs de *wrong-code* no núcleo verificado do CompCert**. Prova formal
   entrega de verdade.
2. **Mas os ~6 bugs que o CSmith achou no CompCert estavam todos no front-end
   *não-verificado*** (parsing, elaboração, promoções de inteiro). **Lição direta
   para o YANC**: o front-end (onde mora o C±/C++) é justamente a parte mais
   difícil de verificar e onde os bugs persistem.

E o ponto crucial para esta decisão: **o ecossistema maduro de verificação de
compiladores é Coq/HOL/Isabelle, não Lean.** Não existe um CompCert em Lean. Se a
motivação for "queremos a ferramenta com mais precedente em compiladores", o Lean
**não** é a escolha óbvia — Coq/Rocq tem muito mais. O Lean brilha em
*matemática* (provar os kernels numéricos), que por acaso é onde o YANC mais
sangra.

---

## 6. O custo real (por que quase ninguém verifica compilador inteiro)

- **CompCert (números originais, Leroy/CACM 2009):** 42.000 linhas de Coq, ~3
  pessoas-ano; destas, só **14% definem o algoritmo de compilação** e **10% a
  semântica** — os **76% restantes são a prova**. Cada passe levou 1.500–3.000
  linhas de prova.
- **CompCert (números atuais, AbsInt):** ~100.000–135.000 linhas de Coq,
  **~6 pessoas-ano**. A prova/spec é **~6× o tamanho do compilador**.
- **Curva de aprendizado:** dominar um ITP (Lean/Coq) é uma habilidade escassa e
  leva meses até produtividade real. Para uma equipe que **não conhece Lean
  hoje**, some isso ao cronograma.
- **Fragilidade/manutenção:** toda mudança no compilador pode **quebrar provas**.
  Num projeto ativo (o YANC tem 662 commits, 84 de fix), isso é um imposto
  recorrente. A verificação favorece código **estável** — outra razão para mirar
  os kernels numéricos (estáveis) e não o codegen (em evolução).

---

## 7. Uma IA substituiria o Lean 4? (resposta direta)

**Não. IA e Lean respondem perguntas fundamentalmente diferentes.**

- Um **LLM aumenta a probabilidade de você achar bugs barato**: é excelente em
  gerar testes, *fuzzers*, mutadores, ler um passe e levantar hipóteses de onde
  quebra, e até **rascunhar specs e provas**. *Fuzzing* dirigido por LLM
  (WhiteFox, MetaMut/Mut4All) já supera CSmith/YARPGen em alguns benchmarks; há
  relatos de **100+ defeitos de compilador achados em 72h**. Isso é a zona doce
  da IA: entrada criativa e barata num espaço de busca gigante.
- Um **LLM é probabilístico, não dedutivo**: a saída dele **não é uma prova
  checada por máquina**. Ele pode dizer "parece correto" — não pode **garantir**
  ausência de uma classe de bugs. Nenhuma melhora no LLM transforma a primeira
  coisa na segunda (é a distinção *presença* vs. *ausência* de bugs).
- **A fronteira é complementar, não substitutiva:** os provadores de teorema mais
  avançados com IA (**AlphaProof**, **Lean Copilot**, *neural theorem proving*)
  são construídos **em cima do checador do Lean** — usam a IA para *propor* a
  prova e o **kernel do Lean para *checá-la***. Ou seja: o estado da arte usa IA
  **e** Lean juntos, com o Lean como árbitro final de verdade.

**Tradução prática para o YANC:** use IA (inclusive este tipo de assistente) para
gerar testes, *fuzzers*, e para **acelerar a escrita das provas Lean** dos
kernels. Não espere que a IA *substitua* a garantia que só uma prova checada dá —
nem que a prova substitua a IA na hora de caçar bugs barato.

---

## 8. Estratégias avaliadas

Notação: **Esforço** (↑ = mais caro), **Garantia** (↑ = mais forte),
**Aderência** ao YANC (↑ = melhor encaixe hoje).

### A. Verificação total do front-end em Lean (estilo CompCert)
- **O que é:** semântica de C± + assembly SAPHO + prova de preservação.
- **Esforço:** altíssimo (pessoas-ano). **Garantia:** máxima. **Aderência:** baixíssima.
- **Veredito:** ❌ **Inviável** para a equipe. Precisaria formalizar a ISA e o
  float não-IEEE antes mesmo de começar o compilador. Não recomendado.

### B. Validação de tradução / certificado por compilação
- **O que é:** em vez de provar o compilador uma vez, emitir um **certificado
  checável a cada compilação** e verificar equivalência fonte↔gerado para *aquele*
  programa (em Lean ou via checador SMT).
- **Esforço:** médio-alto. **Garantia:** alta (por execução). **Aderência:** média.
- **Veredito:** ⚠️ Interessante a médio prazo, mas ainda exige semântica do alvo.
  Melhor como evolução *depois* do piloto, não como ponto de partida.

### C. ISA + kernels numéricos em Lean
- **O que é:** formalizar a semântica da ISA SAPHO (a parte inteira/bitwise é
  barata) + **verificar a biblioteca de kernels numéricos** (`atan2`, `f2mf`,
  folding, transcendentais) + a **consistência de opcodes**.
- **Esforço:** médio (focado). **Garantia:** alta **onde mais dói**. **Aderência:** alta.
- **Veredito:** ✅ **Melhor uso de Lean.** Menor superfície, maior densidade de
  bugs, código estável, matematicamente especificável — o ponto forte do Lean.

### D. Pilha pragmática **sem Lean**
- **O que é:** manter golden+sim, **adicionar** geração aleatória de programas
  (CSmith-style), *property-based testing*, e **ASan/UBSan/fuzzing** para a classe
  *memory-safety* (hoje o repo tem **zero** infra de sanitizer/fuzzing).
- **Esforço:** baixo. **Garantia:** média (probabilística, mas larga). **Aderência:** altíssima.
- **Veredito:** ✅ **A linha de base obrigatória.** Maior retorno por esforço no
  curto prazo; ataca os ~18% de bugs *memory-safety* que o Lean **não** cobre.

### E. Híbrido faseado (D agora + piloto C com gate)
- **O que é:** D imediatamente; **um** kernel verificado em Lean como piloto;
  decisão go/no-go para expandir.
- **Esforço:** baixo→médio, incremental. **Garantia:** crescente. **Aderência:** altíssima.
- **Veredito:** ✅✅ **Recomendado** (ver §9). Constrói capacidade sem apostar o
  cronograma; cada fase entrega valor isolado.

---

## 9. Recomendação e roadmap (com gates de go/no-go)

### Fase 0 — Linha de base pragmática (semanas, sem Lean)
- Adicionar **ASan + UBSan** ao build de CI e rodar os testes existentes sob eles.
  *(Ataca a classe memory-safety: buffer overflow no `cpppp`, `fopen` NULL etc.)*
- Adicionar um **gerador de programas C±/C++ aleatórios** (estilo CSmith) +
  diferencial contra a simulação que já existe.
- Implementar o **checador de consistência da tabela de opcodes** (`ASMComp.l` ↔
  `instr_dec.v` ↔ `ula.v`) — decidível, barato, pega erro silencioso de execução.
- **Gate 0 →:** infra estável e rodando em CI. *(Esta fase vale a pena
  independentemente do Lean.)*

### Fase 1 — Piloto Lean de um kernel (1–2 meses, 1 pessoa)
- Escolher **um** alvo: **recomendo `atan2`/`fase`** (quebrou 2×) **ou** `f2mf`.
- Escrever o **spec** (a parte difícil) + a prova em Lean; conectar com o que o
  hardware/macro realmente computa (modelo do float-SAPHO no nível necessário).
- Usar IA para acelerar rascunho de spec/prova; o kernel do Lean é o árbitro.
- **Gate 1 (go/no-go):** o piloto (a) achou/preveniu um bug real **ou** deu
  confiança mensurável, **e** (b) coube no orçamento de tempo?
  - **Sim →** Fase 2. **Não →** congelar Lean; manter D; documentar o aprendizado.

### Fase 2 — Expansão seletiva (condicional)
- Verificar os demais kernels numéricos + decodificação de opcode por BMC.
- Avaliar **validação de tradução** (estratégia B) para o codegen, se houver
  fôlego.
- **Nunca** mirar a gramática C++ nem a prova end-to-end.

---

## 10. O que **NÃO** fazer

- ❌ Tentar verificar o compilador **inteiro** ou a **gramática C++**.
- ❌ Tratar Lean como **substituto** de teste/fuzzing — ele cobre uma classe
  diferente (semântica), não a *memory-safety* nem o build/infra (~63% dos bugs
  juntos).
- ❌ Adotar Lean **antes** de ter a pilha pragmática (Fase 0) — seria otimizar a
  parte cara antes da barata.
- ❌ Esperar que uma **IA** entregue garantia: ela acelera, não certifica.
- ❌ Escolher Lean "porque é da moda" sem notar que o **precedente em
  compiladores está em Coq/Rocq** — o caso do Lean aqui é a *matemática dos
  kernels*, não o compilador como um todo.

---

## 11. Vantagens e desvantagens reais (resumo)

### Vantagens de adotar Lean (pontualmente)
- **Garantia categórica** de ausência de bugs numéricos numa classe — algo que
  teste nenhum dá.
- Os kernels do YANC são **matemática pura**, o ponto mais forte do Lean/Mathlib.
- Escrever o **spec** força clareza sobre o que "correto" significa (valor mesmo
  que a prova nunca termine).
- Constrói **capacidade** acadêmica diferenciada (publicável).

### Desvantagens / riscos reais
- **Custo e curva de aprendizado** altos; habilidade escassa.
- **Spec errado = prova inútil**; o risco migra para a especificação.
- **Fragilidade**: provas quebram quando o código evolui (imposto recorrente).
- **Cobertura parcial**: não pega memory-safety, build/infra, nem o Verilog
  gerado/ISA sem investimento adicional enorme.
- **Lean tem pouco precedente em compiladores** (vs. Coq) — mais terreno novo.

---

## 12. Conclusão

Para o YANC, **a pergunta certa não é "verificar o compilador?", e sim "verificar
o quê?"**. Verificação total é inviável e desnecessária; a linha de base de teste
já é boa. O ganho real e realista vem de **(1) uma pilha pragmática de
fuzzing/sanitizers + checador de opcodes** (ataca ~60% dos bugs, custo baixo) e
**(2) um uso cirúrgico de Lean nos kernels numéricos** (ataca os bugs semânticos
mais teimosos, onde o Lean é mais forte). A IA é uma **aliada** em ambas as
frentes — acelera testes e provas — mas **não substitui** a garantia que só uma
prova checada por máquina entrega. Comece pequeno, meça no Gate 1, e expanda só
se o piloto pagar.

---

## 13. Apêndice — Taxonomia de bugs do YANC (o dado decisivo)

Categorização dos ~30 fixes "reais" (excluindo warnings/churn) do histórico:

| Classe | % aprox. | Lean ajuda? | Exemplos (commits) |
| --- | --- | --- | --- |
| **(a) Semântico / numérico** | **~40%** | ✅ **Sim** | `atan2`/`fase` quadrante errado + div-por-zero (`648063f`, `872b154` — 2×); folding de literal complexo (`77b9005`); subtração float invertida `SF_SU1`/`SF_SU2` (`88f0d9b`); operando de memória invertido p/ SUB/DIV/MOD não-comutativo (`29e3d9e`, `a3a74b4`); F2I floor-vs-truncate (`a2753a2`); JIZ testa só bit 0 (`542ad7d`); cast int→float corrompe `cur_base` (`e9c0b04`); indexação de matriz (`3f0af20`); índice de FFT (`735025e`) |
| **(b) Memory-safety / crash** | **~18%** | ❌ Não (fuzzing/sanitizer) | overflow em `path_canon` do `cpppp` (`0f3898f`); `fopen` NULL → segfault (`1f02dc9`); `sprintf`→`snprintf` em paths (`e9927bb`, `5bb1363`); destrutor virtual recursivo → corrupção de heap (`704cea6`) |
| **(c) Build / infra / path / HDL-lint** | **~45%** | ❌ Não | caminhos de ferramenta fantasma (`80fce2d`); dir `C:` lixo (`25e1fbd`); `TMP/TEMP` na cadeia bash→make→g++ (`3ad7ace`); DTW flaky (`cc26403`); campanha de limpeza Verilator (`2a0be92`, `d8dc8f6`, …) |

> Observação: o repositório **não tem** infra de ASan/UBSan/Valgrind/fuzzing hoje
> — os bugs de memory-safety foram achados **à mão** escrevendo exemplos. É o
> *gap* mais barato de fechar (Fase 0).

---

## 14. Referências

- CompCert — semântica preservada, estrutura da prova:
  <https://compcert.org/man/manual001.html>,
  <https://www.absint.com/compcert/structure.htm>,
  <https://xavierleroy.org/publi/compcert-backend.pdf>
- CompCert vs. CSmith (79 bugs GCC / 202 LLVM / 0 no núcleo verificado):
  Yang et al., PLDI'11 — <https://users.cs.utah.edu/~regehr/papers/pldi11-preprint.pdf>;
  <https://blog.regehr.org/archives/492>
- Custo/escala (6 pessoas-ano, ~86% prova): Leroy, CACM 2009;
  "Lessons from Formally Verified Deployed Software Systems", ACM Computing
  Surveys 2025 / arXiv:2301.02206
- Lean 4 (descrição do sistema, kernel, Curry-Howard):
  <https://lean-lang.org/papers/lean4.pdf>; survey arXiv:2501.18639;
  Quanta (2023) sobre Curry-Howard
- IA + provadores (fuzzing por LLM; AlphaProof/Lean Copilot sobre o Lean):
  WhiteFox arXiv:2310.15991; survey de compiler fuzzing arXiv:2306.06884
- CakeML (compilador ML verificado até código de máquina): <https://cakeml.org>
- seL4 (referência de método formal): <https://sel4.systems>

---

> **Nota de método:** este documento foi produzido a partir de (1) leitura direta
> do código do YANC (`CMMComp`, `CPPComp`, `ASMComp`, `APPComp`, `HDL/`,
> `Scripts/regress.sh`, `CHANGELOG.md` e `git log`) e (2) pesquisa web com
> verificação adversarial das afirmações externas (números do CompCert, track
> record do Lean, capacidades de IA). A taxonomia de bugs (§13) vem da
> categorização do histórico real de commits do repositório.
