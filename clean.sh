#!/bin/bash

IPCS_S=`ipcs -s | egrep "0x[0-9a-f]+ [0-9]+" | cut -f2 -d" "`

for id in $IPCS_S; do
  ipcrm -s $id;
done

