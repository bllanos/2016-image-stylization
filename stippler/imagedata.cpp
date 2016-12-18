/*!
** \file imagedata.cpp
** \brief Implementation of the ImageData class.
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
** EasyRGB was used to validate the Processing code and also verify
** the D65 white point:
** - http://www.easyrgb.com/index.php?X=MATH&H=02#text2 (RGB to and from XYZ)
** - http://www.easyrgb.com/index.php?X=MATH&H=07#text7 (XYZ to and from CIE L*a*b*)
*/

#include <algorithm>
#include "imagedata.h"

ImageData::ImageData(const QImage &image) :
    r(0), g(0), bl(0), l(0), a(0), bs(0),
    w(image.width()), h(image.height()), nPixels(image.width() * image.height())
{
    QImage formattedImage = image;
    if(image.format() != QImage::Format_ARGB32) {
        formattedImage = image.convertToFormat(QImage::Format_ARGB32);
    }
    /* `static_cast<>()` will not work here because the pointers are pointing
     * to values of different sizes.
     */
    const QRgb *pixels = reinterpret_cast<const QRgb*>(formattedImage.constBits());

    r = new uchar[nPixels];
    g = new uchar[nPixels];
    bl = new uchar[nPixels];

    QRgb px = 0;
    for(pxind i = 0; i < nPixels; i += 1) {
        px = pixels[i];
        r[i] = qRed(px);
        g[i] = qGreen(px);
        bl[i] = qBlue(px);
    }
}

ImageData::ImageData(qreal *&lightness, const pxind width, const pxind height) :
    r(0), g(0), bl(0), l(0), a(0), bs(0),
    w(width), h(height), nPixels(width * height)
{
    l = lightness;
    lightness = 0;
    a = new qreal[nPixels];
    std::fill(a, a + nPixels, 0.0);
    bs = new qreal[nPixels];
    std::fill(bs, bs + nPixels, 0.0);
}

ImageData::ImageData(qreal *& lStar, qreal *& aStar, qreal *& bStar, const pxind width, const pxind height) :
    r(0), g(0), bl(0), l(lStar), a(aStar), bs(bStar),
    w(width), h(height), nPixels(width * height)
{
    lStar = 0;
    aStar = 0;
    bStar = 0;
}

ImageData::~ImageData() {
    if( r != 0 ) {
        delete [] r;
        r = 0;
    }
    if( g != 0 ) {
        delete [] g;
        g = 0;
    }
    if( bl != 0 ) {
        delete [] bl;
        bl = 0;
    }
    if( l != 0 ) {
        delete [] l;
        l = 0;
    }
    if( a != 0 ) {
        delete [] a;
        a = 0;
    }
    if( bs != 0 ) {
        delete [] bs;
        bs = 0;
    }
}

bool ImageData::toImage(QImage &image) {
    if(r == 0) {
        lab2rgb();
    }

    image = QImage(w, h, QImage::Format_ARGB32);

    pxind k = 0;
    for(pxind i = 0; i < h; i += 1) {
        for(pxind j = 0; j < w; j += 1) {
            image.setPixel(j, i, qRgb(r[k], g[k], bl[k]));
            k += 1;
        }
    }
    return !image.isNull();
}

const uchar * ImageData::red() {
    if(r == 0) {
        lab2rgb();
    }
    return r;
}

const uchar * ImageData::green() {
    if(g == 0) {
        lab2rgb();
    }
    return g;
}

const uchar * ImageData::blue() {
    if(bl == 0) {
        lab2rgb();
    }
    return bl;
}

const qreal * ImageData::lStar() {
    if(l == 0) {
        rgb2lab();
    }
    return l;
}

const qreal * ImageData::aStar() {
    if(a == 0) {
        rgb2lab();
    }
    return a;
}

const qreal * ImageData::bStar() {
    if(bs == 0) {
        rgb2lab();
    }
    return bs;
}

pxind ImageData::width() const {
    return w;
}

pxind ImageData::height() const {
    return h;
}

QSize ImageData::size() const {
    return QSize(w, h);
}

pxind ImageData::pixelCount() const {
    return nPixels;
}

bool ImageData::rgbPixelAt(const pxind &x, const pxind &y, uchar &pxr, uchar &pxg, uchar& pxb) {
    if(!checkXY(x, y)) {
        return false;
    }
    pxind k = xyToK(x, y);
    pxr = red()[k];
    pxg = green()[k];
    pxb = blue()[k];
    return true;
}

bool ImageData::rgbPixelAt(const pxind &k, uchar &pxr, uchar &pxg, uchar& pxb) {
    if(!checkK(k)) {
        return false;
    }
    pxr = red()[k];
    pxg = green()[k];
    pxb = blue()[k];
    return true;
}

bool ImageData::labPixelAt(const pxind &x, const pxind &y, qreal &pxl, qreal &pxa, qreal& pxb) {
    if(!checkXY(x, y)) {
        return false;
    }
    pxind k = xyToK(x, y);
    pxl = lStar()[k];
    pxa = aStar()[k];
    pxb = bStar()[k];
    return true;
}

bool ImageData::labPixelAt(const pxind &k, qreal &pxl, qreal &pxa, qreal& pxb) {
    if(!checkK(k)) {
        return false;
    }
    pxl = lStar()[k];
    pxa = aStar()[k];
    pxb = bStar()[k];
    return true;
}

pxind ImageData::xyToK(const pxind &x, const pxind &y) const {
    Q_ASSERT(checkXY(x, y));
    return w * y + x;
}

void ImageData::kToXY(const pxind &k, pxind &x, pxind &y) const {
    Q_ASSERT(checkK(k));
    x = k % w;
    y = k / w;
}

void ImageData::fourNeighbours(pxind (&neighbours)[4], pxind& nNeighbours, const pxind &k) const {
    pxind x = 0;
    pxind y = 0;
    kToXY(k, x, y);
    nNeighbours = 0;

    // Right
    pxind val =  x + 1;
    if( val < w ) {
        neighbours[nNeighbours++] = xyToK(val, y);
    }
    // Up
    val =  y - 1;
    if( val >= 0 ) {
        neighbours[nNeighbours++] = xyToK(x, val);
    }
    // Left
    val =  x - 1;
    if( val >= 0 ) {
        neighbours[nNeighbours++] = xyToK(val, y);
    }
    // Bottom
    val =  y + 1;
    if( val < h ) {
        neighbours[nNeighbours++] = xyToK(x, val);
    }
}

void ImageData::eightNeighbours(pxind (&neighbours)[8], pxind& nNeighbours, const pxind &k) const {
    pxind x = 0;
    pxind y = 0;
    kToXY(k, x, y);
    nNeighbours = 0;

    // Right
    pxind valX =  x + 1;
    pxind valY =  y;
    if( valX < w ) {
        neighbours[nNeighbours++] = xyToK(valX, valY);
    }
    // Up Right
    valY =  y - 1;
    if( valY >= 0 && valX < w ) {
        neighbours[nNeighbours++] = xyToK(valX, valY);
    }
    // Up
    valX =  x;
    if( valY >= 0) {
        neighbours[nNeighbours++] = xyToK(valX, valY);
    }
    // Up Left
    valX =  x - 1;
    if( valX >= 0 && valY >= 0 ) {
        neighbours[nNeighbours++] = xyToK(valX, valY);
    }
    // Left
    valY =  y;
    if( valX >= 0 ) {
        neighbours[nNeighbours++] = xyToK(valX, valY);
    }
    // Bottom Left
    valY =  y + 1;
    if( valX >= 0 && valY < h ) {
        neighbours[nNeighbours++] = xyToK(valX, valY);
    }
    // Bottom
    valX =  x;
    if( valY < h ) {
        neighbours[nNeighbours++] = xyToK(valX, valY);
    }
    // Bottom Right
    valX =  x + 1;
    if( valX < w && valY < h ) {
        neighbours[nNeighbours++] = xyToK(valX, valY);
    }
}

void ImageData::eightNeighboursReplicate(pxind (&neighbours)[8], const pxind &k) const {
    pxind x = 0;
    pxind y = 0;
    kToXY(k, x, y);

    // Right
    pxind valX =  x + 1;
    pxind valY =  y;
    if( valX >= w ) valX = x;
    neighbours[0] = xyToK(valX, valY);
    // Up Right
    if( y > 0 ) valY =  y - 1;
    neighbours[1] = xyToK(valX, valY);
    // Up
    valX =  x;
    neighbours[2] = xyToK(valX, valY);
    // Up Left
    valX =  x - 1;
    if( valX < 0 ) valX = x;
    neighbours[3] = xyToK(valX, valY);
    // Left
    valY =  y;
    neighbours[4] = xyToK(valX, valY);
    // Bottom Left
    valY =  y + 1;
    if( valY >= h ) valY =  y;
    neighbours[5] = xyToK(valX, valY);
    // Bottom
    valX =  x;
    neighbours[6] = xyToK(valX, valY);
    // Bottom Right
    valX =  x + 1;
    if( valX >= w ) valX = x;
    neighbours[7] = xyToK(valX, valY);
}

void ImageData::neighbours(
        pxind *&neighbours,
        pxind& nNeighbours,
        const pxind &centerX,
        const pxind &centerY,
        const pxind& dx,
        const pxind& dy
    ) const {

    nNeighbours = 0;
    pxind startX = centerX - dx;
    if( startX < 0 ) {
        startX = 0;
    }
    pxind startY = centerY - dy;
    if( startY < 0 ) {
        startY = 0;
    }
    pxind endX = centerX + dx;
    if( endX >= w ) {
        endX = w - 1;
    }
    pxind endY = centerY + dy;
    if( endY >= h ) {
        endY = h - 1;
    }

    // Loop over neighbours
    for(pxind xi = startX; xi <= endX; xi += 1) {
        for(pxind yi = startY; yi <= endY; yi += 1) {
            neighbours[nNeighbours++] = xyToK(xi, yi);
        }
    }
}

void ImageData::sobelLabAt(const pxind &k, QVector2D &gl, QVector2D &ga, QVector2D &gb) {
    pxind neighbours[8] = {0};
    eightNeighboursReplicate(neighbours, k);
    const qreal* channels[] = { lStar(), aStar(), bStar() };
    QVector2D* output[] = { &gl, &ga, &gb };
    for(int j = 0; j < 3; j += 1) {
        output[j]->setX(
                (channels[j][neighbours[0]] * -2) +
                (channels[j][neighbours[1]] * -1) +
                (channels[j][neighbours[3]] * 1) +
                (channels[j][neighbours[4]] * 2) +
                (channels[j][neighbours[5]] * 1) +
                (channels[j][neighbours[7]] * -1)
            );
        output[j]->setY(
                (channels[j][neighbours[1]] * 1) +
                (channels[j][neighbours[2]] * 2) +
                (channels[j][neighbours[3]] * 1) +
                (channels[j][neighbours[5]] * -1) +
                (channels[j][neighbours[6]] * -2) +
                (channels[j][neighbours[7]] * -1)
            );
    }
}

void ImageData::sobelLabAt(const pxind &k, QVector2D &g) {
    QVector2D gl, ga, gb;
    sobelLabAt(k, gl, ga, gb);
    g.setX(gl.x() + ga.x() + gb.x());
    g.setY(gl.y() + ga.y() + gb.y());
}

bool ImageData::checkXY(const pxind &x, const pxind &y) const {
    return (x >= 0 && x < w && y >= 0 && y < h );
}

bool ImageData::checkK(const pxind &k) const {
    return (k >= 0 && k < nPixels);
}

void ImageData::rgb2lab() {
    Q_ASSERT(r != 0);
    Q_ASSERT(g != 0);
    Q_ASSERT(bl != 0);
    Q_ASSERT(l == 0);
    Q_ASSERT(a == 0);
    Q_ASSERT(bs == 0);

    l = new qreal[nPixels];
    a = new qreal[nPixels];
    bs = new qreal[nPixels];

    rgb2xyz();
    xyz2lab();
}

void ImageData::lab2rgb() {
    Q_ASSERT(r == 0);
    Q_ASSERT(g == 0);
    Q_ASSERT(bl == 0);
    Q_ASSERT(l != 0);
    Q_ASSERT(a != 0);
    Q_ASSERT(bs != 0);

    r = new uchar[nPixels];
    g = new uchar[nPixels];
    bl = new uchar[nPixels];

    /* This isn't necessary in rgb2lab() because L*a*b* channels are
     * already in floating-point format, and so can be used for both
     * XYZ and CIE L*a*b* values in turn during the conversion.
     */
    qreal* xyz = new qreal[nPixels*3];

    lab2xyz(xyz);
    xyz2rgb(xyz);

    delete [] xyz;
}

void ImageData::rgb2xyz() {
    qreal pxRGB[3] = {0};
    qreal pxXYZ[3] = {0};
    for(pxind i = 0; i < nPixels; i += 1) {
        // RGB values
        pxRGB[0] = r[i];
        pxRGB[1] = g[i];
        pxRGB[2] = bl[i];
        rgb2xyz(pxXYZ, pxRGB);
        l[i] = pxXYZ[0];
        a[i] = pxXYZ[1];
        bs[i] = pxXYZ[2];
    }
}

void ImageData::xyz2lab() {
    // Convert every pixel
    qreal pxXYZ[3] = {0};
    qreal pxLAB[3] = {0};
    for(pxind i = 0; i < nPixels; i +=1){
        pxXYZ[0] = l[i];
        pxXYZ[1] = a[i];
        pxXYZ[2] = bs[i];
        xyz2lab(pxLAB, pxXYZ);
        l[i] = pxLAB[0];
        a[i] = pxLAB[1];
        bs[i] = pxLAB[2];
    }
}

void ImageData::lab2xyz(qreal *&xyz) {
    // Convert every pixel
    qreal pxXYZ[3] = {0};
    qreal pxLAB[3] = {0};
    pxind i3 = 0;
    for(pxind i = 0; i < nPixels; i +=1){
        pxLAB[0] = l[i];
        pxLAB[1] = a[i];
        pxLAB[2] = bs[i];
        lab2xyz(pxXYZ, pxLAB);
        i3 = i * 3;
        xyz[i3] = pxXYZ[0];
        xyz[i3 + 1] = pxXYZ[1];
        xyz[i3 + 2] = pxXYZ[2];
    }
}

void ImageData::xyz2rgb(qreal *&xyz) {
    qreal pxXYZ[3] = {0};
    qreal pxRGBReal[3] = {0};
    uchar pxRGBInt[3] = {0};
    pxind i3 = 0;
    // Convert every pixel
    for(pxind i = 0; i < nPixels; i +=1){
        i3 = i * 3;
        pxXYZ[0] = xyz[i3];
        pxXYZ[1] = xyz[i3 + 1];
        pxXYZ[2] = xyz[i3 + 2];
        xyz2rgb(pxRGBReal, pxXYZ);
        rgbReal2rgbInt(pxRGBInt, pxRGBReal);
        r[i] = pxRGBInt[0];
        g[i] = pxRGBInt[1];
        bl[i] = pxRGBInt[2];
    }
}
