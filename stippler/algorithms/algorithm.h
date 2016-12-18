#ifndef ALGORITHM_H
#define ALGORITHM_H

/*!
** \file algorithm.h
** \brief Definition of the Algorithm class.
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
**
*/

#include <QElapsedTimer>
#include <QVector>

class QColor;
class QImage;
class QPainter;
class QString;
class QByteArray;
class QSvgGenerator;
class QBuffer;
class ImageData;

/*!
 * \brief Abstract image processing algorithm
 *
 * Creates a new raster image, and optionally, a vector image, given an
 * input image. The image is processed in steps, in order to provide
 * progress information to the user.
 *
 * Objects of this class are intended to be passed to a worker thread
 * which runs the image processing algorithm, in order to prevent the user
 * interface from becoming unresponsive.
 */
class Algorithm
{
public:
    /*!
     * \brief Construct an instance with default parameters
     */
    Algorithm();

    virtual ~Algorithm(void);

    /*!
     * \brief Indicate that human-readable output (images,
     * SVG files, etc.) is not required and should not be generated.
     *
     * Internally, this sets Algorithm::outputIsEnabled to `false`.
     * Note that initialize() does not reset Algorithm::outputIsEnabled.
     * Therefore, this function presently has a permanent effect.
     */
    void disableOutput(void);

    /*!
     * \brief Describe what additional images are required to initialize this algorithm
     *
     * All algorithms process at least one image - The first image is the
     * one currently in the image viewer, and does not need a description.
     * This function outputs an empty list of descriptions if the algorithm
     * processes only one image.
     * \param [out] imageDescriptions A list of descriptions of the additional input
     * images that the algorithm requires. This parameter is expected to
     * be passed in empty.
     * \see initialize(QVector<ImageData *> *&)
     */
    virtual void additionalRequiredImages(QVector<QString> &imageDescriptions);

    /*!
     * \brief A proxy for initialize(ImageData * &image)
     *
     * The Algorithm class's implementation of this function takes the first
     * image and passes it to initialize(ImageData * &image), then deletes the
     * other images.
     *
     * Derived classes must override this function to make use of multiple
     * input images.
     * \param [in] images The input images. The algorithm takes ownership of this
     * vector, even if this function returns a failure result, and sets
     * the pointer to null.
     * \return Success (true) or failure (false) to initialize
     */
    virtual bool initialize(QVector<ImageData *> *&images);

    /*!
     * \brief Perform one unit of processing
     * \param [out] finished Whether or not processing has completed or failed (true),
     * or is incomplete (false).
     * \param [out] status A string describing the current progress towards completion
     * \return Success (true), or failure to process (false). In the latter case,
     * this object should be destroyed.
     */
    virtual bool increment(bool & finished, QString & status) = 0;

    /*!
     * \brief Collect the results of processing
     *
     * Note for derived class implementation: This function depends on the values
     * of Algorithm::finished and Algorithm::failed being correctly updated.
     * \param [out] image Raster image output, which must be deallocated by the caller
     * \param [out] svgData Vector image output, in the form of an SVG file,
     * which must be deallocated by the caller. `svgData` is null if the
     * algorithm does not produce vector output.
     * \return Success (true) or failure (false). If false, `image` and
     * `svgData` are both null.
     */
    virtual bool output(QImage *&image, QByteArray *& svgData);

    /*!
     * \brief Indicates if processing has completed.
     *
     * This function will return `false` until increment() outputs `true`
     * through its `finished` parameter, after which it will return `true`.
     * \return The status of processing (complete/incomplete)
     */
    virtual bool isFinished(void) const;

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
    virtual bool initialize(ImageData * &image);

    /*!
     * \brief Set up data members relating to image output
     *
     * Algorithm::outputImage and Algorithm::outputSVG are initialized with
     * the same dimensions as Algorithm::input.
     * \param [in] fillColor The background colour with which to fill the
     * vector and raster output images (Algorithm::outputImage and
     * Algorithm::outputSVG, respectively)
     * \param [in] vectorOutput If true, set up data members relating to vector
     * image output, not only data members relating to raster image output.
     * \param [in] title The title to use for the Algorithm::outputSVG SVG file
     * \param [in] description The description to use for the
     * Algorithm::outputSVG SVG file
     * \return Success (true) or failure (false)
     */
    virtual bool initializeOutput(
            const QColor& fillColor,
            const bool& vectorOutput,
            const QString * const&title = 0,
            const QString * const&description = 0);

    /*!
     * \brief Clean up data members relating to image output
     *
     * Only the intermediate objects used to produce the output objects are
     * cleaned up, not the output objects.
     */
    virtual void finalizeOutput(void);

    /*!
     * \brief The effective destructor
     *
     * This function is called by the destructor as well as by initialize().
     * Note that the behaviour of virtual functions is abnormal in the contexts
     * of constructors and destructors, as discussed
     * [here](http://stackoverflow.com/questions/12092933/calling-virtual-function-from-destructor#12093250).
     */
    virtual void cleanup(void);

    // Data members
protected:
    /*!
     * \brief The input image
     */
    ImageData* input;

    // Output
    /*!
     * \brief Whether or not human-readable output will be produced
     * \see disableOutput()
     */
    bool outputIsEnabled;
    /*!
     * \brief Rasterized output of the algorithm
     *
     * Dimensions are the same as those of Algorithm::input
     */
    QImage *outputImage;
    /*!
     * \brief Vector output of the algorithm
     *
     * An image in the form of an SVG file
     */
    QByteArray *outputSVG;
    /*!
     * \brief The QPainter used to render Algorithm::outputSVG
     */
    QPainter* outputSVGPainter;
private:
    /*!
     * \brief The QSvgGenerator used as the paint device for Algorithm::outputSVGPainter
     */
    QSvgGenerator* svgGenerator;
    /*!
     * \brief The output device used to connect Algorithm::svgGenerator with
     * Algorithm::outputSVG
     */
    QBuffer* svgFileIOWrapper;

    // State variables
protected:
    /*!
     * \brief A member to be used by derived classes to keep track of whether
     * an internal failure has occurred
     */
    bool failed;
    /*!
     * \brief A member to be used by derived classes to keep track of whether
     * processing has finished
     */
    bool finished;
    /*!
     * \brief Used to measure processing time
     *
     * The timer is started in initialize()
     */
    QElapsedTimer timer;
};

#endif // ALGORITHM_H
