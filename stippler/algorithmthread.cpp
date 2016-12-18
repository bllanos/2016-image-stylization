/*!
** \file algorithmthread.cpp
** \brief Implementation of the AlgorithmThread class.
**
** ### About
** Created for: COMP4905A Honours Project\n
** Fall 2016\n
** Bernard Llanos, ID 100793648\n
** Supervised by Dr. David Mould\n
** School of Computer Science, Carleton University
**
** ## References
** - [Qt QThread documentation](http://doc.qt.io/qt-5/qthread.html)
** - [Qt Queued Custom Type Example](http://doc.qt.io/qt-5/qtcore-threads-queuedcustomtype-example.html)
** - [Qt Progress Dialog Example](http://doc.qt.io/qt-5/qtconcurrent-progressdialog-example.html)
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
*/

#include "algorithmthread.h"
#include "algorithms/algorithm.h"
#include "imagedata.h"

AlgorithmThread::AlgorithmThread(QObject *parent) : QThread(parent),
    m_abort(false), alg(0)
{
}

AlgorithmThread::~AlgorithmThread() {
    mutex.lock();
    m_abort = true;
    mutex.unlock();

    cleanupAlgorithm();

    wait();
}

void AlgorithmThread::processImages(Algorithm *& algorithm, const QVector<QImage> &images)
{
    // Expect at least one image
    Q_ASSERT(images.size() > 0 && !images[0].isNull());

    // Locking here is not necessary, but is future-proof
    mutex.lock();
    m_abort = false;
    mutex.unlock();

    cleanupAlgorithm();

    alg = algorithm;
    algorithm = 0;
    m_images = images;
    start();
}

void AlgorithmThread::stopProcess()
{
    mutex.lock();
    m_abort = true;
    mutex.unlock();
}

void AlgorithmThread::run() {
    QVector<ImageData*>* input = new QVector<ImageData*>;

    foreach(const QImage &img, m_images) {
          input->append(new ImageData(img));
    }
    if(alg->initialize(input)) {
        if (m_abort) {
            return;
        }

        bool finished = false;
        bool ok = true;
        QString status;
        while(!finished && ok) {
            ok = alg->increment(finished, status);
            if (m_abort) {
                return;
            }
            if(ok) {
                emit sendStatus(status);
            }
        }
        if(ok) {
            QImage* outputImage = 0;
            QByteArray* svgOutputPtr = 0;
            ok = alg->output(outputImage, svgOutputPtr);
            if (m_abort) {
                if(outputImage != 0) {
                    delete outputImage;
                    outputImage = 0;
                }
                if(svgOutputPtr != 0) {
                    delete svgOutputPtr;
                    svgOutputPtr = 0;
                }
                return;
            }
            if(ok) {
                QByteArray svgOutput;
                if(svgOutputPtr != 0) {
                    svgOutput = *svgOutputPtr;
                }
                AlgorithmResultPair pair(*outputImage, svgOutput);
                if(outputImage != 0) {
                    delete outputImage;
                    outputImage = 0;
                }
                if(svgOutputPtr != 0) {
                    delete svgOutputPtr;
                    svgOutputPtr = 0;
                }
                emit sendOutput(pair);
                return; // Success - All done
            } else {
                if(outputImage != 0) {
                    delete outputImage;
                    outputImage = 0;
                }
                if(svgOutputPtr != 0) {
                    delete svgOutputPtr;
                    svgOutputPtr = 0;
                }
            }
        }
    }
    emit sendFail();
}

void AlgorithmThread::cleanupAlgorithm() {
    if(alg != 0) {
        delete alg;
        alg = 0;
    }
}
