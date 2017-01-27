QT += core
QT -= gui

CONFIG += c++11

TARGET = slicer
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    graph_builder.cpp \
    slicer.cpp \
    graph.cpp

CXXFLAGS=$$system(llvm-config --cxxflags)
LDFLAGS=$$system(llvm-config --ldflags --libs --system-libs)

QMAKE_CXX=clang++
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
    graph_builder.h \
    slicer.h \
    graph.h
