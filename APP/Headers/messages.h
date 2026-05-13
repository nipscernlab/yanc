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
#define MSG_ERR_TOO_MANY_VARS \
    M("Erro: número de variáveis > %d", \
      "Error: variable count blew past %d")

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

#endif
