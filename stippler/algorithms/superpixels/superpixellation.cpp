/*!
** \file superpixellation.cpp
** \brief Implementation of the Superpixellation class.
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

#include <math.h>
#include "superpixellation.h"

Superpixellation::Superpixel::Superpixel(const pxind &l,
        pxind *&allPixels,
        const pxind nPx,
        const pxind * const labels,
        ImageData &img
    ) :
    id(l),
    center(),
    centerRGB(0),
    allPx(allPixels),
    nInteriorPixels(0),
    nBoundaryPixels(0),
    nPixels(nPx),
    stdDevColor(0.0),
    stdDevColorChannels()
{
    allPixels = 0; // Transfer ownership

    // Compute superpixel center and boundary/interior pixel classifications
    pxind xc = 0, yc = 0, xi = 0, yi = 0;
    qreal lc = 0.0, ac = 0.0, bc = 0.0;
    qreal redC = 0, greenC = 0, blueC = 0;
    const qreal *lChannel = img.lStar();
    const qreal *aChannel = img.aStar();
    const qreal *bChannel = img.bStar();
    const uchar *redChannel = img.red();
    const uchar *greenChannel = img.green();
    const uchar *blueChannel = img.blue();

    bool *isBoundary = new bool[nPixels];
    std::fill(isBoundary, isBoundary + nPixels, false);
    pxind neighbours[4] = {0};
    pxind nNeighbours = 0;
    pxind px = 0;
    pxind label = SUPERPIXELLATION_NONE_LABEL;
    for(pxind i = 0; i < nPixels; i += 1) {
        px = allPx[i];
        img.kToXY(px, xi, yi);
        xc += xi;
        yc += yi;
        lc += lChannel[px];
        ac += aChannel[px];
        bc += bChannel[px];
        redC += static_cast<qreal>(redChannel[px]);
        greenC += static_cast<qreal>(greenChannel[px]);
        blueC += static_cast<qreal>(blueChannel[px]);
        img.fourNeighbours(neighbours, nNeighbours, px);
        if( nNeighbours < 4 ) {
            isBoundary[i] = true;
            nBoundaryPixels += 1;
        } else {
            label = labels[px];
            for(pxind j = 0; j < 4; j += 1) {
                if(label != labels[neighbours[j]]) {
                    isBoundary[i] = true;
                    nBoundaryPixels += 1;
                    break;
                }
            }
        }
    }
    xc = round(static_cast<qreal>(xc) / static_cast<qreal>(nPixels));
    if(xc < 0) xc = 0;
    if(xc >= img.width()) xc = img.width();
    yc = round(static_cast<qreal>(yc) / static_cast<qreal>(nPixels));
    if(yc < 0) yc = 0;
    if(yc >= img.height()) yc = img.height();
    center.position.setX(xc);
    center.position.setY(yc);
    lc /= static_cast<qreal>(nPixels);
    ac /= static_cast<qreal>(nPixels);
    bc /= static_cast<qreal>(nPixels);
    center.color.setX(lc);
    center.color.setY(ac);
    center.color.setZ(bc);

    redC /= static_cast<qreal>(nPixels);
    if(redC > IMAGEDATA_MAX_RGB) {
        redC = IMAGEDATA_MAX_RGB;
    } else {
        redC = floor(redC);
    }
    greenC /= static_cast<qreal>(nPixels);
    if(greenC > IMAGEDATA_MAX_RGB) {
        greenC = IMAGEDATA_MAX_RGB;
    } else {
        greenC = floor(greenC);
    }
    blueC /= static_cast<qreal>(nPixels);
    if(blueC > IMAGEDATA_MAX_RGB) {
        blueC = IMAGEDATA_MAX_RGB;
    } else {
        blueC = floor(blueC);
    }
    centerRGB = qRgb(static_cast<int>(redC), static_cast<int>(greenC), static_cast<int>(blueC));

    // Arrange pixels into interior and boundary pixel sub-arrays
    nInteriorPixels = nPixels - nBoundaryPixels;
    pxind *orderedPixels = new pxind[nPixels];
    pxind *boundaryPointer = orderedPixels;
    pxind *interiorPointer = orderedPixels + nBoundaryPixels;
    for(pxind i = 0; i < nPixels; i += 1) {
        if(isBoundary[i]) {
            *boundaryPointer = allPx[i];
            boundaryPointer++;
        } else {
            *interiorPointer = allPx[i];
            interiorPointer++;
        }
    }
    delete [] isBoundary;
    isBoundary = 0;
    delete [] allPx;
    allPx = orderedPixels;
    orderedPixels = 0;

    // Calculate the standard deviation of colour
    if( nPixels > 1 ) { // Avoid division by zero
        qreal dli = 0.0, dai = 0.0, dbi = 0.0;
        qreal dlSum = 0.0, daSum = 0.0, dbSum = 0.0;
        for(pxind i = 0; i < nPixels; i += 1) {
            px = allPx[i];
            dli = lc - lChannel[px];
            dli *= dli;
            dlSum += dli;
            dai = ac - aChannel[px];
            dai *= dai;
            daSum += dai;
            dbi = bc - bChannel[px];
            dbi *= dbi;
            dbSum += dbi;
            stdDevColor += dli + dai + dbi;
        }
        stdDevColor = sqrt(stdDevColor / (nPixels - 1));
        stdDevColorChannels.setX(sqrt(dlSum / (nPixels-1)));
        stdDevColorChannels.setY(sqrt(daSum / (nPixels-1)));
        stdDevColorChannels.setZ(sqrt(dbSum / (nPixels-1)));
    }
}

Superpixellation::Superpixel::~Superpixel() {
    if(allPx != 0) {
        delete [] allPx;
        allPx = 0;
    }
}

pxind Superpixellation::Superpixel::label(void) const {
    return id;
}

void Superpixellation::Superpixel::centerPosition(QPoint& p) const {
    p = center.position;
}

void Superpixellation::Superpixel::centerColor(QVector3D& c) const {
    c = center.color;
}

void Superpixellation::Superpixel::centerColorRGB(QRgb& c) const {
    c = centerRGB;
}

qreal Superpixellation::Superpixel::size(void) const {
    return static_cast<qreal>(nPixels);
}

qreal Superpixellation::Superpixel::areaToPerimeterRatio(void) const {
    return static_cast<qreal>(nPixels) / static_cast<qreal>(nBoundaryPixels);
}

void Superpixellation::Superpixel::standardColorDeviation(qreal &s) const {
    s = stdDevColor;
}

void Superpixellation::Superpixel::standardColorDeviation(QVector3D &s) const {
    s = stdDevColorChannels;
}

void Superpixellation::Superpixel::interiorPixels(const pxind*& px, pxind& n) const {
    px = allPx + nBoundaryPixels;
    n = nInteriorPixels;
}

void Superpixellation::Superpixel::boundaryPixels(const pxind*& px, pxind& n) const {
    px = allPx;
    n = nBoundaryPixels;
}

void Superpixellation::Superpixel::allPixels(const pxind*& px, pxind& n) const {
    px = allPx;
    n = nPixels;
}

Superpixellation::Superpixellation(
        ImageData *& i,
        pxind *&sLabels,
        Superpixel **&s,
        const pxind& n,
        const bool sI) :
    superpixels(s),
    img(i),
    superpixelLabels(sLabels),
    nSuperpixels(n),
    shareImage(sI),
    shareAll(false)
{
    if(!shareImage) {
        i = 0;
    }
    sLabels = 0;
    s = 0;
}

Superpixellation::Superpixellation(Superpixellation& other) :
    superpixels(other.superpixels),
    img(other.img),
    superpixelLabels(other.superpixelLabels),
    nSuperpixels(other.nSuperpixels),
    shareImage(other.shareImage),
    shareAll(other.shareAll)
{
    other.shareAll = true;
}

Superpixellation::~Superpixellation(void) {
    if(!shareAll) {
        if(superpixels != 0) {
            for(pxind i = 0; i < nSuperpixels; i += 1) {
                delete superpixels[i];
            }
            delete [] superpixels;
        }
        if(!shareImage && img != 0) {
            delete img;
        }
        if(superpixelLabels != 0) {
            delete [] superpixelLabels;
        }
    }
}

void Superpixellation::transferImageOwnership(ImageData *& i) {
    Q_ASSERT(i == 0);
    i = img;
    shareImage = true;
}
