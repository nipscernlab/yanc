#!/bin/bash
# synthesis only (area estimate) of the original sapho_all and of sapho_ptr on the prototype HDL
cd /c/nipscern/yanc-dirac
P=/c/nipscern/yanc-dirac/.smoke/dirac-proto/proto/SAPHO
MAPONLY=1 TAG=base bash Scripts/hw/fmax.sh sapho_all 5CSEBA6U23I7
MAPONLY=1 TAG=ptr HDL=$P bash Scripts/hw/fmax.sh sapho_ptr 5CSEBA6U23I7
