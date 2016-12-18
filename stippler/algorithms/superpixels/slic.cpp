/*!
** \file slic.cpp
** \brief Implementation of the SLIC class.
**
** ### About
** Created for: COMP4905A Honours Project\n
** Fall 2016\n
** Bernard Llanos\n
** Supervised by Dr. David Mould\n
** School of Computer Science, Carleton University
**
** ## Primary basis
** BasicSPS.cpp
**
** ## References
** - R. Achanta et. al. "SLIC superpixels compared to state-of-the-art superpixel methods."
**   IEEE Transactions on Pattern Analysis and Machine Intelligence, vol. 34,
**   no. 11, pp. 2274-2281, Nov. 2012.
**   - [Authors' website for SLIC](http://ivrg.epfl.ch/research/superpixels)
** - P. Morin. (2014, Feb. 4). Open Data Structures. (Edition 0.1G). [On-line].
**   Available: http://opendatastructures.org/ [Oct. 9, 2016].
**   - Specifically, I consulted the description of counting sort in
**     [section 11.2](http://opendatastructures.org/ods-python/11_2_Counting_Sort_Radix_So.html).
*/

#include <math.h>
#include <algorithm>
#include <QDoubleValidator>
#include "slic.h"

/*!
 * \brief A typedef to shorten the typename for convenience
 */
typedef Superpixellation::Superpixel Superpixel;
/*!
 * \brief A typedef to shorten the typename for convenience
 */
typedef Superpixellation::Center<QVector2D> Center;

/*!
  \brief If true, the output image will be a greyscale image where
  lightness values correspond to superpixel labels

  Conflicts with #SLIC_VISUALIZE_CONNECTED_COMPONENT_LABELS (Both flags cannot
  be set at the same time.)

  Superpixel centers will be marked with the colour #SLIC_DEBUG_CENTER_COLOR
 */
#define SLIC_VISUALIZE_LABELS 0

/*!
  \brief If true, the output image will be a greyscale image where
  lightness values correspond to connected component labels

  Conflicts with #SLIC_VISUALIZE_LABELS (Both flags cannot
  be set at the same time.)

  Superpixel centers will be marked with the colour #SLIC_DEBUG_CENTER_COLOR
 */
#define SLIC_VISUALIZE_CONNECTED_COMPONENT_LABELS 0

/*!
  \brief A flag determining whether the connected components of clusters
  are identified and reassigned such that all clusters consist of single
  connected components
*/
#define SLIC_ENABLE_POSTPROCESSING 1

/*!
  \brief The colour to use for debugging visualization of cluster centers
 */
#define SLIC_DEBUG_CENTER_COLOR qRgb(255, 0, 0)

/*!
  \brief The default value of the 'k' parameter
  \see SLIC::kParam
 */
#define SLIC_DEFAULT_K 500

/*!
  \brief The default value of the 'm' parameter
  \see SLIC::m
 */
#define SLIC_DEFAULT_M 10

/*!
  \brief The minimum dimensions of the search window around a cluster center,
  specified as a multiple of SLIC::S
  \see SLIC::searchHalfWidth
  \see SLIC::searchHalfHeight
 */
#define SLIC_MIN_SEARCH_WINDOW_SIZE 2

/*!
  \brief One of the stopping criteria for K-Means iteration
  \see SLIC::updateKAndProgress()
 */
#define SLIC_ERROR_THRESHOLD 0.05

/*!
  \brief One of the stopping criteria for K-Means iteration
  \see SLIC::updateKAndProgress()
 */
#define SLIC_MAX_KMEANS_ITERATIONS 15

/*!
  \brief The number of pixels to loop over per increment of processing
 */
#define SLIC_PIXEL_GRANULARITY 1000

/*!
  \brief The number of clusters/superpixels to loop over per increment of processing
 */
#define SLIC_CLUSTER_GRANULARITY 10

/*!
 * \brief The border colour for SLIC regions
 */
#define SLIC_BORDER_COLOR qRgb(0, 0, 0)

/*!
 * \brief The background fill colour for output images
 *
 * Set to yellow for debugging purposes. (All pixels should be coloured over.)
 */
#define SLIC_DEFAULT_OUTPUT_IMAGE_BACKGROUND qRgb(255, 255, 0)

SLIC::SLIC() :
    kParam(SLIC_DEFAULT_K),
    m(SLIC_DEFAULT_M),
    mSquared(0.0),
    S(0),
    sSquared(0.0),
    searchHalfWidth(0),
    searchHalfHeight(0),
    lStarOrigin(0),
    aStarOrigin(0),
    bStarOrigin(0),
    previousCenters(0),
    currentCenters(0),
    clusterSearchWindow(0),
    previousResidualError(0.0),
    residualError(0.0),
    distancesToCenters(0),
    clusterLabels(0),
    nPixelsPerCluster(0),
    connectedComponentLabels(0),
    nConnectedComponents(0),
#if SLIC_SELECT_LARGEST_COMPONENTS
    connectedComponentHeap(0),
#endif //SLIC_SELECT_LARGEST_COMPONENTS
    connectedComponentClassifications(0),
    visited(0),
    unvisitedPixels(0),
    lastVisitedPixel(0),
    visitedPx(0),
    pixelSortingOffsets(0),
    sortedPixels(0),
    superpixels(0),
    progress(Progress::START),
    k(0),
    iterationCount(0)
{

}

SLIC::~SLIC() {
    cleanup();
}

bool SLIC::initialize(ImageData * &image) {
    failed = !Algorithm::initialize(image);
    if(failed) {
        return false;
    }
    mSquared = m * m;
    S = static_cast<pxind>(round(sqrt(static_cast<qreal>(input->pixelCount()) / static_cast<qreal>(kParam))));
    sSquared = S * S;
    searchHalfWidth = 0; // To be updated by initializeCenters()
    searchHalfHeight = 0; // To be updated by initializeCenters()
    previousCenters = new Center[kParam];
    currentCenters = new Center[kParam];
    clusterSearchWindow = 0; // To be updated by initializeCenters()
    previousResidualError = 0.0;
    residualError = 0.0;
    distancesToCenters = new qreal[input->pixelCount()];
    clusterLabels = new pxind[input->pixelCount()];
    nPixelsPerCluster = new pxind[kParam];
    connectedComponentLabels = new pxind[input->pixelCount()];
    nConnectedComponents = 0;
#if SLIC_SELECT_LARGEST_COMPONENTS
    connectedComponentHeap = new ods::BinaryHeap<SizeLabelsPair, pxind>();
#endif //SLIC_SELECT_LARGEST_COMPONENTS
    visited = new bool[input->pixelCount()];
    unvisitedPixels = new QLinkedList<pxind>();
    lastVisitedPixel = 0;
    visitedPx = new pxind[input->pixelCount()];
    pixelSortingOffsets = new pxind[kParam];
    sortedPixels = new pxind[input->pixelCount()];
    progress = Progress::START;
    k = 0;
    iterationCount = 0;
    return true;
}

bool SLIC::increment(bool & f, QString & status) {
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
        lStarOrigin = input->lStar();
        aStarOrigin = input->aStar();
        bStarOrigin = input->bStar();
        status = QObject::tr("Converted the input image to the CIE L*a*b* colour space.");
        break;
    }
    case Progress::SEED_CENTERS: {
        initializeCenters(incEnd);
        status = QObject::tr("Initializing cluster centers (%1 / %2)")
                .arg(k)
                .arg(kParam);
        break;
    }
    case Progress::K_MEANS_LABEL_PIXELS: {
        if(k == 0) {
            QDoubleValidator validator;
            std::fill(distancesToCenters, distancesToCenters + input->pixelCount(), validator.top());
            std::fill(clusterLabels, clusterLabels + input->pixelCount(), SUPERPIXELLATION_NONE_LABEL);
        }
        kmeansLabelPixels(incEnd);
        status = QObject::tr("K-means iteration %1, labelling pixels (%2 / %3)")
                .arg(iterationCount)
                .arg(k)
                .arg(kParam);
        break;
    }
    case Progress::K_MEANS_UPDATE_CENTERS: {
        if(k == 0) {
            std::fill(nPixelsPerCluster, nPixelsPerCluster + kParam, 0);
            Center blankCenter;
            std::fill(currentCenters, currentCenters+kParam, blankCenter);
        }
        kmeansUpdateCenters(incEnd);
        status = QObject::tr("K-means iteration %1, recomputing cluster centers (%2 / %3)")
                .arg(iterationCount)
                .arg(k)
                .arg(input->pixelCount());
        break;
    }
    case Progress::K_MEANS_ASSESS_ITERATION: {
        if( iterationCount > 0 ) {
            kmeansResidualError(incEnd);
            status = QObject::tr("K-means iteration %1, calculating residual error (%2 / %3)")
                    .arg(iterationCount)
                    .arg(k)
                    .arg(kParam);
        } else {
            kmeansResidualError(incEnd, true);
            status = QObject::tr("K-means iteration %1, normalizing cluster centers (%2 / %3)")
                    .arg(iterationCount)
                    .arg(k)
                    .arg(kParam);
        }
        break;
    }
    case Progress::FIND_CONNECTED_COMPONENTS: {
        if(k == 0) {
            std::fill(visited, visited + input->pixelCount(), false);
            std::fill(connectedComponentLabels, connectedComponentLabels + input->pixelCount(), SUPERPIXELLATION_NONE_LABEL);
        }
        labelConnectedComponents(incEnd);
        status = QObject::tr("Finding connected components (%1 / %2)")
                .arg(k)
                .arg(input->pixelCount());
        break;
    }
    case Progress::CLASSIFY_CONNECTED_COMPONENTS: {
        if(k == 0) {
            connectedComponentClassifications = new bool[nConnectedComponents];
            std::fill(
                    connectedComponentClassifications,
                    connectedComponentClassifications + nConnectedComponents,
                    false
                );
        }
        classifyConnectedComponents(incEnd);
        status = QObject::tr("Finding cluster centers in connected components (%1 / %2)")
                .arg(k)
                .arg(kParam);
        break;
    }
    case Progress::REASSIGN_CONNECTED_COMPONENTS: {
        if(k == 0) {
            std::fill(visited, visited + input->pixelCount(), false);
        }
        reassignConnectedComponents(incEnd);
        status = QObject::tr("Reassigning connected components (%1 / %2)")
                .arg(k)
                .arg(input->pixelCount());
        break;
    }
    case Progress::SORT_PIXELS_AS_SUPERPIXELS: {
        if(k == (input->pixelCount() - 1)) {
            pixelSortingOffsets[0] = nPixelsPerCluster[0];
            for(pxind i = 1; i < kParam; i += 1) {
                pixelSortingOffsets[i] = nPixelsPerCluster[i] + pixelSortingOffsets[i - 1];
            }
        }
        sortPixelsIntoSuperpixels(incEnd);
        status = QObject::tr("Sorting pixels into superpixels (%1 / %2)")
                .arg(k)
                .arg(input->pixelCount());
        break;
    }
    case Progress::CREATE_SUPERPIXEL_OBJECTS: {
        if(k == 0) {
            superpixels = new Superpixel*[kParam];
        }
        createSuperpixels(incEnd);
        status = QObject::tr("Creating and measuring superpixels (%1 / %2)")
                .arg(k)
                .arg(kParam);
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
                .arg(kParam);
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
        status = QObject::tr("Unexpected progress information - Corrupted internal state.");
        Q_ASSERT(false);
    }

    f = finished;
    return !failed;
}

bool SLIC::outputSuperpixellation(Superpixellation *& superpixellation) {
    if(failed || !finished) {
        return false;
    }
    Q_ASSERT(superpixellation == 0);
    superpixellation = new Superpixellation(input, clusterLabels, superpixels, kParam);
    return true;
}

pxind SLIC::updateKAndProgress(void) {

    // Set the end of a loop
    pxind loopLimit = getLoopLimit();

    // Update to the next stage or iteration
    if( (k == loopLimit && progress != Progress::SORT_PIXELS_AS_SUPERPIXELS) ||
           (progress == Progress::SORT_PIXELS_AS_SUPERPIXELS && k < loopLimit) ) {
        k = 0;

        switch(progress) {
        case Progress::START: {
            progress = Progress::RGB2LAB;
            break;
        }
        case Progress::RGB2LAB: {
            progress = Progress::SEED_CENTERS;
            break;
        }
        case Progress::SEED_CENTERS: {
            progress = Progress::K_MEANS_LABEL_PIXELS;
            break;
        }
        case Progress::K_MEANS_LABEL_PIXELS: {
            progress = Progress::K_MEANS_UPDATE_CENTERS;
            break;
        }
        case Progress::K_MEANS_UPDATE_CENTERS: {
            progress = Progress::K_MEANS_ASSESS_ITERATION;
            break;
        }
        case Progress::K_MEANS_ASSESS_ITERATION: {
            qreal errorChange = abs((sqrt(residualError) - sqrt(previousResidualError))/ sqrt(previousResidualError));
            if(
                (iterationCount == (SLIC_MAX_KMEANS_ITERATIONS - 1)) || (
                    (iterationCount > 1) && (errorChange <= SLIC_ERROR_THRESHOLD)
                    )
                ) {
#if SLIC_ENABLE_POSTPROCESSING
                progress = Progress::FIND_CONNECTED_COMPONENTS;
#else
                k = input->pixelCount() - 1;
                progress = Progress::SORT_PIXELS_AS_SUPERPIXELS;
#endif //SLIC_ENABLE_POSTPROCESSING
            } else {
                progress = Progress::K_MEANS_LABEL_PIXELS;
                iterationCount += 1;
                std::copy(currentCenters, currentCenters + kParam, previousCenters);
                previousResidualError = residualError;
                residualError = 0.0;
            }
            break;
        }
        case Progress::FIND_CONNECTED_COMPONENTS: {
            progress = Progress::CLASSIFY_CONNECTED_COMPONENTS;
            break;
        }
        case Progress::CLASSIFY_CONNECTED_COMPONENTS: {
            progress = Progress::REASSIGN_CONNECTED_COMPONENTS;
            break;
        }
        case Progress::REASSIGN_CONNECTED_COMPONENTS: {
            k = input->pixelCount() - 1;
            progress = Progress::SORT_PIXELS_AS_SUPERPIXELS;
            break;
        }
        case Progress::SORT_PIXELS_AS_SUPERPIXELS: {
            progress = Progress::CREATE_SUPERPIXEL_OBJECTS;
            break;
        }
        case Progress::CREATE_SUPERPIXEL_OBJECTS: {
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

    // Set increment size and direction
    pxind inc = 0;

    switch(progress) {
    case Progress::START: {
        break;
    }
    case Progress::RGB2LAB: {
        break;
    }
    case Progress::SEED_CENTERS: {
        inc = SLIC_CLUSTER_GRANULARITY;
        break;
    }
    case Progress::K_MEANS_LABEL_PIXELS: {
        inc = SLIC_CLUSTER_GRANULARITY;
        break;
    }
    case Progress::K_MEANS_UPDATE_CENTERS: {
        inc = SLIC_PIXEL_GRANULARITY;
        break;
    }
    case Progress::K_MEANS_ASSESS_ITERATION: {
        inc = SLIC_CLUSTER_GRANULARITY;
        break;
    }
    case Progress::FIND_CONNECTED_COMPONENTS: {
        inc = SLIC_PIXEL_GRANULARITY;
        break;
    }
    case Progress::CLASSIFY_CONNECTED_COMPONENTS: {
        inc = SLIC_CLUSTER_GRANULARITY;
        break;
    }
    case Progress::REASSIGN_CONNECTED_COMPONENTS: {
        inc = SLIC_PIXEL_GRANULARITY;
        break;
    }
    case Progress::SORT_PIXELS_AS_SUPERPIXELS: {
        inc = -(SLIC_PIXEL_GRANULARITY);
        break;
    }
    case Progress::CREATE_SUPERPIXEL_OBJECTS: {
        inc = SLIC_CLUSTER_GRANULARITY;
        break;
    }
    case Progress::INITIALIZE_OUTPUT: {
        break;
    }
    case Progress::FILL_OUTPUT: {
        inc = SLIC_CLUSTER_GRANULARITY;
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

    if(incEnd > loopLimit && progress != Progress::SORT_PIXELS_AS_SUPERPIXELS) {
        incEnd = loopLimit;
    } else if(progress == Progress::SORT_PIXELS_AS_SUPERPIXELS && incEnd < loopLimit) {
        incEnd = loopLimit;
    }
    return incEnd;
}

pxind SLIC::getLoopLimit(void) const {
    pxind loopLimit = 0;

    switch(progress) {
    case Progress::START: {
        break;
    }
    case Progress::RGB2LAB: {
        break;
    }
    case Progress::SEED_CENTERS: {
        loopLimit = kParam;
        break;
    }
    case Progress::K_MEANS_LABEL_PIXELS: {
        loopLimit = kParam;
        break;
    }
    case Progress::K_MEANS_UPDATE_CENTERS: {
        loopLimit = input->pixelCount();
        break;
    }
    case Progress::K_MEANS_ASSESS_ITERATION: {
        loopLimit = kParam;
        break;
    }
    case Progress::FIND_CONNECTED_COMPONENTS: {
        loopLimit = input->pixelCount();
        break;
    }
    case Progress::CLASSIFY_CONNECTED_COMPONENTS: {
        loopLimit = kParam;
        break;
    }
    case Progress::REASSIGN_CONNECTED_COMPONENTS: {
        loopLimit = input->pixelCount();
        break;
    }
    case Progress::SORT_PIXELS_AS_SUPERPIXELS: {
        break;
    }
    case Progress::CREATE_SUPERPIXEL_OBJECTS: {
        loopLimit = kParam;
        break;
    }
    case Progress::INITIALIZE_OUTPUT: {
        break;
    }
    case Progress::FILL_OUTPUT: {
        loopLimit = kParam;
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

void SLIC::initializeCenters(const pxind &endCluster) {
    pxind sampleX = 0;
    pxind sampleY = 0;
    pxind sampleK = 0;

    /* Find the smallest grid with square dimensions of `S` containing
     * the input image.
     */
    pxind widthInS = ceil(static_cast<qreal>(input->width()) / static_cast<qreal>(S));
    pxind width = widthInS * S;
    qreal widthConversion = S * static_cast<qreal>(input->width()) / static_cast<qreal>(width);
    pxind sDiv2Width = static_cast<pxind>(widthConversion) / 2;
    pxind heightInS = ceil(static_cast<qreal>(input->height()) / static_cast<qreal>(S));
    pxind height = heightInS * S;
    qreal heightConversion = S * static_cast<qreal>(input->height()) / static_cast<qreal>(height);
    pxind sDiv2Height = static_cast<pxind>(heightConversion) / 2;
    pxind lengthInSSquares = widthInS * heightInS;
    qreal kConversion = static_cast<qreal>(lengthInSSquares) / static_cast<qreal>(kParam);

    /* `kConversion` is the ratio of the number of squares in the initialization
     * grid to the number of cluster centers.
     * For each square containing a cluster center, there is at most
     * `ceil(kConversion) - 1` squares that follow it which do not contain cluster
     * centers. Consequently, for all pixels to be labelled, the search window
     * around a cluster center must extend `ceil(kConversion) - 0.5` squares
     * in the four directions from a cluster center.
     *
     * The cluster search window half width is `ceil(kConversion) - 0.5` times
     * `widthConversion`, plus 2 pixels to account for the shifting of cluster
     * centers to lowest gradient positions, plus 2 pixels to account for the rounding
     * of cluster center positions.
     *
     * A similar calculation is applied to determine the cluster search window
     * half height.
     */
    if(clusterSearchWindow == 0) {
        searchHalfWidth = static_cast<pxind>(ceil((ceil(kConversion) - 0.5) * widthConversion)) + 4;
        searchHalfHeight = static_cast<pxind>(ceil((ceil(kConversion) - 0.5) * heightConversion)) + 4;
        /* However, for smaller values of SLIC::m, I will get visible clipping
         * of clusters to the search window, so I prefer to be more generous.
         */
        if(searchHalfWidth < (SLIC_MIN_SEARCH_WINDOW_SIZE * S)) {
            searchHalfWidth = (SLIC_MIN_SEARCH_WINDOW_SIZE * S);
        }
        if(searchHalfHeight < (SLIC_MIN_SEARCH_WINDOW_SIZE * S)) {
            searchHalfHeight = (SLIC_MIN_SEARCH_WINDOW_SIZE * S);
        }
        clusterSearchWindow = new pxind[((2 * searchHalfWidth) + 1) * ((2 * searchHalfHeight) + 1)];
    }

    pxind adjustedK = 0;
    pxind neighbours[8] = {0};
    pxind nNeighbours = 0;
    qreal minSobelMagnitude = 0.0;
    QVector2D sobelVector;
    for(;k < endCluster; k += 1) {
        /* Sample in a grid with side length S.
         * Pick grid square centers by adding `sDiv2`.
         */
        adjustedK = static_cast<pxind>(round(static_cast<qreal>(k) * kConversion));
        sampleX = adjustedK % widthInS;
        sampleX = floor(sampleX * widthConversion) + sDiv2Width;
        sampleY = adjustedK / widthInS;
        sampleY = floor(sampleY * heightConversion) + sDiv2Height;
        sampleK = input->xyToK(sampleX, sampleY);

        /* Pick the neighbouring pixel with the lowest gradient
         */
        input->sobelLabAt(sampleK, sobelVector);
        minSobelMagnitude = sobelVector.lengthSquared();
        input->eightNeighbours(neighbours, nNeighbours, sampleK);
        for(pxind i = 0; i < nNeighbours; i += 1) {
            input->sobelLabAt(neighbours[i], sobelVector);
            if(sobelVector.lengthSquared() < minSobelMagnitude) {
                minSobelMagnitude = sobelVector.lengthSquared() ;
                sampleK = neighbours[i];
            }
        }

        // Store the result
        input->kToXY(sampleK, sampleX, sampleY);
        currentCenters[k].position.setX(sampleX);
        currentCenters[k].position.setY(sampleY);
        currentCenters[k].color.setX(lStarOrigin[sampleK]);
        currentCenters[k].color.setY(aStarOrigin[sampleK]);
        currentCenters[k].color.setZ(bStarOrigin[sampleK]);
    }
}

void SLIC::kmeansLabelPixels(const pxind &endCluster) {
    pxind searchWindowSize = 0;
    qreal distance = 0.0;
    pxind ki = 0;
    for(;k < endCluster; k += 1) {
        input->neighbours(
                    clusterSearchWindow,
                    searchWindowSize,
                    floor(currentCenters[k].position.x()),
                    floor(currentCenters[k].position.y()),
                    searchHalfWidth,
                    searchHalfHeight
                );
        for(pxind i = 0; i < searchWindowSize; i += 1) {
            ki = clusterSearchWindow[i];
            distance = distanceToCenter(ki, currentCenters[k]);
            if(distance < distancesToCenters[ki]) {
                distancesToCenters[ki] = distance;
                clusterLabels[ki] = k;
            }
        }
    }
}

void SLIC::kmeansUpdateCenters(const pxind &endPx) {
    pxind label = SUPERPIXELLATION_NONE_LABEL;
    pxind x = 0, y = 0;
    for(; k < endPx; k += 1) {
        label = clusterLabels[k];
        Q_ASSERT(label != SUPERPIXELLATION_NONE_LABEL);
        input->kToXY(k, x, y);
        currentCenters[label].color += QVector3D(lStarOrigin[k], aStarOrigin[k], bStarOrigin[k]);
        currentCenters[label].position += QVector2D(x, y);
        nPixelsPerCluster[label] += 1;
    }
}

void SLIC::kmeansResidualError(const pxind &endCluster, const bool& firstIteration) {
    if(firstIteration) {
        for(;k < endCluster; k += 1) {
            Center& currCenter = currentCenters[k];
            currCenter.position /= static_cast<qreal>(nPixelsPerCluster[k]);
            currCenter.color /= static_cast<qreal>(nPixelsPerCluster[k]);
        }
    } else {
        for(;k < endCluster; k += 1) {
            const QVector2D& prevPosition = previousCenters[k].position;
            QVector2D& currPosition = currentCenters[k].position;
            // Normalize center first
            currPosition /= static_cast<qreal>(nPixelsPerCluster[k]);
            currentCenters[k].color /= static_cast<qreal>(nPixelsPerCluster[k]);

            /* Note that errors are computed as spatial distances only,
             * not combined spatial and colour distances.
             */
            residualError += (currPosition - prevPosition).lengthSquared();
        }
    }
}

void SLIC::labelConnectedComponents(const pxind &endPx) {
    pxind px = SUPERPIXELLATION_NONE_LABEL;
    pxind pxNeighbour = 0;
    pxind neighbours[4] = {0};
    pxind nNeighbours = 0;
    pxind clusterLabel = SUPERPIXELLATION_NONE_LABEL;
    pxind componentLabel = SUPERPIXELLATION_NONE_LABEL;
    for(; k < endPx; k += 1) {

        // Get the next pixel to process
        if(!unvisitedPixels->isEmpty()) {
            do {
                px = unvisitedPixels->first();
                unvisitedPixels->removeFirst();
            } while(visited[px] && !unvisitedPixels->isEmpty());
        }
        if(px == SUPERPIXELLATION_NONE_LABEL || visited[px]) {
            // Start a new connected component
            componentLabel = nConnectedComponents;
            nConnectedComponents += 1;
            pxind n = input->pixelCount();
            for(; lastVisitedPixel < n; lastVisitedPixel += 1) {
                if(!visited[lastVisitedPixel]) {
                    px = lastVisitedPixel;
                    break;
                }
            }
#if SLIC_SELECT_LARGEST_COMPONENTS
            connectedComponentHeap->add(SizeLabelsPair(clusterLabels[px], componentLabel));
#endif //SLIC_SELECT_LARGEST_COMPONENTS
        }

        /* Add its neighbours to the queue of pixels to visit, if they are
         * in the same connected component
         */
        clusterLabel = clusterLabels[px];
        input->fourNeighbours(neighbours, nNeighbours, px);
        for(pxind j = 0; j < nNeighbours; j += 1) {
            pxNeighbour = neighbours[j];
            if(clusterLabel == clusterLabels[pxNeighbour]) {
                if(visited[pxNeighbour]) {
                    componentLabel = connectedComponentLabels[pxNeighbour];
                } else {
                    unvisitedPixels->append(pxNeighbour);
                }
            }
        }

        // Visit the pixel
        visited[px] = true;
        Q_ASSERT(componentLabel != SUPERPIXELLATION_NONE_LABEL);
        connectedComponentLabels[px] = componentLabel;
#if SLIC_SELECT_LARGEST_COMPONENTS
        (*connectedComponentHeap)[componentLabel].size += 1;
        connectedComponentHeap->increase(componentLabel);
#endif //SLIC_SELECT_LARGEST_COMPONENTS
    }
}

void SLIC::classifyConnectedComponents(const pxind &endCluster) {
#if SLIC_SELECT_LARGEST_COMPONENTS
    SizeLabelsPair item;
#else
    QPoint centerIntegerPosition;
    pxind px = 0;
#endif //SLIC_SELECT_LARGEST_COMPONENTS

    for(;k < endCluster; k += 1) {
#if SLIC_SELECT_LARGEST_COMPONENTS
        while(item.cluster < k && connectedComponentHeap->size() > 0) {
            item = connectedComponentHeap->remove();
        }
        if(item.cluster == k) {
            connectedComponentClassifications[item.component] = true;
        }
#else
        centerIntegerPosition = currentCenters[k].position.toPoint();
        px = input->xyToK(centerIntegerPosition.x(), centerIntegerPosition.y());
        /* Don't blindly assume that the cluster center is over a connected
         * component of pixels associated with it.
         */
        if(clusterLabels[px] == k) {
            connectedComponentClassifications[connectedComponentLabels[px]] = true;
        }
#endif //SLIC_SELECT_LARGEST_COMPONENTS
    }
}

void SLIC::reassignConnectedComponents(const pxind &endPx) {
    pxind px = SUPERPIXELLATION_NONE_LABEL;
    pxind pxNeighbour = 0;
    pxind neighbours[4] = {0};
    pxind nNeighbours = 0;
    pxind nVisited = SUPERPIXELLATION_NONE_LABEL;
    bool done = false;
    for(; k < endPx; k += 1) {
        /* Process pixels not in the same connected components
         * as their cluster centers
         */
        if(!connectedComponentClassifications[connectedComponentLabels[k]]) {

            /* Initialize breadth-first search for nearest connected component
             * containing a cluster center.
             */
            unvisitedPixels->clear();
            unvisitedPixels->append(k);
            done = false;

            while(!done) {
                if(!unvisitedPixels->isEmpty()) {
                    do {
                        px = unvisitedPixels->first();
                        unvisitedPixels->removeFirst();
                    } while(visited[px] && !unvisitedPixels->isEmpty());
                }

                // Add its neighbours to the queue of pixels to visit
                input->fourNeighbours(neighbours, nNeighbours, px);
                for(pxind j = 0; j < nNeighbours; j += 1) {
                    pxNeighbour = neighbours[j];
                    if(!visited[pxNeighbour]) {
                        if(connectedComponentClassifications[connectedComponentLabels[pxNeighbour]]) {
                            nPixelsPerCluster[clusterLabels[k]] -= 1;
                            clusterLabels[k] = clusterLabels[pxNeighbour];
                            nPixelsPerCluster[clusterLabels[k]] += 1;
                            done = true;
                            break;
                        } else {
                            unvisitedPixels->append(pxNeighbour);
                        }
                    }
                }

                // Visit the pixel
                visited[px] = true;
                nVisited += 1;
                visitedPx[nVisited] = px;
            }

            // Cleanup for the next iteration
            for(; nVisited >= 0;nVisited -= 1) {
                visited[visitedPx[nVisited]] = false;
            }
        }
    }
}

void SLIC::sortPixelsIntoSuperpixels(const pxind &endPx) {
    pxind label = SUPERPIXELLATION_NONE_LABEL;
    for(; k >= endPx; k -= 1) {
        label = clusterLabels[k];
        Q_ASSERT(label != SUPERPIXELLATION_NONE_LABEL);
        pixelSortingOffsets[label] -= 1;
        sortedPixels[pixelSortingOffsets[label]] = k;
    }
}

void SLIC::createSuperpixels(const pxind &endCluster) {
    pxind *superpixelPx = 0;
    pxind nPx = 0;
    pxind startPx = 0;
    pxind endPx = 0;
    for(;k < endCluster; k += 1) {
        nPx = nPixelsPerCluster[k];
        startPx = pixelSortingOffsets[k];
        endPx = startPx + nPx;
        superpixelPx = new pxind[nPx];
        std::copy(sortedPixels + startPx, sortedPixels + endPx, superpixelPx);
        superpixels[k] = new Superpixel(k, superpixelPx, nPx, clusterLabels, *input);
    }
}

void SLIC::fillOutputImage(const pxind &endCluster) {
#if SLIC_VISUALIZE_LABELS || SLIC_VISUALIZE_CONNECTED_COMPONENT_LABELS
    qreal label = SUPERPIXELLATION_NONE_LABEL;
    int labelGrey = 0;
    QPoint centerPosition;
    pxind neighbours[4] = {0};
    pxind nNeighbours = 0;
#endif //SLIC_VISUALIZE_LABELS || SLIC_VISUALIZE_CONNECTED_COMPONENT_LABELS
    QRgb pixelColor = 0;
    Superpixel *superpixel = 0;
    pxind x = 0, y = 0;
    const pxind* interiorPx;
    pxind nInteriorPx = 0;
    const pxind* boundaryPx;
    pxind nBoundaryPx = 0;

    for(; k < endCluster; k += 1) {
        superpixel = superpixels[k];
#if SLIC_VISUALIZE_LABELS && !SLIC_VISUALIZE_CONNECTED_COMPONENT_LABELS
        label = (static_cast<qreal>(superpixel->label()) * static_cast<qreal>(IMAGEDATA_RGB_RANGE)) / static_cast<qreal>(kParam);
        if(label > IMAGEDATA_MAX_RGB) {
            labelGrey = IMAGEDATA_MAX_RGB;
        } else {
            labelGrey = static_cast<int>(floor(label));
        }
        pixelColor = qRgb(labelGrey, labelGrey, labelGrey);
#elif !SLIC_VISUALIZE_CONNECTED_COMPONENT_LABELS
        superpixel->centerColorRGB(pixelColor);
#endif //SLIC_VISUALIZE_LABELS

#if !SLIC_VISUALIZE_CONNECTED_COMPONENT_LABELS
        superpixel->interiorPixels(interiorPx, nInteriorPx);
        for(pxind i = 0; i < nInteriorPx; i += 1) {
            input->kToXY(interiorPx[i], x, y);
            outputImage->setPixel(x, y, pixelColor);
        }
        superpixel->boundaryPixels(boundaryPx, nBoundaryPx);
        for(pxind i = 0; i < nBoundaryPx; i += 1) {
            input->kToXY(boundaryPx[i], x, y);
            outputImage->setPixel(x, y, SLIC_BORDER_COLOR);
        }
#else
        superpixel->allPixels(interiorPx, nInteriorPx);
        for(pxind i = 0; i < nInteriorPx; i += 1) {
            input->kToXY(interiorPx[i], x, y);
            label = (static_cast<qreal>(connectedComponentLabels[interiorPx[i]]) * static_cast<qreal>(IMAGEDATA_RGB_RANGE)) / static_cast<qreal>(nConnectedComponents);
            if(label > IMAGEDATA_MAX_RGB) {
                labelGrey = IMAGEDATA_MAX_RGB;
            } else {
                labelGrey = static_cast<int>(floor(label));
            }
            if(connectedComponentClassifications[connectedComponentLabels[interiorPx[i]]]) {
                pixelColor = qRgb(0, labelGrey, 0);
            } else {
                pixelColor = qRgb(0, 0, labelGrey);
            }
            outputImage->setPixel(x, y, pixelColor);
        }
#endif // !SLIC_VISUALIZE_CONNECTED_COMPONENT_LABELS

#if SLIC_VISUALIZE_LABELS || SLIC_VISUALIZE_CONNECTED_COMPONENT_LABELS
        superpixel->centerPosition(centerPosition);
        outputImage->setPixel(centerPosition.x(), centerPosition.y(), SLIC_DEBUG_CENTER_COLOR);
        input->fourNeighbours(neighbours, nNeighbours, input->xyToK(centerPosition.x(), centerPosition.y()));
        for(pxind i = 0; i < nNeighbours; i += 1) {
            input->kToXY(neighbours[i], x, y);
            outputImage->setPixel(x, y, SLIC_DEBUG_CENTER_COLOR);
        }
#endif //SLIC_VISUALIZE_LABELS || SLIC_VISUALIZE_CONNECTED_COMPONENT_LABELS
    }
}

qreal SLIC::distanceToCenter(const pxind& px, const Center &center) const {
    pxind x = 0, y = 0;
    input->kToXY(px, x, y);
    qreal dsSq = (center.position - QVector2D(x, y)).lengthSquared();
    qreal dcSq = (center.color - QVector3D(lStarOrigin[px], aStarOrigin[px], bStarOrigin[px])).lengthSquared();
    return sqrt(dcSq + ((dsSq * mSquared) / sSquared));
}

bool SLIC::initializeOutput(void) {
    return Algorithm::initializeOutput(
                SLIC_DEFAULT_OUTPUT_IMAGE_BACKGROUND,
                false
            );
}

void SLIC::cleanup(void) {
    finalizeOutput();
    if(previousCenters != 0) {
        delete [] previousCenters;
        previousCenters = 0;
    }
    if(currentCenters != 0) {
        delete [] currentCenters;
        currentCenters = 0;
    }
    if(clusterSearchWindow != 0) {
        delete [] clusterSearchWindow;
        clusterSearchWindow = 0;
    }
    if(distancesToCenters != 0) {
        delete [] distancesToCenters;
        distancesToCenters = 0;
    }
    if(clusterLabels != 0) {
        delete [] clusterLabels;
        clusterLabels = 0;
    }
    if(nPixelsPerCluster != 0) {
        delete [] nPixelsPerCluster;
        nPixelsPerCluster = 0;
    }
    if(connectedComponentLabels != 0) {
        delete [] connectedComponentLabels;
        connectedComponentLabels = 0;
    }
#if SLIC_SELECT_LARGEST_COMPONENTS
    if(connectedComponentHeap != 0) {
        delete connectedComponentHeap;
        connectedComponentHeap = 0;
    }
#endif //SLIC_SELECT_LARGEST_COMPONENTS
    if(connectedComponentClassifications != 0) {
        delete [] connectedComponentClassifications;
        connectedComponentClassifications = 0;
    }
    if(visited != 0) {
        delete [] visited;
        visited = 0;
    }
    if(unvisitedPixels != 0) {
        delete unvisitedPixels;
        unvisitedPixels = 0;
    }
    if(visitedPx != 0) {
        delete [] visitedPx;
        visitedPx = 0;
    }
    if(pixelSortingOffsets != 0) {
        delete [] pixelSortingOffsets;
        pixelSortingOffsets = 0;
    }
    if(sortedPixels != 0) {
        delete [] sortedPixels;
        sortedPixels = 0;
    }
    if(superpixels != 0) {
        for(pxind i = 0; i < kParam; i += 1) {
            if(superpixels[i] != 0) {
                delete superpixels[i];
                superpixels[i] = 0;
            }
        }
        delete [] superpixels;
        superpixels = 0;
    }
    Algorithm::cleanup();
}

void SLIC::finalizeOutput(void) {
    Algorithm::finalizeOutput();
}
