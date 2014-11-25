#!/bin/sh

/root/load_module.sh
/root/clean_deth.sh

(cd drop/src/ce && screen -S CE -d -m ./ce --debug=All)
