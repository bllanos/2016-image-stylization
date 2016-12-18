#ifndef ALGORITHMMANAGER_H
#define ALGORITHMMANAGER_H

/*!
** \file algorithmmanager.h
** \brief Definition of the AlgorithmManager class.
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
**
*/

#include <QObject>
#include <QVector>
#include "algorithmresultpair.h"

class ImageViewer;
class ImageManager;
class Algorithm;
class AlgorithmThread;
class QMenu;
class QAction;

/*!
 * \brief Controller for image processing
 *
 * A class for selecting, running, and aborting image processing algorithms.
 * It manages the "Algorithms" menu of the application.
 */
class AlgorithmManager: public QObject
{
    Q_OBJECT

public:
    /*!
     * \brief Create an algorithm manager
     * \param [in] viewer The image viewer which is the parent of this object,
     * and which will display the algorithms management menu options
     * constructed by this object
     * \param [in] imageManager The image manager which will provide input images
     * for processing and which will receive the output images of image
     * processing algorithms.
     */
    AlgorithmManager(ImageViewer* viewer, ImageManager* imageManager);

    /*!
     * \brief Create a menu for controlling algorithm execution
     * \return The new menu, ownership of which is transferred to the caller
     */
    QMenu *createAlgorithmsMenu(void);

    /*!
     * \brief Enable running image processing algorithms.
     *
     * Activates the appropriate user interface elements. This function is to be
     * called when the first image is loaded.
     *
     * Menu options for running image processing algorithms will not be
     * enabled if an algorithm is currently running.
     */
    void enableAlgorithms(void);

public slots:

    /*!
     * \brief Run the algorithm which converts the image to the CIE L*a*b*
     * colour space, keeping only the lightness channel (Rgb2LabGreyAlgorithm)
     */
    void runGreyscale();

    /*!
     * \brief Run the algorithm which selects midtone lightnesses in the
     * CIE L*a*b* colour space (MidtoneFilter)
     */
    void runMidtoneFilter();

//    /*!
//     * \brief Run the basic stippling algorithm, BasicSPS
//     */
//    void runBasicSPS();

//    /*!
//     * \brief Run the flexible stippling algorithm, FlexibleSPS, but
//     * with default parameters (for testing purposes).
//     *
//     * The output should be the same as for BasicSPS.
//     * \see runBasicSPS()
//     */
//    void runFlexibleSPS();

    /*!
     * \brief Run Simple Linear Iterative Clustering superpixels, SLIC
     */
    void runSLIC();

    /*!
     * \brief Run Simple Linear Iterative Clustering superpixels, SLIC,
     * then filter the superpixels based on size
     */
    void runLocalDataFilter_SIZE();

    /*!
     * \brief Run Simple Linear Iterative Clustering superpixels, SLIC,
     * then filter the superpixels based on lightness standard deviation
     */
    void runLocalDataFilter_STDDEV_LSTAR();

    /*!
     * \brief Run Simple Linear Iterative Clustering superpixels, SLIC,
     * then filter the superpixels using an externally-provided pixel selection map
     */
    void runLocalDataFilter_EXTERNAL();

//    /*!
//     * \brief Run the basic superpixel and stipple overlay rendering algorithm,
//     * OverlayRenderer
//     */
//    void runOverlayRenderer();

//    /*!
//     * \brief Run the blended superpixel and stipple rendering algorithm,
//     * FilteredBlendRenderer
//     */
//    void runFilteredBlendRenderer();

//    /*!
//     * \brief Run the blended superpixel and stipple rendering algorithm,
//     * FilteredBlendRenderer, omitting stippling
//     */
//    void runFilteredBlendRenderer_noStipples();

//    /*!
//     * \brief Run the blended superpixel and stipple rendering algorithm,
//     * FilteredBlendRenderer, omitting superpixel rendering
//     */
//    void runFilteredBlendRenderer_stipplesOnly();

//    /*!
//     * \brief Run the blended superpixel and stipple rendering algorithm,
//     * FilteredBlendRenderer, highlighting borders
//     */
//    void runFilteredBlendRenderer_borders();

//    /*!
//     * \brief Run the blended superpixel and stipple rendering algorithm,
//     * FilteredBlendRenderer, with sharp lightness transitions between
//     * selected and rejected superpixels
//     */
//    void runFilteredBlendRenderer_sharp();

//    /*!
//     * \brief Run the blended superpixel and stipple rendering algorithm,
//     * FilteredBlendRenderer, omitting stippling, with sharp lightness transitions between
//     * selected and rejected superpixels
//     */
//    void runFilteredBlendRenderer_noStipples_sharp();

//    /*!
//     * \brief Run the blended superpixel and stipple rendering algorithm,
//     * FilteredBlendRenderer, omitting superpixel rendering,
//     * with sharp lightness transitions between selected and rejected superpixels
//     */
//    void runFilteredBlendRenderer_stipplesOnly_sharp();

    /*!
     * \brief Stop the currently running algorithm, triggered by user input
     */
    void abort();

    /*!
     * \brief Stop the currently running algorithm, triggered by algorithm failure
     */
    void receiveFail();

    /*!
     * \brief Called when the image processing thread has finished running
     */
    void receiveFinished();

    /*!
     * \brief Display algorithm progress information to the user
     * \param [in] status Progress message
     */
    void receiveStatus(const QString &status);

    /*!
     * \brief Receive the output image from an algorithm
     *
     * This slot is intended to be used to pass data between threads.
     * \param [in] pair Raster and vector image output in a single object
     */
    void receiveOutput(const AlgorithmResultPair &pair);

private:
    /*!
     * \brief Enable or disable menu items for running image processing algorithms
     *
     * This function also updates AlgorithmManager::algorithmIsRunning
     * \param [in] runAlgorithm If `true`, menu options for running algorithms
     * are enabled, and the menu option for aborting a running algorithm is disabled.
     * If `false`, the opposite occurs.
     */
    void toggleActions(bool runAlgorithm);
    void runAlgorithm(Algorithm *algorithm);

//    /*!
//     * \brief Run the blended superpixel and stipple rendering algorithm,
//     * FilteredBlendRenderer
//     * \param [in] stippleGradient The `stippleGradient` parameter of FilteredBlendRenderer()
//     * \param [in] outputType The `outputType` parameter of FilteredBlendRenderer()
//     * \param [in] highlightBorders The `highlightBorders` parameter of FilteredBlendRenderer()
//     */
//    void runFilteredBlendRendererHelper(const bool stippleGradient,
//            const FilteredBlendRenderer::Output outputType,
//            const bool highlightBorders
//        );

private:
    ImageViewer* viewer;
    ImageManager* imManager;
    AlgorithmThread* algThread;

    QVector<QAction*> algorithmActions;
    QAction* abortAction;

    /*!
     * \brief Whether or not an image processing algorithm is running
     */
    bool algorithmIsRunning;
};

#endif // ALGORITHMMANAGER_H
