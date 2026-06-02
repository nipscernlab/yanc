:: ****************************************************************************
:: Like go_proc.bat, but simulates with VERILATOR instead of Icarus/vvp, so
:: the waveform shows ALL the user variables / arrays / PC pipeline in GTKWave.
::
:: It feeds the SAME testbench asmcomp generates for Icarus (<proc>_tb.v) to
:: Verilator in --binary --timing mode (Verilator 5 understands the tb's
:: `always #5 clk` / `#10` delays and its $finish). +define+YANC_TRACE turns on
:: the simulation-visibility harness inside <proc>.v (the in_sim_*/out_sig_*
:: port mirrors, the me*_f_* user-variable mirrors, the arr_* array mirrors,
:: linetabs and the valr* PC tap -- each tagged /* verilator public_flat */),
:: and --trace dumps the whole hierarchy to <proc>_tb.vcd. Because we reuse the
:: generated testbench, this adapts to each proc's own I/O automatically -- no
:: hand-written C++ driver, exactly the signals the Icarus flow shows.
:: ****************************************************************************

:: Set up the terminal --------------------------------------------------------

cls
echo off

:: Set up the environment -----------------------------------------------------

:: current directory
set ROOT_DIR=%cd%

:: required tools (compiler toolchain, same as go_proc.bat)
set BISON=C:\packs\msys64\usr\bin\bison.exe
set FLEX=C:\packs\msys64\usr\bin\flex.exe
set GCC=C:\packs\msys64\mingw64\bin\x86_64-w64-mingw32-gcc.exe

:: Verilator + GTKWave. Verilator's internal make -> g++ -> python3 chain needs
:: msys64\mingw64\bin on PATH, and VERILATOR_ROOT pointing at its share dir.
set VERILATOR=C:\packs\msys64\mingw64\bin\verilator_bin.exe
set VERILATOR_ROOT=C:/packs/msys64/mingw64/share/verilator
:: nipscern GTKWave v4 -- opened with a pre-generated .gtkw (gen_gtkw) via -a;
:: it ignores --script, so the Tcl formatters are replaced by gen_gtkw.
set GTKWAVE=C:\nipscern\Aurora\components\Packages\gtkwave-nipscern\gtkwave.exe
set PATH=C:\packs\msys64\mingw64\bin;%PATH%

set    TESTE_DIR=%ROOT_DIR%\Teste
rmdir %TESTE_DIR% /s /q

:: Parameters defined by the SAPHO user for compilation -----------------------

:: project folder name
set PROJET=FFT
:: processor type name to simulate (a subfolder of the project)
set PROC=proc_fft
:: cmm filename where the processor is defined
set FNAM=proc_fft.cmm
:: gtkwave layout filename: if Simulation\%GTKW% exists it is applied via -a;
:: otherwise gen_gtkw writes the layout from the VCD header
set GTKW=teste.gtkw
:: processor operating frequency in MHz
set FRE_CLK=100
:: number of clocks to simulate
set NUM_CLK=1000000

:: Parameters that SAPHO must know --------------------------------------------

:: folder tree after installation
set INST_DIR=%TESTE_DIR%\saphoComponents
set BIN_DIR=%INST_DIR%\bin
set HDL_DIR=%INST_DIR%\HDL
set MAC_DIR=%INST_DIR%\Macros
set SCR_DIR=%INST_DIR%\Scripts
set TMP_DIR=%INST_DIR%\Temp

:: project folder tree being executed
set USER_DIR=%TESTE_DIR%\Projetos
set PROC_DIR=%USER_DIR%\%PROC%
set SOFT_DIR=%PROC_DIR%\Software
set HARD_DIR=%PROC_DIR%\Hardware
set SIMU_DIR=%PROC_DIR%\Simulation
set TMP_PRO=%TMP_DIR%\%PROC%

:: Verilator obj_dir (generated C++ model + the V<proc>_tb sim exe)
set VL_DIR=%TMP_PRO%\vl

:: Create test directories ----------------------------------------------------

mkdir %TESTE_DIR%
    mkdir %INST_DIR%
        mkdir %BIN_DIR%
        mkdir %HDL_DIR%
        mkdir %MAC_DIR%
        mkdir %SCR_DIR%
        mkdir %TMP_DIR%
            mkdir %TMP_PRO%
    mkdir %USER_DIR%

:: Verilator's g++ wants a writable TMP for its intermediate files
set TMP=%TMP_PRO%
set TEMP=%TMP_PRO%

:: Copy files into the test directories ---------------------------------------

xcopy Compilers\CMMComp\Tests %USER_DIR% /e /i /q>%TMP_PRO%\log.txt
xcopy HDL      %HDL_DIR%  /q    /y>%TMP_PRO%\log.txt
xcopy Compilers\CMMComp\Includes %MAC_DIR% /q /y>%TMP_PRO%\log.txt
xcopy Scripts  %SCR_DIR%  /q    /y>%TMP_PRO%\log.txt

:: Build the CMM compiler -----------------------------------------------------

cd %ROOT_DIR%\Compilers\CMMComp\Sources

%BISON% -y -d CMMComp.y
%FLEX%        CMMComp.l
%GCC%      -o cmmcomp.exe ast.c data_assign.c data_declar.c data_use.c itr.c diretivas.c funcoes.c labels.c lex.yy.c oper.c saltos.c stdlib.c t2t.c variaveis.c array_index.c global.c macros.c messages.c args.c y.tab.c

move cmmcomp.exe %BIN_DIR%>%TMP_PRO%\log.txt
del  lex.yy.c
del  y.tab.c
del  y.tab.h

:: Build the Assembler pre-processor ------------------------------------------

cd %ROOT_DIR%\Compilers\APPComp\Sources

%FLEX% -o app.c app.l
%GCC%  -o appcomp.exe app.c eval.c variaveis.c messages.c args.c

move appcomp.exe %BIN_DIR%>%TMP_PRO%\log.txt
del  app.c

:: Build the Assembler compiler -----------------------------------------------

cd %ROOT_DIR%\Compilers\ASMComp\Sources

%FLEX% -o ASMComp.c ASMComp.l
%GCC%  -o asmcomp.exe ASMComp.c eval.c labels.c opcodes.c variaveis.c t2t.c hdl.c simulacao.c array.c messages.c args.c

move asmcomp.exe %BIN_DIR%>%TMP_PRO%\log.txt
del  ASMComp.c

:: Build the GTKWave tools ------------------------------------------------------
::
:: comp2gtkw.exe is the "comp" (complex number) process filter; gen_gtkw.exe
:: reads the VCD header and writes the formatted .gtkw save file (replacing the
:: gtk_proc_init.tcl formatter).

cd %SCR_DIR%

%GCC% -o comp2gtkw.exe comp2gtkw.c
%GCC% -o gen_gtkw.exe  gen_gtkw.c

move comp2gtkw.exe %BIN_DIR%>%TMP_PRO%\log.txt
move gen_gtkw.exe  %BIN_DIR%>%TMP_PRO%\log.txt

:: Run the CMM compiler -------------------------------------------------------

echo #### Running the CMM compiler

cd %BIN_DIR%

cmmcomp.exe -i %FNAM% -n %PROC% -p %PROC_DIR% -m %MAC_DIR% -t %TMP_PRO%

:: Run the Assembler pre-processor --------------------------------------------

echo #### Running the Pre-assembler

set ASM_FILE=%SOFT_DIR%\%PROC%.asm

appcomp.exe -i %ASM_FILE% -t %TMP_PRO%

:: Run the Assembler compiler -------------------------------------------------
::
:: -t %TMP_PRO% makes asmcomp emit <proc>_tb.v and pc_<proc>_mem.txt there, and
:: <proc>.v + the _data/_inst .mif into %HARD_DIR%.

echo #### Running the Assembler

asmcomp.exe -i %ASM_FILE% -p %PROC_DIR% -d %HDL_DIR% -m %MAC_DIR% -t %TMP_PRO% -f %FRE_CLK% -c %NUM_CLK%

:: Build the simulation with Verilator ----------------------------------------
::
:: Reuse the generated <proc>_tb.v as the Verilator top. --timing handles the
:: testbench's #-delays / clock generator; +define+YANC_TRACE pulls in the
:: visibility harness; --trace dumps every signal to <proc>_tb.vcd.

echo #### Running Verilator (+define+YANC_TRACE, --binary --timing --trace)

set UPROC=%HARD_DIR%\%PROC%
set TB=%TMP_PRO%\%PROC%_tb.v

%VERILATOR% --binary --timing --trace +define+YANC_TRACE ^
    --top-module %PROC%_tb ^
    -Wno-lint -Wno-UNOPTFLAT -Wno-MULTIDRIVEN -Wno-BLKANDNBLK -Wno-WIDTH ^
    -Wno-CASEINCOMPLETE -Wno-IMPLICIT -Wno-COMBDLY -Wno-STMTDLY -Wno-INFINITELOOP ^
    --Mdir %VL_DIR% ^
    %TB% %UPROC%.v ^
    %HDL_DIR%\processor.v %HDL_DIR%\core.v %HDL_DIR%\ula.v ^
    %HDL_DIR%\addr_dec.v %HDL_DIR%\instr_dec.v

:: Run the simulation ---------------------------------------------------------
::
:: cd into TMP_PRO first: <proc>.v does $readmemb("pc_<proc>_mem.txt") with a
:: RELATIVE path (the PC -> source-line table that feeds `linetabs`), and the
:: testbench's $dumpfile writes <proc>_tb.vcd relative to the CWD too. The
:: _data/_inst .mif are referenced by absolute path, so they resolve anywhere.
:: The testbench self-terminates at $finish (PC reaches end-of-program).

echo #### Running the Verilator simulation

cd %TMP_PRO%

%VL_DIR%\V%PROC%_tb.exe

:: Run GtkWave ----------------------------------------------------------------
::
:: gen_gtkw reads the <proc>_tb.vcd header and writes <proc>_tb.gtkw: the same
:: layout the gtk_proc_init.tcl produced (clk/rst, I/O ports, Assembly via
:: trad_opcode.txt, C+- via trad_cmm.txt, the int/float/comp variable + array
:: mirrors, and the Stack/ALU flag groups), as a static save file. GTKWave then
:: opens the VCD with -a <gtkw>. trad files live at %TMP_DIR%\<proc-type>\, so
:: gen_gtkw takes %TMP_DIR% as its base.
::
:: NOTE: the Stack (core.sp.*) and ALU (ula.delta_*) flag groups are absent under
:: Verilator -- those signals are fenced out of the trace (Icarus-only) -- so
:: gen_gtkw simply omits them. Every other group is formatted as in the Icarus flow.

echo #### Generating the .gtkw layout and launching GTKWave

if exist %SIMU_DIR%\%GTKW% (
    %GTKWAVE% --rcvar "hide_sst on" --dark %TMP_PRO%\%PROC%_tb.vcd -a %SIMU_DIR%\%GTKW%
) else (
    %BIN_DIR%\gen_gtkw.exe %TMP_PRO%\%PROC%_tb.vcd %TMP_PRO%\%PROC%_tb.gtkw %TMP_DIR% %BIN_DIR%\comp2gtkw.exe
    %GTKWAVE% --rcvar "hide_sst on" --dark %TMP_PRO%\%PROC%_tb.vcd -a %TMP_PRO%\%PROC%_tb.gtkw
)

cd %ROOT_DIR%
