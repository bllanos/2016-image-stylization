#ifndef IMAGEDATA_H
#define IMAGEDATA_H

/*!
** \file imagedata.h
** \brief Definition of the ImageData class.
**
** ### About
** Created for: COMP4905A Honours Project\n
** Fall 2016\n
** Bernard Llanos\n
** Supervised by Dr. David Mould\n
** School of Computer Science, Carleton University
**
** ## Primary basis
** Processing Java code for colour space conversions by Bernard Llanos,
** created during an NSERC USRA at the Graphics, Imaging and Games Lab,
** Carleton University, May-August 2013 (supervised by Dr. David Mould).
**
** The Processing code has the following references:
** - http://www.brucelindbloom.com/index.html?Eqn_RGB_to_XYZ.html
** - http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
** - Verification of the D65 white point:
**   http://en.wikipedia.org/wiki/CIE_Standard_Illuminant_D65#Definition
**
** Concerning how to handle out-of-gamut colours
** - http://www.poynton.com/notes/colour_and_gamma/ColorFAQ.html
** - http://python-colormath.readthedocs.io/en/latest/conversions.html#rgb-conversions-and-out-of-gamut-coordinates
**
** EasyRGB was used to validate the Processing code and also verify
** the D65 white point:
** - http://www.easyrgb.com/index.php?X=MATH&H=02#text2 (RGB to and from XYZ)
** - http://www.easyrgb.com/index.php?X=MATH&H=07#text7 (XYZ to and from CIE L*a*b*)
*/

#include <math.h>
#include <QVector2D>
#include <QImage>

/*!
  \brief The maximum possible RGB channel value
 */
#define IMAGEDATA_MAX_RGB 255

/*!
  \brief The number of possible RGB channel values
 */
#define IMAGEDATA_RGB_RANGE 256

/*!
  \brief The maximum possible CIE L*a*b* lightness channel value
 */
#define IMAGEDATA_MAX_LIGHTNESS 100.0

/*!
  \brief The minimum possible CIE L*a*b* lightness channel value
 */
#define IMAGEDATA_MIN_LIGHTNESS 0.0

/*!
 * \brief The range of CIE L*a*b* lightness channel values
 * from #IMAGEDATA_MIN_LIGHTNESS to #IMAGEDATA_MAX_LIGHTNESS
 */
#define IMAGEDATA_RANGE_LIGHTNESS (IMAGEDATA_MAX_LIGHTNESS - IMAGEDATA_MIN_LIGHTNESS)

/*!
 * \brief A type representing an image size in pixels, or the index of a pixel
 *
 * This is always a **signed** integer type.
 */
typedef qint32 pxind;

/*!
 * \brief A convenience class for working with image data, in the RGB and
 * CIE L*a*b* colour spaces.
 */
class ImageData
{
public:
    /*!
     * \brief Split an image into colour channels
     * \param [in] image The image from which to extract colour data
     */
    ImageData(const QImage &image);

    /*!
     * \brief Create a greyscale image
     * \param [in, out] lightness The image lightness component in the L*a*b* colour
     * space. The a* and b* values of the image are assumed to be zero.
     * The object takes ownership of `lightness`, and sets `lightness` to a null
     * pointer.
     * \param [in] width Image width
     * \param [in] height Image height
     */
    ImageData(qreal *& lightness, const pxind width, const pxind height);

    /*!
     * \brief Create an image from CIE L*a*b* colour channels
     * The object takes ownership of the image channels, and sets them to null
     * pointers.
     * \param [in, out] lStar Lightness channel, L*
     * \param [in, out] aStar a* colour channel
     * \param [in, out] bStar b* colour channel
     * \param [in] width Image width
     * \param [in] height Image height
     */
    ImageData(qreal *& lStar, qreal *& aStar, qreal *& bStar, const pxind width, const pxind height);

    ~ImageData();

    /*!
     * \brief Returns a copy of this object's data as a QImage object.
     * \param [out] image A displayable image
     * \return Success (true) or failure (false)
     */
    bool toImage(QImage &image);

    /*!
     * \brief Red channel, in the form of 0-255 values.
     */
    const uchar * red();
    /*!
     * \brief Green channel, in the form of 0-255 values.
     */
    const uchar * green();
    /*!
     * \brief Blue channel, in the form of 0-255 values.
     */
    const uchar * blue();
    /*!
     * \brief L channel from the CIE L*a*b* colour space, in the form of 0-100.0 values.
     */
    const qreal * lStar();
    /*!
     * \brief a channel from the CIE L*a*b* colour space
     */
    const qreal * aStar();
    /*!
     * \brief b channel from the CIE L*a*b* colour space
     */
    const qreal * bStar();
    /*!
     * \brief Image width
     *
     * The pixel at position `(x, y)` has a red value of `red[w * y + x]`,
     * where the origin is at the top-left corner of the image.
     */
    pxind width() const;
    /*!
     * \brief Image height
     *
     * The pixel at position `(x, y)` has a red value of `red[w * y + x]`,
     * where the origin is at the top-left corner of the image.
     */
    pxind height() const;
    /*!
     * \brief Image width and height in pixels
     * \return The image dimensions
     */
    QSize size() const;
    /*!
     * \brief The number of pixels in the image
     */
    pxind pixelCount() const;

    /*!
     * \brief Retrieve the RGB channel values of the pixel at the given 2D coordinates
     * \param [in] x Pixel x-coordinate
     * \param [in] y Pixel y-coordinate
     * \param [out] pxr Pixel red channel value
     * \param [out] pxg Pixel green channel value
     * \param [out] pxb Pixel blue channel value
     * \return Success (true) or failure (false). A failure result is returned
     * if the pixel coordinates are invalid.
     */
    bool rgbPixelAt(const pxind &x, const pxind &y, uchar &pxr, uchar &pxg, uchar& pxb);
    /*!
     * \brief Retrieve the RGB channel values of the pixel at the given 1D coordinate
     * \param [in] k Pixel index, equal to `width() * y + x`
     * \param [out] pxr Pixel red channel value
     * \param [out] pxg Pixel green channel value
     * \param [out] pxb Pixel blue channel value
     * \return Success (true) or failure (false). A failure result is returned
     * if the pixel coordinate is invalid.
     */
    bool rgbPixelAt(const pxind &k, uchar &pxr, uchar &pxg, uchar& pxb);
    /*!
     * \brief Retrieve the CIE L*a*b* channel values of the pixel at the given 2D coordinates
     * \param [in] x Pixel x-coordinate
     * \param [in] y Pixel y-coordinate
     * \param [out] pxl Pixel lightness channel value
     * \param [out] pxa Pixel a* channel value
     * \param [out] pxb Pixel b* channel value
     * \return Success (true) or failure (false). A failure result is returned
     * if the pixel coordinates are invalid.
     */
    bool labPixelAt(const pxind &x, const pxind &y, qreal &pxl, qreal &pxa, qreal& pxb);
    /*!
     * \brief Retrieve the CIE L*a*b* channel values of the pixel at the given 1D coordinate
     * \param [in] k Pixel index, equal to `width() * y + x`
     * \param [out] pxl Pixel lightness channel value
     * \param [out] pxa Pixel a* channel value
     * \param [out] pxb Pixel b* channel value
     * \return Success (true) or failure (false). A failure result is returned
     * if the pixel coordinate is invalid.
     */
    bool labPixelAt(const pxind &k, qreal &pxl, qreal &pxa, qreal& pxb);

    /*!
     * \brief Convert pixel 2D coordinates to a 1D index
     *
     * Note that this function does not validate the input 2D pixel coordinates
     * \param [in] x Pixel x-coordinate, in the range [0, width() - 1]
     * \param [out] y Pixel y-coordinate, in the range [0, height() - 1]
     * \return The equivalent 1D pixel coordinate.
     */
    pxind xyToK(const pxind &x, const pxind &y) const;

    /*!
     * \brief Convert pixel 1D coordinate to 2D coordinates
     *
     * Note that this function does not validate the input 1D pixel coordinate
     * \param [in] k Pixel index
     * \param [out] x Pixel x-coordinate
     * \param [out] y Pixel y-coordinate
     */
    void kToXY(const pxind &k, pxind &x, pxind &y) const;

    /*!
     * \brief Locate the four nearest neighbours of a pixel
     *
     * Neighbours are output in the following order (i.e. counter-clockwise):
     * right, up, left, down
     * \param [out] neighbours The indices of the neighbouring pixels
     * \param [out] nNeighbours The actual number of neighbours, which depends on whether or not
     * the pixel is on the image border.
     * \param [in] k The index of the pixel whose neighbours are to be located
     */
    void fourNeighbours(pxind (&neighbours)[4], pxind& nNeighbours, const pxind &k) const;

    /*!
     * \brief Locate the eight nearest neighbours of a pixel
     *
     * Neighbours are output in the following order (i.e. counter-clockwise):
     * right, up right, up, up left, left, down left, down, down right
     * \param [out] neighbours The indices of the neighbouring pixels
     * \param [out] nNeighbours The actual number of neighbours, which depends on whether or not
     * the pixel is on the image border.
     * \param [in] k The index of the pixel whose neighbours are to be located
     */
    void eightNeighbours(pxind (&neighbours)[8], pxind& nNeighbours, const pxind &k) const;

    /*!
     * \brief Locate the eight nearest neighbours of a pixel, with replication
     * of border pixels to locations outside the image
     *
     * Neighbours are output in the following order (i.e. counter-clockwise):
     * right, up right, up, up left, left, down left, down, down right
     *
     * If a given neighbour does not exist because the pixel is on the border
     * of the image, the index of the pixel is used in its place.
     * \param [out] neighbours The indices of the neighbouring pixels
     * \param [in] k The index of the pixel whose neighbours are to be located
     */
    void eightNeighboursReplicate(pxind (&neighbours)[8], const pxind &k) const;

    /*!
     * \brief Find the indices of pixels in a rectangular region of the image
     *
     * Neighbours are output in left-to-right, top-to-bottom order
     * \param [out] neighbours The indices of pixels in the rectangular region.
     * The caller must allocate this array to have size at least
     * `(2 * dx + 1) * (2 * dy + 1)`.
     * \param [out] nNeighbours The number of pixels in the rectangular region.
     * (The rectangular region is cropped such that it does not protrude
     * outside the image.)
     * \param [in] centerX The x-coordinate of the center of the rectangle
     * \param [in] centerY The y-coordinate of the center of the rectangle
     * \param [in] dx Half of the width of the rectangle. The full width of the
     * rectangle is `2 * dx + 1` pixels, such that the rectangle extends `dx`
     * pixels to the left and right of the central pixel (assuming the central
     * pixel is at least `dx + 1` pixels away from the image border).
     * \param [in] dy Half of the height of the rectangle. The full height of the
     * rectangle is `2 * dy + 1` pixels, such that the rectangle extends `dy`
     * pixels above and below the central pixel (assuming the central
     * pixel is at least `dy + 1` pixels away from the image border).
     */
    void neighbours(
            pxind *&neighbours,
            pxind& nNeighbours,
            const pxind &centerX,
            const pxind &centerY,
            const pxind& dx,
            const pxind& dy
        ) const;

    /*!
     * \brief The Sobel gradient operator evaluated at each colour channel of a pixel
     *
     * The calculation is performed in the CIE L*a*b* colour space.
     *
     * The output values are vectors containing the x and y-components of the Sobel gradients.
     * \param [in] k The index of the pixel
     * \param [out] gl The Sobel gradient operator evaluated on the lightness channel
     * of the image at the given pixel
     * \param [out] ga The Sobel gradient operator evaluated on the a* channel
     * of the image at the given pixel
     * \param [out] gb The Sobel gradient operator evaluated on the b* channel
     * of the image at the given pixel
     */
    void sobelLabAt(const pxind &k, QVector2D &gl, QVector2D &ga, QVector2D &gb);

    /*!
     * \brief The Sobel gradient operator evaluated at a pixel
     *
     * The calculation is performed in the CIE L*a*b* colour space. The Sobel
     * gradient is determined separately for each colour channel, and the
     * output is the sum of the vectors for the individual channels.
     *
     * \param [in] k The index of the pixel
     * \param [out] g A vector containing the x and y-components of the Sobel gradient
     */
    void sobelLabAt(const pxind &k, QVector2D &g);

    // Colour space conversion functions
public:
    /*!
     * \brief Convert a pixel from the RGB colour space to the CIE L*a*b* colour
     * space
     * \param [out] lab The CIE L*a*b* colour value
     * \param [in] rgb The RGB colour value
     */
    static void rgb2lab(qreal (&lab)[3], const uchar (&rgb)[3]);
    /*!
     * \brief Convert a pixel from the CIE L*a*b* colour space to the RGB colour
     * space
     * \param [out] rgb The RGB colour value
     * \param [in] lab The CIE L*a*b* colour value
     */
    static void lab2rgb(uchar (&rgb)[3], const qreal (&lab)[3]);

private:
    static void rgb2xyz(qreal (&xyz)[3], qreal (&rgb)[3]);
    static void xyz2lab(qreal (&lab)[3], const qreal (&xyz)[3]);
    static void lab2xyz(qreal (&xyz)[3], const qreal (&lab)[3]);
    static void xyz2rgb(qreal (&rgb)[3], const qreal (&xyz)[3]);
    static void rgbReal2rgbInt(uchar (&rgbInt)[3], const qreal (&rgbReal)[3]);

private:
    void rgb2lab();
    void lab2rgb();

    bool checkXY(const pxind &x, const pxind &y) const;
    bool checkK(const pxind &k) const;

    // Helper functions

    /*!
     * \brief Produces XYZ colour values in the range [0, 1]
     */
    void rgb2xyz();
    void xyz2lab();
    void lab2xyz(qreal*& xyz);
    void xyz2rgb(qreal*& xyz);

    // Data members
private:
    uchar * r;
    uchar * g;
    uchar * bl;
    qreal * l;
    qreal * a;
    qreal * bs;
    const pxind w;
    const pxind h;
    const pxind nPixels;
};

// Constants
/*!
  \brief XYZ to CIE L*a*b* colour space conversion parameter 'epsilon'

  From http://www.brucelindbloom.com/index.html?Eqn_RGB_to_XYZ.html
  */
#define EPS   (216.0/24389.0)
/*!
  \brief XYZ to CIE L*a*b* colour space conversion parameter 'kappa'

  From http://www.brucelindbloom.com/index.html?Eqn_RGB_to_XYZ.html
  */
#define KAPPA (24389.0/27.0)
/*!
  \brief Product of the XYZ to CIE L*a*b* colour space conversion parameters
  'epsilon' and 'kappa'

  From http://www.brucelindbloom.com/index.html?Eqn_RGB_to_XYZ.html
  */
#define KAPPA_EPS (216.0/27.0)
/*!
 * \brief Reference white point is CIE Standard Illuminant D65
 *
 * See http://en.wikipedia.org/wiki/CIE_Standard_Illuminant_D65#Definition
 */
#define REF_WHITE {95.047/100.0, 100.00/100.0, 108.883/100.0}

inline void ImageData::rgb2lab(qreal (&lab)[3], const uchar (&rgb)[3]) {
    qreal xyz[3] = {0};
    qreal rgbReal[3] = {
        static_cast<qreal>(rgb[0]),
        static_cast<qreal>(rgb[1]),
        static_cast<qreal>(rgb[2])
    };
    rgb2xyz(xyz, rgbReal);
    xyz2lab(lab, xyz);
}

inline void ImageData::lab2rgb(uchar (&rgb)[3], const qreal (&lab)[3]) {
    qreal xyz[3] = {0};
    qreal rgbReal[3] = {0};
    lab2xyz(xyz, lab);
    xyz2rgb(rgbReal, xyz);
    rgbReal2rgbInt(rgb, rgbReal);
}

inline void ImageData::rgb2xyz(qreal (&xyz)[3], qreal (&rgb)[3]) {
    // Assuming sRGB Companding
    for(int j = 0; j < 3; j += 1){
        rgb[j] /= IMAGEDATA_MAX_RGB;
        if(rgb[j] > 0.04045){
            rgb[j] = pow((rgb[j]+0.055)/1.055,2.4);
        } else {
            rgb[j] /= 12.92;
        }
    }
    // Conversion matrix, assuming sRGB (with D65 reference white)
    xyz[0] = 0.4124564*rgb[0] + 0.3575761*rgb[1] + 0.1804375*rgb[2];
    xyz[1] = 0.2126729*rgb[0] + 0.7151522*rgb[1] + 0.0721750*rgb[2];
    xyz[2] = 0.0193339*rgb[0] + 0.1191920*rgb[1] + 0.9503041*rgb[2];
}

inline void ImageData::xyz2lab(qreal (&lab)[3], const qreal (&xyz)[3]) {
    qreal refWhite[] = REF_WHITE;
    qreal pxXYZ[3] = {0};
    for(int j = 0; j < 3; j +=1){
        pxXYZ[j] = xyz[j]/refWhite[j];
        if(pxXYZ[j] > EPS){
            pxXYZ[j] = pow(pxXYZ[j],1.0/3.0);
        } else {
            pxXYZ[j] = (KAPPA*pxXYZ[j]+16)/116.0;
        }
    }
    lab[0] = 116*pxXYZ[1]-16;
    lab[1] = 500*(pxXYZ[0]-pxXYZ[1]);
    lab[2] = 200*(pxXYZ[1]-pxXYZ[2]);
}

inline void ImageData::lab2xyz(qreal (&xyz)[3], const qreal (&lab)[3]) {
    qreal refWhite[] = REF_WHITE;
    qreal pxXYZ[3] = {0};
    pxXYZ[1] = (lab[0]+16)/116;
    pxXYZ[0] = lab[1]/500+pxXYZ[1];
    pxXYZ[2] = pxXYZ[1]-lab[2]/200;
    // The next step is different for y_r relative to x_r and z_r
    for(int j = 0; j < 3; j +=2){
        if(pow(pxXYZ[j],3) > EPS){
            pxXYZ[j] = pow(pxXYZ[j],3);
        } else {
            pxXYZ[j] = (116*pxXYZ[j]-16)/KAPPA;
        }
    }
    // Version for y_r
    if(lab[0] > KAPPA_EPS){
        pxXYZ[1] = pow((lab[0]+16)/116,3);
    } else {
        pxXYZ[1] = lab[0]/KAPPA;
    }
    for(int j = 0; j < 3; j +=1){
        xyz[j] = pxXYZ[j]*refWhite[j];
    }
}

inline void ImageData::xyz2rgb(qreal (&rgb)[3], const qreal (&xyz)[3]) {
    rgb[0] =  3.2404542*xyz[0] + -1.5371385*xyz[1] + -0.4985314*xyz[2];
    rgb[1] = -0.9692660*xyz[0] +  1.8760108*xyz[1] +  0.0415560*xyz[2];
    rgb[2] =  0.0556434*xyz[0] + -0.2040259*xyz[1] +  1.0572252*xyz[2];

    // Nonlinearization, assuming sRGB companding
    for(int j = 0; j < 3; j +=1){
        if(rgb[j] > 0.0031308){
            rgb[j] = 1.055*pow(rgb[j],1.0/2.4)-0.055;
        } else {
            rgb[j] = 12.92*rgb[j];
        }
        rgb[j] *= IMAGEDATA_RGB_RANGE;
    }
}

inline void ImageData::rgbReal2rgbInt(uchar (&rgbInt)[3], const qreal (&rgbReal)[3]) {
    for(int j = 0; j < 3; j +=1){
        // Use clamping to deal with out-of-gamut colours
        if(rgbReal[j] > IMAGEDATA_MAX_RGB) {
            rgbInt[j] = IMAGEDATA_MAX_RGB;
        } else if(rgbReal[j] < 0) {
            rgbInt[j] = 0;
        } else {
            rgbInt[j] = static_cast<uchar>(floor(rgbReal[j]));
        }
    }
}

#endif // IMAGEDATA_H
