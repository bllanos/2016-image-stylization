/*!
** \file superpixelfilter.cpp
** \brief Implementation of the SuperpixelFilter class.
**
** ### About
** Created for: COMP4905A Honours Project\n
** Fall 2016\n
** Bernard Llanos, ID 100793648\n
** Supervised by Dr. David Mould\n
** School of Computer Science, Carleton University
**
** ## Primary basis
** None
*/

#include "superpixelfilter.h"

SuperpixelFilter::SuperpixelFilter(ISuperpixelGenerator*& generator) :
    superpixelGenerator(generator),
    superpixellation(0),
    selectedSuperpixels(0),
    selectedPixels(0),
    isSuperpixelGenerationFinished(false),
    canDeleteInput(true)
{
    generator = 0;
    superpixelGenerator->disableOutput();
}

SuperpixelFilter::~SuperpixelFilter(void) {
    cleanup();
    if(superpixelGenerator != 0) {
        delete superpixelGenerator;
        superpixelGenerator = 0;
    }
}

void SuperpixelFilter::additionalRequiredImages(QVector<QString> &imageDescriptions) {
    superpixelGenerator->additionalRequiredImages(imageDescriptions);
}

bool SuperpixelFilter::initialize(QVector<ImageData *> *&images) {
    ImageData *temp = (*images)[0];
    failed = !superpixelGenerator->initialize(images); // Sets `images` to null
    canDeleteInput = false;
    if(failed) {
        return false;
    }
    return initialize(temp);
}

bool SuperpixelFilter::initialize(ImageData * &image) {
    isSuperpixelGenerationFinished = false;
    failed = !Algorithm::initialize(image); // Sets `image` to null
    return !failed;
}

bool SuperpixelFilter::increment(bool & f, QString & status) {
    f = isSuperpixelGenerationFinished;

    if(failed) {
        status = QObject::tr("Cannot increment - Processing has failed.");
        return false;
    } else if(finished) {
        status = QObject::tr("Cannot increment - Processing has already finished.");
        f = finished;
        return false;
    } else if(superpixelGenerator == 0) {
        status = QObject::tr("Cannot increment - Superpixel generator is uninitialized or deallocated.");
        return false;
    }

    if(superpixelGenerator->isFinished()) {
        failed = !superpixelGenerator->outputSuperpixellation(superpixellation);
        if(failed) {
            status = QObject::tr("Superpixel retrieval failed.");
        } else {
            // Cleanup superpixel generation data
            /* Actually, I cannot, otherwise it is impossible to re-run
             * this algorithm on a new input image (via initialize() ).
             */
            // delete superpixelGenerator;
            // superpixelGenerator = 0;
            // Initialize superpixel filtering data
            selectedSuperpixels = new bool[superpixellation->nSuperpixels];
            std::fill(selectedSuperpixels, selectedSuperpixels + superpixellation->nSuperpixels, false);
            selectedPixels = new bool[input->pixelCount()];
            std::fill(selectedPixels, selectedPixels + input->pixelCount(), false);
            status = QObject::tr("Initialized superpixel filtering data.");
            isSuperpixelGenerationFinished = true;
        }
    } else {
        bool ignored = false;
        failed = !superpixelGenerator->increment(ignored, status);
    }

    f = isSuperpixelGenerationFinished;
    return !failed;
}

bool SuperpixelFilter::isFinished(void) const {
    return isSuperpixelGenerationFinished;
}

bool SuperpixelFilter::outputFilteredSuperpixellation(FilteredSuperpixellation *& fs) {
    if(failed || !finished || !isSuperpixelGenerationFinished) {
        return false;
    }
    Q_ASSERT(fs == 0);
    fs = new FilteredSuperpixellation(
                input,
                superpixellation,
                selectedSuperpixels,
                selectedPixels
            );
    return true;
}

void SuperpixelFilter::cleanup(void) {
    if(!canDeleteInput) {
        input = 0;
    }
    if(superpixellation != 0) {        
        delete superpixellation;
        superpixellation = 0;
    }
    if(selectedSuperpixels != 0) {
        delete [] selectedSuperpixels;
        selectedSuperpixels = 0;
    }
    if(selectedPixels != 0) {
        delete [] selectedPixels;
        selectedPixels = 0;
    }
    Algorithm::cleanup();
}
