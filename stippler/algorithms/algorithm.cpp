/*!
** \file algorithm.cpp
** \brief Implementation of the Algorithm class.
**
** ### About
** Created for: COMP4905A Honours Project\n
** Fall 2016\n
** Bernard Llanos\n
** Supervised by Dr. David Mould\n
** School of Computer Science, Carleton University
**
** ## Primary basis
** None
**
** ## Other references
** [Qt Queued Custom Type Example](http://doc.qt.io/qt-5/qtcore-threads-queuedcustomtype-example.html)
**
** The original license from Qt examples is provided below:
**
** --------------------------------------
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** --------------------------------------
**
*/

#include <QSvgGenerator>
#include <QPainter>
#include <QBuffer>
#include "algorithm.h"
#include "imagedata.h"
#include "imagemanager.h"

Algorithm::Algorithm() :
    input(0),
    outputIsEnabled(true),
    outputImage(0),
    outputSVG(0),
    outputSVGPainter(0),
    svgGenerator(0),
    svgFileIOWrapper(0),
    failed(false),
    finished(false)
{

}

Algorithm::~Algorithm(void) {
    cleanup();
}

void Algorithm::disableOutput(void) {
    outputIsEnabled = false;
}

void Algorithm::additionalRequiredImages(QVector<QString> &imageDescriptions) {
    Q_ASSERT(imageDescriptions.isEmpty());
}

bool Algorithm::initialize(QVector<ImageData *> *&images) {
    QVector<ImageData *>::size_type size = images->size();
    Q_ASSERT(size > 0);
    ImageData * firstImage = (*images)[0];
    for(QVector<ImageData *>::size_type i = 1; i < size; i += 1) {
        if((*images)[i] != 0) {
            delete (*images)[i];
            (*images)[i] = 0;
        }
    }
    delete images;
    images = 0;
    return initialize(firstImage);
}

bool Algorithm::initialize(ImageData *&image) {
    cleanup();
    timer.start();
    input = image;
    image = 0;
    return true;
}

bool Algorithm::output(QImage *&image, QByteArray *& svgData) {
    image = 0;
    svgData = 0;
    if(!finished || failed || !outputIsEnabled) {
        return false;
    }
    image = outputImage;
    outputImage = 0; // Transfer ownership
    svgData = outputSVG;
    outputSVG = 0; // Transfer ownership
    return true;
}

bool Algorithm::isFinished(void) const {
    return finished;
}

bool Algorithm::initializeOutput(const QColor& fillColor,
        const bool& vectorOutput,
        const QString * const &title,
        const QString * const &description
) {
    if(!outputIsEnabled) {
        return false;
    }

    // Setup raster output objects
    outputImage = new QImage(input->size(), QImage::Format_ARGB32_Premultiplied);
    outputImage->fill(fillColor);

    // Setup vector output objects
    if(vectorOutput) {
        ImageManager::prepareSVGOutputBuffer(
            outputSVGPainter,
            svgGenerator,
            svgFileIOWrapper,
            outputSVG,
            input->size(),
            *title,
            *description
        );
        outputSVGPainter->fillRect(
                QRect(QPoint(), input->size()),
                fillColor
            );
    }
    return !(outputImage->isNull());
}

void Algorithm::finalizeOutput(void) {
    // Note that the QPainter::end() function is called by QPainter::~QPainter.
    if(outputSVGPainter != 0) {
        delete outputSVGPainter;
        outputSVGPainter = 0;
    }
    if(svgGenerator != 0) {
        delete svgGenerator;
        svgGenerator = 0;
    }
    if(svgFileIOWrapper != 0) {
        delete svgFileIOWrapper;
        svgFileIOWrapper = 0;
    }
}

void Algorithm::cleanup(void) {
    finalizeOutput();
    if(input != 0) {
        delete input;
        input = 0;
    }
    failed = false;
    finished = false;
}
