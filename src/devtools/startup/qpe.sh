#!/bin/sh
rm /tmp/restart-qtopia
while true
do
	${0%.sh} $* 2>&1 | logger -t Qtopia
	[ -e /tmp/restart-qtopia ] || break
done
