#------------------------------------------------------------------------------
# Helper functions ------------------------------------------------------------
#------------------------------------------------------------------------------

# fetches the first variable that matches the given text pattern
proc getVar {padrao} {
    for {set i 0} {$i < [gtkwave::getNumFacs]} {incr i} {
        set facname [gtkwave::getFacName $i]
        if {[string first $padrao $facname] != -1} {return $facname}
    }
    return ""
}

# lists all variables matching the same given text pattern
proc listVar {padrao} {
    set lista [list]
    for {set i 0} {$i < [gtkwave::getNumFacs]} {incr i} {
        set facname [gtkwave::getFacName $i]
        if {[string first $padrao $facname] != -1} {lappend lista $facname}
    }
    return $lista
}

# adds a variable to gtkwave
proc addVar {facname dataFormat color alias tradutor filter} {
    gtkwave::addSignalsFromList $facname
    gtkwave::/Edit/Data_Format/$dataFormat
    if {![string equal $tradutor ""]} {gtkwave::installFileFilter [gtkwave::setCurrentTranslateFile $tradutor]}
    if {![string equal $filter   ""]} {gtkwave::installProcFilter [gtkwave::setCurrentTranslateProc $filter  ]}
    gtkwave::/Edit/Color_Format/$color
    gtkwave::/Edit/Alias_Highlighted_Trace $alias
}

# adds variables according to the data type
proc addVars {tipo padrao dataFormat tradutor} {
    set var_typ [listVar $padrao]

    puts "Info: found [llength $var_typ] '$tipo' type variables"

    for {set i 0} {$i < [llength $var_typ] } {incr i} {
        set facname [lindex $var_typ $i]
        regexp {_f_(.*?)_v_} $facname -> funcao
        regexp {_v_(.*?)_e_} $facname -> var
        if {[string compare $funcao global]!=0} {append funcao "()"}
        addVar $facname $dataFormat "Orange" "$tipo $var in $funcao" "" $tradutor
    }
}

# adds arrays according to the data type
proc addArrs {tipo padrao dataFormat tradutor} {
    array set grupos {}

    # pick all arrays of the given type and split them into groups
    for {set i 0} {$i < [gtkwave::getNumFacs]} {incr i} {
        set facname [gtkwave::getFacName $i]

        if {[string compare $tipo "float"] == 0} {
            if {[string match "*$padrao*" $facname] &&
                [regexp {^(.*?)(\d{4})$} $facname -> base _]} {

                lappend grupos($base) $facname
            }
        } else {
            if {[string match "*$padrao*" $facname] &&
                [regexp {^(.*?)(\d{4}\[\d+:\d+\])$} $facname -> base _]} {

                lappend grupos($base) $facname
            }
        }
    }

    # add each array found
    foreach base [lsort [array names grupos]] {

        # add the array to gtkwave and highlight it
        gtkwave::addSignalsFromList $grupos($base)

        # translator
        gtkwave::/Edit/Data_Format/$dataFormat
        gtkwave::installProcFilter [gtkwave::setCurrentTranslateProc $tradutor]

        # extract function and array name
        regexp {_f_(.*?)_v_} $base -> funcao
        regexp {_v_(.*?)_e_} $base -> var
        if {[string compare $funcao global]!=0} {append funcao "()"}

        # create group in gtkwave and remove highlight
        gtkwave::/Edit/Create_Group "$tipo $var in $funcao"
        gtkwave::/Edit/Toggle_Group_Open|Close
        gtkwave::/Edit/UnHighlight_All

        # rename each index of the array
        set i 0
        foreach sinal $grupos($base) {
            gtkwave::highlightSignalsFromList $sinal
            gtkwave::/Edit/Alias_Highlighted_Trace "$var $i"
            incr i
        }
    }
}

#------------------------------------------------------------------------------
# Start here ------------------------------------------------------------------
#------------------------------------------------------------------------------

# Read parameters from the configuration file ---------------------------------

puts "Info: running standard GTKWave configuration..."

set    fileID [open "tcl_infos.txt" r]
gets  $fileID tmp_dir
gets  $fileID bin_dir
close $fileID

# Insert basic signals --------------------------------------------------------

set clk      [getVar "clk"]
set rst      [getVar "rst"]

gtkwave::addSignalsFromList [list $clk $rst]

# I/O separator ---------------------------------------------------------------

gtkwave::/Edit/Insert_Comment {I/O ****************}

# Input signals ---------------------------------------------------------------

set req_in  [listVar "proc.req_in_sim"]
set entrada [listVar "proc.in_sim"    ]

puts "Info: detected [llength $req_in] input ports in use"

for {set i 0} {$i < [llength $req_in] } {incr i} {

    #puts "Info: adding signals for input port $i"

    addVar [list [lindex $req_in  $i]] "Binary"         "Yellow" "req_in $i" "" ""
    addVar [list [lindex $entrada $i]] "Signed_Decimal" "Yellow" "input  $i" "" ""
}

# Output signals --------------------------------------------------------------

set out_en [listVar "proc.out_en_sim"]
set saida  [listVar "proc.out_sig"   ]

puts "Info: detected [llength $out_en] output ports in use"

for {set i 0} {$i < [llength $out_en] } {incr i} {

    #puts "Info: adding signals for output port $i"

    addVar [list [lindex $out_en $i]] "Binary"         "Yellow" "out_en $i" "" ""
    addVar [list [lindex $saida  $i]] "Signed_Decimal" "Yellow" "output $i" "" ""
}

# Instructions separator ------------------------------------------------------

gtkwave::/Edit/Insert_Comment {Instructions *******}

puts "Info: adding Assembly and C+- instructions..."

# Assembly --------------------------------------------------------------------

addVar [getVar "proc.valr2"   ] "Decimal" "Indigo" "Assembly" "$tmp_dir/trad_opcode.txt" ""

# C+- -------------------------------------------------------------------------

addVar [getVar "proc.linetabs"] "Signed_Decimal" "Violet" "C+-" "$tmp_dir/trad_cmm.txt" ""

# Variables separator ---------------------------------------------------------

gtkwave::/Edit/Insert_Comment {Variables **********}

puts "Info: adding variables..."

addVars "int"   "proc.me1"      "Signed_Decimal" ""
addVars "float" "proc.me2"      "BitsToReal"     ""
addVars "comp"  "proc.comp_me3" "Binary"         "$bin_dir/comp2gtkw.exe"
addArrs "int"   "arr_me1"       "Signed_Decimal" ""
addArrs "float" "arr_me2"       "BitsToReal"     ""
addArrs "comp"  "comp_arr_me3"  "Binary"         "$bin_dir/comp2gtkw.exe"

# Flags separator -------------------------------------------------------------

gtkwave::/Edit/Insert_Comment {Flags **************}

puts "Info: adding flags..."

# Stack -----------------------------------------------------------------------

set lista_flags [list [getVar "core.sp.pointeri"] [getVar "core.sp.fl_max"] [getVar "core.sp.fl_full"] [getVar "isp.pointeri"] [getVar "isp.fl_max"] [getVar "isp.fl_full"]]
gtkwave::addSignalsFromList $lista_flags
gtkwave::/Edit/Create_Group "Stack"
gtkwave::/Edit/Toggle_Group_Open|Close
gtkwave::/Edit/UnHighlight_All

gtkwave::highlightSignalsFromList [getVar "core.sp.pointeri"]
gtkwave::/Edit/Data_Format/Analog/Step
gtkwave::/Edit/Alias_Highlighted_Trace "Data Stack Pointer"

gtkwave::highlightSignalsFromList [getVar "core.sp.fl_max"]
gtkwave::/Edit/Data_Format/Decimal
gtkwave::/Edit/Alias_Highlighted_Trace "Data Stack Max"

gtkwave::highlightSignalsFromList [getVar "core.sp.fl_full"]
gtkwave::/Edit/Alias_Highlighted_Trace "Data Stack Overflow"

gtkwave::highlightSignalsFromList [getVar "isp.pointeri"]
gtkwave::/Edit/Data_Format/Analog/Step
gtkwave::/Edit/Alias_Highlighted_Trace "Inst Stack Pointer"

gtkwave::highlightSignalsFromList [getVar "isp.fl_max"]
gtkwave::/Edit/Data_Format/Decimal
gtkwave::/Edit/Alias_Highlighted_Trace "Inst Stack Max"

gtkwave::highlightSignalsFromList [getVar "isp.fl_full"]
gtkwave::/Edit/Alias_Highlighted_Trace "Inst Stack Overflow"

# ALU -------------------------------------------------------------------------

set lista_flags [list [getVar "ula.delta_int"] [getVar "ula.delta_float"]]
gtkwave::addSignalsFromList $lista_flags
gtkwave::/Edit/Create_Group "ALU"
gtkwave::/Edit/Toggle_Group_Open|Close
gtkwave::/Edit/UnHighlight_All

gtkwave::highlightSignalsFromList [getVar "ula.delta_int"]
gtkwave::/Edit/Data_Format/Analog/Step
gtkwave::/Edit/Alias_Highlighted_Trace "Rounding Error (int)"

gtkwave::highlightSignalsFromList [getVar "ula.delta_float"]
gtkwave::/Edit/Data_Format/Analog/Step
gtkwave::/Edit/Alias_Highlighted_Trace "Rounding Error (float)"

# Visualization ---------------------------------------------------------------

gtkwave::/Time/Zoom/Zoom_Best_Fit
gtkwave::/View/Left_Justified_Signals

# bug workaround -> creates an empty tab in gtkwave. refresh only works this way with GTK3
gtkwave::loadFile "fix.vcd"
gtkwave::setTabActive 0

puts "Info: standard GTKWave configuration finished"
