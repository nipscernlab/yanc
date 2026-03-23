// ----------------------------------------------------------------------------
// rotinas e variaveis de estado para parser de funcoes -----------------------
// ----------------------------------------------------------------------------

// includes globais
#include <string.h>
#include <stdlib.h>

// includes locais
#include "..\Headers\t2t.h"
#include "..\Headers\labels.h"
#include "..\Headers\global.h"
#include "..\Headers\data_use.h"
#include "..\Headers\variaveis.h"
#include "..\Headers\data_declar.h"

// ----------------------------------------------------------------------------
// redeclaracao de variaveis globais ------------------------------------------
// ----------------------------------------------------------------------------

int  fun_id;      // guarda id da funcao sendo usada
int  mainok  = 0; // status da funcao main: 0 -> indefinido, 1 -> resolvido (como sera chamada)
char fname [512]; // nome da funcao atual sendo parseada

// ----------------------------------------------------------------------------
// variaveis locais -----------------------------------------------------------
// ----------------------------------------------------------------------------

int ret_ok;       // diz se teve um retorno da funcao corretamente
int fun_parse;    // guarda id da funcao sendo parseada
int p_test;       // identifica parametros na chamada de funcoes (parecido com OFST, mas de valor 10)

// ----------------------------------------------------------------------------
// funcoes auxiliares ---------------------------------------------------------
// ----------------------------------------------------------------------------

// calcula quantos parametros uma funcao tem
int get_npar(int par)
{
    int t_fun = par;
    int n_par = 0;

    while (t_fun != 0) {t_fun = t_fun/10; n_par++;}

    return n_par;
}

// checa se o argumento passado pra funcao esta ok
void par_check(int et)
{
    // pega numero de parametros original
    int n_par = get_npar(v_fpar[fun_id]);

    // pega tipo e posicao do parametro atual a ser chamado
    int  t_cal = p_test; // vai guardar o tipo de parametro (0, 1, 2 ou 3)
    int  aux   = p_test;
    int id_cal = n_par ;
    int  index = 1;      // vai guardar a posicao do parametro
    while (aux > 10)
    {
           aux = aux   / 10;
         t_cal = t_cal % 10;
        id_cal--;
         index++;
    }

    // pega tipo do parametro atual na funcao original
    int t_fun = v_fpar[fun_id];
    int i;
    for (i = 1; i < id_cal; i++) t_fun = t_fun/10;
    t_fun = t_fun % 10;

    char ld [10]; if (acc_ok == 0) strcpy(ld ,"LOD"  ); else strcpy(ld ,"P_LOD"  );
    char i2f[10]; if (acc_ok == 0) strcpy(i2f,"I2F_M"); else strcpy(i2f,"P_I2F_M");

    // ------------------------------------------------------------------------
    // checando todas as possibilidades ---------------------------------------
    // ------------------------------------------------------------------------

    int etr, eti;

    // original eh int e chamada eh int var -------------------------------

    if ((t_fun == 1) && (t_cal == 1) && (et % OFST != 0))
    {
        add_instr("%s %s\n", ld, v_name[et%OFST]);
    }

    // original eh int e chamada eh int acc -------------------------------

    if ((t_fun == 1) && (t_cal == 1) && (et % OFST == 0))
    {
        // nao faz nada
    }

    // original eh int e chamada eh float var -----------------------------

    if ((t_fun == 1) && (t_cal == 2) && (et % OFST != 0))
    {
        fprintf(stdout, "Atenção na linha %d: convertendo float para int no parâmetro %d da função '%s'.\n", line_num+1, index, v_name[fun_id]);

        add_instr("%s %s\n", ld, v_name[et%OFST]);
        add_instr("F2I\n");
    }

    // original eh int e chamada eh float acc -----------------------------

    if ((t_fun == 1) && (t_cal == 2) && (et % OFST == 0))
    {
        fprintf(stdout, "Atenção na linha %d: convertendo float para int no parâmetro %d da função '%s'.\n", line_num+1, index, v_name[fun_id]);

        add_instr("F2I\n");
    }

    // original eh int e chamada eh comp const ----------------------------

    if ((t_fun == 1) && (t_cal == 5))
    {
        fprintf(stdout, "Atenção na linha %d: convertendo comp para int no parâmetro %d da função '%s'.\n", line_num+1, index, v_name[fun_id]);

        get_cmp_cst(et,&etr,&eti);

        add_instr("%s %s\n", ld, v_name[etr%OFST]);
        add_instr("F2I\n");
    }

    // original eh int e chamada eh comp var ------------------------------

    if ((t_fun == 1) && (t_cal == 3) && (et % OFST != 0))
    {
        fprintf(stdout, "Atenção na linha %d: convertendo comp para int no parâmetro %d da função '%s'.\n", line_num+1, index, v_name[fun_id]);

        add_instr("%s %s\n", ld, v_name[et%OFST]);
        add_instr("F2I\n");
    }

    // original eh int e chamada eh comp acc ------------------------------

    if ((t_fun == 1) && (t_cal == 3) && (et % OFST == 0))
    {
        fprintf(stdout, "Atenção na linha %d: convertendo comp para int no parâmetro %d da função '%s'.\n", line_num+1, index, v_name[fun_id]);

        add_instr("POP\n");
        add_instr("F2I\n");
    }

    // original eh float e chamada eh int var -----------------------------

    if ((t_fun == 2) && (t_cal == 1) && (et % OFST != 0))
    {
        fprintf(stdout, "Atenção na linha %d: convertendo int para float no parâmetro %d da função '%s'.\n", line_num+1, index, v_name[fun_id]);
        
        add_instr("%s %s\n", i2f, v_name[et%OFST]);
    }

    // original eh float e chamada eh int acc -----------------------------

    if ((t_fun == 2) && (t_cal == 1) && (et % OFST == 0))
    {
        fprintf(stdout, "Atenção na linha %d: convertendo int para float no parâmetro %d da função '%s'.\n", line_num+1, index, v_name[fun_id]);

        add_instr("I2F\n");
    }

    // original eh float e chamada eh float var ---------------------------

    if ((t_fun == 2) && (t_cal == 2) && (et % OFST != 0))
    {
        add_instr("%s %s\n", ld, v_name[et%OFST]);
    }

    // original eh float e chamada eh float acc ---------------------------

    if ((t_fun == 2) && (t_cal == 2) && (et % OFST == 0))
    {
        // nao faz nada
    }

    // original eh float e chamada eh comp const --------------------------

    if ((t_fun == 2) && (t_cal == 5))
    {
        fprintf(stdout, "Atenção na linha %d: convertendo comp para float no parâmetro %d da função '%s'.\n", line_num+1, index, v_name[fun_id]);

        get_cmp_cst(et,&etr,&eti);

        add_instr("%s %s\n", ld, v_name[etr%OFST]);
    }

    // original eh float e chamada eh comp var ----------------------------

    if ((t_fun == 2) && (t_cal == 3) && (et % OFST != 0))
    {
        fprintf(stdout, "Atenção na linha %d: convertendo comp para float no parâmetro %d da função '%s'.\n", line_num+1, index, v_name[fun_id]);

        add_instr("%s %s\n", ld, v_name[et%OFST]);
    }

    // original eh float e chamada eh comp acc ----------------------------

    if ((t_fun == 2) && (t_cal == 3) && (et % OFST == 0))
    {
        fprintf(stdout, "Atenção na linha %d: convertendo comp para float no parâmetro %d da função '%s'.\n", line_num+1, index, v_name[fun_id]);

        add_instr("POP\n");
    }

    // original eh comp e chamada eh int var ------------------------------

    if ((t_fun == 3) && (t_cal == 1) && (et % OFST != 0))
    {
        fprintf(stdout, "Atenção na linha %d: convertendo int para comp no parâmetro %d da função '%s'.\n", line_num+1, index, v_name[fun_id]);
        
        add_instr("%s %s\n", i2f, v_name[et%OFST]);
        add_instr("P_LOD 0.0\n");
    }

    // original eh comp e chamada eh int acc ------------------------------

    if ((t_fun == 3) && (t_cal == 1) && (et % OFST == 0))
    {
        fprintf(stdout, "Atenção na linha %d: convertendo int para comp no parâmetro %d da função '%s'.\n", line_num+1, index, v_name[fun_id]);
        
        add_instr("I2F\n");
        add_instr("P_LOD 0.0\n");
    }

    // original eh comp e chamada eh float var ----------------------------

    if ((t_fun == 3) && (t_cal == 2) && (et % OFST != 0))
    {
        fprintf(stdout, "Atenção na linha %d: convertendo float para comp no parâmetro %d da função '%s'.\n", line_num+1, index, v_name[fun_id]);

        add_instr("%s %s\n", ld, v_name[et%OFST]);
        add_instr("P_LOD 0.0\n");
    }

    // original eh comp e chamada eh float acc ----------------------------

    if ((t_fun == 3) && (t_cal == 2) && (et % OFST == 0))
    {
        fprintf(stdout, "Atenção na linha %d: convertendo float para comp no parâmetro %d da função '%s'.\n", line_num+1, index, v_name[fun_id]);

        add_instr("P_LOD 0.0\n");
    }

    // original eh comp e chamada eh comp const ---------------------------

    if ((t_fun == 3) && (t_cal == 5))
    {
        get_cmp_cst(et,&etr,&eti);

        add_instr("%s %s\n", ld, v_name[etr%OFST]);
        add_instr("P_LOD %s\n",  v_name[eti%OFST]);
    }

    // original eh comp e chamada eh comp var -----------------------------

    if ((t_fun == 3) && (t_cal == 3) && (et % OFST != 0))
    {
        get_cmp_ets(et,&etr,&eti); // pega os IDs estendidos do right na memoria

        add_instr("%s %s\n" , ld, v_name[etr%OFST]);
        add_instr("P_LOD %s\n",     v_name[eti%OFST]);
    }

    // original eh comp e chamada eh comp acc -----------------------------

    if ((t_fun == 3) && (t_cal == 3) && (et % OFST == 0))
    {
        // nao faz nada
    }
}

// ----------------------------------------------------------------------------
// declaracao -----------------------------------------------------------------
// ----------------------------------------------------------------------------

// declara uma funcao
void declar_fun(int id1, int id2) //id1 -> tipo, id2 -> indice para o nome
{
    // entra nesse if se a primeira funcao declarada nao for a main
    // nesse case tem que dar um JMP pra ela antes
    // pois main deve ser a primeira funcao do processador depois do reset
    if ((mainok == 0) && (strcmp(v_name[id2], "main") != 0))
    {
        add_sinst(-2, "JMP main\n");

        mainok = 1; // resolvido a questao da funcao main
    }
    // entra nesse if se a primeira funcao declarada for a main
    // nesse caso nao precisa dar JMP
    // soh marca em mainok que essa questao ja foi resolvida
    else if ((mainok == 0) && (strcmp(v_name[id2], "main") == 0))
    {
        mainok = 1; // definido como a funcao main sera usada
    }

    add_sinst(0, "@%s ", v_name[id2]);

    strcpy(fname, v_name[id2]); // seta a variavel de estado fname para o nome da funcao a ser analisada
    v_type[id2] = id1+6       ; // v_type vai ser funcao (void, int, float, comp) (6, 7, 8, 9)
    fun_parse   = id2         ; // seta a variavel de estado fun_parse para o id do nome da funcao
    ret_ok      = 0           ; // seta a variavel de estado ret_ok para zero (vai comecar o parser da funcao)
}

// pega o primeiro parametro
void declar_fst(int id)
{
    // se for comp ...
    if (v_type[id] > 2)
    {
        // primeiro pega o img da pilha
        int idi = get_img_id(id);
        add_instr("SET_P %s\n", v_name[idi]);
    }

    // o primeiro parametro da funcao eh com SET (pq eh o ultimo a ser chamado)
    // os proximos (se houver) sao com SET_P em outra funcao
    add_instr("SET %s\n", v_name[id]);
}

// pega a partir do segundo parametro
int declar_par(int type, int id)
{
    declar_var(id); // nao pode passar array como parametro de funcao

    // armazena informacao sobre o tipo de dado de todos os parametro em um unico numero
    v_fpar[fun_parse] = v_fpar[fun_parse]*10 + type;

    return id;
}

// vai dando SET_P nos parametros, a medida que for achando eles
void set_par(int id)
{
    // se for comp
    if (v_type[id] > 2)
    {
        int idi = get_img_id(id);
        add_instr("SET_P %s\n", v_name[idi]);
    }
        add_instr("SET_P %s\n", v_name[id] );
}

// quando acha a palavra chave return
void declar_ret(int et, int ret)
{
    // checa se eh funcao mesmo, ou void por engano
    if (v_type[fun_parse] == 6)
        {fprintf (stderr, "Erro na linha %d: valor de retorno em função void? viajou!\n", line_num+1); exit(EXIT_FAILURE);}

    // testa se esta dentro de um if/else
    //if ((get_if() > 0) && (v_type[fun_parse] != 6))
        //fprintf(stdout, "Cuidado na linha %d: usar return dentro de if/else pode dar pau, caso você esqueça em algum lugar!\n", line_num+1);

    // ------------------------------------------------------------------------
    // checa todas as combinacoes ---------------------------------------------
    // ------------------------------------------------------------------------

    int etr, eti;
    int left_type = v_type[fun_parse];

    // int com int var
    if ((left_type == 7) && (get_type(et) == 1) && (et%OFST!=0))
    {
        add_instr("LOD %s\n", v_name[et%OFST]);
    }

    // int com int acc
    if ((left_type == 7) && (get_type(et) == 1) && (et%OFST==0))
    {
        // nao faz nada
    }

    // int com float var
    if ((left_type == 7) && (get_type(et) == 2) && (et%OFST!=0))
    {
        fprintf(stdout, "Atenção na linha %d: vai converter float para int no retorno da função '%s'? Dá-lhe código!\n", line_num+1, v_name[fun_parse]);

        add_instr("F2I_M %s\n", v_name[et%OFST]);
    }

    // int com float acc
    if ((left_type == 7) && (get_type(et) == 2) && (et%OFST==0))
    {
        fprintf(stdout, "Atenção na linha %d: vai converter float para int no retorno da função '%s'? Dá-lhe código!\n", line_num+1, v_name[fun_parse]);
        add_instr("F2I\n");
    }

    // int com comp const
    if ((left_type == 7) && (get_type(et) == 5))
    {
        fprintf (stdout, "Atenção na linha %d: nessa conversão, eu vou arredondar a parte real hein!\n", line_num+1);

        get_cmp_cst(et,&etr,&eti);
        
        add_instr("F2I_M %s\n", v_name[etr % OFST]);
    }

    // int com comp var
    if ((left_type == 7) && (get_type(et) == 3) && (et%OFST!=0))
    {
        fprintf (stdout, "Atenção na linha %d: nessa conversão, eu vou arredondar a parte real hein!\n", line_num+1);

        get_cmp_ets(et,&etr,&eti);
        
        add_instr("F2I_M %s\n", v_name[etr % OFST]);
    }

    // int com comp acc
    if ((left_type == 7) && (get_type(et) == 3) && (et%OFST==0))
    {
        fprintf (stdout, "Atenção na linha %d: nessa conversão, eu vou arredondar a parte real hein!\n", line_num+1);

        add_instr("POP\n");
        add_instr("F2I\n");
    }

    // float com int var
    if ((left_type == 8) && (get_type(et) == 1) && (et%OFST!=0))
    {
        fprintf(stdout, "Atenção na linha %d: retorno é float, mas recebe int.\n", line_num+1);

        add_instr("I2F_M %s\n", v_name[et % OFST]);
    }

    // float com int acc
    if ((left_type == 8) && (get_type(et) == 1) && (et%OFST==0))
    {
        fprintf(stdout, "Atenção na linha %d: retorno é float, mas recebe int.\n", line_num+1);
        
        add_instr("I2F\n");
    }

    // float com float var
    if ((left_type == 8) && (get_type(et) == 2) && (et%OFST!=0))
    {
        add_instr("LOD %s\n", v_name[et % OFST]);
    }

    // float com float acc
    if ((left_type == 8) && (get_type(et) == 2) && (et%OFST==0))
    {
        // nao faz nada
    }

    // float com comp const
    if ((left_type == 8) && (get_type(et) == 5))
    {
        fprintf (stdout, "Atenção na linha %d: nessa conversão, eu vou pegar só a parte real hein!\n", line_num+1);

        get_cmp_cst(et,&etr,&eti);
        
        add_instr("LOD %s\n", v_name[etr % OFST]);
    }

    // float com comp var
    if ((left_type == 8) && (get_type(et) == 3) && (et%OFST!=0))
    {
        fprintf (stdout, "Atenção na linha %d: nessa conversão, eu vou pegar só a parte real hein!\n", line_num+1);

        get_cmp_ets(et,&etr,&eti);
        
        add_instr("LOD %s\n", v_name[etr % OFST]);
    }

    // float com comp acc
    if ((left_type == 8) && (get_type(et) == 3) && (et%OFST==0))
    {
        fprintf (stdout, "Atenção na linha %d: nessa conversão, eu vou pegar só a parte real hein!\n", line_num+1);

        add_instr("POP\n");
    }

    // comp com int var
    if ((left_type == 9) && (get_type(et) == 1) && (et%OFST!=0))
    {
        fprintf(stdout, "Atenção na linha %d: retorno da função é comp, mas recebe int.\n", line_num+1);

        add_instr("I2F_M %s\n", v_name[et % OFST]);
        add_instr("P_LOD 0.0\n");
    }

    // comp com int acc
    if ((left_type == 9) && (get_type(et) == 1) && (et%OFST==0))
    {
        fprintf(stdout, "Atenção na linha %d: retorno da função é comp, mas recebe int.\n", line_num+1);
        
        add_instr("I2F\n");
        add_instr("P_LOD 0.0\n");
    }

    // comp com float var
    if ((left_type == 9) && (get_type(et) == 2) && (et%OFST!=0))
    {
        fprintf(stdout, "Atenção na linha %d: retorno da função é comp, mas recebe float.\n", line_num+1);

        add_instr("LOD %s\n", v_name[et % OFST]);
        add_instr("P_LOD 0.0\n");
    }

    // comp com float acc
    if ((left_type == 9) && (get_type(et) == 2) && (et%OFST==0))
    {
        fprintf(stdout, "Atenção na linha %d: retorno da função é comp, mas recebe float.\n", line_num+1);

        add_instr("P_LOD 0.0\n");
    }

    // comp com comp const
    if ((left_type == 9) && (get_type(et) == 5))
    {
        get_cmp_cst(et,&etr,&eti);
        
        add_instr("LOD %s\n"  , v_name[etr % OFST]);
        add_instr("P_LOD %s\n", v_name[eti % OFST]);
    }

    // comp com comp var
    if ((left_type == 9) && (get_type(et) == 3) && (et%OFST!=0))
    {
        get_cmp_ets(et,&etr,&eti);
        
        add_instr("LOD %s\n"  , v_name[etr % OFST]);
        add_instr("P_LOD %s\n", v_name[eti % OFST]);
    }

    // comp com comp acc
    if ((left_type == 9) && (get_type(et) == 3) && (et%OFST==0))
    {
        // nao faz nada
    }

    // ------------------------------------------------------------------------
    // finaliza ---------------------------------------------------------------
    // ------------------------------------------------------------------------

    if (ret == 0) return;

    add_instr("RET\n");

    acc_ok = 0; // apesar de ter exp no acc, tem q liberar para comecar outra funcao
    ret_ok = 1; // apareceu a palavra chave return na funcao certinho
}

// fim do parser da declaracao de uma funcao
void func_ret(int id) // id -> id da funcao atual
{
    // checa se a funcao teve a instrucao return x;
    if ((v_type[id] != 6) && (ret_ok == 0))
        {fprintf (stderr, "Erro na função %s: cadê o retorno pra essa função?\n", v_name[id]); exit(EXIT_FAILURE);}

    // se eh funcao main, da um JMP fim
    if (strcmp(v_name[id], "main") == 0)
    {
        add_sinst(-3, "@fim JMP fim\n");

        v_used[id] = 1; // funcao main foi usada (evita warning de funcao main declarada mas nao usada)
    }
    else if (v_type[id] == 6) {add_instr("RET\n");} // se eh tipo void, ainda precisa gerar um RET

    // variavel de ambiente fname fica vazia (saiu de uma funcao)
    strcpy(fname, "");
}

// retorno sem exp (return;)
void void_ret()
{
    // checa se eh void mesmo, ou funcao por engano
    if (v_type[fun_parse] != 6)
        {fprintf (stderr, "Erro na linha %d: cadê o valor de retorno da função?\n", line_num+1); exit(EXIT_FAILURE);}

    // se eh funcao main, usa JMP fim ao inves de RET
    if ((strcmp(fname, "main") == 0))
         add_sinst(-3, "@fim JMP fim\n");
    else add_instr(             "RET\n");
}

// ----------------------------------------------------------------------------
// utilizacao -----------------------------------------------------------------
// ----------------------------------------------------------------------------

// da LOD no primeiro parametro (se houver)
// get_type da o tipo de parametro (0, 1, 2, 3) (void, int, float, comp)
// p_test consegue guardar a posicao e tipo de todos os parametros na chamada da funcao
void par_exp(int et)
{
    p_test = 0; // inicializa a variavel de estado p_test
    p_test = p_test*10 + get_type(et);
    par_check(et);
    acc_ok = 1;
}

// da LOD nos proximos parametros
void par_listexp(int et)
{
    p_test = p_test*10 + get_type(et);
    par_check(et);
}

// executa instrucao CAL para funcoes tipo void (por isso o v de void)
void vcall(int id)
{
    // posso usar funcao com chamada void tb, por isso testar tudo aqui
    if  (v_type[id] < 6)
    {
        fprintf(stderr, "Erro na linha %d: cadê essa função '%s'?\n", line_num+1, rem_fname(v_name[id], fname));
        exit(EXIT_FAILURE);
    }

    // checa numero de parametros
    if (get_npar(p_test) != get_npar(v_fpar[id])) // p_test tem a lista de par na chamada e v_fpar na declaracao
    {
        fprintf(stderr, "Erro na linha %d: olha lá direito quantos parâmetros tem a função '%s'.\n", line_num+1, rem_fname(v_name[id], fname));
        exit(EXIT_FAILURE);
    }

    add_instr("CAL %s\n", v_name[id]);

    v_used[id] = 1; // funcao ja foi chamada
    acc_ok     = 0; // acc ta liberado
}

// executa instrucao CAL para funcoes com retorno (por isso o f de funcao)
int fcall(int id)
{
    if (v_type[id] == 6)
    {
        fprintf (stderr, "Erro na linha %d: olha lá a funcao '%s', você vai ver que ela nao retorna nada.\n", line_num+1, v_name[id]);
        exit(EXIT_FAILURE);
    }
    else if (v_type[id] < 6)
    {
        fprintf (stderr, "Erro na linha %d: A função '%s' tá onde?\n", line_num+1, rem_fname(v_name[id], fname));
        exit(EXIT_FAILURE);
    }

    if (get_npar(p_test) != get_npar(v_fpar[id]))
    {
        fprintf(stderr, "Erro na linha %d: lista de parâmetros da função '%s' difere da original.\n", line_num+1, v_name[id]);
        exit(EXIT_FAILURE);
    }

    add_instr("CAL %s\n",v_name[id]);

    v_used[id] = 1;             // funcao ja foi usada
    acc_ok     = 1;             // acc ta ocupado

    return (v_type[id]-6)*OFST; // retorna o tipo de dado (void, int, float ou comp)
}