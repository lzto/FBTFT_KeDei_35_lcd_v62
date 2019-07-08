#!/bin/bash

cp /home/tong/fbtft/*.ko /lib/modules/`uname -r`/kernel/misc && depmod -a

