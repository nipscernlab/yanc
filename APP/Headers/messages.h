// ----------------------------------------------------------------------------
// PT/EN bilingual support for appcomp messages -------------------------------
// ----------------------------------------------------------------------------
// usage:
//   printf(MSG_XXX, args...);   or   fprintf(stderr, MSG_XXX, args...);
// to see both versions of the string, just search for MSG_XXX below.
// ----------------------------------------------------------------------------

#ifndef MESSAGES_H
#define MESSAGES_H

// 0 = Portuguese (default), 1 = English
extern int lang_en;

// string-selection macro. takes PT and EN and picks one based on lang_en
#define M(pt, en) (lang_en ? (en) : (pt))

// scans argv for -en/-pt, adjusts lang_en and removes the flag from argv/argc
void parse_lang_flag(int *argc, char **argv);

// ----------------------------------------------------------------------------
// message catalog ------------------------------------------------------------
// ----------------------------------------------------------------------------

// errors
#define MSG_ERR_OUT_OF_MEMORY \
    M("Erro: memória insuficiente pra alocar a tabela de símbolos!\n", \
      "Error: out of memory while growing the symbol table!\n")

#define MSG_ERR_CANT_CREATE_LOG \
    M("Erro: não deu pra criar o arquivo %s/app_log.txt.\n", \
      "Error: couldn't create the file %s/app_log.txt.\n")

#define MSG_ERR_USELESS_PROC \
    M("Erro: esse processador não serve pra nada. Você não tem nada útil pra fazer não?\n", \
      "Error: this processor is totally useless. Don't you have anything actually fun to do?\n")

// info messages
#define MSG_INFO_ITR_HANDLING \
    M("Info: implementando tratamento de interrupção\n", \
      "Info: implementing interruption handling\n")

#define MSG_INFO_INS_VAR_FOUND \
    M("Info: foram encontradas %d instruções e %d variáveis\n", \
      "Info: %d instructions and %d variables were found\n")

// success messages
#define MSG_OK_APP_DONE \
    M("Sucesso: já sei a quantidade de memória! Vamo vê agora quais circuitos você precisa...\n", \
      "Sweet: got the memory count nailed down! Now let's see which circuits you'll need...\n")

// command-line interface ----------------------------------------------------

#define MSG_ERR_CLI_NEEDS_VALUE \
    M("Erro: a opção '%s' precisa de um valor.\n", \
      "Error: option '%s' needs a value.\n")

#define MSG_ERR_CLI_UNKNOWN_OPT \
    M("Erro: opção desconhecida '%s'.\n", \
      "Error: unknown option '%s'.\n")

#define MSG_ERR_CLI_MISSING \
    M("Erro: faltou a opção obrigatória '%s'.\n", \
      "Error: missing required option '%s'.\n")

#define MSG_CLI_VERSION \
    M("appcomp (YANC) %s\n", \
      "appcomp (YANC) %s\n")

#define MSG_CLI_HELP \
    M("appcomp - pré-processador de assembly do YANC (%s)\n" \
      "\n" \
      "Uso:\n" \
      "  appcomp [opções] -i <arq.asm> -t <dir>\n" \
      "\n" \
      "Opções obrigatórias:\n" \
      "  -i, --input <arq>     arquivo-fonte assembly (caminho completo)\n" \
      "  -t, --temp-dir <dir>  diretório Temp (recebe o app_log.txt)\n" \
      "\n" \
      "Outras opções:\n" \
      "  -en, -pt              idioma das mensagens (padrão: -pt)\n" \
      "  -h, --help            mostra esta ajuda e sai\n" \
      "  -V, --version         mostra a versão e sai\n", \
      "appcomp - YANC assembly pre-processor (%s)\n" \
      "\n" \
      "Usage:\n" \
      "  appcomp [options] -i <file.asm> -t <dir>\n" \
      "\n" \
      "Required options:\n" \
      "  -i, --input <file>    assembly source file (full path)\n" \
      "  -t, --temp-dir <dir>  Temp directory (receives app_log.txt)\n" \
      "\n" \
      "Other options:\n" \
      "  -en, -pt              diagnostic message language (default: -pt)\n" \
      "  -h, --help            show this help and exit\n" \
      "  -V, --version         show version and exit\n")

#endif
