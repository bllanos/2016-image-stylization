#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

/*!
** \file imageviewer.h
** \brief Definition of the ImageViewer class.
**
** ### About
** Created for: COMP4905A Honours Project\n
** Fall 2016\n
** Bernard Llanos, ID 100793648\n
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

#include <QMainWindow>
#include <QImage>
#include "imagemanager.h"
#include "algorithmmanager.h"

QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QMenu;
class QScrollArea;
class QScrollBar;
QT_END_NAMESPACE

/*!
 * \brief Image display window with zoom and scroll controls.
 *
 * An image viewer with basic graphical controls for changing the view of the image,
 * as well as graphical controls for loading and exporting image files.
 *
 * In addition, it serves as the parent for the AlgorithmsManager object that
 * processes images, and the ImageManager object that handles image input
 * and output.
 */
class ImageViewer : public QMainWindow
{
    Q_OBJECT

public:
    /*!
     * \brief Open an image viewer window
     */
    ImageViewer(void);

    /*!
     * \brief Set the image to be displayed
     * \param [in] newImage The new image to display
     * \param [in] message The text to display in the status bar
     */
    void setImage(const QImage &newImage, const QString &message);

    /*!
     * \brief Set the status bar text
     * \param [in] message The text to display in the status bar
     */
    void setStatusBarMessage(const QString &message);

    /*!
     * \brief Load an image
     *
     * Load an image file via the ImageManager object associated with this object.
     * \param [in] fileName Image filepath
     * \return `true`, if the file was loaded.
     */
    bool loadFile(const QString & fileName);

    /*!
     * \brief Enable or disable the menu action for exporting an SVG file
     * \param [in] newStatus The new enabled (true) or disabled (false) state
     * of the menu item.
     */
    void setExportSVGActionStatus(bool newStatus);

private slots:
    #ifndef QT_NO_CLIPBOARD
    void copy();
    #endif // !QT_NO_CLIPBOARD
    void zoomIn();
    void zoomOut();
    void normalSize();
    void fitToWindow();
    void about();

private:
    void createActions();
    void createMenus();
    void updateActions();

    void scaleImage(qreal factor);
    void adjustScrollBar(QScrollBar *scrollBar, qreal factor);

private:
    ImageManager imagemanager;
    AlgorithmManager algManager;
    QImage image;
    QLabel *imageLabel;
    QScrollArea *scrollArea;
    qreal scaleFactor;

    QAction *copyAct;
    QAction *saveAsAct;
    QAction *exportSVGAct;
    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *normalSizeAct;
    QAction *fitToWindowAct;
};

#endif // IMAGEVIEWER_H
