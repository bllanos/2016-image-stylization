/*!
** \file rgb2labgreyalgorithm.cpp
** \brief Implementation of the Rgb2LabGreyAlgorithm class.
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
*/

#include "rgb2labgreyalgorithm.h"
#include "imagedata.h"

/*!
 * \brief If true, the algorithm will sleep between processing stages.
 *
 * This was used to prove that the system is:
 * - Actually running the algorithm on a separate thread, such that
 *   the user interface remains responsive.
 * - Displaying status updates to the bottom status bar of the window,
 *   one for each processing increment of the algorithm.
 */
#define RGB2LABGREYALGORITHM_WAIT 0
#if RGB2LABGREYALGORITHM_WAIT
#include <QThread>
#endif // RGB2LABGREYALGORITHM_WAIT

Rgb2LabGreyAlgorithm::Rgb2LabGreyAlgorithm() :
    progress(0), l(0), lCopy(0), outputData(0)
{

}

Rgb2LabGreyAlgorithm::~Rgb2LabGreyAlgorithm(void) {
    cleanup();
}

bool Rgb2LabGreyAlgorithm::increment(bool & f, QString & status) {

    switch(progress) {
    case 0: {
        l = input->lStar();
        status = "Converted image to CIE L*a*b* colour space.";
        break;
    }
    case 1: {
        pxind n = input->pixelCount();
        lCopy = new qreal[n];
        for(pxind i = 0; i < n; i += 1) {
            lCopy[i] = l[i];
        }
        status = "Copied the L* colour channel.";
        break;
    }
    case 2: {
        outputData = new ImageData(lCopy, input->width(), input->height());
        status = "Produced image data containing only the L* channel.";
        break;
    }
    case 3: {
        outputData->red();
        status = "Converted the greyscale image data to the RGB colour space.";
        finished = true;
        break;
    }
    default:
        Q_ASSERT(false);
    }

#if RGB2LABGREYALGORITHM_WAIT
    QThread::sleep(1);
#endif // RGB2LABGREYALGORITHM_WAIT

    progress += 1;
    f = finished;
    return !failed;
}

bool Rgb2LabGreyAlgorithm::output(QImage *&image, QByteArray *& svgData) {
    image = 0;
    svgData = 0;
    if(!outputIsEnabled) {
        return false;
    }

    progress = 0;
    finished = false;
    image = new QImage();
    if(!outputData->toImage(*image)) {
        if(image != 0) {
            delete image;
            image = 0;
        }
    }
    delete outputData;
    outputData = 0;
    return image != 0;
}

void Rgb2LabGreyAlgorithm::cleanup(void) {
    if(lCopy != 0) {
        delete lCopy;
        lCopy = 0;
    }
    if(outputData != 0) {
        delete outputData;
        outputData = 0;
    }
    Algorithm::cleanup();
}
