#ifndef ALGORITHMTHREAD_H
#define ALGORITHMTHREAD_H

/*!
** \file algorithmthread.h
** \brief Definition of the AlgorithmThread class.
**
** ### About
** Created for: COMP4905A Honours Project\n
** Fall 2016\n
** Bernard Llanos\n
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
**
*/

#include <QThread>
#include <QMutex>
#include <QImage>
#include <QVector>
#include "algorithmresultpair.h"

class Algorithm;

/*!
 * \brief A worker thread which will perform image processing operations
 *
 * The execution of image processing operations onto a separate thread allows
 * the user interface to remain responsive. In particular, the user can
 * cancel an image processing operation before it completes, due to the
 * incremental nature of the Algorithm class's processing functions.
 */
class AlgorithmThread : public QThread
{
    Q_OBJECT

public:
    /*!
     * \brief Create an object that can spawn a separate thread using processImages()
     * \param [in] parent The parent object which takes ownership of this object
     */
    AlgorithmThread(QObject *parent = 0);
    ~AlgorithmThread();

    /*!
     * \brief Start an image processing operation
     * \param [in] algorithm The image processing algorithm to execute. This object
     * takes ownership of `algorithm` and sets `algorithm` to null in the caller.
     * \param [in] images The images on which to operate
     */
    void processImages(Algorithm *& algorithm, const QVector<QImage> &images);

signals:
    /*!
     * \brief Provide a status update regarding the current state of processing
     * \param [out] status Status message to display
     */
    void sendStatus(const QString &status);

    /*!
     * \brief Signal an internal error in the image processing algorithm
     */
    void sendFail();

    /*!
     * \brief Transfer the results of the image processing algorithm
     *
     * This signal is intended to supply the output image to the main GUI thread.
     * \param [out] pair Output image data
     */
    void sendOutput(const AlgorithmResultPair &pair);

public slots:
    /*!
     * \brief Abort the current image processing algorithm's execution
     *
     * processImages() must be called again to restart processing (the current
     * state of the algorithm is lost).
     */
    void stopProcess();

protected:
    /*!
     * \brief Perform image processing in a separate thread
     */
    void run() Q_DECL_OVERRIDE;

private:
    void cleanupAlgorithm();

private:
    bool m_abort;
    Algorithm* alg;
    QVector<QImage> m_images;
    QMutex mutex;
};

#endif // ALGORITHMTHREAD_H
