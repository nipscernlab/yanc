#!/bin/bash
# full fits, one at a time: the original sapho_all and sapho_ptr on the prototype HDL
cd /c/nipscern/yanc-dirac
P=/c/nipscern/yanc-dirac/.smoke/dirac-proto/proto/SAPHO
date
TAG=base bash Scripts/hw/fmax.sh sapho_all 5CSEBA6U23I7
date
TAG=ptr HDL=$P bash Scripts/hw/fmax.sh sapho_ptr 5CSEBA6U23I7
date
