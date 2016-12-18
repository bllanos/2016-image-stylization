/*!
** \file algorithmmanager.cpp
** \brief Implementation of the AlgorithmManager class.
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

#include <QMenu>
#include <QAction>
#include "algorithmmanager.h"
#include "algorithmthread.h"
#include "imagemanager.h"
#include "imageviewer.h"
#include "algorithms/rgb2labgreyalgorithm.h"
#include "algorithms/midtonefilter.h"
#include "algorithms/superpixels/slic.h"
#include "algorithms/higher_order/filter/localdatafilter.h"

AlgorithmManager::AlgorithmManager(ImageViewer* v, ImageManager* m) :
    QObject(v), viewer(v), imManager(m), algThread(0),
    algorithmActions(), abortAction(0), algorithmIsRunning(false)
{
    algThread = new AlgorithmThread(this);
    connect(algThread, SIGNAL(finished()), this, SLOT(receiveFinished()));
    connect(algThread, SIGNAL(sendStatus(QString)), this, SLOT(receiveStatus(QString)));
    connect(algThread, SIGNAL(sendFail()), this, SLOT(receiveFail()));
    connect(algThread, SIGNAL(sendOutput(AlgorithmResultPair)), this, SLOT(receiveOutput(AlgorithmResultPair)));
}

QMenu * AlgorithmManager::createAlgorithmsMenu(void) {
    QMenu* menu = new QMenu(tr("&Algorithms"));
    algorithmActions.append(menu->addAction(tr("&CIE L*a*b* greyscale"), this, &AlgorithmManager::runGreyscale));
    algorithmActions.append(menu->addAction(tr("&CIE L*a*b* midtones"), this, &AlgorithmManager::runMidtoneFilter));
    algorithmActions.append(menu->addAction(tr("&SLIC"), this, &AlgorithmManager::runSLIC));
    algorithmActions.append(menu->addAction(tr("&SLIC superpixel size filter"), this,
                                            &AlgorithmManager::runLocalDataFilter_SIZE));
    algorithmActions.append(menu->addAction(tr("&SLIC superpixel greyscale stddev filter"), this,
                                            &AlgorithmManager::runLocalDataFilter_STDDEV_LSTAR));
    algorithmActions.append(menu->addAction(tr("&SLIC superpixel external selection map filter"), this,
                                            &AlgorithmManager::runLocalDataFilter_EXTERNAL));

    menu->addSeparator();
    abortAction = menu->addAction(tr("&Stop algorithm"), this, &AlgorithmManager::abort);

    // Initially, no image is present, and so no algorithms can be run
    toggleActions(false);
    abortAction->setEnabled(false);
    algorithmIsRunning = false;
    return menu;
}

void AlgorithmManager::enableAlgorithms(void) {
    if(!algorithmIsRunning) {
        toggleActions(true);
    }
}

void AlgorithmManager::runGreyscale() {
    viewer->setStatusBarMessage(tr("Running CIE L*a*b* greyscale algorithm"));
    Algorithm* alg = new Rgb2LabGreyAlgorithm();
    runAlgorithm(alg);
}

void AlgorithmManager::runMidtoneFilter() {
    viewer->setStatusBarMessage(tr("Running CIE L*a*b* midtone selection algorithm"));
    Algorithm* alg = new MidtoneFilter();
    runAlgorithm(alg);
}

void AlgorithmManager::runSLIC() {
    viewer->setStatusBarMessage(tr("Running SLIC algorithm"));
    Algorithm* alg = new SLIC();
    runAlgorithm(alg);
}

void AlgorithmManager::runLocalDataFilter_SIZE() {
    viewer->setStatusBarMessage(tr("Running SLIC superpixel size-filtering algorithm"));
    ISuperpixelGenerator* slic = new SLIC();
    Algorithm* alg = new LocalDataFilter(slic, LocalDataFilter::ScoreBasis::SIZE);
    runAlgorithm(alg);
}

void AlgorithmManager::runLocalDataFilter_STDDEV_LSTAR() {
    viewer->setStatusBarMessage(tr("Running SLIC superpixel lightness stddev-filtering algorithm"));
    ISuperpixelGenerator* slic = new SLIC();
    Algorithm* alg = new LocalDataFilter(slic, LocalDataFilter::ScoreBasis::STDDEV_LSTAR);
    runAlgorithm(alg);
}

void AlgorithmManager::runLocalDataFilter_EXTERNAL() {
    viewer->setStatusBarMessage(tr("Running SLIC superpixel external selection map filtering algorithm"));
    ISuperpixelGenerator* slic = new SLIC();
    Algorithm* alg = new LocalDataFilter(slic, LocalDataFilter::ScoreBasis::EXTERNAL);
    runAlgorithm(alg);
}

void AlgorithmManager::abort() {
    viewer->setStatusBarMessage(tr("Aborting algorithm"));
    algThread->stopProcess();
}

void AlgorithmManager::receiveFail() {
    viewer->setStatusBarMessage(tr("Algorithm failed"));
}

void AlgorithmManager::receiveFinished() {
    toggleActions(true);
}

void AlgorithmManager::receiveStatus(const QString &status) {
    viewer->setStatusBarMessage(status);
}

void AlgorithmManager::receiveOutput(const AlgorithmResultPair &pair) {
    const QByteArray* svgDataPtr = 0;
    if(!pair.svgData().isEmpty()) {
        svgDataPtr = new QByteArray(pair.svgData());
    }
    imManager->setImage(pair.image(), svgDataPtr);
}

void AlgorithmManager::toggleActions(bool runAlgorithm) {
    algorithmIsRunning = !runAlgorithm;
    foreach (QAction *act, algorithmActions) {
        act->setEnabled(runAlgorithm);
    }
    abortAction->setEnabled(!runAlgorithm);
}

void AlgorithmManager::runAlgorithm(Algorithm *algorithm) {

    // Retrieve the current image
    QImage image;
    imManager->getImage(image);
    QVector<QImage> images;
    images.append(image);

    // Load more input images if necessary
    QVector<QString> imageDescriptions;
    algorithm->additionalRequiredImages(imageDescriptions);
    foreach(const QString &str, imageDescriptions) {
          if(!imManager->browseForImage(image, str)) {
              viewer->setStatusBarMessage(tr("Cancelled"));
              return;
          }
          images.append(image);
    }
    toggleActions(false);
    algThread->processImages(algorithm, images);
}
