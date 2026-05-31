// ----------------------------------------------------------------------------
// routines for generating the .v files ---------------------------------------
// ----------------------------------------------------------------------------

// global includes
#include <string.h>
#include <stdlib.h>
#include  <stdio.h>
#include   <math.h>

// local includes
#include "..\Headers\t2t.h"
#include "..\Headers\eval.h"
#include "..\Headers\opcodes.h"
#include "..\Headers\simulacao.h"
#include "..\Headers\messages.h"

// ----------------------------------------------------------------------------
// helper functions -----------------------------------------------------------
// ----------------------------------------------------------------------------

// Verilator drops internal signals that don't fan out to a port, and forbids a
// hierarchical reference into a module (e.g. the _tb.v doing proc.valr10) unless
// the target is public. Every declaration in the sim-visibility harness below
// therefore carries this attribute so the signals survive Verilator's optimiser
// and stay reachable for waveform viewing (GTKWave). To Icarus it is a comment.
#define VPUB " /* verilator public_flat */"

// replaces \ with / on a path
void force_rightbar(char *str){while (*str) {if (*str == '\\') *str = '/'; str++;}}

// ----------------------------------------------------------------------------
// generates the verilog file with a processor instance -----------------------
// ----------------------------------------------------------------------------

void hdl_vv_file(int n_ins, int n_dat, int nbopr, int itr_addr, int toaqui_addr)
{
    // ------------------------------------------------------------------------
    // create the .v file for writing -----------------------------------------
    // ------------------------------------------------------------------------

    char    tmp[2048];
    snprintf(tmp, sizeof(tmp), "%s/Hardware/%s.v", proc_dir, prname);

    FILE *f_veri = fopen(tmp,"w");

    // ------------------------------------------------------------------------
    // write the module header and ports --------------------------------------
    // ------------------------------------------------------------------------

    int comma;
    int nbioin = (nuioin > 1) ? (int)ceil(log2(nuioin)) : 1;
    int nbioou = (nuioou > 1) ? (int)ceil(log2(nuioou)) : 1;

    // the prologue is fixed ....
    fprintf(f_veri, "module %s (\n\n", prname);
    fprintf(f_veri, "input  clk, rst,\n");

    // check whether the 'in' input bus needs to be added
    if (nuioin > 0 && opc_inn()) fprintf(f_veri, "input  signed [%d:0] in ,\n", nubits-1);
    // always add the 'out' output bus
    /*needs at least one output*/ fprintf(f_veri, "output signed [%d:0] out"   , nubits-1); comma = 0;

    // check whether anything else may come next, to add a trailing comma
    if ((nuioin > 0 && opc_inn()) || (nuioou > 0 && opc_out()) || (itr_addr != 0) || (toaqui_addr != 0)) {fprintf(f_veri, ",\n"); comma = 1;}

    // check whether input-port control needs to be added
    if (nuioin > 0 && opc_inn()) {fprintf(f_veri, "output [%d:0] req_in", nuioin-1); comma = 0;}

    // check whether anything else may come next, to add a trailing comma
    if ((nuioou > 0 && opc_out()) || (itr_addr != 0) || (toaqui_addr != 0)) if (comma == 0) {fprintf(f_veri, ",\n"); comma = 1;}

    // check whether output-port control needs to be added
    if (nuioou > 0 && opc_out()) {fprintf(f_veri, "output [%d:0] out_en", nuioou-1); comma = 0;}

    // check whether anything else may come next, to add a trailing comma
    if ((itr_addr != 0) || (toaqui_addr != 0)) if (comma == 0) {fprintf(f_veri, ",\n"); comma = 1;}

    // check whether the interrupt pin needs to be added
    if (itr_addr != 0) {fprintf(f_veri, "input  itr"); comma = 0;}

    // check whether the #TOAQUI tracker pin needs to be added
    if (toaqui_addr != 0) if (comma == 0) {fprintf(f_veri, ",\n"); comma = 1;}
    if (toaqui_addr != 0) {fprintf(f_veri, "output cheguei"); comma = 0;}

    // close the port list
    fprintf(f_veri, ");\n\n");

    // ------------------------------------------------------------------------
    // I/O interface wires ----------------------------------------------------
    // ------------------------------------------------------------------------

    // no input bus: a wire is needed so it can be tied off into the processor
    if (nuioin == 0 || !opc_inn()) fprintf(f_veri, "wire [%d:0] in;\n", nubits-1);

    // no interrupt pin: a wire is needed for the processor connection
    if (itr_addr == 0) fprintf(f_veri, "wire itr = 1'b0;\n");

    // no #TOAQUI marker: a local wire absorbs the processor's cheguei output
    if (toaqui_addr == 0) fprintf(f_veri, "wire cheguei;\n");

    // always needed for the processor connection
    fprintf(f_veri, "wire proc_req_in, proc_out_en;\n");

    // always needed for the processor connection
    fprintf(f_veri, "wire [%d:0] addr_in;\n"   , nbioin-1);
    fprintf(f_veri, "wire [%d:0] addr_out;\n\n", nbioou-1);

    // ------------------------------------------------------------------------
    // simulation interface wires ---------------------------------------------
    // ------------------------------------------------------------------------

    // The simulation-visibility harness (mem/PC taps below, plus the user
    // variable / array mirrors further down) is for waveform viewing only and
    // must stay out of synthesis. It is compiled for Icarus -- which predefines
    // __ICARUS__ -- and for Verilator when the user passes +define+YANC_TRACE
    // (Verilator drops un-read signals, so each mirror is also tagged VPUB).
    // Verilog `ifdef has no OR, so fold both triggers into one guard macro; the
    // `ifndef keeps a multi-.v compile from warning on a second definition.
    fprintf(f_veri, "`ifdef __ICARUS__\n `ifndef YANC_SIM_VIS\n  `define YANC_SIM_VIS\n `endif\n`endif\n");
    fprintf(f_veri, "`ifdef YANC_TRACE\n `ifndef YANC_SIM_VIS\n  `define YANC_SIM_VIS\n `endif\n`endif\n\n");

    fprintf(f_veri, "`ifdef YANC_SIM_VIS\n");
    // access to the variables in data memory
    fprintf(f_veri, "wire mem_wr;\n");
    fprintf(f_veri, "wire [%d:0] mem_addr_wr;\n" , (int)ceil(log2(n_dat)-1));
    // access to the instruction index in the program counter
    fprintf(f_veri, "wire [%d:0] pc_sim_val;\n"  , (int)ceil(log2(n_ins)-1));
    fprintf(f_veri, "`endif\n\n");

    // ------------------------------------------------------------------------
    // processor parameters ---------------------------------------------------
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

    // if there's an interrupt, set its address on the processor
    if (itr_addr != 0) fprintf(f_veri, ".ITRADD(%d),\n", itr_addr);

    // if there's a #TOAQUI marker, set its address on the processor
    if (toaqui_addr != 0) fprintf(f_veri, ".TOAQUIADDR(%d),\n", toaqui_addr);

    // ------------------------------------------------------------------------
    // dynamic resource allocation --------------------------------------------
    // ------------------------------------------------------------------------

    for (int i = 0; i < opc_cnt(); i++) fprintf(f_veri, ".%s(1),\n", opc_get(i));

    // this estimate is rough, but it's better than nothing
    printf(MSG_INFO_ISA_USAGE, opc_cnt()*100/102);
    printf(MSG_INFO_ULA_USAGE, opc_ucnt()*100/(49-15)); // 49 in the mux, 15 duplicated or unused

    // ------------------------------------------------------------------------
    // finalize the processor instance ----------------------------------------
    // ------------------------------------------------------------------------

    char path[2048]; snprintf(path, sizeof(path), "%s/Hardware/%s", proc_dir, prname); force_rightbar(path);

    fprintf(f_veri, ".DFILE(\"%s_data.mif\"),\n"  , path);
    fprintf(f_veri, ".IFILE(\"%s_inst.mif\"))\n\n", path);
    fprintf(f_veri, "`ifdef YANC_SIM_VIS\n");
    fprintf(f_veri, "p_%s (clk, rst, in, out, addr_in, addr_out, proc_req_in, proc_out_en, itr, cheguei, mem_wr, mem_addr_wr,pc_sim_val);\n", prname);
    fprintf(f_veri, "`else\n");
    fprintf(f_veri, "p_%s (clk, rst, in, out, addr_in, addr_out, proc_req_in, proc_out_en, itr, cheguei);\n", prname);
    fprintf(f_veri, "`endif\n\n");

    // ------------------------------------------------------------------------
    // I/O address decoders ---------------------------------------------------
    // ------------------------------------------------------------------------

    // check whether an address decoder is needed for the input ports
    if (opc_inn())
    {
        // single port: hook it straight to proc_req_in
        if (nuioin == 1) fprintf(f_veri, "assign req_in = proc_req_in;\n");
        else             fprintf(f_veri, "addr_dec #(%d) dec_in (proc_req_in, addr_in , req_in);\n"  , nuioin);
    }

    // check whether an address decoder is needed for the output ports
    if (opc_out())
    {
        // single port: hook it straight to proc_out_en
        if (nuioou == 1) fprintf(f_veri, "assign out_en = proc_out_en;\n\n");
        else             fprintf(f_veri, "addr_dec #(%d) dec_out(proc_out_en, addr_out, out_en);\n\n", nuioou);
    }

    // ------------------------------------------------------------------------
    // simulation interface start (iverilog/verilator + gtkwave) -------------
    // ------------------------------------------------------------------------

    fprintf(f_veri, "// ----------------------------------------------------------------------------\n");
    fprintf(f_veri, "// Simulation -----------------------------------------------------------------\n");
    fprintf(f_veri, "// ----------------------------------------------------------------------------\n\n");

    // YANC_SIM_VIS was defined up by the simulation-interface wires above.
    fprintf(f_veri, "`ifdef YANC_SIM_VIS\n\n");

    // ------------------------------------------------------------------------
    // register I/O ports for the simulation ----------------------------------
    // ------------------------------------------------------------------------

    fprintf(f_veri, "// I/O ------------------------------------------------------------------------\n\n");

    // register input ports for the simulation
    for(int i=0; i<nuioin; i++)
    {
        if (inn_used(i))
        {
            fprintf(f_veri, "reg signed [%d:0] in_sim_%d" VPUB " = 0;\n", nubits-1, i);
            fprintf(f_veri, "reg req_in_sim_%d" VPUB " = 0;\n", i);
        }
    }
    if (opc_inn()) fprintf(f_veri,"\n");

    // register output ports for the simulation
    for(int i=0;i<nuioou;i++)
    {
        if (out_used(i))
        {
            fprintf(f_veri, "reg signed [%d:0] out_sig_%d" VPUB " = 0;\n", nubits-1, i);
            fprintf(f_veri, "reg out_en_sim_%d" VPUB " = 0;\n", i);
        }
    }

    // decode input ports
    if (opc_inn())
    {
        // in_sim_N captures-and-holds the presented input -- that is state, so
        // clock it (a conditional assign in always @(*) infers a latch that
        // Verilator rejects). req_in_sim_N is an unconditional combinational
        // mirror, so it never latches.
        if (nuioin > 0) fprintf(f_veri, "\nalways @ (posedge clk) begin\n");
        for(int i=0; i<nuioin; i++)
            if (inn_used(i))
                fprintf(f_veri, "   if (req_in == %d) in_sim_%d <= in;\n", (int)pow(2,i),i);
        if (nuioin > 0) fprintf(f_veri, "end\n");

        if (nuioin > 0) fprintf(f_veri, "always @ (*) begin\n");
        for(int i=0; i<nuioin; i++)
        {
            if (inn_used(i))
                fprintf(f_veri, "   req_in_sim_%d = req_in == %d;\n",  i, (int)pow(2,i));
            else printf(MSG_WARN_UNUSED_IN_PORT, i);
        }
        if (nuioin > 0) fprintf(f_veri, "end\n");
    }

    // decode output ports
    if (opc_out())
    {
        // out_sig_N captures-and-holds the last written output -- that is state,
        // so clock it (a conditional assign in always @(*) infers a latch that
        // Verilator rejects). out_en_sim_N is an unconditional combinational
        // mirror, so it never latches.
        if (nuioou > 0) fprintf(f_veri, "\nalways @ (posedge clk) begin\n");
        for(int i=0;i<nuioou;i++)
            if (out_used(i))
                fprintf(f_veri, "   if (out_en == %d) out_sig_%d <= out;\n", (int)pow(2,i),i);
        if (nuioou > 0) fprintf(f_veri, "end\n");

        if (nuioou > 0) fprintf(f_veri, "always @ (*) begin\n");
        for(int i=0;i<nuioou;i++)
        {
            if (out_used(i))
                fprintf(f_veri, "   out_en_sim_%d = out_en == %d;\n",     i, (int)pow(2,i));
            else printf(MSG_WARN_UNUSED_OUT_PORT, i);
        }
        if (nuioou > 0) fprintf(f_veri, "end\n\n");
    }

    // ------------------------------------------------------------------------
    // register user variables for the simulation -----------------------------
    // ------------------------------------------------------------------------

    fprintf(f_veri, "// variables ------------------------------------------------------------------\n\n");

    int has_float = 0;
    // create a register for each found variable
    for (int i = 0; i < sim_cont(); i++)
    {
        // int: use the reg data type in the simulation
        if (sim_type(i) == 1)
        {
            fprintf(f_veri, "reg [%d:0] %s" VPUB " = 0;\n", nubits-1, sim_name(i));
        }

        // float: use the real data type in the simulation
        if (sim_type(i) == 2)
        {
            // first float variable: emit the helpers
            if (has_float == 0)
            {
                has_float = 1;
                fprintf(f_veri, "integer sm_me2; always @ (*) sm_me2 = (out[%d]) ? -out[%d:0] : out[%d:0];\n", nbmant+nbexpo  , nbmant-1, nbmant-1);
                fprintf(f_veri, "integer  e_me2; always @ (*)  e_me2 = $signed(out[%d:%d]);\n"               , nbmant+nbexpo-1, nbmant);
            }
            // create the real variable with initial value 0.0
            fprintf(f_veri, "real %s" VPUB " = 0.0;\n", sim_name(i));
        }

        // comp: use reg in the simulation
        if (sim_type(i) > 2)
        {
            // currently using dx, but it should be the binary equivalent of 0.0
            // try it with itob(f2mf("0.0",NULL), nubits)
            fprintf(f_veri, "reg [%d:0] %s" VPUB " = %d'dx;\n", nubits-1, sim_name(i), nubits-1);
        }
    }

    // begin the always block that registers the variables
    fprintf(f_veri, "\nalways @ (posedge clk) begin\n");
    // register each variable based on its address
    for (int i = 0; i < sim_cont(); i++)
    {
        if (sim_type(i) == 1) fprintf(f_veri, "   if (mem_addr_wr == %d && mem_wr) %s <= out;\n"                   , sim_addr(i), sim_name(i));
        if (sim_type(i) == 2) fprintf(f_veri, "   if (mem_addr_wr == %d && mem_wr) %s <= sm_me2*$pow(2.0,e_me2);\n", sim_addr(i), sim_name(i));
        if (sim_type(i) >  2) fprintf(f_veri, "   if (mem_addr_wr == %d && mem_wr) %s <= out;\n"                   , sim_addr(i), sim_name(i));
    }
    fprintf(f_veri, "end\n\n");

    // for comp variables ...
    // combine real and imag parts into a variable twice the width
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
                    fprintf(f_veri,"wire [16+%d*2-1:0] comp_%s" VPUB " = {8'd%d, 8'd%d, %s, %s};\n", nubits, sim_name(i), nbmant, nbexpo, sim_name(i), sim_name(j));
            }
        }
    }

    if (has_comp == 1) fprintf(f_veri, "\n");

    // ------------------------------------------------------------------------
    // register user arrays for the simulation --------------------------------
    // ------------------------------------------------------------------------

    if (sim_cont_arr()>0)
    fprintf(f_veri, "// arrays ---------------------------------------------------------------------\n\n");

    // create a register for each array element
    for (int i = 0; i < sim_cont_arr(); i++)
    {
        for (int j = 0; j < sim_size_arr(i); j++)
        {
            // grab the value from the .mif file
            char val[256]; sim_mem(sim_addr_arr(i)+j, val);

            // int: use the reg data type in the simulation
            if (sim_type_arr(i) == 1)
            {
                fprintf(f_veri, "reg [%d-1:0] %s%04d" VPUB "=%d'b%s;\n", nubits, sim_name_arr(i), j, nubits, val);
            }

            // float: use the real data type in the simulation
            if (sim_type_arr(i) == 2)
            {
                if (has_float == 0)
                {
                    has_float = 1;
                    fprintf(f_veri, "integer sm_me2; always @ (*) sm_me2 = (out[%d]) ? -out[%d:0] : out[%d:0];\n", nbmant+nbexpo  , nbmant-1, nbmant-1);
                    fprintf(f_veri, "integer  e_me2; always @ (*)  e_me2 = $signed(out[%d:%d]);\n"               , nbmant+nbexpo-1, nbmant);
                }
                fprintf(f_veri, "real %s%04d" VPUB " = %f;\n", sim_name_arr(i), j, mf2f(val));
            }

            // comp: use reg in the simulation
            if (sim_type_arr(i) > 2)
            {
                fprintf(f_veri, "reg [%d-1:0] %s%04d" VPUB "=%d'b%s;\n", nubits, sim_name_arr(i), j, nubits, val);
            }
        }
    }

    // begin the always block that registers the array variables
    if (sim_cont_arr()>0) fprintf(f_veri, "\nalways @ (posedge clk) begin\n");
    // register each variable based on its address
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

    // for comp arrays ...
    // combine real and imag parts into a variable twice the width
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
                        fprintf(f_veri,"wire [16+%d*2-1:0] comp_%s%04d" VPUB " = {8'd%d, 8'd%d, %s%04d, %s%04d};\n", nubits, sim_name_arr(i), k, nbmant, nbexpo, sim_name_arr(i), k, sim_name_arr(j), k);
            }
        }
    }

    if (sim_cont_arr()>0) fprintf(f_veri, "\n");

    // ------------------------------------------------------------------------
    // interface with cmm and asm instructions --------------------------------
    // ------------------------------------------------------------------------

    fprintf(f_veri, "// instructions ---------------------------------------------------------------\n\n");

    // create 10 registers to delay the @fim instruction
    int nreg = 10;
    for (int i = 0; i < nreg; i++) fprintf(f_veri, "reg [%d:0] valr%d" VPUB "=0;\n", nubits-1, i+1);
    fprintf(f_veri, "\n");

    int num_ins = get_n_ins();

    // create the memory that will hold the instruction table
    fprintf(f_veri, "reg [19:0] min [0:%d-1];\n\n", num_ins);
    // create the interface to that memory
    fprintf(f_veri, "reg signed [19:0] linetab" VPUB " =-1;\n"  );
    fprintf(f_veri, "reg signed [19:0] linetabs" VPUB "=-1;\n\n");
    // initialize the memory from the .txt file
    fprintf(f_veri, "initial	$readmemb(\"pc_%s_mem.txt\",min);\n\n"  , prname );
    // run the registers
    fprintf(f_veri, "always @ (posedge clk) begin\n");
    fprintf(f_veri, "if (pc_sim_val < %d) linetab <= min[pc_sim_val];\n", num_ins);
    fprintf(f_veri, "linetabs <= linetab;   \n");
    fprintf(f_veri, "valr1    <= pc_sim_val;\n");
    // delay stages for @fim
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

    // No $finish in the synthesizable proc .v -- it would be a sim-only
    // construct sneaking into hardware. The matching $finish at @fim now
    // lives in the auto-generated _tb.v (hdl_tb_file below); multi-proc
    // workflows that ship their own top-level testbench bypass the auto
    // _tb.v and drive termination themselves.

    // ------------------------------------------------------------------------
    // finalize the file ------------------------------------------------------
    // ------------------------------------------------------------------------

    fprintf(f_veri, "`endif\n\n");        // YANC_SIM_VIS
    fprintf(f_veri, "endmodule" );

    fclose(f_veri);
}

// ----------------------------------------------------------------------------
// generates the verilog file with the processor's test-bench -----------------
// ----------------------------------------------------------------------------

void hdl_tb_file(int itr_addr, int toaqui_addr)
{
    // ------------------------------------------------------------------------
    // create the .v file -----------------------------------------------------
    // ------------------------------------------------------------------------

    char    tmp[2048];
    snprintf(tmp, sizeof(tmp), "%s/%s_tb.v", temp_dir, prname);

    FILE *f_veri = fopen(tmp,"w");

    // ------------------------------------------------------------------------
    // header -----------------------------------------------------------------
    // ------------------------------------------------------------------------

    fprintf(f_veri, "`timescale 1ns/1ps\n\n");

    // Same YANC_SIM_VIS guard as <proc>.v / core.v / ula.v, so the stack + ULA
    // rounding-error $dumpvars further down (gated `ifdef YANC_SIM_VIS) are
    // emitted under Icarus AND under +define+YANC_TRACE. Defines do not reliably
    // cross the bash command-line file order, so the tb defines its own.
    fprintf(f_veri, "`ifdef __ICARUS__\n `ifndef YANC_SIM_VIS\n  `define YANC_SIM_VIS\n `endif\n`endif\n");
    fprintf(f_veri, "`ifdef YANC_TRACE\n `ifndef YANC_SIM_VIS\n  `define YANC_SIM_VIS\n `endif\n`endif\n\n");

    fprintf(f_veri,    "module %s_tb();\n\n", prname);

    // ------------------------------------------------------------------------
    // clock and reset generation ---------------------------------------------
    // ------------------------------------------------------------------------

    fprintf(f_veri, "// clock and reset generation -------------------------------------------------\n\n");

    double T = 1000.0/sim_clk(); // clock period in ns (clk frq in MHz)

    // clock and reset variables
    fprintf(f_veri, "reg clk, rst;\n\n");

    // initial block start
    fprintf(f_veri, "initial begin\n");
    // initialize clock and assert reset
    fprintf(f_veri, "    clk = 0;\n");
    fprintf(f_veri, "    rst = 1;\n");
    fprintf(f_veri, "    #%f;\n",  T);
    fprintf(f_veri, "    rst = 0;\n");
    // initial block end
    fprintf(f_veri, "end\n\n");

    // clock generation -------------------------------------------------------
    fprintf(f_veri, "always #%f clk = ~clk;\n\n", T/2.0);

    // ------------------------------------------------------------------------
    // processor interface ports ----------------------------------------------
    // ------------------------------------------------------------------------

    fprintf(f_veri, "// processor instance ---------------------------------------------------------\n\n");

    // check whether the 'in' input bus needs to be added
    if (nuioin > 0 && opc_inn()) fprintf(f_veri, "reg  signed [%d:0] proc_io_in = 0;\n", nubits-1);
    // always add the 'out' output bus
    /*needs at least one output*/ fprintf(f_veri, "wire signed [%d:0] proc_io_out;\n"   , nubits-1);
    // check whether input-port control needs to be added
    if (nuioin > 0 && opc_inn()) fprintf(f_veri, "wire [%d:0] proc_req_in;\n"          , nuioin-1);
    // check whether output-port control needs to be added
    if (nuioou > 0 && opc_out()) fprintf(f_veri, "wire [%d:0] proc_out_en;\n\n"        , nuioou-1);
    // declare the cheguei wire when the wrapper exposes it
    if (toaqui_addr != 0)        fprintf(f_veri, "wire proc_cheguei;\n\n"              );

    // ------------------------------------------------------------------------
    // processor instance -----------------------------------------------------
    // ------------------------------------------------------------------------

    // the prologue is fixed
    fprintf(f_veri, "%s proc(clk,rst", prname);
    // check whether the 'in' input bus needs to be added
    if (nuioin > 0 && opc_inn()) fprintf(f_veri, ",proc_io_in" );
    // always add the 'out' output bus
    /*needs at least one output*/ fprintf(f_veri, ",proc_io_out");
    // check whether input-port control needs to be added
    if (nuioin > 0 && opc_inn()) fprintf(f_veri, ",proc_req_in");
    // check whether output-port control needs to be added
    if (nuioou > 0 && opc_out()) fprintf(f_veri, ",proc_out_en");
    // check whether the interrupt pin needs to be added
    if (itr_addr != 0)           fprintf(f_veri, ",1'b0"       );
    // check whether the cheguei output is exposed
    if (toaqui_addr != 0)        fprintf(f_veri, ",proc_cheguei");
    // close the instance
                                 fprintf(f_veri, ");\n\n"      );

    // ------------------------------------------------------------------------
    // input-port interface ---------------------------------------------------
    // ------------------------------------------------------------------------

    if (opc_inn()) fprintf(f_veri, "// input ports ----------------------------------------------------------------\n\n");

    // register input ports for the simulation
    for(int i=0;i<nuioin;i++)
    {
        if (inn_used(i))
        {
            fprintf(f_veri, "// port %d variables\n", i);
            fprintf(f_veri, "integer data_in_%d;\n", i);
            fprintf(f_veri, "reg signed [%d:0] in_%d = 0;\n", nubits-1, i);
            fprintf(f_veri, "reg req_in_%d = 0;\n\n", i);
        }
    }

    // open the read files for the input ports
    if (opc_inn())
    {
        fprintf(f_veri, "// open a file for reading on each port\n");
        fprintf(f_veri, "initial begin\n");
    }
    for(int i=0;i<nuioin;i++)
    {
        if (inn_used(i))
        {
            force_rightbar(proc_dir);
            fprintf(f_veri, "    data_in_%d = $fopen(\"%s/Simulation/input_%d.txt\", \"r\"); // place your input data in this file\n",i,proc_dir,i);
        }
    }
    if (opc_inn()) fprintf(f_veri, "end\n\n");

    // decode input ports
    if (opc_inn())
    {
        fprintf(f_veri, "// decode input ports\n");
        fprintf(f_veri, "always @ (*) begin\n");
        // Default assignment so proc_io_in is driven on every path -- a bare
        // `if (proc_req_in == N) proc_io_in = in_N;` with no else infers a latch
        // under Verilator. The proc only samples `in` while req_in is asserted,
        // so the otherwise-0 value is never read: behaviour is unchanged.
        fprintf(f_veri, "    proc_io_in = 0;\n");
    }
    for(int i=0;i<nuioin;i++)
    {
        if (inn_used(i))
        {
            fprintf(f_veri, "    // port %d decoding\n", i);
            fprintf(f_veri, "    if (proc_req_in == %d) proc_io_in = in_%d;\n", (int)pow(2,i),i);
            fprintf(f_veri, "    req_in_%d = proc_req_in == %d;\n",                 i, (int)pow(2,i));
        }
    }
    if (opc_inn()) fprintf(f_veri, "end\n\n");

    // implement reading of the input data
    if (opc_inn())
    {
        fprintf(f_veri, "// implement reading of the input data\n");
        fprintf(f_veri, "integer scan_result;\n");
        fprintf(f_veri, "always @ (negedge clk) begin  \n");
    }
    for(int i=0;i<nuioin;i++)
    {
        if (inn_used(i))
        {
            fprintf(f_veri, "    // reading port %d\n", i);
            fprintf(f_veri, "    if (data_in_%d != 0 && proc_req_in == %d) scan_result = $fscanf(data_in_%d, \"%%d\", in_%d);\n", i,(int)pow(2,i),i,i);
        }
    }
    if (opc_inn()) fprintf(f_veri, "end\n\n");

    // ------------------------------------------------------------------------
    // output-port interface --------------------------------------------------
    // ------------------------------------------------------------------------

    if (opc_out()) fprintf(f_veri, "// output ports ---------------------------------------------------------------\n\n");

    // register output ports
    for(int i=0;i<nuioou;i++)
    {
        if (out_used(i))
        {
            fprintf(f_veri, "// port %d variables\n", i);
            fprintf(f_veri, "integer data_out_%d;\n", i);
            fprintf(f_veri, "reg signed [%d:0] out_sig_%d = 0;\n", nubits-1, i);
            fprintf(f_veri, "reg out_en_%d = 0;\n\n", i);
        }
    }

    // open the write files for the output ports
    if (opc_out())
    {
        fprintf(f_veri, "// open a file for writing on each port\n");
        fprintf(f_veri, "initial begin\n");
    }
    for(int i=0;i<nuioou;i++)
    {
        if (out_used(i))
        {
            force_rightbar(proc_dir);
            fprintf(f_veri, "    data_out_%d = $fopen(\"%s/Simulation/output_%d.txt\", \"w\"); // check the output data in this file\n",i,proc_dir,i);
        }
    }
    if (opc_out()) fprintf(f_veri, "end\n\n");

    // decode output ports
    //
    // Combinational with UNCONDITIONAL assignment to out_sig_N: the file-write
    // block below gates on out_en_N at posedge clk and reads out_sig_N at the
    // same instant, so what matters is that out_sig_N == proc_io_out exactly
    // when out_en_N is high. The original form `if (proc_out_en == N) out_sig_N <= proc_io_out;`
    // inferred a latch (no else) and used `<=` in combinational (COMBDLY).
    // Both Verilator warnings go away with the unconditional `=` form, and
    // the file output is unchanged because the gate is the same.
    if (opc_out())
    {
        fprintf(f_veri, "// decode output ports\n");
        fprintf(f_veri, "always @ (*) begin\n");
    }
    for(int i=0;i<nuioou;i++)
    {
        if (out_used(i))
        {
            fprintf(f_veri, "    // port %d decoding\n", i);
            fprintf(f_veri, "    out_sig_%d = proc_io_out;\n", i);
            fprintf(f_veri, "    out_en_%d  = proc_out_en == %d;\n", i, (int)pow(2,i));
        }
    }
    if (opc_out()) fprintf(f_veri, "end\n\n");

    // implement writing to the file
    if (opc_out())
    {
        fprintf(f_veri, "// implement writing to the file\n");
        fprintf(f_veri, "always @ (posedge clk) begin\n");
    }
    for(int i=0;i<nuioou;i++)
    {
        if (out_used(i))
        {
            fprintf(f_veri, "    // write to port %d\n", i);
            // flush immediately so a produced value survives even if the sim is
            // interrupted before $finish (heavy/long sims under I/O pressure)
            fprintf(f_veri, "    if (out_en_%d == 1'b1) begin $fdisplay(data_out_%d, \"%%0d\", out_sig_%d); $fflush(data_out_%d); end\n", i,i,i,i);
        }
    }
    if (opc_out()) fprintf(f_veri, "end\n\n");

    // ------------------------------------------------------------------------
    // signal registration, progress bar, and finish --------------------------
    // ------------------------------------------------------------------------
    // valr10 is the pipelined PC inside the <prname> module. The early-$finish
    // handler is necessary because the progress-bar block uses wall-clock
    // #delays instead of monitoring the PC, so without this the sim would run
    // the full cycle budget even after the program has terminated. progress
    // is declared above the handler so the handler can $fclose it; the initial
    // block opens it at time 0, before any posedge clk can trigger the
    // handler, so the handle is guaranteed valid by the time we get there.
    // This is the AUTO-generated testbench, bypassed by multi-proc workflows
    // that ship their own top-level testbench (e.g. DTW), so the unconditional
    // $finish here is fine -- those projects ignore this _tb.v entirely.

    fprintf(f_veri, "// signal registration, progress bar and finish ------------------------------\n\n");

    fprintf(f_veri, "integer progress, chrys;\n\n");

    fprintf(f_veri, "always @ (posedge clk) if (proc.valr10 == %d) begin\n", sim_get_fim());
    fprintf(f_veri, "    $display(\"Info: end of program!\");\n");
    fprintf(f_veri, "    $fclose(progress);\n");
    fprintf(f_veri, "    $finish;\n");
    fprintf(f_veri, "end\n\n");

    fprintf(f_veri, "initial begin\n\n");
    // needed for iverilog to create the .vcd
    fprintf(f_veri, "    $dumpfile(\"%s_tb.vcd\");\n\n", prname);
    //fprintf(f_veri, "    $dumpvars(0,%s_tb);\n\n"    , prname);
    fprintf(f_veri, "    $dumpvars(0,%s_tb.clk);\n"    , prname);
    fprintf(f_veri, "    $dumpvars(0,%s_tb.rst);\n"    , prname);

    // register input ports for the simulation --------------------------------

    for(int i=0; i<nuioin; i++)
    {
        if (inn_used(i))
        {
            fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.req_in_sim_%d);\n", prname,i);
            fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.in_sim_%d);\n"    , prname,i);
        }
    }

    // register output ports for the simulation -------------------------------

    for(int i=0;i<nuioou;i++)
    {
        if (out_used(i))
        {
            fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.out_en_sim_%d);\n", prname,i);
            fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.out_sig_%d);\n"   , prname,i);
        }
    }

    // register C+- and Assembly instructions ---------------------------------

    fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.valr2);\n"    , prname);
    fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.linetabs);\n" , prname);

    // register user variables ------------------------------------------------

    for (int i = 0; i < sim_cont(); i++)
    {
        if (sim_type(i) == 3)
        {
            // need the name of the variable that is the real part of the complex
            // and combine it with "comp_" to get the correct name
            // so first check it's not the imaginary part (doesn't end with "_i_e_")
            if (strcmp(sim_name(i)+strlen(sim_name(i))-5,"_i_e_") != 0)
                fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.comp_%s);\n" , prname, sim_name(i));
        }
        else    fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.%s);\n"      , prname, sim_name(i));
    }

    // register user arrays ---------------------------------------------------

    for (int i = 0; i < sim_cont_arr(); i++)
    {
        // if it's complex, take only the variable that is the real part
        // to compose the final name with "comp_"
        if (sim_type_arr(i) == 3)
        {
            // first check it's not the imaginary part (doesn't end with "_i_e_")
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

    // The stack-pointer flags and ULA rounding-error signals below live in the
    // hand-written core/ula, gated by YANC_SIM_VIS (Icarus or +define+YANC_TRACE).
    // The self-referential fl_max assignment and the real modulo they use compile
    // and simulate fine under current Verilator, so gate the dumps the same way as
    // the signals: present in both the Icarus and the YANC_TRACE Verilator builds.
    fprintf(f_veri, "`ifdef YANC_SIM_VIS\n");

    // if there's a CAL, register the subroutine-stack flags ------------------

    if (opc_cal())
    {
        fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.p_%s.core.instr_fetch.genblk2.isp.pointeri);\n", prname, prname);
        fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.p_%s.core.instr_fetch.genblk2.isp.fl_max);\n"  , prname, prname);
        fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.p_%s.core.instr_fetch.genblk2.isp.fl_full);\n" , prname, prname);
    }

    // register data-stack flags ----------------------------------------------

    fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.p_%s.core.sp.pointeri);\n", prname, prname);
    fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.p_%s.core.sp.fl_max);\n"  , prname, prname);
    fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.p_%s.core.sp.fl_full);\n" , prname, prname);

    // register rounding-error flags ------------------------------------------

    fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.p_%s.core.ula.delta_float);\n" , prname, prname);
    fprintf(f_veri, "    $dumpvars(0,%s_tb.proc.p_%s.core.ula.delta_int);\n" , prname, prname);

    fprintf(f_veri, "`endif\n\n");

    // progress bar -----------------------------------------------------------

    fprintf(f_veri, "    progress = $fopen(\"progress.txt\", \"w\");\n" );
    fprintf(f_veri, "    for (chrys = 10; chrys <= 100; chrys = chrys + 10) begin\n");
    fprintf(f_veri, "        #%f;\n"  , T*sim_clk_num()/10          );
    fprintf(f_veri, "        $fdisplay(progress,\"%%0d\",chrys);\n");
    fprintf(f_veri, "        $fflush(progress);\n");
    fprintf(f_veri, "    end\n\n");
    fprintf(f_veri, "    $fclose(progress);\n");

    // end the simulation -----------------------------------------------------

    fprintf(f_veri, "    $finish;\n\n");
    fprintf(f_veri, "end\n\n"); // end of initial

    // ------------------------------------------------------------------------
    // finalize the file ------------------------------------------------------
    // ------------------------------------------------------------------------

    fprintf(f_veri, "endmodule\n");

    fclose (f_veri);
}
