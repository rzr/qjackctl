INCLUDEPATH += ./src

SOURCES += src/main.cpp \
           src/qjackctlPatchbay.cpp \
           src/qjackctlPatchbayRack.cpp \
           src/qjackctlPatchbayFile.cpp \
           src/qjackctlConnections.cpp \
           src/qjackctlSetup.cpp

HEADERS += src/qjackctlPatchbay.h \
           src/qjackctlPatchbayRack.h \
           src/qjackctlPatchbayFile.h \
           src/qjackctlConnections.h \
           src/qjackctlStatus.h \
           src/qjackctlSetup.h \
           src/qjackctlAbout.h

FORMS    = src/qjackctlMainForm.ui \
           src/qjackctlMessagesForm.ui \
           src/qjackctlStatusForm.ui \
           src/qjackctlConnectionsForm.ui \
           src/qjackctlPatchbayForm.ui \
           src/qjackctlSocketForm.ui \
           src/qjackctlSetupForm.ui \
           src/qjackctlAboutForm.ui

IMAGES   = icons/qjackctl.xpm \
           icons/start1.png \
           icons/stop1.png \
           icons/quit1.png \
           icons/play1.png \
           icons/pause1.png \
           icons/accept1.png \
           icons/new1.png \
           icons/open1.png \
           icons/save1.png \
           icons/apply1.png \
           icons/add1.png \
           icons/up1.png \
           icons/down1.png \
           icons/socket1.png \
           icons/qtlogo.png

TEMPLATE = app
CONFIG  += qt warn_on release
LANGUAGE = C++

LIBS    += -ljack

