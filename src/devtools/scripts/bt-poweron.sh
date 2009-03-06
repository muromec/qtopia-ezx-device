#!/bin/sh

export PATH=/tmp/mbtdload/:$PATH
[ -z "$BT_ADDR" ] && export BT_ADDR=0009DD502319

mkdir /tmp/mbtdload/
dd if=/dev/mtd1 of=/tmp/mbtdload/mbtdload bs=28808 count=1

chmod +x /tmp/mbtdload/mbtdload

hwtool -B
ezx-hciattach -n

rm -rf /tmp/mbtdload/
