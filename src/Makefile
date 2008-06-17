define RULE
@+QPEDIR=$(QTOPIA_BUILD);\
if [ -z "$$QPEDIR" ]; then\
    QPEDIR=$(QTOPIA_BUILD_TREE);\
fi;\
if [ -z "$$QPEDIR" ]; then\
    QPEDIR=$(QPEDIR);\
fi;\
if [ -z "$$QPEDIR" ]; then\
    echo "ERROR: You must set QTOPIA_BUILD if you want to use depot hopping.";\
    echo "When this file was created, the Qtopia build tree was:";\
    echo "    /home/muromec/ming/build";\
    exit 1;\
fi;\
QPEDIR=`cd $$QPEDIR; /bin/pwd | sed 's/\/$$//'`;\
shadowdir=`echo $$PWD | perl -ne 's,/home/muromec/ming/moko/qtopia-opensource-4.3.1,'$$QPEDIR',;\
                                    print'`;\
if [ "$$shadowdir" != "$$PWD" -a -d "$$shadowdir" ]; then\
    cd "$$shadowdir";\
    $(MAKE) $@;\
else\
    echo "ERROR: Can't resolve shadowdir.";\
    echo "       SOURCE tree = '/home/muromec/ming/moko/qtopia-opensource-4.3.1'";\
    echo "       BUILD  tree = '$$QPEDIR'";\
    echo "       PWD         = '$$PWD'";\
    echo "       shadowdir   = '$$shadowdir'";\
    exit 1;\
fi
endef

all:
	$(RULE)
install:
	$(RULE)
install_target:
	$(RULE)
packages:
	$(RULE)
lupdate:
	$(RULE)
sdk:
	$(RULE)
cleaninstall:
	$(RULE)
cinstall:
	$(RULE)
syncqtopia:
	$(RULE)
first_syncqtopia:
	$(RULE)
checkp4:
	$(RULE)
remove_target:
	$(RULE)
relink:
	$(RULE)
printdependrules:
	$(RULE)
qmake_check:
	$(RULE)
test:
	$(RULE)
install_tests:
	$(RULE)
listtests:
	$(RULE)
qtopia_first:
	$(RULE)
first:
	$(RULE)
qtopia_all:
	$(RULE)
qtopia_clean:
	$(RULE)
clean:
	$(RULE)
qtopia_distclean:
	$(RULE)
distclean:
	$(RULE)
qtopia_install:
	$(RULE)
qmake_all:
	$(RULE)
make_default:
	$(RULE)
make_first:
	$(RULE)
install_subtargets:
	$(RULE)
uninstall_subtargets:
	$(RULE)
qtopia_uninstall:
	$(RULE)
sub-plugins-qtopiacore-kbddrivers-ezx_syncqtopia:
	$(RULE)
sub-plugins-qtopiacore-kbddrivers-ezx_first_syncqtopia:
	$(RULE)
sub-plugins-qtopiacore-kbddrivers-ezx_qtopia_first:
	$(RULE)
sub-plugins-qtopiacore-kbddrivers-ezx:
	$(RULE)
sub-plugins-qtopiacore-kbddrivers-ezx_qtopia_clean:
	$(RULE)
sub-plugins-qtopiacore-kbddrivers-ezx_qtopia_distclean:
	$(RULE)
sub-plugins-qtopiacore-kbddrivers-ezx_qtopia_install:
	$(RULE)
sub-plugins-qtopiacore-kbddrivers-ezx_qtopia_uninstall:
	$(RULE)
sub-plugins-qtopiacore-kbddrivers-ezx_regenerate_all:
	$(RULE)
sub-plugins-qtopiacore-kbddrivers-ezx_test:
	$(RULE)
sub-plugins-qtopiacore-kbddrivers-ezx_install_tests:
	$(RULE)
sub-plugins-qtopiacore-kbddrivers-ezx_checkp4:
	$(RULE)
sub-plugins-qtopiacore-kbddrivers-ezx_lupdate:
	$(RULE)
sub-plugins-qtopiacore-kbddrivers-ezx_packages:
	$(RULE)
sub-plugins-qtopiacore-kbddrivers-ezx_install_target:
	$(RULE)
sub-plugins-qtopiacore-kbddrivers-ezx_remove_target:
	$(RULE)
sub-plugins-qtopiacore-kbddrivers-ezx_nodeps:
	$(RULE)
sub-plugins-qtopiacore-kbddrivers-ezx_mocables:
	$(RULE)
sub-plugins-qtopiacore-kbddrivers-ezx_printdependrules:
	$(RULE)
sub-plugins-qtopiacore-kbddrivers-ezx_qmake_check:
	$(RULE)
sub-devtools_syncqtopia:
	$(RULE)
sub-devtools_first_syncqtopia:
	$(RULE)
sub-devtools_qtopia_first:
	$(RULE)
sub-devtools:
	$(RULE)
sub-devtools_qtopia_clean:
	$(RULE)
sub-devtools_qtopia_distclean:
	$(RULE)
sub-devtools_qtopia_install:
	$(RULE)
sub-devtools_qtopia_uninstall:
	$(RULE)
sub-devtools_regenerate_all:
	$(RULE)
sub-devtools_test:
	$(RULE)
sub-devtools_install_tests:
	$(RULE)
sub-devtools_checkp4:
	$(RULE)
sub-devtools_lupdate:
	$(RULE)
sub-devtools_packages:
	$(RULE)
sub-devtools_install_target:
	$(RULE)
sub-devtools_remove_target:
	$(RULE)
sub-devtools_nodeps:
	$(RULE)
sub-devtools_mocables:
	$(RULE)
sub-devtools_printdependrules:
	$(RULE)
sub-devtools_qmake_check:
	$(RULE)
mocables:
	$(RULE)
printsubdirsdependrules:
	$(RULE)
check_sdk:
	$(RULE)
qmake:
	$(RULE)
qmake-debug:
	$(RULE)
regenerate:
	$(RULE)
regenerate_all:
	$(RULE)
nodeps:
	$(RULE)
loud:
	$(RULE)
silent:
	$(RULE)
.PHONY: all install install_target packages lupdate sdk cleaninstall cinstall syncqtopia first_syncqtopia checkp4 remove_target relink printdependrules qmake_check test install_tests listtests qtopia_first first qtopia_all qtopia_clean clean qtopia_distclean distclean qtopia_install qmake_all make_default make_first install_subtargets uninstall_subtargets qtopia_uninstall sub-plugins-qtopiacore-kbddrivers-ezx_syncqtopia sub-plugins-qtopiacore-kbddrivers-ezx_first_syncqtopia sub-plugins-qtopiacore-kbddrivers-ezx_qtopia_first sub-plugins-qtopiacore-kbddrivers-ezx sub-plugins-qtopiacore-kbddrivers-ezx_qtopia_clean sub-plugins-qtopiacore-kbddrivers-ezx_qtopia_distclean sub-plugins-qtopiacore-kbddrivers-ezx_qtopia_install sub-plugins-qtopiacore-kbddrivers-ezx_qtopia_uninstall sub-plugins-qtopiacore-kbddrivers-ezx_regenerate_all sub-plugins-qtopiacore-kbddrivers-ezx_test sub-plugins-qtopiacore-kbddrivers-ezx_install_tests sub-plugins-qtopiacore-kbddrivers-ezx_checkp4 sub-plugins-qtopiacore-kbddrivers-ezx_lupdate sub-plugins-qtopiacore-kbddrivers-ezx_packages sub-plugins-qtopiacore-kbddrivers-ezx_install_target sub-plugins-qtopiacore-kbddrivers-ezx_remove_target sub-plugins-qtopiacore-kbddrivers-ezx_nodeps sub-plugins-qtopiacore-kbddrivers-ezx_mocables sub-plugins-qtopiacore-kbddrivers-ezx_printdependrules sub-plugins-qtopiacore-kbddrivers-ezx_qmake_check sub-devtools_syncqtopia sub-devtools_first_syncqtopia sub-devtools_qtopia_first sub-devtools sub-devtools_qtopia_clean sub-devtools_qtopia_distclean sub-devtools_qtopia_install sub-devtools_qtopia_uninstall sub-devtools_regenerate_all sub-devtools_test sub-devtools_install_tests sub-devtools_checkp4 sub-devtools_lupdate sub-devtools_packages sub-devtools_install_target sub-devtools_remove_target sub-devtools_nodeps sub-devtools_mocables sub-devtools_printdependrules sub-devtools_qmake_check mocables printsubdirsdependrules check_sdk qmake qmake-debug regenerate regenerate_all nodeps loud silent
