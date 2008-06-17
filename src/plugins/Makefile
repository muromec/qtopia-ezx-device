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
.PHONY: all install install_target packages lupdate sdk cleaninstall cinstall syncqtopia first_syncqtopia checkp4 remove_target relink printdependrules qmake_check test install_tests listtests qtopia_first first qtopia_all qtopia_clean clean qtopia_distclean distclean qtopia_install qmake qmake-debug regenerate regenerate_all nodeps loud silent
