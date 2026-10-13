// ----------------------------------------------------------------------------
// yanc -- the driver: one command from the source to the SAPHO processor -------
// ----------------------------------------------------------------------------
// As gcc runs cpp, cc1 and as, yanc runs the YANC tools found next to it:
//
//   yanc prog.cmm        cmmcomp, then asmcomp  -> prog/prog.v (+ testbench)
//   yanc prog.cpp        cppcomp, then asmcomp
//   yanc -S prog.cmm     the compiler only      -> prog/prog.asm
//   yanc prog/prog.asm   asmcomp only           -> prog/prog.v
//
// It stops at the Verilog (no simulation) and at the first tool that fails,
// returning that tool's exit code. -v shows each command before it runs.
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <process.h>
#define EXE ".exe"
#else
#include <unistd.h>
#include <sys/wait.h>
#define EXE ""
#endif

#include "../yanc_version.h"
#include "../common/exe_dir.h"
#include "../common/yanc_lang.h"
#include "../common/console_utf8.h"

static int lang_en = 1;
#define M(pt, en) (lang_en ? (en) : (pt))

static void usage(FILE *f)
{
    fprintf(f, M(
        "yanc %s - do fonte ao processador SAPHO\n"
        "\n"
        "Uso:\n"
        "  yanc [opções] <arq.cmm | arq.cpp | arq.asm>\n"
        "\n"
        "Compila o programa (cmmcomp ou cppcomp) e gera o processador (asmcomp):\n"
        "o Verilog, as memórias e o testbench, numa pasta com o nome do programa.\n"
        "Um .asm só é montado, na pasta dele.\n"
        "\n"
        "Opções:\n"
        "  -o <dir>         pasta de saída (padrão: ./<nome>)\n"
        "  -S               só compila, para no <nome>.asm\n"
        "  -I <dir>         pasta extra de #include (C++; repetível)\n"
        "  -f <MHz>         clock do testbench (padrão: 100)\n"
        "  -c <n>           clocks que o testbench simula (padrão: 10000)\n"
        "  --wave-arrays    cada elemento de vetor vira um sinal na forma de onda (C±)\n"
        "  -v               mostra cada comando antes de rodar\n"
        "  --lang=en|pt     idioma das mensagens (padrão: en, ou YANC_LANG)\n"
        "  -h, --help       mostra esta ajuda e sai\n"
        "  -V, --version    mostra a versão e sai\n",
        "yanc %s - from the source to the SAPHO processor\n"
        "\n"
        "Usage:\n"
        "  yanc [options] <file.cmm | file.cpp | file.asm>\n"
        "\n"
        "Compiles the program (cmmcomp or cppcomp) and builds the processor\n"
        "(asmcomp): the Verilog, the memories and the testbench, in a folder named\n"
        "after the program. A .asm is only assembled, in its own folder.\n"
        "\n"
        "Options:\n"
        "  -o <dir>         output folder (default: ./<name>)\n"
        "  -S               compile only, stop at <name>.asm\n"
        "  -I <dir>         extra #include folder (C++; repeatable)\n"
        "  -f <MHz>         testbench clock (default: 100)\n"
        "  -c <n>           clocks the testbench simulates (default: 10000)\n"
        "  --wave-arrays    each array element as its own waveform signal (C±)\n"
        "  -v               show each command before it runs\n"
        "  --lang=en|pt     message language (default: en, or YANC_LANG)\n"
        "  -h, --help       show this help and exit\n"
        "  -V, --version    show version and exit\n"), YANC_VERSION);
}

static void fail(const char *pt, const char *en, const char *arg)
{
    fprintf(stderr, "yanc: ");
    fprintf(stderr, M(pt, en), arg);
    fputc('\n', stderr);
    exit(EXIT_FAILURE);
}

// ---- running a tool --------------------------------------------------------

#define MAX_ARGS 64
static const char *cmd[MAX_ARGS];
static int         ncmd;
static int         verbose = 0;

static void arg(const char *a)
{
    if (ncmd >= MAX_ARGS - 1) fail("argumentos demais", "too many arguments", "");
    cmd[ncmd++] = a;
    cmd[ncmd]   = NULL;
}

static void tool(const char *name)
{
    static char path[4][4096];
    static int  k = 0;
    char *p = path[k++ & 3];
    snprintf(p, sizeof(path[0]), "%s/%s%s", exe_dir(), name, EXE);
    ncmd = 0;
    arg(p);
}

#ifdef _WIN32
// the Windows C runtime joins the arguments with spaces: quote each one that
// needs it, by the rules its parser undoes
static char *quote(const char *s)
{
    if (*s && !strpbrk(s, " \t\"")) return (char *)s;
    size_t n = strlen(s);
    char *q = malloc(2 * n + 3), *o = q;
    *o++ = '"';
    for (const char *c = s; ; c++) {
        int bs = 0;
        while (*c == '\\') { bs++; c++; }
        if (!*c)            { for (int i = 0; i < 2 * bs; i++) *o++ = '\\'; break; }
        if (*c == '"')      { for (int i = 0; i < 2 * bs + 1; i++) *o++ = '\\'; *o++ = '"'; }
        else                { for (int i = 0; i < bs; i++) *o++ = '\\'; *o++ = *c; }
    }
    *o++ = '"'; *o = 0;
    return q;
}
#endif

static void run(void)
{
    if (verbose) {
        for (int i = 0; i < ncmd; i++)
            printf(strchr(cmd[i], ' ') ? "%s\"%s\"" : "%s%s", i ? " " : "", cmd[i]);
        printf("\n");
        fflush(stdout);
    }
    fflush(stdout); fflush(stderr);
    int rc;
#ifdef _WIN32
    const char *q[MAX_ARGS];
    for (int i = 0; i <= ncmd; i++) q[i] = cmd[i] ? quote(cmd[i]) : NULL;
    rc = (int)_spawnv(_P_WAIT, cmd[0], q);
    if (rc == -1) fail("não achei %s", "can't run %s", cmd[0]);
#else
    pid_t pid = fork();
    if (pid < 0) fail("não deu pra rodar %s", "can't run %s", cmd[0]);
    if (pid == 0) { execv(cmd[0], (char *const *)cmd); perror(cmd[0]); _exit(127); }
    int st;
    if (waitpid(pid, &st, 0) < 0) fail("não deu pra esperar %s", "can't wait for %s", cmd[0]);
    rc = WIFEXITED(st) ? WEXITSTATUS(st) : 128;
#endif
    if (rc != 0) exit(rc);
}

// ---- main ------------------------------------------------------------------

static const char *ext_of(const char *path)
{
    const char *b = path;
    for (const char *c = path; *c; c++) if (*c == '/' || *c == '\\') b = c + 1;
    const char *d = strrchr(b, '.');
    return d ? d : "";
}

int main(int argc, char **argv)
{
    console_utf8();
    lang_en = yanc_lang(&argc, argv, "yanc");
    const char *lang = lang_en ? "--lang=en" : "--lang=pt";

    const char *in = NULL, *out = NULL, *freq = NULL, *clocks = NULL;
    const char *inc[MAX_ARGS / 4]; int ninc = 0;
    int only_asm = 0, wave = 0;

    for (int i = 1; i < argc; i++) {
        const char *a = argv[i];
        int has = i + 1 < argc;
        if      (!strcmp(a, "-o") && has)               out    = argv[++i];
        else if (!strcmp(a, "-I") && has) { if (ninc >= MAX_ARGS / 4) fail("-I demais", "too many -I", ""); inc[ninc++] = argv[++i]; }
        else if (!strcmp(a, "-f") && has)               freq   = argv[++i];
        else if (!strcmp(a, "-c") && has)               clocks = argv[++i];
        else if (!strcmp(a, "-S"))                      only_asm = 1;
        else if (!strcmp(a, "--wave-arrays"))           wave = 1;
        else if (!strcmp(a, "-v"))                      verbose = 1;
        else if (!strcmp(a, "-h") || !strcmp(a, "--help"))    { usage(stdout); return 0; }
        else if (!strcmp(a, "-V") || !strcmp(a, "--version")) { printf("yanc %s\n", YANC_VERSION); return 0; }
        else if (a[0] != '-' && !in)                    in = a;
        else { fprintf(stderr, M("yanc: opção desconhecida '%s'\n", "yanc: unknown option '%s'\n"), a); usage(stderr); return 1; }
    }
    if (!in) { usage(stderr); return 1; }

    const char *ext  = ext_of(in);
    int is_cmm = !strcmp(ext, ".cmm"), is_cpp = !strcmp(ext, ".cpp"), is_asm = !strcmp(ext, ".asm");
    if (!is_cmm && !is_cpp && !is_asm)
        fail("não sei o que fazer com '%s' (um .cmm, .cpp ou .asm)", "don't know what to do with '%s' (a .cmm, .cpp or .asm)", in);
    if (ninc && !is_cpp) fail("-I é só para C++ (%s)", "-I is for C++ only (%s)", in);
    if (wave && !is_cmm) fail("--wave-arrays é só para C± (%s)", "--wave-arrays is for C± only (%s)", in);
    if (is_asm && out)   fail("um .asm é montado na pasta dele, sem -o (%s)", "a .asm is assembled in its own folder, no -o (%s)", in);
    if (is_asm && only_asm) fail("-S num .asm não tem o que fazer (%s)", "-S on a .asm has nothing to do (%s)", in);

    char *name = path_stem(in);
    char  dir[4096], asm_file[2 * 4096 + 8];
    if (is_asm) {
        snprintf(asm_file, sizeof(asm_file), "%s", in);
        const char *b = in;
        for (const char *c = in; *c; c++) if (*c == '/' || *c == '\\') b = c;
        if (b == in) snprintf(dir, sizeof(dir), ".");
        else         snprintf(dir, sizeof(dir), "%.*s", (int)(b - in), in);
    } else {
        snprintf(dir, sizeof(dir), "%s", out ? out : name);
        snprintf(asm_file, sizeof(asm_file), "%s/%s.asm", dir, name);

        if (is_cmm) {
            tool("cmmcomp"); arg("-o"); arg(dir);
            if (wave) arg("--wave-arrays");
        } else {
            tool("cppcomp"); arg("-o"); arg(dir);
            for (int i = 0; i < ninc; i++) { arg("-I"); arg(inc[i]); }
        }
        arg(lang); arg(in);
        run();
        if (only_asm) {
            printf(M("Assembly pronto: %s\n", "Assembly ready: %s\n"), asm_file);
            return 0;
        }
    }

    tool("asmcomp");
    if (freq)   { arg("-f"); arg(freq);   }
    if (clocks) { arg("-c"); arg(clocks); }
    arg(lang); arg(asm_file);
    run();

    printf(M("Processador SAPHO pronto: %s/%s.v\n", "SAPHO processor ready: %s/%s.v\n"), dir, name);
    return 0;
}
