/*!
** \file midtonefilter.cpp
** \brief Implementation of the MidtoneFilter class.
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

#include "midtonefilter.h"
#include <cmath>

/*!
 * \brief The default CIE L*a*b* lightness threshold marking the lower
 * end of the midtone range.
 * \see MidtoneFilter::lowThreshold
 */
#define MIDTONEFILTER_DEFAULT_LOW_THRESHOLD 30.0

/*!
 * \brief The default CIE L*a*b* lightness threshold marking the upper
 * end of the midtone range.
 * \see MidtoneFilter::highThreshold
 */
#define MIDTONEFILTER_DEFAULT_HIGH_THRESHOLD 70.0

/*!
 * \brief The default CIE L*a*b* lightness difference from the lower
 * threshold at which the low thresholding function reaches
 * values of 5% and 95%.
 * \see MidtoneFilter::lowBandwidth
 */
#define MIDTONEFILTER_DEFAULT_LOW_BANDWIDTH 20.0

/*!
 * \brief The default CIE L*a*b* lightness difference from the upper
 * threshold at which the high thresholding function reaches
 * values of 95% and 5%.
 * \see MidtoneFilter::highBandwidth
 */
#define MIDTONEFILTER_DEFAULT_HIGH_BANDWIDTH 20.0

/*!
 * \brief The number of pixels to loop over per increment of processing
 */
#define MIDTONEFILTER_PIXEL_GRANULARITY 10000

MidtoneFilter::MidtoneFilter(void) :
    lStarInput(0),
    lowThreshold(MIDTONEFILTER_DEFAULT_LOW_THRESHOLD),
    highThreshold(MIDTONEFILTER_DEFAULT_HIGH_THRESHOLD),
    lowBandwidth(MIDTONEFILTER_DEFAULT_LOW_BANDWIDTH),
    highBandwidth(MIDTONEFILTER_DEFAULT_HIGH_BANDWIDTH),
    lowFactor(0.0),
    highFactor(0.0),
    lStarThresholded(0),
    minLStar(IMAGEDATA_MAX_LIGHTNESS),
    maxLStar(IMAGEDATA_MIN_LIGHTNESS),
    thresholdedImage(0),
    progress(Progress::START),
    k(0)
{
    lowFactor = -log((1.0 - 0.95) / 0.95) / lowBandwidth;
    highFactor = -log((1.0 - 0.95) / 0.95) / highBandwidth;
}

MidtoneFilter::~MidtoneFilter(void) {
    cleanup();
}

bool MidtoneFilter::increment(bool & f, QString & status) {
    if(failed) {
        status = QObject::tr("Cannot increment - Processing has failed.");
        return false;
    } else if(finished) {
        status = QObject::tr("Cannot increment - Processing has already finished.");
        f = finished;
        return false;
    }

    pxind incEnd = updateKAndProgress();

    switch(progress) {
    case Progress::RGB2LAB: {
        lStarInput = input->lStar();
        status = QObject::tr("Converted the input image to the CIE L*a*b* colour space.");
        break;
    }
    case Progress::THRESHOLD: {
        if(k == 0) {
            lStarThresholded = new qreal[input->pixelCount()];
        }
        thresholdImage(incEnd);
        status = QObject::tr("Thresholding pixels (%1 / %2)")
                .arg(k)
                .arg(input->pixelCount());
        break;
    }
    case Progress::RESCALE: {
        rescaleImage(incEnd);
        status = QObject::tr("Rescaling pixels (%1 / %2)")
                .arg(k)
                .arg(input->pixelCount());
        break;
    }
    case Progress::CREATE_LAB_IMAGE: {
        thresholdedImage = new ImageData(lStarThresholded, input->width(), input->height());
        status = QObject::tr("Created output image data in the CIE L*a*b* colour space.");
        break;
    }
    case Progress::LAB2RGB: {
        thresholdedImage->red();
        status = QObject::tr("Converted the output image data to the RGB colour space.");
        break;
    }
    case Progress::FILL_OUTPUT: {
        outputImage = new QImage;
        thresholdedImage->toImage(*outputImage);
        status = QObject::tr("Converted the output image data to a displayable image.");
        break;
    }
    case Progress::END: {
        status = QObject::tr("Finished.");
        finished = true;
        break;
    }
    default:
        failed = true;
        status = QObject::tr("MidtoneFilter: Unexpected progress information - Corrupted internal state.");
        Q_ASSERT(false);
    }

    f = finished;
    return !failed;
}

bool MidtoneFilter::initialize(ImageData * &image) {
    failed = !Algorithm::initialize(image);
    if(failed) {
        return false;
    }
    minLStar = IMAGEDATA_MAX_LIGHTNESS;
    maxLStar = IMAGEDATA_MIN_LIGHTNESS;
    progress = Progress::START;
    k = 0;
    return true;
}

void MidtoneFilter::cleanup(void) {
    lStarInput = 0; // Owned by Algorithm::input
    if(thresholdedImage == 0) {
        delete [] lStarThresholded;
    } else {
        // `lStarThresholded` is owned by `thresholdedImage`.
        delete thresholdedImage;
        thresholdedImage = 0;
    }
    lStarThresholded = 0;
}

pxind MidtoneFilter::updateKAndProgress(void) {

    pxind loopLimit = getLoopLimit();

    // Update to the next stage or iteration
    if(k == loopLimit) {
        k = 0;

        switch(progress) {
        case Progress::START: {
            progress = Progress::RGB2LAB;
            break;
        }
        case Progress::RGB2LAB: {
            progress = Progress::THRESHOLD;
            break;
        }
        case Progress::THRESHOLD: {
            if(abs(maxLStar - minLStar) > 1.0) {
                progress = Progress::RESCALE;
            } else {
                /* Avoid dividing by small values.
                 * This case would occur for a single-coloured image.
                 */
                progress = Progress::CREATE_LAB_IMAGE;
            }
            break;
        }
        case Progress::RESCALE: {
            progress = Progress::CREATE_LAB_IMAGE;
            break;
        }
        case Progress::CREATE_LAB_IMAGE: {
            if(outputIsEnabled) {
                progress = Progress::LAB2RGB;
            } else {
                progress = Progress::END;
            }
            break;
        }
        case Progress::LAB2RGB: {
            progress = Progress::FILL_OUTPUT;
            break;
        }
        case Progress::FILL_OUTPUT: {
            progress = Progress::END;
            break;
        }
        case Progress::END: {
            break;
        }
        default:
            failed = true;
            Q_ASSERT(false);
        }

        // Update the end of a loop
        loopLimit = getLoopLimit();
    }

    // Set increment size
    pxind inc = 0;
    if(progress == Progress::THRESHOLD || progress == Progress::RESCALE) {
        inc = MIDTONEFILTER_PIXEL_GRANULARITY;
    }

    pxind incEnd = k + inc;

    if(incEnd > loopLimit) {
        incEnd = loopLimit;
    }
    return incEnd;
}

pxind MidtoneFilter::getLoopLimit(void) const {
    if(progress == Progress::THRESHOLD || progress == Progress::RESCALE) {
        return input->pixelCount();
    } else {
        return 0;
    }
}

void MidtoneFilter::thresholdImage(const pxind &endPixel) {
    qreal lowValue = 0.0;
    qreal highValue = 0.0;
    qreal currentValue = 0.0;
    for(; k < endPixel; k += 1) {
        lowValue = sigmoid(
                    IMAGEDATA_MIN_LIGHTNESS,
                    IMAGEDATA_MAX_LIGHTNESS,
                    lowFactor,
                    lowThreshold,
                    true,
                    lStarInput[k]
                );
        highValue = sigmoid(
                    IMAGEDATA_MIN_LIGHTNESS,
                    IMAGEDATA_MAX_LIGHTNESS,
                    highFactor,
                    highThreshold,
                    false,
                    lStarInput[k]
                );
        currentValue = 0.5 * (lowValue + highValue);
        if(currentValue > maxLStar) {
            maxLStar = currentValue;
        }
        if(currentValue < minLStar) {
            minLStar = currentValue;
        }
        lStarThresholded[k] = currentValue;
    }
}

void MidtoneFilter::rescaleImage(const pxind &endPixel) {
    qreal rangeInv = 1.0 / (maxLStar - minLStar);
    for(; k < endPixel; k += 1) {
        lStarThresholded[k] =
                rangeInv * (lStarThresholded[k] -minLStar) * IMAGEDATA_RANGE_LIGHTNESS
                + IMAGEDATA_MIN_LIGHTNESS;
    }
}

qreal MidtoneFilter::sigmoid(
            const qreal& min,
            const qreal& max,
            const qreal& scale,
            const qreal& center,
            const bool& sign,
            const qreal& x
        ) {
    qreal exponent = 0.0;
    if(sign) {
        exponent = -scale * (x - center);
    } else {
        exponent = scale * (x - center);
    }
    qreal range = max - min;
    return min + range / (1.0 + exp(exponent));
}
