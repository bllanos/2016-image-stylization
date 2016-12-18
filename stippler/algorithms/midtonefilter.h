#ifndef MIDTONEFILTER_H
#define MIDTONEFILTER_H

/*!
** \file midtonefilter.h
** \brief Definition of the MidtoneFilter class.
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

#include "algorithm.h"
#include "imagedata.h"

/*!
 * \brief A double-thresholding filter which selects pixels with CIE L*a*b*
 * lightness values between two thresholds.
 *
 * Pixels with lightness values between the two thresholds will be set to light values,
 * whereas pixels with lightness values outside this range will be set to dark values.
 * The lightness of a pixel is determined using sigmoidal functions
 * of its difference in lightness from the two thresholds. As such, the
 * softness of the threshold is adjustable.
 *
 * Following thresholding, the image lightnesses are linearly rescaled to
 * the full range.
 */
class MidtoneFilter : public Algorithm
{
public:
    /*!
     * \brief Construct an instance with default parameters
     */
    MidtoneFilter(void);

    virtual ~MidtoneFilter(void);

    /*!
     * \brief Perform one unit of processing
     * \param [out] finished Whether or not processing has completed or failed (true),
     * or is incomplete (false).
     * \param [out] status A string describing the current progress towards completion
     * \return Success (true), or failure to process (false). In the latter case,
     * this object should be destroyed.
     */
    virtual bool increment(bool & finished, QString & status) Q_DECL_OVERRIDE;

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

    /*!
     * \brief Update state control variables and choose the next stage of processing
     *
     * Updates MidtoneFilter::k and MidtoneFilter::progress,
     * and finds the end of the current processing increment.
     *
     * MidtoneFilter::k is set to zero if the value of
     * MidtoneFilter::progress and the current
     * value of MidtoneFilter::k indicate that a new loop is about to begin.
     * MidtoneFilter::progress is updated when each stage of the algorithm ends.
     *
     * \return The final value that should be reached by MidtoneFilter::k
     * during the current processing increment
     */
    pxind updateKAndProgress(void);

    /*!
     * \brief Finds the end of the current processing increment
     *
     * A helper function for updateKAndProgress().
     * \return The final value that should be reached by MidtoneFilter::k
     * during the current processing increment
     */
    pxind getLoopLimit(void) const;

    /*!
     * \brief Apply soft thresholding to the image
     *
     * \param [in] endPixel The pixel index bounding the current
     * processing increment
     */
    void thresholdImage(const pxind &endPixel);

    /*!
     * \brief Linearly rescale image lightness values to the full range
     *
     * \param [in] endPixel The pixel index bounding the current
     * processing increment
     */
    void rescaleImage(const pxind &endPixel);

    /*!
     * \brief Sigmoidal function used for thresholding
     * \param [in] min The lower horizontal asymptote of the function
     * \param [in] max The upper horizontal asymptote of the function
     * \param [in] scale The multiplier of the deviation from the center of the function
     * (i.e. the horizontal scaling factor)
     * \param [in] center The `x` value at which the sigmoid reaches the midpoint
     * between `min` and `max`.
     * \param [in] sign Whether the sigmoid increases (true) or decreases (false)
     * with a positive deviation from `center`.
     * \param [in] x The value of the independent variable
     * \return The sigmoidal function's value
     */
    static qreal sigmoid(
            const qreal& min,
            const qreal& max,
            const qreal& scale,
            const qreal& center,
            const bool& sign,
            const qreal& x
        );

protected:

    /*!
     * \brief Identifiers for the various stages in processing
     */
    enum class Progress : unsigned int {
        START,
        RGB2LAB,
        THRESHOLD,
        RESCALE,
        CREATE_LAB_IMAGE,
        LAB2RGB,
        FILL_OUTPUT,
        END
    };

    // Data members
protected:
    /*!
     * \brief The CIE L*a*b* lightness channel owned by this object's
     * Algorithm::input data member
     */
    const qreal* lStarInput;

    // Parameters
    /*!
     * \brief The CIE L*a*b* lightness threshold marking the lower
     * end of the midtone range.
     */
    const qreal lowThreshold;

    /*!
     * \brief The CIE L*a*b* lightness threshold marking the upper
     * end of the midtone range.
     */
    const qreal highThreshold;

    /*!
     * \brief The deviation on either side of MidtoneFilter::lowThreshold
     * at which the output of thresholding with respect to this lower lightness
     * threshold is 5% or 95% grey, for negative or positive deviations, respectively.
     */
    const qreal lowBandwidth;

    /*!
     * \brief The deviation on either side of MidtoneFilter::highThreshold
     * at which the output of thresholding with respect to this upper lightness
     * threshold is 95% or 5% grey, for negative or positive deviations, respectively.
     */
    const qreal highBandwidth;

    // Derived parameters
    /*!
     * \brief Sigmoidal function x-value scaling derived from MidtoneFilter::lowBandwidth
     */
    qreal lowFactor;
    /*!
     * \brief Sigmoidal function x-value scaling derived from MidtoneFilter::highBandwidth
     */
    qreal highFactor;

    // Other data members
    /*!
     * \brief The CIE L*a*b* lightness channel of the thresholded
     * result image
     */
    qreal* lStarThresholded;

    /*!
     * \brief The lowest lightness value in MidtoneFilter::lStarThresholded
     * prior to rescaling to the full lightness range
     */
    qreal minLStar;

    /*!
     * \brief The highest lightness value in MidtoneFilter::lStarThresholded
     * prior to rescaling to the full lightness range
     */
    qreal maxLStar;

    /*!
     * \brief The output thresholded image
     */
    ImageData* thresholdedImage;

    // Processing state variables
    /*!
     * \brief The algorithm's current stage of processing
     */
    Progress progress;
    /*!
     * \brief Index of the next pixel to process
     */
    pxind k;
};

#endif // MIDTONEFILTER_H
