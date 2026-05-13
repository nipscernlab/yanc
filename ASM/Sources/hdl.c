// ----------------------------------------------------------------------------
// rotinas para geracao dos arquivos .v ---------------------------------------
// ----------------------------------------------------------------------------

// includes globais
#include <string.h>
#include <stdlib.h>
#include  <stdio.h>
#include   <math.h>

// includes locais
#include "..\Headers\t2t.h"
#include "..\Headers\eval.h"
#include "..\Headers\opcodes.h"
#include "..\Headers\simulacao.h"
#include "..\Headers\messages.h"

// ----------------------------------------------------------------------------
// funcoes auxiliares ---------------------------------------------------------
// ----------------------------------------------------------------------------

// troca \ por / em um path
void force_rightbar(char *str){while (*str) {if (*str == '\\') *str = '/'; str++;}}

// ----------------------------------------------------------------------------
// gera arquivo verilog com uma instancia do processador ----------------------
// ----------------------------------------------------------------------------

void hdl_vv_file(int n_ins, int n_dat, int nbopr, int itr_addr)
{
    // ------------------------------------------------------------------------
    // cria o arquivo .v para escrita -----------------------------------------
    // ------------------------------------------------------------------------

    char    tmp[512];
    sprintf(tmp, "%s/Hardware/%s.v", proc_dir, prname);

    FILE *f_veri = fopen(tmp,"w");

    // ------------------------------------------------------------------------
    // cria cabecalho e portas do modulo --------------------------------------
    // ------------------------------------------------------------------------

    int comma;
    int nbioin = (nuioin > 1) ? (int)ceil(log2(nuioin)) : 1;
    int nbioou = (nuioou > 1) ? (int)ceil(log2(nuioou)) : 1;

    // o comeco eh padrao ....
    fprintf(f_veri, "module %s (\n\n", prname);
    fprintf(f_veri, "input  clk, rst,\n");

    // verifica se precisa adicionar barramento de entrada in
    if (nuioin > 0 && opc_inn()) fprintf(f_veri, "input  signed [%d:0] in ,\n", nubits-1);
    // sempre adiciona o barramento de saida out
    /*tem que ter alguma saida*/ fprintf(f_veri, "output signed [%d:0] out"   , nubits-1); comma = 0;

    // verifica se pode ter mais alguma coisa depois pra colocar uma virgula
    if ((nuioin > 0 && opc_inn()) || (nuioou > 0 && opc_out()) || (itr_addr != 0)) {fprintf(f_veri, ",\n"); comma = 1;}

    // verifica se vai ter que adicionar controle pra porta de entrada
    if (nuioin > 0 && opc_inn()) {fprintf(f_veri, "output [%d:0] req_in", nuioin-1); comma = 0;}

    // verifica se pode ter mais alguma coisa depois pra colocar uma virgula
    if ((nuioou > 0 && opc_out()) || (itr_addr != 0)) if (comma == 0) {fprintf(f_veri, ",\n"); comma = 1;}

    // verifica se vai ter que adicionar controle pra porta de saida
    if (nuioou > 0 && opc_out()) {fprintf(f_veri, "output [%d:0] out_en", nuioou-1); comma = 0;}

    // verifica se pode ter mais alguma coisa depois pra colocar uma virgula
    if ((itr_addr != 0)) if (comma == 0) {fprintf(f_veri, ",\n"); comma = 1;}

    // verifica se vai ter que adicionar pino de interrupcao
    if (itr_addr != 0) fprintf(f_veri, "input  itr);\n\n"); else fprintf(f_veri, ");\n\n");

    // ------------------------------------------------------------------------
    // wires de interface com I/O ---------------------------------------------
    // ------------------------------------------------------------------------

    // se nao tem barramento de entrada, precisa criar um wire pra ligar a toa no processador
    if (nuioin == 0 || !opc_inn()) fprintf(f_veri, "wire [%d:0] in;\n", nubits-1);

    // se nao tem pino de interrupcao, precisa criar um wire pra ligar no processador
    if (itr_addr == 0) fprintf(f_veri, "wire itr = 1'b0;\n");

    // sempre precisa pra ligar no processador
    fprintf(f_veri, "wire proc_req_in, proc_out_en;\n");

    // sempre precisa pra ligar no processador
    fprintf(f_veri, "wire [%d:0] addr_in;\n"   , nbioin-1);
    fprintf(f_veri, "wire [%d:0] addr_out;\n\n", nbioou-1);

    // ------------------------------------------------------------------------
    // wires de interface com simulacao ---------------------------------------
    // ------------------------------------------------------------------------

    fprintf(f_veri, "`ifdef __ICARUS__\n");
    // acesso as variaveis na memoria de dados
    fprintf(f_veri, "wire mem_wr;\n");
    fprintf(f_veri, "wire [%d:0] mem_addr_wr;\n" , (int)ceil(log2(n_dat)-1));
    // acesso ao indice da instrucao no program counter
    fprintf(f_veri, "wire [%d:0] pc_sim_val;\n"  , (int)ceil(log2(n_ins)-1));
    fprintf(f_veri, "`endif\n\n");

    // ------------------------------------------------------------------------
    // parametros do processador ----------------------------------------------
    // ------------------------------------------------------------------------

    fprintf(f_veri, "processor#(.NUBITS(%d),\n", nubits);
    fprintf(f_veri,            ".NBMANT(%d),\n", nbmant);
    fprintf(f_veri,            ".NBEXPO(%d),\n", nbexpo);
    fprintf(f_veri,            ".NBOPER(%d),\n", nbopr );
    fprintf(f_veri,            ".NUGAIN(%d),\n", nugain);
    fprintf(f_veri,            ".MDATAS(%d),\n", n_dat );
    fprintf(f_veri,            ".MINSTS(%d),\n", n_ins );
    fprintf(f_veri,            ".SDEPTH(%d),\n", sdepth);
    fprintf(f_veri,            ".DDEPTH(%d),\n", ddepth);
    fprintf(f_veri,            ".NBIOIN(%d),\n", nbioin);
    fprintf(f_veri,            ".NBIOOU(%d),\n", nbioou);
    fprintf(f_veri,            ".FFTSIZ(%d),\n", fftsiz);

    // se tem interrupcao, coloca o endereco dela no processador
    if (itr_addr != 0) fprintf(f_veri, ".ITRADD(%d),\n", itr_addr);

    // ------------------------------------------------------------------------
    // alocacao dinamica de recursos ------------------------------------------
    // ------------------------------------------------------------------------

    for (int i = 0; i < opc_cnt(); i++) fprintf(f_veri, ".%s(1),\n", opc_get(i));

    // essa conta ta aproximada, mas é melhor que nada
    printf(MSG_INFO_ISA_USAGE, opc_cnt()*100/102);
    printf(MSG_INFO_ULA_USAGE, opc_ucnt()*100/(49-15)); // 49 no mux, 15 repetidos ou sem uso

    // ------------------------------------------------------------------------
    // finalizacao da instancia do processador --------------------------------
    // ------------------------------------------------------------------------

    char path[1024]; sprintf(path, "%s/Hardware/%s", proc_dir, prname); force_rightbar(path);

    fprintf(f_veri, ".DFILE(\"%s_data.mif\"),\n"  , path);
    fprintf(f_veri, ".IFILE(\"%s_inst.mif\"))\n\n", path);
    fprintf(f_veri, "`ifdef __ICARUS__\n");
    fprintf(f_veri, "p_%s (clk, rst, in, out, addr_in, addr_out, proc_req_in, proc_out_en, itr, mem_wr, mem_addr_wr,pc_sim_val);\n", prname);
    fprintf(f_veri, "`else\n");
    fprintf(f_veri, "p_%s (clk, rst, in, out, addr_in, addr_out, proc_req_in, proc_out_en, itr);\n", prname);
    fprintf(f_veri, "`endif\n\n");

    // ------------------------------------------------------------------------
    // decodificadores de endereco de I/O -------------------------------------
    // ------------------------------------------------------------------------

    // verifica se precisa de decodificador de endereco pra portas de entrada
    if (opc_inn())
    {
        // se for soh uma porta, liga direto no proc_req_in
        if (nuioin == 1) fprintf(f_veri, "assign req_in = proc_req_in;\n"                            , nuioin);
        else             fprintf(f_veri, "addr_dec #(%d) dec_in (proc_req_in, addr_in , req_in);\n"  , nuioin);
    }
    
    // verifica se precisa de decodificador de endereco pra portas de saida
    if (opc_out())
    {
        // se for soh uma porta, liga direto no proc_out_en
        if (nuioou == 1) fprintf(f_veri, "assign out_en = proc_out_en;\n\n"                          , nuioou);
        else             fprintf(f_veri, "addr_dec #(%d) dec_out(proc_out_en, addr_out, out_en);\n\n", nuioou);
    }

    // ------------------------------------------------------------------------
    // inicio de interface com simulacao (iverilog+gtkwave) -------------------
    // ------------------------------------------------------------------------

    fprintf(f_veri, "// ----------------------------------------------------------------------------\n");
    fprintf(f_veri, "// Simulacao ------------------------------------------------------------------\n");
    fprintf(f_veri, "// ----------------------------------------------------------------------------\n\n");

    fprintf(f_veri, "`ifdef __ICARUS__\n\n");

    // ------------------------------------------------------------------------
    // cadastra portas de I/O para simulacao ----------------------------------
    // ------------------------------------------------------------------------

    fprintf(f_veri, "// I/O ------------------------------------------------------------------------\n\n");

    // cadastra portas de entrada pra simulacao
    for(int i=0; i<nuioin; i++)
    {
        if (inn_used(i))
        {
            fprintf(f_veri, "reg signed [%d:0] in_sim_%d = 0;\n", nubits-1, i);
            fprintf(f_veri, "reg req_in_sim_%d = 0;\n", i);
        }
    }
    if (opc_inn()) fprintf(f_veri,"\n");

    // cadastra portas de saida pra simulacao
    for(int i=0;i<nuioou;i++)
    {
        if (out_used(i))
        {
            fprintf(f_veri, "reg signed [%d:0] out_sig_%d = 0;\n", nubits-1, i);
            fprintf(f_veri, "reg out_en_sim_%d = 0;\n", i);
        }
    }

    // decodifica porta de entrada
    if (opc_inn())
    {
        if (nuioin > 0) fprintf(f_veri, "\nalways @ (*) begin\n");
        for(int i=0; i<nuioin; i++)
        {
            if (inn_used(i))
            {
                fprintf(f_veri, "   if (req_in == %d) in_sim_%d = in;\n", (int)pow(2,i),i);
                fprintf(f_veri, "   req_in_sim_%d = req_in == %d;\n",  i, (int)pow(2,i),i);
            }
            else printf(MSG_WARN_UNUSED_IN_PORT, i);
        }
        if (nuioin > 0) fprintf(f_veri, "end\n");
    }

    // decodifica portas de saida
    if (opc_out())
    {
        if (nuioou > 0) fprintf(f_veri, "\nalways @ (*) begin\n");
        for(int i=0;i<nuioou;i++)
        {
            if (out_used(i))
            {
                fprintf(f_veri, "   if (out_en == %d) out_sig_%d <= out;\n", (int)pow(2,i),i);
                fprintf(f_veri, "   out_en_sim_%d = out_en == %d;\n",     i, (int)pow(2,i),i);
            }
            else printf(MSG_WARN_UNUSED_OUT_PORT, i);
        }
        if (nuioou > 0) fprintf(f_veri, "end\n\n");
    }

    // ------------------------------------------------------------------------
    // cadastra variaveis do usuario para simulacao ---------------------------
    // ------------------------------------------------------------------------

    fprintf(f_veri, "// variaveis ------------------------------------------------------------------\n\n");

    int has_float = 0;
    // cria um registrador para cada variavel encontrada
    for (int i = 0; i < sim_cont(); i++)
    {
        // se for int, usa o tipo de dado reg na simulacao
        if (sim_type(i) == 1)
        {
            fprintf(f_veri, "reg [%d:0] %s = 0;\n", nubits-1, sim_name(i));
        }

        // se for float, usa o tipo de dado real na simulacao
        if (sim_type(i) == 2)
        {
            // se for a primeira variavel float, cria os auxiliares
            if (has_float == 0)
            {
                has_float = 1;
                fprintf(f_veri, "integer sm_me2; always @ (*) sm_me2 = (out[%d]) ? -out[%d:0] : out[%d:0];\n", nbmant+nbexpo  , nbmant-1, nbmant-1);
                fprintf(f_veri, "integer  e_me2; always @ (*)  e_me2 = $signed(out[%d:%d]);\n"               , nbmant+nbexpo-1, nbmant);
            }
            // cria a variavel real com valor inicial 0.0
            fprintf(f_veri, "real %s = 0.0;\n", sim_name(i));
        }

        // se for comp, usa tipo de dado reg na simulacao
        if (sim_type(i) > 2)
        {
            // esta funcionando com dx, mas deveria ser o equivalente de 0.0 em binario
            // testar com itob(f2mf("0.0",NULL), nubits)
            fprintf(f_veri, "reg [%d:0] %s = %d'dx;\n", nubits-1, sim_name(i), nubits-1);
        }
    }
    
    // inicia o always para registrar as variaveis
    fprintf(f_veri, "\nalways @ (posedge clk) begin\n");
    // registra cada variavel, dependendo do endereco de cada uma
    for (int i = 0; i < sim_cont(); i++)
    {
        if (sim_type(i) == 1) fprintf(f_veri, "   if (mem_addr_wr == %d && mem_wr) %s <= out;\n"                   , sim_addr(i), sim_name(i));
        if (sim_type(i) == 2) fprintf(f_veri, "   if (mem_addr_wr == %d && mem_wr) %s <= sm_me2*$pow(2.0,e_me2);\n", sim_addr(i), sim_name(i));
        if (sim_type(i) >  2) fprintf(f_veri, "   if (mem_addr_wr == %d && mem_wr) %s <= out;\n"                   , sim_addr(i), sim_name(i));  
    }
    fprintf(f_veri, "end\n\n");

    // se a variavel for comp ...
    // junta a parte real e imag em uma variavel do dobro de tamanho
    char ni[64],nj[64],im[64];
    int has_comp = 0;
    for (int i = 0; i < sim_cont(); i++)
    {
        if (sim_type(i) == 3)
        {
            has_comp = 1;
            for (int j = 0; j < sim_cont(); j++)
            {
                strcpy(ni,sim_name(i));
                strcpy(nj,sim_name(j));
                ni[strlen(ni)-3] = '\0';
                nj[strlen(nj)-3] = '\0';

                sprintf(im, "%s_i", ni);
                if (strcmp(nj,im) == 0)
                    fprintf(f_veri,"wire [16+%d*2-1:0] comp_%s = {8'd%d, 8'd%d, %s, %s};\n", nubits, sim_name(i), nbmant, nbexpo, sim_name(i), sim_name(j));
            }
        }
    }

    if (has_comp == 1) fprintf(f_veri, "\n");

    // ------------------------------------------------------------------------
    // cadastra arrays do usuario para simulacao ------------------------------
    // ------------------------------------------------------------------------

    if (sim_cont_arr()>0)
    fprintf(f_veri, "// arrays ---------------------------------------------------------------------\n\n");

    // cria um registrador para cada variavel do array
    for (int i = 0; i < sim_cont_arr(); i++)
    {
        for (int j = 0; j < sim_size_arr(i); j++)
        {
            // pega o valor no arquivo .mif
            char val[256]; sim_mem(sim_addr_arr(i)+j, val);

            // se for int, usa o tipo de dado reg na simulacao
            if (sim_type_arr(i) == 1)
            {
                fprintf(f_veri, "reg [%d-1:0] %s%04d=%d'b%s;\n", nubits, sim_name_arr(i), j, nubits, val);
            }

            // se for float, usa tipo de dado real na simulacao
            if (sim_type_arr(i) == 2)
            {
                if (has_float == 0)
                {
                    has_float = 1;
                    fprintf(f_veri, "integer sm_me2; always @ (*) sm_me2 = (out[%d]) ? -out[%d:0] : out[%d:0];\n", nbmant+nbexpo  , nbmant-1, nbmant-1);
                    fprintf(f_veri, "integer  e_me2; always @ (*)  e_me2 = $signed(out[%d:%d]);\n"               , nbmant+nbexpo-1, nbmant);
                }
                fprintf(f_veri, "real %s%04d = %f;\n", sim_name_arr(i), j, mf2f(val));
            }

            // se for comp usa tipo reg na simulacao
            if (sim_type_arr(i) > 2)
            {
                fprintf(f_veri, "reg [%d-1:0] %s%04d=%d'b%s;\n", nubits, sim_name_arr(i), j, nubits, val);
            }
        }
    }

    // inicia o always para registrar as variaveis do array
    if (sim_cont_arr()>0) fprintf(f_veri, "\nalways @ (posedge clk) begin\n");
    // registra cada variavel, dependendo do endereco de cada uma
    for (int i = 0; i < sim_cont_arr(); i++)
    {
        for (int j = 0; j < sim_size_arr(i); j++)
        {
            if (sim_type_arr(i) == 2)
                fprintf(f_veri, "   if (mem_addr_wr == %d && mem_wr) %s%04d <= sm_me2*$pow(2.0,e_me2);\n", sim_addr_arr(i)+j, sim_name_arr(i), j);
            else
                fprintf(f_veri, "   if (mem_addr_wr == %d && mem_wr) %s%04d <= out;\n"                   , sim_addr_arr(i)+j, sim_name_arr(i), j);
        }
    }
    if (sim_cont_arr()>0) fprintf(f_veri, "end\n\n");

    // se a variavel for comp ...
    // junta a parte real e imag em uma variavel do dobro de tamanho
    for (int i = 0; i < sim_cont_arr(); i++)
    {
        if (sim_type_arr(i) == 3)
        {
            for (int j = 0; j < sim_cont_arr(); j++)
            {
                strcpy(ni,sim_name_arr(i));
                strcpy(nj,sim_name_arr(j));
                ni[strlen(ni)-3] = '\0';
                nj[strlen(nj)-3] = '\0';

                sprintf(im, "%s_i", ni);
                if (strcmp(nj,im) == 0)
                    for (int k = 0; k < sim_size_arr(i); k++)
                        fprintf(f_veri,"wire [16+%d*2-1:0] comp_%s%04d = {8'd%d, 8'd%d, %s%04d, %s%04d};\n", nubits, sim_name_arr(i), k, nbmant, nbexpo, sim_name_arr(i), k, sim_name_arr(j), k);
            }
        }
    }

    if (sim_cont_arr()>0) fprintf(f_veri, "\n");

    // ------------------------------------------------------------------------
    // interface com istrucoes cmm e asm --------------------------------------
    // ------------------------------------------------------------------------

    fprintf(f_veri, "// instrucoes -----------------------------------------------------------------\n\n");

    // cria 10 registradores para retardar a instrucao @fim
    int nreg = 10;
    for (int i = 0; i < nreg; i++) fprintf(f_veri, "reg [%d:0] valr%d=0;\n", nubits-1, i+1);
    fprintf(f_veri, "\n");

    int num_ins = get_n_ins();

    // cria memoria que vai guardar a tabela de instrucoes
    fprintf(f_veri, "reg [19:0] min [0:%d-1];\n\n", num_ins);
    // cria interface com essa memoria
    fprintf(f_veri, "reg signed [19:0] linetab =-1;\n"  );
    fprintf(f_veri, "reg signed [19:0] linetabs=-1;\n\n");
    // inicializa a memoria com o arquivo .txt
    fprintf(f_veri, "initial	$readmemb(\"pc_%s_mem.txt\",min);\n\n"  , prname );
    // executa os registros
    fprintf(f_veri, "always @ (posedge clk) begin\n");
    fprintf(f_veri, "if (pc_sim_val < %d) linetab <= min[pc_sim_val];\n", num_ins);
    fprintf(f_veri, "linetabs <= linetab;   \n");
    fprintf(f_veri, "valr1    <= pc_sim_val;\n");
    // delays para o @fim
    fprintf(f_veri, "valr2    <= valr1;\n");
    fprintf(f_veri, "valr3    <= valr2;\n");
    fprintf(f_veri, "valr4    <= valr3;\n");
    fprintf(f_veri, "valr5    <= valr4;\n");
    fprintf(f_veri, "valr6    <= valr5;\n");
    fprintf(f_veri, "valr7    <= valr6;\n");
    fprintf(f_veri, "valr8    <= valr7;\n");
    fprintf(f_veri, "valr9    <= valr8;\n");
    fprintf(f_veri, "valr10   <= valr9;\n");

    fprintf(f_veri, "end\n\n");

    // se for simulacao tipo single, cadastra $finish no endewreco @fim
    if (sim_multi()==0)
    {
    fprintf(f_veri, "always @ (posedge clk) if (valr10 == %d) begin\n", sim_get_fim());
    fprintf(f_veri, "   $display(\"Info: end of program!\");\n");
    fprintf(f_veri, "   $finish;\n");
    fprintf(f_veri, "end\n\n");
    }

    // ------------------------------------------------------------------------
    // finaliza arquivo -------------------------------------------------------
    // ------------------------------------------------------------------------

    fprintf(f_veri, "`endif\n\n");
    fprintf(f_veri, "endmodule" );

    fclose(f_veri);
}

// ----------------------------------------------------------------------------
// gera arquivo verilog com o test-bench do processador -----------------------
// ----------------------------------------------------------------------------

void hdl_tb_file(int itr_addr)
{
    // ------------------------------------------------------------------------
    // cria o arquivo .v ------------------------------------------------------
    // ------------------------------------------------------------------------

    int     aux;
    char    tmp[512];
    sprintf(tmp, "%s/%s_tb.v", temp_dir, prname);

    FILE *f_veri = fopen(tmp,"w");

    // ------------------------------------------------------------------------
    // cabecalho --------------------------------------------------------------
    // ------------------------------------------------------------------------

    fprintf(f_veri, "`timescale 1ns/1ps\n\n", prname);
    fprintf(f_veri,    "module %s_tb();\n\n", prname);
    
    // ------------------------------------------------------------------------
    // geracao de clock e reset -----------------------------------------------
    // ------------------------------------------------------------------------

    fprintf(f_veri, "// geracao de clock e reset ---------------------------------------------------\n\n");

    double T = 1000.0/sim_clk(); // periodo do clock em ns (clk frq em MHz)

    // variaveis de clock e reset
    fprintf(f_veri, "reg clk, rst;\n\n");

    // inicio do initial
    fprintf(f_veri, "initial begin\n");
    // inicializa clock e da um reset
    fprintf(f_veri, "    clk = 0;\n");
    fprintf(f_veri, "    rst = 1;\n");
    fprintf(f_veri, "    #%f;\n",  T);
    fprintf(f_veri, "    rst = 0;\n");
    // fim do initial
    fprintf(f_veri, "end\n\n");

    // geracao do clock -------------------------------------------------------
    fprintf(f_veri, "always #%f clk = ~clk;\n\n", T/2.0);

    // ------------------------------------------------------------------------
    // portas de interface com o processador ----------------------------------
    // ------------------------------------------------------------------------

    fprintf(f_veri, "// instancia do processador ---------------------------------------------------\n\n");

    // verifica se precisa adicionar barramento de entrada in
    if (nuioin > 0 && opc_inn()) fprintf(f_veri, "reg  signed [%d:0] proc_io_in = 0;\n", nubits-1);
    // sempre adiciona o barramento de saida out
    /*tem que ter alguma saida*/ fprintf(f_veri, "wire signed [%d:0] proc_io_out;\n"   , nubits-1);
    // verifica se vai ter que adicionar controle pra porta de entrada
    if (nuioin > 0 && opc_inn()) fprintf(f_veri, "wire [%d:0] proc_req_in;\n"          , nuioin-1);
    // verifica se vai ter que adicionar controle pra porta de saida
    if (nuioou > 0 && opc_out()) fprintf(f_veri, "wire [%d:0] proc_out_en;\n\n"        , nuioou-1);

    // ------------------------------------------------------------------------
    // instancia do processador -----------------------------------------------
    // ------------------------------------------------------------------------

    // o comeco eh padrao
    fprintf(f_veri, "%s proc(clk,rst", prname);
    // verifica se precisa adicionar barramento de entrada in
    if (nuioin > 0 && opc_inn()) fprintf(f_veri, ",proc_io_in" );
    // sempre adiciona o barramento de saida out
    /*tem que ter alguma saida*/ fprintf(f_veri, ",proc_io_out");
    // verifica se vai ter que adicionar controle pra porta de entrada
    if (nuioin > 0 && opc_inn()) fprintf(f_veri, ",proc_req_in");
    // verifica se vai ter que adicionar controle pra porta de saida
    if (nuioou > 0 && opc_out()) fprintf(f_veri, ",proc_out_en");
    // verifica se vai ter que adicionar pino de interrupcao
    if (itr_addr != 0)           fprintf(f_veri, ",1'b0"       );
    // finaliza instancia
                                 fprintf(f_veri, ");\n\n"      );

    // ------------------------------------------------------------------------
    // interface com portas de entrada ----------------------------------------
    // ------------------------------------------------------------------------

    if (opc_inn()) fprintf(f_veri, "// portas de entrada ----------------------------------------------------------\n\n");

    // cadastra portas de entrada pra simulacao
    for(int i=0;i<nuioin;i++)
    {
        if (inn_used(i))
        {
            fprintf(f_veri, "// variaveis da porta %d\n", i);
            fprintf(f_veri, "integer data_in_%d;\n", i);
            fprintf(f_veri, "reg signed [%d:0] in_%d = 0;\n", nubits-1, i);
            fprintf(f_veri, "reg req_in_%d = 0;\n\n", i);
        }
    }

    // abre os arquivos de leitura para as portas de entrada
    if (opc_inn())
    {
        fprintf(f_veri, "// abre um arquivo para leitura em cada porta\n");
        fprintf(f_veri, "initial begin\n");
    }
    for(int i=0;i<nuioin;i++)
    {
        if (inn_used(i))
        {
            force_rightbar(proc_dir);
            fprintf(f_veri, "    data_in_%d = $fopen(\"%s/Simulation/input_%d.txt\", \"r\"); // coloque os seus dados de entrada neste arquivo\n",i,proc_dir,i); 
        }
    }
    if (opc_inn()) fprintf(f_veri, "end\n\n");

    // decodifica portas de entrada
    if (opc_inn())
    {
        fprintf(f_veri, "// decodifica portas de entrada\n");
        fprintf(f_veri, "always @ (*) begin\n");
    }
    for(int i=0;i<nuioin;i++)
    {
        if (inn_used(i))
        {
            fprintf(f_veri, "    // decodificacao da porta %d\n", i);
            fprintf(f_veri, "    if (proc_req_in == %d) proc_io_in = in_%d;\n", (int)pow(2,i),i);
            fprintf(f_veri, "    req_in_%d = proc_req_in == %d;\n",                 i, (int)pow(2,i),i);
        }
    }
    if (opc_inn()) fprintf(f_veri, "end\n\n");

    // implementa a leitura dos dados de entrada
    if (opc_inn())
    {
        fprintf(f_veri, "// implementa a leitura dos dados de entrada\n");
        fprintf(f_veri, "integer scan_result;\n");
        fprintf(f_veri, "always @ (negedge clk) begin  \n");
    }
    for(int i=0;i<nuioin;i++)
    {
        if (inn_used(i))
        {
            fprintf(f_veri, "    // lendo a porta %d\n", i);
            fprintf(f_veri, "    if (data_in_%d != 0 && proc_req_in == %d) scan_result = $fscanf(data_in_%d, \"%%d\", in_%d);\n", i,(int)pow(2,i),i,i);
        }
    }
    if (opc_inn()) fprintf(f_veri, "end\n\n");

    // ------------------------------------------------------------------------
    // interface com portas de saida ------------------------------------------
    // ------------------------------------------------------------------------

    if (opc_out()) fprintf(f_veri, "// portas de saida ------------------------------------------------------------\n\n");

    // cadastra portas de saida
    for(int i=0;i<nuioou;i++)
    {
        if (out_used(i))
        {
            fprintf(f_veri, "// variaveis da porta %d\n", i);
            fprintf(f_veri, "integer data_out_%d;\n", i);
            fprintf(f_veri, "reg signed [%d:0] out_sig_%d = 0;\n", nubits-1, i);
            fprintf(f_veri, "reg out_en_%d = 0;\n\n", i);
        }
    }

    // abre os arquivos de escrita para as portas de saida
    if (opc_out())
    {
        fprintf(f_veri, "// abre um arquivo para escrita de cada porta\n");
        fprintf(f_veri, "initial begin\n");
    }
    for(int i=0;i<nuioou;i++)
    {
        if (out_used(i))
        {
            force_rightbar(proc_dir);
            fprintf(f_veri, "    data_out_%d = $fopen(\"%s/Simulation/output_%d.txt\", \"w\"); // veja os dados de saida neste arquivo\n",i,proc_dir,i); 
        }
    }
    if (opc_out()) fprintf(f_veri, "end\n\n");

    // decodifica portas de saida
    if (opc_out())
    {
        fprintf(f_veri, "// decodifica portas de saida\n");
        fprintf(f_veri, "always @ (*) begin\n");
    }
    for(int i=0;i<nuioou;i++)
    {
        if (out_used(i))
        {
            fprintf(f_veri, "    // decodificacao da porta %d\n", i);
            fprintf(f_veri, "    if (proc_out_en == %d) out_sig_%d <= proc_io_out;\n", (int)pow(2,i),i);
            fprintf(f_veri, "    out_en_%d = proc_out_en == %d;\n",                 i, (int)pow(2,i),i);
        }
    }
    if (opc_out()) fprintf(f_veri, "end\n\n");

    // implementa escrita no arquivo
    if (opc_out())
    {
        fprintf(f_veri, "// implementa escrita no arquivo\n");
        fprintf(f_veri, "always @ (posedge clk) begin\n");
    }
    for(int i=0;i<nuioou;i++)
    {
        if (out_used(i))
        {
            fprintf(f_veri, "    // escreve na porta %d\n", i);
            fprintf(f_veri, "    if (out_en_%d == 1'b1) $fdisplay(data_out_%d, \"%%0d\", out_sig_%d);\n", i,i,i);
        }
    }
    if (opc_out()) fprintf(f_veri, "end\n\n");

    // ------------------------------------------------------------------------
    // geracao do progress e $finish ------------------------------------------
    // ------------------------------------------------------------------------

    fprintf(f_veri, "// cadastro de sinais, barra de progresso e finish ----------------------------\n\n");

    fprintf(f_veri, "integer progress, chrys;\n");
    fprintf(f_veri, "initial begin\n\n");
    // necessario pro iverilog criar o .vcd
    fprintf(f_veri, "    $dumpfile(\"%s_tb.vcd\");\n\n", prname);
    //fprintf(f_veri, "    $dumpvars(0,%s_tb);\n\n"    , prname);
    fprintf(f_veri, "    $dumpvars(0,%s_tb.clk);\n"    , prname);
    fprintf(f_veri, "    $dumpvars(0,%s_tb.rst);\n"    , prname);

    // cadastra portas de entrada pra simulacao -------------------------------

    for(int i=0; i<nuioin; i++)
    {
        if (inn_used(i))
        {
            fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.req_in_sim_%d);\n", prname,i);
            fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.in_sim_%d);\n"    , prname,i);
        }
    }

    // cadastra portas de saida pra simulacao ---------------------------------

    for(int i=0;i<nuioou;i++)
    {
        if (out_used(i))
        {
            fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.out_en_sim_%d);\n", prname,i);
            fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.out_sig_%d);\n"   , prname,i);
        }
    }

    // cadastra instrucoes C+- e Assembly -------------------------------------

    fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.valr2);\n"    , prname);
    fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.linetabs);\n" , prname);

    // cadastra variaveis do usuario ------------------------------------------

    for (int i = 0; i < sim_cont(); i++)
    {
        if (sim_type(i) == 3)
        {
            // tenho que pegar o nome da variavel que eh a parte real do complexo
            // e juntar com a palavra "comp_" pra achar o nome correto
            // pra isso, checa primeiro se nao eh a parte imaginaria (nao termina com "_i_e_")
            if (strcmp(sim_name(i)+strlen(sim_name(i))-5,"_i_e_") != 0)
                fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.comp_%s);\n" , prname, sim_name(i));
        }
        else    fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.%s);\n"      , prname, sim_name(i));
    }

    // cadastra arrays do usuario ---------------------------------------------

    for (int i = 0; i < sim_cont_arr(); i++)
    {
        // se for complexo, pega so a variavel que é a parte real
        // pra compor o nome final com "comp_"
        if (sim_type_arr(i) == 3)
        {
            // checa primeiro se nao eh a parte imaginaria (nao termina com "_i_e_")
            if (strcmp(sim_name_arr(i)+strlen(sim_name_arr(i))-5,"_i_e_") != 0)
                for (int j = 0; j < sim_size_arr(i); j++)
                    fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.comp_%s%04d);\n" , prname, sim_name_arr(i), j);
        }
        else
        {
            for (int j = 0; j < sim_size_arr(i); j++)
                fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.%s%04d);\n" , prname, sim_name_arr(i), j);
        }
    }

    // se tiver CAL, cadastra flags da pilha de subrotinas --------------------

    if (opc_cal())
    {
        fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.p_%s.core.instr_fetch.genblk2.isp.pointeri);\n", prname, prname);
        fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.p_%s.core.instr_fetch.genblk2.isp.fl_max);\n"  , prname, prname);
        fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.p_%s.core.instr_fetch.genblk2.isp.fl_full);\n" , prname, prname);
    }

    // cadastra flags da pilha de dados ---------------------------------------

    fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.p_%s.core.sp.pointeri);\n", prname, prname);
    fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.p_%s.core.sp.fl_max);\n"  , prname, prname);
    fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.p_%s.core.sp.fl_full);\n" , prname, prname);

    // cadastra flags de erro de arredondamento -------------------------------

    fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.p_%s.core.ula.delta_float);\n" , prname, prname);
    fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.p_%s.core.ula.delta_int);\n\n" , prname, prname);

    // barra de progressao ----------------------------------------------------

    fprintf(f_veri, "    progress = $fopen(\"progress.txt\", \"w\");\n" );
    fprintf(f_veri, "    for (chrys = 10; chrys <= 100; chrys = chrys + 10) begin\n");
    fprintf(f_veri, "        #%f;\n"  , T*sim_clk_num()/10          );
    fprintf(f_veri, "        $fdisplay(progress,\"%%0d\",chrys);\n");
    fprintf(f_veri, "        $fflush(progress);\n");
    fprintf(f_veri, "    end\n\n");
    fprintf(f_veri, "    $fclose(progress);\n");

    // termina a simulacao ----------------------------------------------------

    fprintf(f_veri, "    $finish;\n\n");
    fprintf(f_veri, "end\n\n"); // fim do initial

    // ------------------------------------------------------------------------
    // finaliza arquivo -------------------------------------------------------
    // ------------------------------------------------------------------------

    fprintf(f_veri, "endmodule\n");

    fclose (f_veri);
}