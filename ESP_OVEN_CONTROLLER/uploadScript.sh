#!/bin/bash
FILE=ESP_OVEN_CONTROLLER.ino.d1_mini.bin
MAC=600194b0f8a1
DEST=/home/esp
scp $FILE root@10.8.0.1:$DEST/$MAC/firmware.bin
CODE_FILE=`echo $FILE | sed 's/^\(.*ino\).*$/\1/'`
VERSION=`cat $CODE_FILE | grep "#define FW_VERSION" | cut -d" " -f 3`
echo $VERSION > /tmp/version_$MAC
scp /tmp/version_$MAC root@10.8.0.1:$DEST/$MAC/version
rm /tmp/version_$MAC

