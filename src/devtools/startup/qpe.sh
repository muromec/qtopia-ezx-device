#!/bin/sh
touch /tmp/restart-qtopia
while [ -e /tmp/restart-qtopia ] 
do
  ${0%.sh} -qws 2>&1 | logger -t Qtopia

  if (dmesg | egrep -q 'DLCI.*timeout'); then
    export QTOPIA_PHONE_DUMMY=1
  fi
  
done

