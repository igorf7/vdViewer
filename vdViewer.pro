QT       += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    chartview.cpp \
    csvreader.cpp \
    main.cpp \
    vdviewer.cpp

HEADERS += \
    chartview.h \
    csvreader.h \
    vdviewer.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

win32: RC_ICONS += \
    images/diag1.ico

win32:{
    QMAKE_TARGET_COMPANY = IGORF
    QMAKE_TARGET_PRODUCT = vdViewer
    QMAKE_TARGET_COPYRIGHT = IGORF
}
