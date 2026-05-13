// ----------------------------------------------------------------------------
// suporte bilingue PT/EN para as mensagens do appcomp ------------------------
// ----------------------------------------------------------------------------
// como usar:
//   printf(MSG_XXX, args...);   ou   fprintf(stderr, MSG_XXX, args...);
// pra ver as duas versões da string, basta procurar pelo MSG_XXX abaixo.
// ----------------------------------------------------------------------------

#ifndef MESSAGES_H
#define MESSAGES_H

// 0 = portugues (default), 1 = ingles
extern int lang_en;

// macro de selecao de string. recebe PT e EN e escolhe baseado em lang_en
#define M(pt, en) (lang_en ? (en) : (pt))

// procura -en/-pt em argv, ajusta lang_en e remove a flag de argv/argc
void parse_lang_flag(int *argc, char **argv);

// ----------------------------------------------------------------------------
// catalogo de mensagens ------------------------------------------------------
// ----------------------------------------------------------------------------

// erros
#define MSG_ERR_TOO_MANY_VARS \
    M("Erro: número de variáveis > %d", \
      "Error: variable count blew past %d")

#define MSG_ERR_CANT_CREATE_LOG \
    M("Erro: não deu pra criar o arquivo %s/app_log.txt.\n", \
      "Error: couldn't create the file %s/app_log.txt.\n")

#define MSG_ERR_USELESS_PROC \
    M("Erro: esse processador não serve pra nada. Você não tem nada útil pra fazer não?\n", \
      "Error: this processor is totally useless. Don't you have anything actually fun to do?\n")

// infos
#define MSG_INFO_ITR_HANDLING \
    M("Info: implementando tratamento de interrupção\n", \
      "Info: implementing interruption handling\n")

#define MSG_INFO_INS_VAR_FOUND \
    M("Info: foram encontradas %d instruções e %d variáveis\n", \
      "Info: %d instructions and %d variables were found\n")

// sucessos
#define MSG_OK_APP_DONE \
    M("Sucesso: já sei a quantidade de memória! Vamo vê agora quais circuitos você precisa...\n", \
      "Sweet: got the memory count nailed down! Now let's see which circuits you'll need...\n")

#endif
