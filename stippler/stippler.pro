#-------------------------------------------------
# Qt project file
#
# COMP4905A Honours Project
# Fall 2016
# Bernard Llanos, ID 100793648
# Supervised by Dr. David Mould
# School of Computer Science, Carleton University
#
#-------------------------------------------------

QT       += core gui widgets svg

TARGET = stippler
TEMPLATE = app

SOURCES += main.cpp\
    imageviewer.cpp \
    imagemanager.cpp \
    algorithms/algorithm.cpp \
    imagedata.cpp \
    algorithmmanager.cpp \
    algorithmthread.cpp \
    algorithms/rgb2labgreyalgorithm.cpp \
    algorithmresultpair.cpp \
    algorithms/superpixels/slic.cpp \
    algorithms/superpixels/superpixellation.cpp \
    algorithms/higher_order/filter/superpixelfilter.cpp \
    algorithms/higher_order/filter/localdatafilter.cpp \
    algorithms/higher_order/filter/filteredsuperpixellation.cpp \
    algorithms/midtonefilter.cpp \
    ods/array.cpp \
    ods/BinaryHeap.cpp \
    ods/utils.cpp

HEADERS  += imageviewer.h \
    imagemanager.h \
    algorithms/algorithm.h \
    imagedata.h \
    algorithmmanager.h \
    algorithmthread.h \
    algorithms/rgb2labgreyalgorithm.h \
    algorithmresultpair.h \
    algorithms/superpixels/slic.h \
    algorithms/superpixels/isuperpixelgenerator.h \
    algorithms/superpixels/superpixellation.h \
    algorithms/higher_order/filter/superpixelfilter.h \
    algorithms/higher_order/filter/localdatafilter.h \
    algorithms/higher_order/filter/filteredsuperpixellation.h \
    algorithms/midtonefilter.h \
    ods/array.h \
    ods/BinaryHeap.h \
    ods/utils.h

# This relates to Windows CE
# See http://doc.qt.io/qt-5/wince-with-qt-introduction.html
# See http://doc.qt.io/qt-5/deployment-plugins.html
wince {
   DEPLOYMENT_PLUGIN += qjpeg qgif
}
