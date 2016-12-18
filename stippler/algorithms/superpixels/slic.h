#ifndef SLIC_H
#define SLIC_H

/*!
** \file slic.h
** \brief Definition of the SLIC class.
**
** ### About
** Created for: COMP4905A Honours Project\n
** Fall 2016\n
** Bernard Llanos\n
** Supervised by Dr. David Mould\n
** School of Computer Science, Carleton University
**
** ## Primary basis
** basicsps.h
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

#include <QtGlobal>
#include <QLinkedList>
#include "imagedata.h"
#include "algorithms/superpixels/isuperpixelgenerator.h"
#include "ods/BinaryHeap.h"

/*!
  \brief Choice of the post-processing method

  If true, the largest connected component from each K-means cluster will
  be selected, and the others will have their pixels merged into adjacent
  connected components

  If false, the selected connected components are those which contain
  their cluster centers. It is possible that no connected components will be
  selected for a given cluster.
 */
#define SLIC_SELECT_LARGEST_COMPONENTS 1

/*!
 * \brief SLIC superpixel decomposition of an image
 *
 * Implementation of the Simple Linear Iterative Clustering method for
 * clustering pixels into superpixels.
 */
class SLIC : public ISuperpixelGenerator
{
public:
    /*!
     * \brief Construct an instance with default parameters
     */
    SLIC();

    virtual ~SLIC();

    /*!
     * \brief Perform one unit of processing
     * \param [out] finished Whether or not processing has completed or failed (true),
     * or is incomplete (false).
     * \param [out] status A string describing the current progress towards completion
     * \return Success (true), or failure to process (false). In the latter case,
     * this object should be destroyed.
     */
    virtual bool increment(bool & finished, QString & status) Q_DECL_OVERRIDE;

    /*!
     * \brief Output superpixel data
     * \param [out] superpixellation A superpixellation of an image.
     * The caller is expected to take ownership of this object. A null pointer
     * is expected to be passed in.
     * \return Success (true) or failure (false). For instance, a failure
     * result is returned if the object is not ready to produce superpixel data.
     */
    virtual bool outputSuperpixellation(Superpixellation *& superpixellation) Q_DECL_OVERRIDE;

protected:

    /*!
     * \brief Identifiers for the various stages in processing
     */
    enum class Progress : unsigned int {
        START,
        RGB2LAB,
        SEED_CENTERS,
        K_MEANS_LABEL_PIXELS,
        K_MEANS_UPDATE_CENTERS,
        K_MEANS_ASSESS_ITERATION,
        FIND_CONNECTED_COMPONENTS,
        CLASSIFY_CONNECTED_COMPONENTS,
        REASSIGN_CONNECTED_COMPONENTS,
        SORT_PIXELS_AS_SUPERPIXELS,
        CREATE_SUPERPIXEL_OBJECTS,
        INITIALIZE_OUTPUT,
        FILL_OUTPUT,
        FINALIZE_OUTPUT,
        END
    };

protected:
    /*!
     * \brief Set the algorithm's input data and parameters
     *
     * This function resets the state of the object. It can safely be called
     * multiple times, therefore.
     * \param [in] image The input image. The algorithm takes ownership of this object,
     * even if this function returns a failure result.
     * \return Success (true) or failure (false) to initialize
     */
    virtual bool initialize(ImageData * &image) Q_DECL_OVERRIDE;

    /*!
     * \brief Update state control variables and choose the next stage of processing
     *
     * Updates SLIC::k, SLIC::iterationCount, and SLIC::progress in particular,
     * and finds the end of the current processing increment.
     *
     * SLIC::k is set to zero if the value of SLIC::progress and the current
     * value of SLIC::k indicate that a new loop is about to begin.
     * SLIC::progress is updated when each stage of the algorithm ends.
     *
     * This function also contains the logic for deciding when to stop K-Means
     * iteration. K-Means iteration ends when either:
     * - The maximum number of iterations, #SLIC_MAX_KMEANS_ITERATIONS, is reached, or
     * - When the fractional difference between the changes in cluster center
     *   positions during the current and previous iteration drops below #SLIC_ERROR_THRESHOLD.
     *   - With this criteria, there is no need to set an absolute threshold
     *     on the change in cluster center positions. (Such a threshold would
     *     presumably depend on the image dimensions and the number of clusters.)
     *   - In a sense, the second derivative of cluster center positions
     *     which is being compared with #SLIC_ERROR_THRESHOLD.
     *
     * \return The final value that should be reached by SLIC::k
     * during the current processing increment
     * \see getLoopLimit()
     */
    pxind updateKAndProgress(void);

    /*!
     * \brief Finds the end of the current processing increment
     *
     * A helper function for updateKAndProgress().
     * \return The final value that should be reached by SLIC::k
     * during the current processing increment
     */
    pxind getLoopLimit(void) const;

    /*!
     * \brief Find initial positions for the K-means clusters
     *
     * **Algorithm**
     *
     * 1. Find the smallest grid of SLIC::S by SLIC::S squares that is at least
     *    as large as the input image.
     * 2. Scale the width and height of the grid so that the grid fits within the
     *    input image.
     * 3. Determine the number of squares in the grid and find the ratio of this value
     *    to SLIC::kParam.
     * 4. Assign each cluster center to the center of a grid square, where the
     *    index of the grid square (in raster order) is the index of the cluster
     *    center scaled by the ratio from step 3.
     * 5. Shift the position of the cluster center to the pixel in a 3 x 3 neighbourhood
     *    around the initial position having the lowest Sobel gradient magnitude.
     *
     * As the number of grid squares is at least SLIC::kParam, not all grid squares
     * will contain cluster centers. As such, I have used a larger search window
     * during K-Means iteration. The comments in the function implementation
     * describe how the search window dimensions are calculated.
     *
     * The advantage of the above procedure allows an arbitrary value for
     * SLIC::kParam to be used. There is no need to choose values such that
     * the grid exactly fits the image without scaling, and where the number
     * of squares in the grid is exactly SLIC::kParam.
     *
     * \param [in] endCluster The cluster index at which to end processing
     * during the current increment
     * \see SLIC::searchHalfWidth
     * \see SLIC::searchHalfHeight
     * \see #SLIC_MIN_SEARCH_WINDOW_SIZE
     */
    void initializeCenters(const pxind &endCluster);

    /*!
     * \brief Assign pixels to cluster centers
     *
     * This is the main portion of K-Means iteration. Pixels are assigned
     * to the nearest cluster centers, using the distance measurement
     * implemented in distanceToCenter().
     *
     * In reality, as described in the journal article on SLIC, the iteration
     * is performed from the perspective of the cluster centers rather than
     * that of the pixels. A window of width `(2 * SLIC::searchHalfWidth) + 1`
     * and height `(2 * SLIC::searchHalfHeight) + 1`
     * is centered at each cluster center, and pixel distances are updated
     * only for pixels in these windows.
     * \param [in] endCluster The cluster index at which to end processing
     */
    void kmeansLabelPixels(const pxind &endCluster);

    /*!
     * \brief Update cluster centers
     *
     * After pixels have been assigned to cluster centers during an iteration of K-Means,
     * pixel positions and colours are summed to compute new cluster centers.
     *
     * The normalization of the sums actually takes place in
     * kmeansResidualError(const pxind &, const bool &)
     * \param [in] endPx The pixel index at which to end processing
     */
    void kmeansUpdateCenters(const pxind &endPx);

    /*!
     * \brief Compute the change in cluster centers between K-Means iterations
     *
     * This function finishes the calculation of new cluster centers started
     * in kmeansUpdateCenters(const pxind &) and then calculates the change
     * in cluster center positions from the previous K-Means iteration.
     * \param [in] endCluster The cluster index at which to end processing
     * \param [in] firstIteration A flag indicating whether or not this is the first
     * iteration. For the first iteration, no changes in cluster center positions
     * can be calculated.
     */
    void kmeansResidualError(const pxind &endCluster, const bool &firstIteration = false);

    /*!
     * \brief Find connected components corresponding to K-Means clusters
     *
     * This is the first step in the post-processing portion of SLIC, in which
     * a connected components algorithm is used to find groups of connected
     * pixels corresponding to each cluster.
     *
     * I used breadth-first search to find connected components.
     *
     * If #SLIC_SELECT_LARGEST_COMPONENTS is true, then connected component sizes
     * are calculated as well.
     * \param [in] endPx The pixel index at which to end processing
     */
    void labelConnectedComponents(const pxind &endPx);

    /*!
     * \brief Identify connected components as to be kept or discarded.
     *
     * The method used to classify the connected components depends on the value
     * of #SLIC_SELECT_LARGEST_COMPONENTS
     *
     * Connected components were determined by labelConnectedComponents(const pxind &)
     * \param [in] endCluster The cluster index at which to end processing
     */
    void classifyConnectedComponents(const pxind &endCluster);

    /*!
     * \brief Remove connected components flagged for dissolution
     *
     * Connected components that are to be discarded, as determined by classifyConnectedComponents(const pxind &),
     * have their pixels reassigned to adjacent connected components
     * that will be preserved.
     *
     * As such, this function updates the K-Means cluster labelling of pixels
     * (SLIC::clusterLabels), as well as the sizes of clusters (SLIC::nPixelsPerCluster).
     *
     * A given pixel is assigned to the nearest adjacent connected component,
     * as determined using breadth-first search. Note that this is probably
     * not how most people implement SLIC. For instance, Dr. David Mould assigns
     * all pixels to the largest adjacent connected component. As such, connected
     * components are not split apart when merged into their neighbours,
     * unlike my approach, which treats each pixel independently.
     * \param [in] endPx The pixel index at which to end processing
     */
    void reassignConnectedComponents(const pxind &endPx);

    /*!
     * \brief Organize pixels according to their cluster labels
     *
     * This is a pre-processing step prior to createSuperpixels(const pxind &).
     *
     * Pixels are sorted by cluster label using counting sort.
     * \param [in] endPx The pixel index at which to end processing
     */
    void sortPixelsIntoSuperpixels(const pxind &endPx);

    /*!
     * \brief Create Superpixel objects to represent clusters
     *
     * This function populates the SLIC::superpixels array, which is the
     * non-graphical output of the SLIC algorithm.
     * \param [in] endCluster The cluster index at which to end processing
     */
    void createSuperpixels(const pxind &endCluster);

    /*!
     * \brief Produce an output image to visualize the segmentation of the image
     *
     * The information depicted in the output image depends on the values of:
     * - #SLIC_VISUALIZE_LABELS
     * - #SLIC_VISUALIZE_CONNECTED_COMPONENT_LABELS
     *
     * If neither flag is set, the output image colours pixels with the center
     * colours of the corresponding superpixels.
     * \param [in] endCluster The superpixel index at which to end processing
     */
    void fillOutputImage(const pxind &endCluster);

    /*!
     * \brief Calculate the distance between a pixel and a K-Means cluster center
     * \param [in] px The pixel in question
     * \param [in] center The cluster center
     * \return The distance as calculated using Equation 3 in the article on SLIC
     */
    qreal distanceToCenter(const pxind& px, const Superpixellation::Center<QVector2D> &center) const;

    /*!
     * \brief Set up data members relating to image output
     * \return Success (true) or failure (false)
     */
    virtual bool initializeOutput(void);

    /*!
     * \brief The effective destructor
     */
    virtual void cleanup(void) Q_DECL_OVERRIDE;

    /*!
     * \brief Clean up data members relating to image output
     *
     * Only the intermediate objects used to produce the output objects are
     * cleaned up, not the output objects.
     */
    virtual void finalizeOutput(void) Q_DECL_OVERRIDE;

    // Data members
protected:

    // Algorithm parameters
    /*!
     * \brief The 'k' parameter of the SLIC algorithm, described in the article
     *
     * The number of superpixels
     */
    pxind kParam;
    /*!
     * \brief The 'm' parameter of the SLIC algorithm, described in the article
     *
     * The weight placed on spatial as opposed to colour distances between
     * pixels and K-Means cluster centers
     */
    qreal m;

    // Derived parameters
    /*!
     * \brief The square of SLIC::m
     *
     * Pre-computed for efficiency purposes
     */
    qreal mSquared;
    /*!
     * \brief The 'S' parameter of the SLIC algorithm, derived from 'k',
     * and described in the article
     *
     * `S = sqrt(nPixels / kParam)`, serving as an estimate of the size of a
     * superpixel (assuming square superpixels, for simplicity).
     */
    pxind S;
    /*!
     * \brief The square of SLIC::S
     *
     * Pre-computed for efficiency purposes
     */
    qreal sSquared;
    /*!
     * \brief Half of the width of a rectangle around a cluster center in which
     * the distances of pixels to the cluster center are calculated
     *
     * \see initializeCenters()
     * \see kmeansLabelPixels()
     */
    pxind searchHalfWidth;
    /*!
     * \brief Half of the height of a rectangle around a cluster center in which
     * the distances of pixels to the cluster center are calculated
     *
     * \see initializeCenters()
     * \see kmeansLabelPixels()
     */
    pxind searchHalfHeight;

    // Input
    /*!
     * \brief The CIE L*a*b* lightness channel owned by this object's
     * Algorithm::input data member
     */
    const qreal* lStarOrigin;
    /*!
     * \brief The CIE L*a*b* a* channel owned by this object's
     * Algorithm::input data member
     */
    const qreal* aStarOrigin;
    /*!
     * \brief The CIE L*a*b* b* channel owned by this object's
     * Algorithm::input data member
     */
    const qreal* bStarOrigin;

    // Algorithm state
    /*!
     * \brief The cluster centers obtained from the previous iteration of K-Means
     */
    Superpixellation::Center<QVector2D> *previousCenters;
    /*!
     * \brief The cluster centers obtained from the current iteration of K-Means
     */
    Superpixellation::Center<QVector2D> *currentCenters;
    /*!
     * \brief A buffer for the indices of pixels in the square around a cluster
     * center.
     *
     * Pixels in the search window will have their distances to the cluster
     * center assessed as part of K-Means iteration.
     * \see kmeansLabelPixels()
     */
    pxind *clusterSearchWindow;
    /*!
     * \brief The change in cluster center positions during the previous K-Means
     * iteration
     *
     * \see updateKAndProgress()
     */
    qreal previousResidualError;
    /*!
     * \brief The change in cluster center positions during the current K-Means
     * iteration
     *
     * \see updateKAndProgress()
     */
    qreal residualError;
    /*!
     * \brief An array storing the current distances (as computed by distanceToCenter() )
     * between each pixel and its closest K-Means cluster center
     */
    qreal *distancesToCenters;
    /*!
     * \brief An array storing the cluster identifiers of each pixel
     */
    pxind *clusterLabels;
    /*!
     * \brief An array storing the number of pixels associated with each cluster
     */
    pxind *nPixelsPerCluster;
    /*!
     * \brief An array storing the connected component identifiers of each pixel
     *
     * \see labelConnectedComponents()
     */
    pxind *connectedComponentLabels;
    /*!
     * \brief The number of connected components, which is greater than or equal
     * to the number of K-Means clusters
     *
     * \see labelConnectedComponents()
     */
    pxind nConnectedComponents;

#if SLIC_SELECT_LARGEST_COMPONENTS
    /*!
     * \brief The association of a connected component with its size in pixels,
     * and its corresponding K-Means cluster.
     *
     * This class is needed if #SLIC_SELECT_LARGEST_COMPONENTS is set, in which
     * case labelConnectedComponents() computes connected component sizes.
     */
    struct SizeLabelsPair {
        /*!
         * \brief The number of pixels in the connected component
         */
        pxind size;
        /*!
         * \brief The K-Means cluster whose pixels are a superset of those
         * in this connected component
         */
        pxind cluster;
        /*!
         * \brief The identifier of the connected component
         */
        pxind component;

        /*!
         * \brief Construct an empty instance
         */
        SizeLabelsPair(void) :
            size(0), cluster(SUPERPIXELLATION_NONE_LABEL), component(SUPERPIXELLATION_NONE_LABEL)
        {}

        /*!
         * \brief Construct an instance with initial data
         *
         * The size of the connected component is assumed to be unknown at the time.
         * \param [in] clusterLabel The identifier of the K-Means cluster containing
         * this connected component
         * \param [in] componentLabel The identifier of this connected component
         */
        SizeLabelsPair(const pxind& clusterLabel, const pxind& componentLabel) :
            size(0), cluster(clusterLabel), component(componentLabel)
        {}

        /*!
         * \brief Compare connected component sizes and cluster identifiers
         *
         * In a max-heap, SizeLabelsPair objects will be in descending order
         * by cluster identifier, and in ascending order by size.
         * \param [in] rhs The other connected component
         * \return A comparison result to be used for sorting SizeLabelsPair objects
         */
        bool operator<(SizeLabelsPair const & rhs) const
        {
            if(cluster < rhs.cluster) {
                return false;
            } else if(cluster > rhs.cluster) {
                return true;
            } else { // cluster == rhs.cluster
                return (size < rhs.size);
            }
        }

        /*!
         * \brief Assignment operator
         * \param [in] rhs The other instance
         * \return A reference to this instance
         */
        SizeLabelsPair& operator=(const SizeLabelsPair &rhs) {
            size = rhs.size;
            cluster = rhs.cluster;
            component = rhs.component;
            return *this;
        }
    };
    /*!
     * \brief A max-heap used to determine which connected components are
     * the largest connected components in their corresponding K-Means clusters.
     *
     * This member is needed if #SLIC_SELECT_LARGEST_COMPONENTS is set.
     * It is produced by labelConnectedComponents() and consumed by
     * classifyConnectedComponents().
     */
    ods::BinaryHeap<SizeLabelsPair, pxind>* connectedComponentHeap;

#endif //SLIC_SELECT_LARGEST_COMPONENTS

    /*!
     * \brief The output of classifyConnectedComponents()
     *
     * An array storing the keep/discard statuses of the connected components
     * within K-Means clusters
     */
    bool *connectedComponentClassifications;

    /*!
     * \brief An array storing the visited/not yet visited status of pixels
     *
     * Used in the breadth-first search routines in the post-processing
     * stages of SLIC.
     */
    bool *visited;
    /*!
     * \brief The queue of pixels to be visited
     *
     * Used in the breadth-first search routines in the post-processing
     * stages of SLIC.
     */
    QLinkedList<pxind> *unvisitedPixels;
    /*!
     * \brief One greater than the index of the most recently visited pixel
     *
     * Updated and used by labelConnectedComponents()
     */
    pxind lastVisitedPixel;
    /*!
     * \brief An array storing the indices of pixels which have been visited
     * during breadth-first search
     *
     * Updated and used by reassignConnectedComponents()
     */
    pxind *visitedPx;

    /*!
     * \brief An auxiliary variable used in counting sort, within sortPixelsIntoSuperpixels()
     */
    pxind *pixelSortingOffsets;

    /*!
     * \brief An array storing pixel indices sorted by their corresponding
     * superpixel identifiers
     *
     * Produced by sortPixelsIntoSuperpixels() and consumed by createSuperpixels()
     */
    pxind *sortedPixels;

    /*!
     * \brief The output of the SLIC algorithm
     */
    Superpixellation::Superpixel **superpixels;

    // Processing state variables
    /*!
     * \brief The algorithm's current stage of processing
     */
    Progress progress;
    /*!
     * \brief Index of the next pixel or superpixel/cluster to process
     */
    pxind k;

    /*!
     * \brief The number of K-Means iterations that have been completed so far
     */
    pxind iterationCount;
};

#endif // SLIC_H
