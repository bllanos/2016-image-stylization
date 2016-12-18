/*!
** \file imageviewer.cpp
** \brief Implementation of the ImageViewer class.
**
** ### About
** Created for: COMP4905A Honours Project\n
** Fall 2016\n
** Bernard Llanos\n
** Supervised by Dr. David Mould\n
** School of Computer Science, Carleton University
**
** ## Primary basis
** [Qt Image Viewer Example](http://doc.qt.io/qt-5/qtwidgets-widgets-imageviewer-example.html)
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

#include <QtWidgets>
#include "imageviewer.h"

/*!
  \brief The proportion of the screen initially occupied by the viewer.
  */
#define IMAGEVIEWER_INITIAL_SCREENSIZE (3.0 / 5.0)

/*!
  \brief The initial magnification of the image
  */
#define IMAGEVIEWER_INITIAL_SCALEFACTOR 1.0

/*!
 * \brief The relative change in scale factor upon zooming in by one increment
 */
#define IMAGEVIEWER_ZOOM_IN_FACTOR 1.25

/*!
 * \brief The relative change in scale factor upon zooming out by one increment
 */
#define IMAGEVIEWER_ZOOM_OUT_FACTOR 0.8

ImageViewer::ImageViewer(void)
    : imagemanager(this, &algManager)
    , algManager(this, &imagemanager)
    , imageLabel(new QLabel)
    , scrollArea(new QScrollArea)
    , scaleFactor(IMAGEVIEWER_INITIAL_SCALEFACTOR)
{
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);

    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    scrollArea->setVisible(false);

    setCentralWidget(scrollArea);

    createActions();

    resize(QGuiApplication::primaryScreen()->availableSize() *
           IMAGEVIEWER_INITIAL_SCREENSIZE);
}

bool ImageViewer::loadFile(const QString &fileName)
{
    return imagemanager.loadFile(fileName);
}

void ImageViewer::setExportSVGActionStatus(bool newStatus) {
    exportSVGAct->setEnabled(newStatus);
}

void ImageViewer::setImage(const QImage &newImage, const QString &message)
{
    image = newImage;
    imageLabel->setPixmap(QPixmap::fromImage(image));
    scaleFactor = IMAGEVIEWER_INITIAL_SCALEFACTOR;

    scrollArea->setVisible(true);
    fitToWindowAct->setEnabled(true);
    updateActions();

    if (!fitToWindowAct->isChecked())
        imageLabel->adjustSize();

    setStatusBarMessage(message);
}

void ImageViewer::setStatusBarMessage(const QString &message)
{
    statusBar()->showMessage(message);
}

#ifndef QT_NO_CLIPBOARD
void ImageViewer::copy()
{
    QGuiApplication::clipboard()->setImage(image);
}
#endif // !QT_NO_CLIPBOARD

void ImageViewer::zoomIn()
{
    scaleImage(IMAGEVIEWER_ZOOM_IN_FACTOR);
}

void ImageViewer::zoomOut()
{
    scaleImage(IMAGEVIEWER_ZOOM_OUT_FACTOR);
}

void ImageViewer::normalSize()
{
    imageLabel->adjustSize();
    scaleFactor = 1.0;
}

void ImageViewer::fitToWindow()
{
    bool fitToWindow = fitToWindowAct->isChecked();
    scrollArea->setWidgetResizable(fitToWindow);
    if (!fitToWindow)
        normalSize();
    updateActions();
}

void ImageViewer::about()
{
    QMessageBox::about(this, tr("About"),
            tr("<p><b>COMP4905A Honours Project</b></p>"
               "<p>Fall 2016</p>"
               "<p>Bernard Llanos</p>"
               "<p>Supervised by Dr. David Mould</p>"
               "<p>School of Computer Science, Carleton University</p>"));
}

void ImageViewer::createActions()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    QAction *openAct = fileMenu->addAction(tr("&Open..."), &imagemanager, &ImageManager::open);
    openAct->setShortcut(QKeySequence::Open);

    saveAsAct = fileMenu->addAction(tr("&Save As Raster Image..."), &imagemanager, &ImageManager::saveAsRasterFile);
    saveAsAct->setShortcut(QKeySequence::Save);
    saveAsAct->setEnabled(false);

    exportSVGAct = fileMenu->addAction(tr("&Export As SVG File..."), &imagemanager, &ImageManager::saveAsSVGFile);
    exportSVGAct->setShortcut(tr("Ctrl+E"));
    exportSVGAct->setEnabled(false);

    fileMenu->addSeparator();

    QAction *exitAct = fileMenu->addAction(tr("&Exit"), this, &QWidget::close);
    exitAct->setShortcut(tr("Ctrl+Q"));

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));

    copyAct = editMenu->addAction(tr("&Copy"), this, &ImageViewer::copy);
    copyAct->setShortcut(QKeySequence::Copy);
    copyAct->setEnabled(false);

    QAction *pasteAct = editMenu->addAction(tr("&Paste"), &imagemanager, &ImageManager::paste);
    pasteAct->setShortcut(QKeySequence::Paste);

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));

    zoomInAct = viewMenu->addAction(
                tr("Zoom &In by %1x").arg(IMAGEVIEWER_ZOOM_IN_FACTOR),
                this, &ImageViewer::zoomIn);
    zoomInAct->setShortcut(QKeySequence::ZoomIn);
    zoomInAct->setEnabled(false);

    zoomOutAct = viewMenu->addAction(
                tr("Zoom &Out by %1x").arg(IMAGEVIEWER_ZOOM_OUT_FACTOR),
                this, &ImageViewer::zoomOut);
    zoomOutAct->setShortcut(QKeySequence::ZoomOut);
    zoomOutAct->setEnabled(false);

    normalSizeAct = viewMenu->addAction(tr("&Actual Size"), this, &ImageViewer::normalSize);
    normalSizeAct->setShortcut(tr("Ctrl+A"));
    normalSizeAct->setEnabled(false);

    viewMenu->addSeparator();

    fitToWindowAct = viewMenu->addAction(tr("&Fit to Window"), this, &ImageViewer::fitToWindow);
    fitToWindowAct->setEnabled(false);
    fitToWindowAct->setCheckable(true);
    fitToWindowAct->setShortcut(tr("Ctrl+F"));

    menuBar()->addMenu(algManager.createAlgorithmsMenu());

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

    helpMenu->addAction(tr("&About"), this, &ImageViewer::about);
    helpMenu->addAction(tr("About &Qt"), &QApplication::aboutQt);
}

void ImageViewer::updateActions()
{
    saveAsAct->setEnabled(!image.isNull());
    copyAct->setEnabled(!image.isNull());
    zoomInAct->setEnabled(!fitToWindowAct->isChecked());
    zoomOutAct->setEnabled(!fitToWindowAct->isChecked());
    normalSizeAct->setEnabled(!fitToWindowAct->isChecked());
}

void ImageViewer::scaleImage(qreal factor)
{
    Q_ASSERT(imageLabel->pixmap());
    scaleFactor *= factor;
    imageLabel->resize(scaleFactor * imageLabel->pixmap()->size());

    adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(scrollArea->verticalScrollBar(), factor);

    // Set reasonable limits on zoom
    zoomInAct->setEnabled(scaleFactor < 3.0);
    zoomOutAct->setEnabled(scaleFactor > 0.333);
}

void ImageViewer::adjustScrollBar(QScrollBar *scrollBar, qreal factor)
{
    scrollBar->setValue(qint32(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}
