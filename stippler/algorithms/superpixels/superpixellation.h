#ifndef SUPERPIXELLATION_H
#define SUPERPIXELLATION_H

/*!
** \file superpixellation.h
** \brief Definition of the Superpixellation class.
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

#include "imagedata.h"
#include <QVector3D>

/*!
  \brief An invalid cluster or connected component ID, useful for finding
  non-updated values during debugging.
 */
#define SUPERPIXELLATION_NONE_LABEL (-1)

/*!
 * \brief A class representing an image in terms of superpixels
 */
class Superpixellation
{
public:
    /*!
     * \brief A superpixel center
     *
     * The `point_type` template parameter is a type representing a spatial
     * point (e.g. QPoint or QVector3D). This template parameter allows
     * the class to use either discrete or continuous spatial coordinates,
     * as well as coordinates of arbitrary dimensionality.
     */
    template<class point_type>
    struct Center {

        /*!
         * \brief Construct an empty instance
         */
        Center() {}

        /*!
         * \brief Copy constructor
         * \param [in] other The other instance
         */
        Center(const Center<point_type>& other) :
            position(other.position),
            color(other.color)
        {}

        /*!
         * \brief Assignment operator
         * \param [in] other The other instance
         * \return A reference to this instance
         */
        Center& operator=(const Center<point_type>& other) {
            position = other.position;
            color = other.color;
            return *this;
        }

        /*!
         * \brief Construct an instance with data
         * \param [in] p The spatial position of the center
         * \param [in] c The colour of the center
         */
        Center(const point_type& p, const QVector3D& c) :
            position(p), color(c)
        {}

        /*!
         * \brief Spatial position
         */
        point_type position;
        /*!
         * \brief Colour, in some colour space (use CIE L*a*b* in the context
         * of SLIC)
         */
        QVector3D color;
    };

    /*!
     * \brief The contents and characteristics of a superpixel
     */
    class Superpixel {
    public:
        /*!
         * \brief Construct an object describing a superpixel
         * \param [in] id The ID of the superpixel. This does not
         * need to match the cluster IDs of the pixels in `allPx` in `labels`
         * \param [in] allPx An array of pixels (or, rather, their 1D coordinates)
         * belonging to the superpixel. The Superpixel instance takes ownership
         * of this array and sets the pointer to null.
         * \param [in] nPx The number of pixels in `allPx`
         * \param [in] labels An array with the same number of elements as
         * `img.pixelCount()` (ImageData::pixelCount()) used to determine which
         * pixels in `allPx` are located at the boundary of the superpixel
         * \param [in] img The image corresponding to `allPx` and `labels`,
         * used for contextual information and to retrieve pixel colours.
         *
         * Note: The cluster labels of the pixels in `allPx` in `labels` should
         * all be the same, but do not actually need to match `id`,
         * as presently implemented
         */
        Superpixel(const pxind &id,
                pxind *&allPx,
                const pxind nPx,
                const pxind* const labels,
                ImageData &img
                );

        ~Superpixel();

        /*!
         * \brief Superpixel identifier
         * \return The ID of the superpixel
         */
        pxind label(void) const;

        /*!
         * \brief Average of pixel spatial coordinates in the superpixel
         * \param [out] p The pixel location of the superpixel's center
         *
         * Note that the superpixel's center is computed by the Superpixel::Superpixel()
         * constructor, rather than determined externally.
         */
        void centerPosition(QPoint& p) const;

        /*!
         * \brief Average of pixel colours in the superpixel
         * \param [out] c The colour superpixel's center, in the CIE L*a*b* colour space
         *
         * Note that the superpixel's center is computed by the Superpixel::Superpixel()
         * constructor, rather than determined externally.
         */
        void centerColor(QVector3D& c) const;

        /*!
         * \brief Average of pixel colours in the superpixel
         * \param [out] c The colour superpixel's center, in the RGB colour space
         *
         * Note that the superpixel's center is computed by the Superpixel::Superpixel()
         * constructor, rather than determined externally.
         */
        void centerColorRGB(QRgb& c) const;

        /*!
         * \brief Size
         * \return The number of pixels in the superpixel
         */
        qreal size(void) const;

        /*!
         * \brief Area to perimeter ratio
         * \return The ratio of the number of interior or boundary pixels
         * to the number of boundary pixels
         */
        qreal areaToPerimeterRatio(void) const;

        /*!
         * \brief Standard deviation of colours in the superpixel
         * \param [out] s The standard deviation of CIE L*a*b* colours in the superpixel
         */
        void standardColorDeviation(qreal& s) const;

        /*!
         * \brief Standard deviation of colours in the superpixel
         * \param [out] s The standard deviation, per-channel, of CIE L*a*b* colours in the superpixel
         */
        void standardColorDeviation(QVector3D& s) const;

        /*!
         * \brief Access the pixels on the interior of the superpixel
         * \param [out] px The array of interior pixels
         * \param [in] n The number of pixels in `px`
         */
        void interiorPixels(const pxind*& px, pxind& n) const;

        /*!
         * \brief Access the pixels on the boundary of the superpixel
         * \param [out] px The array of boundary pixels
         * \param [in] n The number of pixels in `px`
         */
        void boundaryPixels(const pxind*& px, pxind& n) const;

        /*!
         * \brief Access all pixels in the superpixel
         * \param [out] px The array of boundary and interior pixels
         * \param [in] n The number of pixels in `px`
         */
        void allPixels(const pxind*& px, pxind& n) const;

    protected:
        /*!
         * \brief Superpixel ID
         */
        pxind id;
        /*!
         * \brief The center of the superpixel, computed by averaging the positions
         * and colours of all pixels in the superpixel
         *
         * The colour is in the CIE L*a*b* colour space.
         */
        Center<QPoint> center;
        /*!
         * \brief The RGB colour of the superpixel center
         * \see Superpixel::center
         */
        QRgb centerRGB;
        /*!
         * \brief A list of all pixels in the superpixel
         */
        const pxind *allPx;
        /*!
         * \brief The number of pixels in the superpixel whose four neighbours
         * are all members of the superpixel
         */
        pxind nInteriorPixels;
        /*!
         * \brief The number of pixels in the superpixel whose four neighbours
         * include members of other superpixel(s), or who are located on the
         * image border
         */
        pxind nBoundaryPixels;
        /*!
         * \brief The total number of pixels in the superpixel
         * \see Superpixel::allPx
         */
        const pxind nPixels;
        /*!
         * \brief The standard deviation of CIE L*a*b*-space colours in the superpixel
         */
        qreal stdDevColor;
        /*!
         * \brief The standard deviation of CIE L*a*b*-space colours, per-channel, in the superpixel
         */
        QVector3D stdDevColorChannels;
    };

public:
    /*!
     * \brief Combine superpixel data into an object
     *
     * This object takes ownership of all input arguments, except for `img`,
     * if `shareImage` is `true`, and the constructor sets
     * the caller's pointers to null so that they are not deallocated by the caller.
     * \param [in] img The image corresponding to the superpixellation
     * \param [in] superpixelLabels An array with the same number of elements as
     * `img.pixelCount()` (ImageData::pixelCount()), where the element at index
     * `k` stores the superpixel ID of the pixel at 1D coordinate `k`.
     * \param [in] superpixels An array of superpixels corresponding to the data
     * in `superpixelLabels`.
     * \param [in] nSuperpixels The number of superpixels in the `superpixels` array
     * \param [in] shareImage If true, this object will not delete `img` when
     * deconstructed
     */
    Superpixellation(ImageData *& img,
            pxind *& superpixelLabels,
            Superpixel **& superpixels,
            const pxind &nSuperpixels,
            const bool shareImage = false
        );

    /*!
     * \brief Create an instance which stores shallow copies of the data
     * in an existing instance
     *
     * This constructor modifies the other instance so that none of its
     * members will be deleted by its destructor.
     *
     * This object's members will be deleted by its destructor only
     * if the other instance's members would have been be deleted by its destructor,
     * had the destructor been called prior to the call to this constructor.
     * \param [in,out] other The other instance
     */
    Superpixellation(Superpixellation &other);

    virtual ~Superpixellation(void);

    /*!
     * \brief Outputs the image corresponding to this Superpixellation,
     * and makes the caller responsible for deallocating the image data.
     * \param [out] img Superpixellation::img. Expected to be passed in as a null pointer.
     */
    void transferImageOwnership(ImageData *& img);

    // Data members
public:
    /*!
     * \brief The superpixels into which the image has been segmented
     */
    Superpixel const* const* const superpixels;
    /*!
     * \brief The image which was the basis for the segmentation
     */
    ImageData * const img;
    /*!
     * \brief An array storing the superpixel identifiers of each pixel.
     *
     * An element at index `k` stores the superpixel ID of the pixel at 1D coordinate `k`.
     */
    pxind const* const superpixelLabels;
    /*!
     * \brief The number of superpixels in the segmentation
     */
    const pxind nSuperpixels;

private:
    /*!
     * \brief Indicates whether or not Superpixellation::img is owned by this object,
     * for memory management purposes
     */
    bool shareImage;
protected:
    /*!
     * \brief Indicates whether or not any members are owned by this object,
     * for memory management purposes
     *
     * I should have used shared pointers throughout the project code
     * in order to avoid ad-hoc solutions like this. Most of the time,
     * I set pointers to zero to enforce transfers of ownership. It isn't possible
     * in this case, because the object is immutable (and all pointer members
     * are `const` pointers).
     */
    bool shareAll;
};

#endif // SUPERPIXELLATION_H
