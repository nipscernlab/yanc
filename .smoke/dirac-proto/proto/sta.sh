#!/bin/bash
cd /c/nipscern/yanc-dirac/.smoke/hw/sapho_ptr_5CSEBA6U23I7_ptr
/c/intelFPGA_lite/24.1std/quartus/bin64/quartus_sta -t newpaths.tcl > newpaths.log 2>&1 || tail -5 newpaths.log
for f in to_ptr from_ptr to_rpt from_rpt; do echo "== $f"; grep "^;" $f.txt | sed -n 3,5p | awk -F';' '{print $2,$3,$4}'; done
