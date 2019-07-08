#!/bin/bash

dtc -@ -I dts -O dtb -o kedei.dtb kedei.dts
cp kedei.dtb /boot/overlays/

echo Add a line to /boot/config.txt:
echo dtoverlay=kedei

