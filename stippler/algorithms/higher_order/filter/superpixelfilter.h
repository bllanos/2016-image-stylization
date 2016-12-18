#ifndef SUPERPIXELFILTER_H
#define SUPERPIXELFILTER_H

/*!
** \file superpixelfilter.h
** \brief Definition of the SuperpixelFilter abstract class.
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

#include "algorithms/superpixels/isuperpixelgenerator.h"
#include "filteredsuperpixellation.h"

/*!
 * \brief An abstract class representing a filter for superpixels.
 *
 * The filter must select certain superpixels according to some perceptually
 * meaningful criteria.
 *
 * Its output is an object of type FilteredSuperpixellation.
 * Its input is an image and an ISuperpixelGenerator instance to use for
 * producing the superpixels to be filtered.
 */
class SuperpixelFilter : public Algorithm
{
protected:
    /*!
     * \brief Construct an instance that will use the given superpixel
     * generation algorithm.
     * \param [in] generator The algorithm to use for generating superpixels.
     * The instance takes ownership of this parameter, and sets the caller's
     * pointer to null.
     */
    SuperpixelFilter(ISuperpixelGenerator*& generator);

public:
    virtual ~SuperpixelFilter(void);

    /*!
     * \brief Describe the additional images required to initialize this algorithm
     * and its sub-algorithms
     *
     * \param [out] imageDescriptions A list of descriptions of the additional input
     * image(s) that the algorithm requires. This parameter is expected to
     * be passed in empty.
     */
    virtual void additionalRequiredImages(QVector<QString> &imageDescriptions) Q_DECL_OVERRIDE;

    /*!
     * \brief Set the algorithm's input data and parameters
     *
     * Internally, calls initialize(ImageData *&) after passing additional
     * images to SuperpixelFilter::superpixelGenerator
     * \param [in] images The input images. The algorithm takes ownership of this
     * vector, even if this function returns a failure result, and sets
     * the pointer to null.
     * \return Success (true) or failure (false) to initialize
     */
    virtual bool initialize(QVector<ImageData *> *&images) Q_DECL_OVERRIDE;

    /*!
     * \brief Set the algorithm's input data and parameters
     *
     * This function should reset the state of the object. It can safely be called
     * multiple times, therefore.
     *
     * ## Notes
     * - This function calls cleanup(), so derived classes should not call
     *   cleanup() within their initialize() functions.
     * - If calling this function in a derived class, remember that the
     *   input argument is set to null.
     * \param [in] image The input image. The algorithm takes ownership of this object,
     * even if this function returns a failure result.
     * \return Success (true) or failure (false) to initialize
     */
    virtual bool initialize(ImageData * &image) Q_DECL_OVERRIDE;

    /*!
     * \brief Perform one unit of processing
     *
     * This function runs the superpixel generation algorithm,
     * SuperpixelFilter::superpixelGenerator, to completion, then initializes
     * SuperpixelFilter::selectedSuperpixels and SuperpixelFilter::selectedPixels
     * to `false` and retrieves SuperpixelFilter::superpixellation from
     * SuperpixelFilter::superpixelGenerator.
     *
     * Derived classes are expected to filter superpixels afterwards.
     *
     * This function does not update Algorithm::finished. In the absence of
     * the value of the `finished` output argument, derived classes
     * can check SuperpixelFilter::isFinished(). (Derived classes should also
     * override isFinished().)
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
     *
     * This class's implementation does not use the value of Algorithm::finished.
     * \return The status of processing (complete/incomplete)
     */
    virtual bool isFinished(void) const Q_DECL_OVERRIDE;

    /*!
     * \brief Output filtered superpixel data
     * \param [out] filteredSuperpixellation A superpixellation of an image,
     * where superpixels are either selected or rejected.
     * The caller is expected to take ownership of this object. A null pointer
     * is expected to be passed in.
     * \return Success (true) or failure (false). For instance, a failure
     * result is returned if the object is not ready to produce a filtering of
     * superpixel data.
     */
    virtual bool outputFilteredSuperpixellation(FilteredSuperpixellation *& filteredSuperpixellation);

protected:

    /*!
     * \brief The effective destructor
     *
     * This function is called by the destructor as well as by initialize().
     * Note that the behaviour of virtual functions is abnormal in the contexts
     * of constructors and destructors, as discussed
     * [here](http://stackoverflow.com/questions/12092933/calling-virtual-function-from-destructor#12093250).
     */
    virtual void cleanup(void) Q_DECL_OVERRIDE;

    // Data members
protected:
    /*!
     * \brief The algorithm used to generate a Superpixellation of the input image
     */
    ISuperpixelGenerator* superpixelGenerator;
    /*!
     * \brief The superpixellation of the input image produced by
     * SuperpixelFilter::superpixelGenerator
     */
    Superpixellation* superpixellation;
    /*!
     * \brief An array that will store the
     * selected/rejected status of each superpixel in SuperpixelFilter::superpixels.
     */
    bool* selectedSuperpixels;
    /*!
     * \brief An array with the same number of elements as the number of pixels
     * in the image Algorithm::input. Each element
     * indicates the selected/rejected status of the superpixel corresponding
     * to the pixel location.
     */
    bool* selectedPixels;

private:
    /*!
     * \brief Indicates whether the generation of superpixels has finished
     * \see isFinished()
     */
    bool isSuperpixelGenerationFinished;

    /*!
     * \brief A flag indicating if Algorithm::input can be deallocated
     * by this object.
     *
     * This flag is used for memory management purposes. I should have used
     * shared pointers throughout the project code in order to avoid
     * ad-hoc solutions like this.
     */
    bool canDeleteInput;
};

#endif // SUPERPIXELFILTER_H
