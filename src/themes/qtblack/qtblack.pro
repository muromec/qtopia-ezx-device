qtopia_project(theme)

qtblackconf.files=$$QTOPIA_DEPOT_PATH/devices/ezx/src/themes/qtblack.conf
qtblackconf.path=/etc/themes
qtblackconf.trtarget=qtblack
qtblackconf.hint=themecfg
qtblackconf.outdir=$$PWD
INSTALLS+=qtblackconf
qtblackdata.files=$$QTOPIA_DEPOT_PATH/devices/ezx/src/themes/qtblack/*.xml $$QTOPIA_DEPOT_PATH/src/themes/qtblack/*rc
qtblackdata.path=/etc/themes/qtblack
INSTALLS+=qtblackdata
qtblackpics.files=$$QTOPIA_DEPOT_PATH/devices/ezx/pics/themes/qtblack/*
qtblackpics.path=/pics/themes/qtblack
qtblackpics.hint=pics
INSTALLS+=qtblackpics


qtblackbgimage.files=$$QTOPIA_DEPOT_PATH/devices/ezx/pics/themes/qtblack/background.png
qtblackbgimage.path=/pics/themes/qtblack
qtblackbgimage.hint=background
# let this install first so we can overwrite the image
qtblackbgimage.depends=install_qtblackpics
INSTALLS+=qtblackbgimage

pkg.name=qpe-theme-qtblack
pkg.desc=qtblack theme
pkg.domain=trusted
