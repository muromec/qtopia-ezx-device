qtopia_project(theme)

qtopiaconf.files=$$QTOPIA_DEPOT_PATH/devices/ezx/src/themes/qtopia.conf
qtopiaconf.path=/etc/themes
qtopiaconf.trtarget=Qtopia
qtopiaconf.hint=themecfg
qtopiaconf.outdir=$$PWD
INSTALLS+=qtopiaconf
qtopiadata.files=$$QTOPIA_DEPOT_PATH/devices/ezx/src/themes/qtopia/*.xml $$QTOPIA_DEPOT_PATH/etc/themes/qtopia/*rc
qtopiadata.path=/etc/themes/qtopia
INSTALLS+=qtopiadata
qtopiapics.files=$$QTOPIA_DEPOT_PATH/devices/ezx/pics/themes/qtopia/*
qtopiapics.path=/pics/themes/qtopia
qtopiapics.hint=pics
INSTALLS+=qtopiapics


pkg.name=qpe-theme-qtopia
pkg.desc=Qtopia theme
pkg.domain=trusted
