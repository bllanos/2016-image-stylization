/*!
** \file localdatafilter.cpp
** \brief Implementation of the LocalDataFilter class.
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
**
** ## References
** - A. Greensted. "Otsu Thresholding." Internet:
**   http://www.labbookpages.co.uk/software/imgProc/otsuThreshold.html ,
**   June 17, 2010 (Accessed Oct. 25, 2016).
*/

#include <math.h>
#include <algorithm>
#include <QObject>
#include <QDoubleValidator>
#include "localdatafilter.h"

/*!
 * \brief The maximum number of bins in the histogram constructed for Otsu
 * thresholding
 *
 * This limit takes precedence over #LOCALDATAFILTER_MIN_HISTOGRAM_BINS_PER_SUPERPIXEL
 * in case of conflict.
 */
#define LOCALDATAFILTER_MAX_HISTOGRAM_BINS 256
/*!
 * \brief The minimum ratio of the number of bins in the histogram constructed for Otsu
 * thresholding to the number of superpixels
 */
#define LOCALDATAFILTER_MIN_HISTOGRAM_BINS_PER_SUPERPIXEL 3
/*!
 * \brief The minimum number of bins in the histogram constructed for Otsu
 * thresholding
 */
#define LOCALDATAFILTER_MIN_HISTOGRAM_BINS 10
/*!
 * \brief The number of superpixels to process in one processing increment.
 */
#define LOCALDATAFILTER_CLUSTER_GRANULARITY 10

/*!
 * \brief The background fill colour for output images
 *
 * Set to yellow for debugging purposes. (All pixels should be coloured over.)
 */
#define LOCALDATAFILTER_DEFAULT_OUTPUT_IMAGE_BACKGROUND qRgb(255, 255, 0)

/*!
 * \brief The border colour for superpixels in the half of the output image
 * showing superpixel scores
 */
#define LOCALDATAFILTER_BORDER_COLOR_STATS qRgb(0, 0, 0)

/*!
 * \brief The border colour for superpixels in the half of the output image
 * showing superpixel selection
 */
#define LOCALDATAFILTER_BORDER_COLOR_CHOICE qRgb(128, 128, 128)

/*!
 * \brief The interior colour of selected superpixels in the half of the output
 * image showing superpixel selection
 */
#define LOCALDATAFILTER_CHOSEN_COLOR qRgb(0, 0, 0)
/*!
 * \brief The interior colour of rejected superpixels in the half of the output
 * image showing superpixel selection
 */
#define LOCALDATAFILTER_REJECTED_COLOR qRgb(255, 255, 255)

/*!
 * \brief The lowest possible standard deviation for the CIE L*a*b* lightness
 * channel values of superpixel pixels
 */
#define LOCALDATAFILTER_MIN_STDDEV_LSTAR 0.0

/*!
 * \brief The highest possible standard deviation for the CIE L*a*b* lightness
 * channel values of superpixel pixels
 */
#define LOCALDATAFILTER_MAX_STDDEV_LSTAR \
    ((IMAGEDATA_MAX_LIGHTNESS - IMAGEDATA_MIN_LIGHTNESS) / 2.0)

LocalDataFilter::LocalDataFilter(ISuperpixelGenerator*& generator, const ScoreBasis &b) :
    SuperpixelFilter(generator),
    basis(b),
    selectionMap(0),
    lStarSelectionMap(0),
    superpixelScores(0),
    maxScore(0.0),
    minScore(0.0),
    inverseBinWidth(0.0),
    histogram(0),
    nHistogramBins(0),
    otsuThreshold(0.0),
    selectBelowThreshold(false),
    outputInRow(false),
    progress(Progress::START),
    k(0),
    collectStatistics(0),
    normalizeStatistics(0)
{
    switch(basis) {
    case ScoreBasis::SIZE:
        collectStatistics = collectSizeStatistics;
        normalizeStatistics = normalizeSizeStatistics;
        selectBelowThreshold = false;
        break;
    case ScoreBasis::STDDEV_LSTAR:
        collectStatistics = collectStddevLStarStatistics;
        normalizeStatistics = normalizeStddevLStarStatistics;
        selectBelowThreshold = true;
        break;
    case ScoreBasis::EXTERNAL:
        collectStatistics = collectExternalStatistics;
        normalizeStatistics = normalizeExternalStatistics;
        selectBelowThreshold = true;
        break;
    default:
        failed = true;
        Q_ASSERT(false);
    }

}

LocalDataFilter::~LocalDataFilter(void) {
    cleanup();
}

void LocalDataFilter::additionalRequiredImages(QVector<QString> &imageDescriptions) {
    SuperpixelFilter::additionalRequiredImages(imageDescriptions);
    if(basis == ScoreBasis::EXTERNAL) {
        imageDescriptions.append(QObject::tr("Open pixel soft selection map"));
    }
}

bool LocalDataFilter::initialize(QVector<ImageData *> *&images) {
    QVector<ImageData *>::size_type size = images->size();
    Q_ASSERT((size == 2 && basis == ScoreBasis::EXTERNAL) ||
             (size == 1 && basis != ScoreBasis::EXTERNAL));
    ImageData * firstImage = (*images)[0];
    ImageData * secondImage = 0;
    if(basis == ScoreBasis::EXTERNAL) {
        secondImage = (*images)[1];
        images->remove(1);
        if(firstImage->width() != secondImage->width() ||
           firstImage->height() != secondImage->height()) {
            failed = true;
            qWarning("Input image and selection map dimensions do not agree.");
        }
    }
    if(!failed) {
        failed = !SuperpixelFilter::initialize(images);
    }
    if(!failed) {
        selectionMap = secondImage;
    } else {
        delete firstImage;
        firstImage = 0;
        if(secondImage != 0) {
            delete secondImage;
            secondImage = 0;
        }
    }
    delete images;
    images = 0;
    return !failed;
}

bool LocalDataFilter::initialize(ImageData * &image) {
    failed = !SuperpixelFilter::initialize(image);
    if(failed) {
        return false;
    }
    QDoubleValidator validator;
    maxScore = validator.bottom();
    minScore = validator.top();
    inverseBinWidth = 0.0;
    nHistogramBins = 0;
    otsuThreshold = 0.0;
    outputInRow = false;
    progress = Progress::START;
    k = 0;
    return true;
}

bool LocalDataFilter::increment(bool & f, QString & status) {
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
    case Progress::GENERATE_SUPERPIXELS: {
        failed = !SuperpixelFilter::increment(f, status);
        break;
    }
    case Progress::RGB2LAB: {
        lStarSelectionMap = selectionMap->lStar();
        status = QObject::tr("Converted the selection map image to the CIE L*a*b* colour space.");
        break;
    }
    case Progress::COLLECT_STATISTICS: {
        if(k == 0) {
            superpixelScores = new qreal[superpixellation->nSuperpixels];
        }
        collectStatistics(*this, incEnd);
        status = QObject::tr("Collecting superpixel statistics (%1 / %2)")
                .arg(k)
                .arg(superpixellation->nSuperpixels);
        break;
    }
    case Progress::NORMALIZE_STATISTICS: {
        normalizeStatistics(*this, incEnd);
        status = QObject::tr("Normalizing superpixel statistics (%1 / %2)")
                .arg(k)
                .arg(superpixellation->nSuperpixels);
        break;
    }
    case Progress::CONSTRUCT_HISTOGRAM: {
        if(k == 0) {
            if((
                    static_cast<qreal>(superpixellation->nSuperpixels) /
                    LOCALDATAFILTER_MIN_HISTOGRAM_BINS_PER_SUPERPIXEL) <
                    LOCALDATAFILTER_MAX_HISTOGRAM_BINS) {
                nHistogramBins = floor(
                            static_cast<qreal>(superpixellation->nSuperpixels) /
                            LOCALDATAFILTER_MIN_HISTOGRAM_BINS_PER_SUPERPIXEL
                        );
                if(nHistogramBins < LOCALDATAFILTER_MIN_HISTOGRAM_BINS) {
                    nHistogramBins = LOCALDATAFILTER_MIN_HISTOGRAM_BINS;
                }
            } else {
                nHistogramBins = LOCALDATAFILTER_MAX_HISTOGRAM_BINS;
            }
            histogram = new pxind[nHistogramBins];
            std::fill(histogram, histogram + nHistogramBins, 0);
            inverseBinWidth = static_cast<qreal>(nHistogramBins - 1) / (maxScore - minScore);
        }
        constructHistogram(incEnd);
        status = QObject::tr("Constructing histogram (%1 / %2)")
                .arg(k)
                .arg(superpixellation->nSuperpixels);
        break;
    }
    case Progress::CHOOSE_OTSU_THRESHOLD: {
        chooseOtsuThreshold();
        status = QObject::tr("Selected Otsu threshold from histogram.");
        break;
    }
    case Progress::FILTER_SUPERPIXELS: {
        filterSuperpixels(incEnd);
        status = QObject::tr("Filtering superpixels (%1 / %2)")
                .arg(k)
                .arg(superpixellation->nSuperpixels);
        break;
    }
    case Progress::INITIALIZE_OUTPUT: {
        failed = !initializeOutput();
        if(failed) {
            status = QObject::tr("Failed to initialize output image.");
        } else {
            status = QObject::tr("Initialized output objects.");
        }
        break;
    }
    case Progress::FILL_OUTPUT: {
        fillOutputImage(incEnd);
        status = QObject::tr("Filling output image (%1 / %2)")
                .arg(k)
                .arg(superpixellation->nSuperpixels);
        break;
    }
    case Progress::FINALIZE_OUTPUT: {
        finalizeOutput();
        status = QObject::tr("Finalized output objects.");
        break;
    }
    case Progress::END: {
        status = QObject::tr("Finished.");
        finished = true;
        break;
    }
    default:
        failed = true;
        status = QObject::tr("LocalDataFilter: Unexpected progress information - Corrupted internal state.");
        Q_ASSERT(false);
    }

    f = finished;
    return !failed;
}

bool LocalDataFilter::isFinished(void) const {
    return finished;
}

pxind LocalDataFilter::updateKAndProgress(void) {
    // Set the end of a loop
    pxind loopLimit = getLoopLimit();

    // Update to the next stage or iteration
    if(k == loopLimit) {
        k = 0;

        switch(progress) {
        case Progress::START: {
            progress = Progress::GENERATE_SUPERPIXELS;
            break;
        }
        case Progress::GENERATE_SUPERPIXELS: {
            if(SuperpixelFilter::isFinished()) {
                if(basis == ScoreBasis::EXTERNAL) {
                    progress = Progress::RGB2LAB;
                } else {
                    progress = Progress::COLLECT_STATISTICS;
                }
            }
            break;
        }
        case Progress::RGB2LAB: {
            progress = Progress::COLLECT_STATISTICS;
            break;
        }
        case Progress::COLLECT_STATISTICS: {
            progress = Progress::NORMALIZE_STATISTICS;
            break;
        }
        case Progress::NORMALIZE_STATISTICS: {
            progress = Progress::CONSTRUCT_HISTOGRAM;
            break;
        }
        case Progress::CONSTRUCT_HISTOGRAM: {
            progress = Progress::CHOOSE_OTSU_THRESHOLD;
            break;
        }
        case Progress::CHOOSE_OTSU_THRESHOLD: {
            progress = Progress::FILTER_SUPERPIXELS;
            break;
        }
        case Progress::FILTER_SUPERPIXELS: {
            if(outputIsEnabled) {
                progress = Progress::INITIALIZE_OUTPUT;
            } else {
                progress = Progress::END;
            }
            break;
        }
        case Progress::INITIALIZE_OUTPUT: {
            progress = Progress::FILL_OUTPUT;
            break;
        }
        case Progress::FILL_OUTPUT: {
            progress = Progress::FINALIZE_OUTPUT;
            break;
        }
        case Progress::FINALIZE_OUTPUT: {
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

    switch(progress) {
    case Progress::START: {
        break;
    }
    case Progress::GENERATE_SUPERPIXELS: {
        break;
    }
    case Progress::RGB2LAB: {
        break;
    }
    case Progress::COLLECT_STATISTICS: {
        inc = LOCALDATAFILTER_CLUSTER_GRANULARITY;
        break;
    }
    case Progress::NORMALIZE_STATISTICS: {
        inc = LOCALDATAFILTER_CLUSTER_GRANULARITY;
        break;
    }
    case Progress::CONSTRUCT_HISTOGRAM: {
        inc = LOCALDATAFILTER_CLUSTER_GRANULARITY;
        break;
    }
    case Progress::CHOOSE_OTSU_THRESHOLD: {
        break;
    }
    case Progress::FILTER_SUPERPIXELS: {
        inc = LOCALDATAFILTER_CLUSTER_GRANULARITY;
        break;
    }
    case Progress::INITIALIZE_OUTPUT: {
        break;
    }
    case Progress::FILL_OUTPUT: {
        inc = LOCALDATAFILTER_CLUSTER_GRANULARITY;
        break;
    }
    case Progress::FINALIZE_OUTPUT: {
        break;
    }
    case Progress::END: {
        break;
    }
    default:
        failed = true;
        Q_ASSERT(false);
    }

    pxind incEnd = k + inc;

    if(incEnd > loopLimit) {
        incEnd = loopLimit;
    }
    return incEnd;
}

pxind LocalDataFilter::getLoopLimit(void) const {
    pxind loopLimit = 0;

    switch(progress) {
    case Progress::START: {
        break;
    }
    case Progress::GENERATE_SUPERPIXELS: {
        break;
    }
    case Progress::RGB2LAB: {
        break;
    }
    case Progress::COLLECT_STATISTICS: {
        loopLimit = superpixellation->nSuperpixels;
        break;
    }
    case Progress::NORMALIZE_STATISTICS: {
        loopLimit = superpixellation->nSuperpixels;
        break;
    }
    case Progress::CONSTRUCT_HISTOGRAM: {
        loopLimit = superpixellation->nSuperpixels;
        break;
    }
    case Progress::CHOOSE_OTSU_THRESHOLD: {
        break;
    }
    case Progress::FILTER_SUPERPIXELS: {
        loopLimit = superpixellation->nSuperpixels;
        break;
    }
    case Progress::INITIALIZE_OUTPUT: {
        break;
    }
    case Progress::FILL_OUTPUT: {
        loopLimit = superpixellation->nSuperpixels;
        break;
    }
    case Progress::FINALIZE_OUTPUT: {
        break;
    }
    case Progress::END: {
        break;
    }
    default:
        Q_ASSERT(false);
    }

    return loopLimit;
}

void LocalDataFilter::constructHistogram(const pxind &endSuperpixel) {
    pxind bin = 0;
    for(; k < endSuperpixel; k += 1) {
        bin = static_cast<pxind>(floor((superpixelScores[k] - minScore) * inverseBinWidth));
        histogram[bin] += 1;
    }
}

/*!
 * \brief Apply Otsu's method to the histogram data to find a threshold
 *
 * ## References
 * - A. Greensted. "Otsu Thresholding." Internet:
 *   http://www.labbookpages.co.uk/software/imgProc/otsuThreshold.html ,
 *   June 17, 2010 (Accessed Oct. 25, 2016).
 *   - I adjusted the code so that the output actually matches their example!
 */
void LocalDataFilter::chooseOtsuThreshold(void) {
    pxind sumAll = 0;
    for(pxind bin = 0; bin < nHistogramBins; bin += 1) {
        sumAll += bin * histogram[bin];
    }
    pxind sumDark = 0;
    pxind weightDark = 0;
    pxind weightLight = 0;

    qreal betweenClassVariance = 0.0;
    qreal maxBetweenClassVariance = 0.0;
    pxind threshold = 0;

    qreal meanDark = 0.0;
    qreal meanLight = 0.0;
    qreal diffMean = 0.0;

    for(pxind bin = 1; bin < nHistogramBins; bin += 1) {
        weightDark += histogram[bin - 1];
        if(weightDark == 0) {
            continue;
        }

        weightLight = superpixellation->nSuperpixels - weightDark;
        if(weightLight == 0) {
            break;
        }

        sumDark += (bin - 1) * histogram[bin - 1];
        meanDark = static_cast<qreal>(sumDark) / static_cast<qreal>(weightDark);
        meanLight = static_cast<qreal>(sumAll - sumDark) / static_cast<qreal>(weightLight);
        diffMean = meanLight - meanDark;

        /* This is actually the between class variance
         * multiplied by the square of the total number of data points.
         * The multiplication is not important for finding the maximum.
         */
        betweenClassVariance = static_cast<qreal>(weightDark * weightLight) *
                diffMean * diffMean;

        if(betweenClassVariance > maxBetweenClassVariance) {
            maxBetweenClassVariance = betweenClassVariance;
            threshold = bin;
        }
    }
    otsuThreshold = (static_cast<qreal>(threshold) / inverseBinWidth) + minScore;
}

void LocalDataFilter::filterSuperpixels(const pxind &endSuperpixel) {
    bool choice = false;
    const pxind* superpixelPx = 0;
    pxind nSuperpixelPx = 0;

    for(; k < endSuperpixel; k += 1) {
        choice = (selectBelowThreshold) ? superpixelScores[k] < otsuThreshold :
                                          superpixelScores[k] >= otsuThreshold;
        selectedSuperpixels[k] = choice;
        superpixellation->superpixels[k]->allPixels(superpixelPx, nSuperpixelPx);
        for(pxind px = 0; px < nSuperpixelPx; px += 1) {
            selectedPixels[superpixelPx[px]] = choice;
        }
    }
}

bool LocalDataFilter::initializeOutput(void) {
    Q_ASSERT(outputIsEnabled);
    if(!outputIsEnabled) {
        return false;
    }

    // Setup raster output objects
    pxind width = input->width();
    pxind height = input->height();
    outputInRow = width <= height;
    if(outputInRow) {
        width *= 2;
    } else {
        height *= 2;
    }
    outputImage = new QImage(QSize(width, height), QImage::Format_ARGB32_Premultiplied);
    outputImage->fill(LOCALDATAFILTER_DEFAULT_OUTPUT_IMAGE_BACKGROUND);

    return !(outputImage->isNull());
}

void LocalDataFilter::fillOutputImage(const pxind &endSuperpixel) {
    qreal stat = 0.0;
    int statGrey = 0;
    QRgb choiceColor = 0;

    const Superpixellation::Superpixel *superpixel = 0;
    pxind x = 0, y = 0;
    const pxind* interiorPx;
    pxind nInteriorPx = 0;
    const pxind* boundaryPx;
    pxind nBoundaryPx = 0;

    pxind width = input->width();
    pxind height = input->height();

    for(; k < endSuperpixel; k += 1) {
        if(selectedSuperpixels[k]) {
            choiceColor = LOCALDATAFILTER_CHOSEN_COLOR;
        } else {
            choiceColor = LOCALDATAFILTER_REJECTED_COLOR;
        }
        superpixel = superpixellation->superpixels[k];
        stat = ((superpixelScores[k] - minScore) *
                static_cast<qreal>(IMAGEDATA_RGB_RANGE)) /
                (maxScore - minScore);
        if(stat > IMAGEDATA_MAX_RGB) {
            statGrey = IMAGEDATA_MAX_RGB;
        } else {
            statGrey = static_cast<int>(floor(stat));
        }

        superpixel->interiorPixels(interiorPx, nInteriorPx);
        for(pxind i = 0; i < nInteriorPx; i += 1) {
            input->kToXY(interiorPx[i], x, y);
            outputImage->setPixel(x, y, qRgb(statGrey, statGrey, statGrey));
            if(outputInRow) {
                outputImage->setPixel(x + width, y, choiceColor);
            } else {
                outputImage->setPixel(x, y + height, choiceColor);
            }
        }
        superpixel->boundaryPixels(boundaryPx, nBoundaryPx);
        for(pxind i = 0; i < nBoundaryPx; i += 1) {
            input->kToXY(boundaryPx[i], x, y);
            outputImage->setPixel(x, y, LOCALDATAFILTER_BORDER_COLOR_STATS);
            if(outputInRow) {
                outputImage->setPixel(x + width, y, LOCALDATAFILTER_BORDER_COLOR_CHOICE);
            } else {
                outputImage->setPixel(x, y + height, LOCALDATAFILTER_BORDER_COLOR_CHOICE);
            }
        }
    }
}

void LocalDataFilter::collectSizeStatistics(LocalDataFilter &alg, const pxind &endSuperpixel) {
    pxind &k = alg.k;
    for(; k < endSuperpixel; k += 1) {
        alg.superpixelScores[k] = alg.superpixellation->superpixels[k]->size();
    }
}

void LocalDataFilter::normalizeSizeStatistics(LocalDataFilter &alg, const pxind &endSuperpixel) {
    pxind &k = alg.k;
    qreal score = 0.0;
    // Assuming that the superpixels completely cover the image
    qreal invNormalizationFactor = alg.superpixellation->nSuperpixels /
            static_cast<qreal>(alg.superpixellation->img->pixelCount());
    for(; k < endSuperpixel; k += 1) {
        score = alg.superpixellation->superpixels[k]->size() * invNormalizationFactor;
        alg.superpixelScores[k] = score;
        if(score > alg.maxScore) {
            alg.maxScore = score;
        }
        // Do not use "else if", because the first score is both the max and the min.
        if(score < alg.minScore) {
            alg.minScore = score;
        }
    }
}

void LocalDataFilter::collectStddevLStarStatistics(LocalDataFilter &alg, const pxind &endSuperpixel) {
    pxind &k = alg.k;
    QVector3D stddev;
    for(; k < endSuperpixel; k += 1) {
        alg.superpixellation->superpixels[k]->standardColorDeviation(stddev);
        alg.superpixelScores[k] = stddev.x();
    }
}

void LocalDataFilter::normalizeStddevLStarStatistics(LocalDataFilter &alg, const pxind &endSuperpixel) {
    pxind &k = alg.k;
    qreal score = 0.0;
    qreal invNormalizationFactor = 1.0 / (LOCALDATAFILTER_MAX_STDDEV_LSTAR - LOCALDATAFILTER_MIN_STDDEV_LSTAR);
    for(; k < endSuperpixel; k += 1) {
        score = (alg.superpixelScores[k] - LOCALDATAFILTER_MIN_STDDEV_LSTAR) * invNormalizationFactor;
        alg.superpixelScores[k] = score;
        if(score > alg.maxScore) {
            alg.maxScore = score;
        }
        // Do not use "else if", because the first score is both the max and the min.
        if(score < alg.minScore) {
            alg.minScore = score;
        }
    }
}

void LocalDataFilter::collectExternalStatistics(LocalDataFilter &alg, const pxind &endSuperpixel) {
    pxind &k = alg.k;
    const pxind* allPx;
    pxind nAllPx = 0;
    qreal sum = 0.0;
    for(; k < endSuperpixel; k += 1) {
        alg.superpixellation->superpixels[k]->allPixels(allPx, nAllPx);
        for(pxind i = 0; i < nAllPx; i += 1) {
            sum += alg.lStarSelectionMap[allPx[i]];
        }
        if(nAllPx > 0) {
            alg.superpixelScores[k] = sum / static_cast<qreal>(nAllPx);
        } else {
            alg.superpixelScores[k] = 0.0;
        }
        sum = 0.0;
    }
}

void LocalDataFilter::normalizeExternalStatistics(LocalDataFilter &alg, const pxind &endSuperpixel) {
    pxind &k = alg.k;
    if(k == 0) {
        alg.maxScore = IMAGEDATA_MAX_LIGHTNESS;
        alg.minScore = IMAGEDATA_MIN_LIGHTNESS;
    }
    k = endSuperpixel;
}

void LocalDataFilter::cleanup(void) {
    if(selectionMap != 0) {
        delete selectionMap;
        selectionMap = 0;
    }
    lStarSelectionMap = 0; // Owned by `selectionMap`
    if(superpixelScores != 0) {
        delete [] superpixelScores;
        superpixelScores = 0;
    }
    if(histogram != 0) {
        delete [] histogram;
        histogram = 0;
    }
    SuperpixelFilter::cleanup();
}
