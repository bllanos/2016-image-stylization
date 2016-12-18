#ifndef LOCALDATAFILTER_H
#define LOCALDATAFILTER_H

/*!
** \file localdatafilter.h
** \brief Definition of the LocalDataFilter class.
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

#include "superpixelfilter.h"

/*!
 * \brief A filter for superpixels, which selects superpixels based on their
 * own data, ignoring their relationships with neighbouring superpixels.
 */
class LocalDataFilter : public SuperpixelFilter
{
public:

    /*!
     * \brief The measurement used as the basis for superpixel scores
     *
     * Each measurement dimension is associated with functions for
     * calculating values of the dimension and functions for normalizing individual
     * values in a set of calculated values. Furthermore, each measurement
     * dimension is associated with a flag determining whether to select
     * superpixels with high values along the dimension, or with low values
     * along the dimension.
     */
    enum class ScoreBasis: unsigned int {
        /*!
         * \brief Select superpixels with larger numbers of pixels
         */
        SIZE,
        /*!
         * \brief Select superpixels with smaller standard deviations of
         * the image lightness channel (CIE L*a*b* colour space).
         */
        STDDEV_LSTAR,
        /*!
         * \brief Select superpixels based on their average lightnesses
         * in an externally-generated pixel (soft) selection map.
         *
         * Superpixels with low mean lightnesses are selected.
         */
        EXTERNAL
    };

public:
    /*!
     * \brief Construct a superpixel filter with a specific scoring function
     * \param [in] generator The algorithm used to generate the superpixels
     * \param [in] basis The measurement dimension used to select/reject superpixels
     */
    LocalDataFilter(ISuperpixelGenerator*& generator, const ScoreBasis &basis);

    virtual ~LocalDataFilter(void);

    /*!
     * \brief Describe the additional images required to initialize this algorithm
     *
     * If the `basis` parameter of LocalDataFilter(ISuperpixelGenerator*&, const ScoreBasis &)
     * is ScoreBasis::EXTERNAL, then an externally-generated soft selection map
     * is required.
     * \param [out] imageDescriptions A list of descriptions of the additional input
     * image(s) that the algorithm requires. This parameter is expected to
     * be passed in empty.
     * \see ScoreBasis::EXTERNAL
     */
    virtual void additionalRequiredImages(QVector<QString> &imageDescriptions) Q_DECL_OVERRIDE;

    /*!
     * \brief If the `basis` parameter of LocalDataFilter(ISuperpixelGenerator*&, const ScoreBasis &)
     * is ScoreBasis::EXTERNAL, then an externally-generated soft selection map
     * is required.
     * \param [in] images The input images. The algorithm takes ownership of this
     * vector, even if this function returns a failure result, and sets
     * the pointer to null.
     * \return Success (true) or failure (false) to initialize
     */
    virtual bool initialize(QVector<ImageData *> *&images) Q_DECL_OVERRIDE;

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
     * \brief Indicates if processing has completed.
     *
     * This function will return `false` until increment() outputs `true`
     * through its `finished` parameter, after which it will return `true`.
     * \return The status of processing (complete/incomplete)
     */
    virtual bool isFinished(void) const Q_DECL_OVERRIDE;

protected:
    /*!
     * \brief Set the algorithm's input data and parameters
     *
     * This function should reset the state of the object. It can safely be called
     * multiple times, therefore.
     *
     * Note: This function calls cleanup(), so derived classes should not call
     * cleanup() within their initialize() functions.
     * \param [in] image The input image. The algorithm takes ownership of this object,
     * even if this function returns a failure result.
     * \return Success (true) or failure (false) to initialize
     */
    virtual bool initialize(ImageData * &image) Q_DECL_OVERRIDE;

    /*!
     * \brief The effective destructor
     *
     * This function is called by the destructor as well as by initialize().
     * Note that the behaviour of virtual functions is abnormal in the contexts
     * of constructors and destructors, as discussed
     * [here](http://stackoverflow.com/questions/12092933/calling-virtual-function-from-destructor#12093250).
     */
    virtual void cleanup(void) Q_DECL_OVERRIDE;

protected:

    /*!
     * \brief Identifiers for the various stages in processing
     */
    enum class Progress : unsigned int {
        START,
        GENERATE_SUPERPIXELS,
        RGB2LAB,
        COLLECT_STATISTICS,
        NORMALIZE_STATISTICS,
        CONSTRUCT_HISTOGRAM,
        CHOOSE_OTSU_THRESHOLD,
        FILTER_SUPERPIXELS,
        INITIALIZE_OUTPUT,
        FILL_OUTPUT,
        FINALIZE_OUTPUT,
        END
    };

protected:

    /*!
     * \brief Update state control variables and choose the next stage of processing
     *
     * Updates LocalDataFilter::k and LocalDataFilter::progress,
     * and finds the end of the current processing increment.
     *
     * LocalDataFilter::k is set to zero if the value of
     * LocalDataFilter::progress and the current
     * value of LocalDataFilter::k indicate that a new loop is about to begin.
     * LocalDataFilter::progress is updated when each stage of the algorithm ends.
     *
     * \return The final value that should be reached by LocalDataFilter::k
     * during the current processing increment
     * \see getLoopLimit()
     */
    pxind updateKAndProgress(void);

    /*!
     * \brief Finds the end of the current processing increment
     *
     * A helper function for updateKAndProgress().
     * \return The final value that should be reached by LocalDataFilter::k
     * during the current processing increment
     */
    pxind getLoopLimit(void) const;

    /*!
     * \brief Assemble superpixel scores into a histogram
     * \param [in] endSuperpixel The superpixel index bounding the current
     * processing increment
     */
    void constructHistogram(const pxind &endSuperpixel);

    /*!
     * \brief Run Otsu's method on LocalDataFilter::histogram to automatically
     * choose a threshold which maximizes the between class variance.
     *
     * Otsu's method may be preferable to manual setting of a threshold.
     *
     * This function finishes processing in one call, rather than having
     * to be called for several processing increments, because the histogram
     * is assumed to have a bounded number of bins.
     *
     * ### Reference
     * A. Greensted. "Otsu Thresholding." Internet:
     * http://www.labbookpages.co.uk/software/imgProc/otsuThreshold.html ,
     * June 17, 2010 (Accessed Oct. 25, 2016).
     * \see #LOCALDATAFILTER_MAX_HISTOGRAM_BINS
     */
    void chooseOtsuThreshold(void);

    /*!
     * \brief Select superpixels based on the values of their scores
     * in relation to the threshold set by chooseOtsuThreshold()
     *
     * Whether superpixels are selected if their scores are above or below
     * the threshold is determined by the ScoreBasis enumeration constant
     * used to construct this object.
     * \param [in] endSuperpixel The superpixel index bounding the current
     * processing increment
     */
    void filterSuperpixels(const pxind &endSuperpixel);

    /*!
     * \brief Set up data members relating to image output
     * \return Success (true) or failure (false)
     */
    virtual bool initializeOutput(void);

    /*!
     * \brief Fill the output image, Algorithm::outputImage
     *
     * The left side of the image is a visualization
     * of the magnitudes of superpixel scores (in shades of grey, where darker
     * shades indicate lower values).
     *
     * The right side of the image indicates which superpixels have been
     * selected (black) or rejected (white).
     *
     * The image is twice as large as the original input image, Algorithm::input.
     *
     * In fact, the image may be divided in half vertically or horizontally.
     * The division is such that the image is more square, and therefore
     * depends on the dimensions of Algorithm::input.
     *
     * \param [in] endSuperpixel The superpixel index bounding the current
     * rendering increment
     */
    void fillOutputImage(const pxind &endSuperpixel);

protected:
    /*!
     * \brief Populate LocalDataFilter::superpixelScores with superpixel sizes
     *
     * This is a static function as opposed to an instance function in order
     * for it to be referred to via a function pointer. It is bound as an instance
     * function, when the value of the `basis` parameter of LocalDataFilter()
     * is LocalDataFilter::ScoreBasis::SIZE.
     * \param [in] alg The instance of the class to update
     * \param [in] endSuperpixel The superpixel index bounding the current
     * processing increment
     * \see Superpixellation::Superpixel::size()
     */
    static void collectSizeStatistics(LocalDataFilter &alg, const pxind &endSuperpixel);
    /*!
     * \brief Normalize the contents of
     * LocalDataFilter::superpixelScores as appropriate for superpixel sizes
     *
     * Superpixels with sizes equal to the number of image pixels divided
     * by the number of superpixels are given normalized scores of 1.
     * Normalization is a linear transformation.
     *
     * This is a static function as opposed to an instance function in order
     * for it to be referred to via a function pointer. It is bound as an instance
     * function, when the value of the `basis` parameter of LocalDataFilter()
     * is LocalDataFilter::ScoreBasis::SIZE.
     * \param [in] alg The instance of the class to update
     * \param [in] endSuperpixel The superpixel index bounding the current
     * processing increment
     * \see collectSizeStatistics()
     */
    static void normalizeSizeStatistics(LocalDataFilter &alg, const pxind &endSuperpixel);

    /*!
     * \brief Populate LocalDataFilter::superpixelScores with superpixel lightness
     * channel standard deviations
     *
     * Superpixel scores are the CIE L*a*b* L* channel standard deviations,
     * as retrieved from the first component of the output parameter of
     * Superpixellation::Superpixel::standardColorDeviation(QVector3D&).
     *
     * This is a static function as opposed to an instance function in order
     * for it to be referred to via a function pointer. It is bound as an instance
     * function, when the value of the `basis` parameter of LocalDataFilter()
     * is LocalDataFilter::ScoreBasis::STDDEV_LSTAR.
     * \param [in] alg The instance of the class to update
     * \param [in] endSuperpixel The superpixel index bounding the current
     * processing increment
     */
    static void collectStddevLStarStatistics(LocalDataFilter &alg, const pxind &endSuperpixel);

    /*!
     * \brief Normalize the contents of LocalDataFilter::superpixelScores
     * as appropriate for superpixel lightness standard deviations
     *
     * Standard deviations of zero are mapped to zero, and standard deviations
     * equal to half of the range of possible lightness values are mapped to 1.
     * Normalization is a linear transformation.
     *
     * This is a static function as opposed to an instance function in order
     * for it to be referred to via a function pointer. It is bound as an instance
     * function, when the value of the `basis` parameter of LocalDataFilter()
     * is LocalDataFilter::ScoreBasis::STDDEV_LSTAR.
     * \param [in] alg The instance of the class to update
     * \param [in] endSuperpixel The superpixel index bounding the current
     * processing increment
     * \see collectStddevLStarStatistics()
     */
    static void normalizeStddevLStarStatistics(LocalDataFilter &alg, const pxind &endSuperpixel);

    /*!
     * \brief Populate LocalDataFilter::superpixelScores with external
     * pixel selection data
     *
     * Superpixel scores are the average lightness values from the positions
     * of their pixels in the external selection data.
     *
     * This is a static function as opposed to an instance function in order
     * for it to be referred to via a function pointer. It is bound as an instance
     * function, when the value of the `basis` parameter of LocalDataFilter()
     * is LocalDataFilter::ScoreBasis::EXTERNAL.
     * \param [in] alg The instance of the class to update
     * \param [in] endSuperpixel The superpixel index bounding the current
     * processing increment
     */
    static void collectExternalStatistics(LocalDataFilter &alg, const pxind &endSuperpixel);
    /*!
     * \brief Normalize the contents of
     * LocalDataFilter::superpixelScores as appropriate for externally-provided
     * pixel selection data
     *
     * There is no normalization to perform. This function simply sets
     * LocalDataFilter::minScore to #IMAGEDATA_MIN_LIGHTNESS and
     * LocalDataFilter::maxScore to #IMAGEDATA_MAX_LIGHTNESS.
     *
     * This is a static function as opposed to an instance function in order
     * for it to be referred to via a function pointer. It is bound as an instance
     * function, when the value of the `basis` parameter of LocalDataFilter()
     * is LocalDataFilter::ScoreBasis::EXTERNAL.
     * \param [in] alg The instance of the class to update
     * \param [in] endSuperpixel The superpixel index bounding the current
     * processing increment
     * \see collectExternalStatistics()
     */
    static void normalizeExternalStatistics(LocalDataFilter &alg, const pxind &endSuperpixel);

    // Data members
protected:
    /*!
     * \brief The `basis` parameter of LocalDataFilter(), which determines
     * the metrics used for filtering superpixels
     */
    const ScoreBasis basis;
    /*!
     * \brief An image where the lightness channel values indicate the strength
     * with which pixels are selected.
     *
     * This input is necessary when `basis` parameter of LocalDataFilter()
     * is LocalDataFilter::ScoreBasis::EXTERNAL.
     */
    ImageData* selectionMap;
    /*!
     * \brief The CIE L*a*b* lightness channel owned by this object's
     * LocalDataFilter::externalSelectionMap data member
     */
    const qreal* lStarSelectionMap;

    /*!
     * \brief Superpixel scores
     *
     * Scores are compared with a threshold to determine which superpixels are
     * selected/rejected by the filter.
     */
    qreal* superpixelScores;
    /*!
     * \brief The largest score in LocalDataFilter::superpixelScores
     */
    qreal maxScore;
    /*!
     * \brief The smallest score in LocalDataFilter::superpixelScores
     */
    qreal minScore;
    /*!
     * \brief The reciprocal of the bin width of LocalDataFilter::inverseBinWidth
     */
    qreal inverseBinWidth;
    /*!
     * \brief A histogram constructed from the superpixel scores in
     * LocalDataFilter::superpixelScores
     */
    pxind* histogram;
    /*!
     * \brief Number of histogram bins
     *
     * The length of LocalDataFilter::histogram
     */
    pxind nHistogramBins;
    /*!
     * \brief A threshold determined using Otsu's method on LocalDataFilter::histogram
     * \see chooseOtsuThreshold()
     */
    qreal otsuThreshold;
    /*!
     * \brief Whether to select superpixels with scores below or above
     * LocalDataFilter::otsuThreshold
     *
     * The value of this member depends on the value of the `basis` parameter of
     * LocalDataFilter()
     */
    bool selectBelowThreshold;

    // Output variables
    /*!
     * \brief Alignment of the sub-images in Algorithm::outputImage
     *
     * Set by initializeOutput()
     * \see fillOutputImage()
     */
    bool outputInRow;

    // Processing state variables
    /*!
     * \brief The algorithm's current stage of processing
     */
    Progress progress;
    /*!
     * \brief Index of the next superpixel to process
     */
    pxind k;

    /*!
     * \brief The function to use for populating LocalDataFilter::superpixelScores
     */
    void (*collectStatistics)(LocalDataFilter &alg, const pxind &);
    /*!
     * \brief The function to use for normalizing the elements in
     * LocalDataFilter::superpixelScores
     *
     * This function is called after LocalDataFilter::collectStatistics
     * has populated all of LocalDataFilter::superpixelScores,
     * and is also responsible for calculating LocalDataFilter::maxScore
     * and LocalDataFilter::minScore.
     */
    void (*normalizeStatistics)(LocalDataFilter &alg, const pxind &);
};

#endif // LOCALDATAFILTER_H
