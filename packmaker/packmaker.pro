include($$PWD/../cplusplus/cpp_boost/cpp_boost.pri)

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

include($$PWD/../PackToolChainBuildPath.pri)
DESTDIR=$$PackToolChainBuildPath



