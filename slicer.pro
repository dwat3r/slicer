QT += core
QT -= gui

CONFIG += c++11

TARGET = slicer
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += src/main.cpp \
        src/pdg.cpp \
        src/pdgBuilder.cpp \
        src/pdgBuilderAction.cpp

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
  -lclangTooling \
  -lclangASTMatchers

HEADERS += include/pdg.h \
        include/pdgBuilder.h \
        include/pdgBuilderAction.h

INCLUDEPATH += include/
