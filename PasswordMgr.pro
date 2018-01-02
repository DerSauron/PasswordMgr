
TEMPLATE = app

QT = core gui widgets

CONFIG += c++14

# enable kwallet system (comment this out if kwallet is not installed)
CONFIG += use_kwallet

SOURCES = \
    src/AES256.cc \
    src/helper.cc \
    src/main.cc \
    src/MainFrame.cc \
    src/PasswordGenerator.cc \
    src/PasswordListItem.cc \
    src/StatusBubble.cc \
    src/WalletDelegate.cc

HEADERS = \
    src/AES256.h \
    src/helper.h \
    src/main.h \
    src/MainFrame.h \
    src/PasswordGenerator.h \
    src/PasswordListItem.h \
    src/StatusBubble.h \
    src/WalletDelegate.h

RESOURCES = \
    src/main.qrc

FORMS = \
    src/MainFrame.ui

CONFIG(use_kwallet) {
    SOURCES += \
        src/kde/WalletContent.cc \
        src/kde/WalletModel.cc \
        src/kde/WalletWidget.cc

    HEADERS += \
        src/kde/WalletContent.h \
        src/kde/WalletModel.h \
        src/kde/WalletWidget.h

    #RESOURCES += src/kde/*.qrc

    FORMS += \
        src/kde/WalletWidget.ui

    QT += KWallet
} else {
    SOURCES += \
        src/dummy/NoWallet.cc
}

linux-g++ {
    QMAKE_CXXFLAGS += -Wno-unused-variable -Wno-unused-parameter

    CONFIG(debug, debug|release) {
        QMAKE_CXXFLAGS -= -g
        QMAKE_CXXFLAGS += -g3 -gdwarf-2
    }
}

CONFIG(release, debug|release) {
    TARGET = $$PWD/dist/passwdmgr
}
