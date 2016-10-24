QT += core
QT -= gui

CONFIG += c++11

TARGET = slicer
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    pdg.cpp \
    astwalker.cpp

CXXFLAGS=$$system(/usr/lib/llvm-3.9/bin/llvm-config --cxxflags)
LDFLAGS=$$system(/usr/lib/llvm-3.9/bin/llvm-config --ldflags --libs --system-libs)
QMAKE_CXX=/usr/lib/llvm-3.9/bin/clang++
QMAKE_CXXFLAGS=$$CXXFLAGS -std=libc++
QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter
QMAKE_LIBS+= $$LDFLAGS \
  -lclangFrontend \
  -lclangParse \
  -lclangSema \
  -lclangStaticAnalyzerFrontend \
  -lclangStaticAnalyzerCheckers \
  -lclangStaticAnalyzerCore \
  -lclangDriver \
  -lclangEdit \
  -lclangAnalysis \
  -lclangSerialization \
  -lclangAST \
  -lclangLex \
  -lclangBasic\
  -lclangTooling

HEADERS += \
    pdg.h \
    astwalker.h
