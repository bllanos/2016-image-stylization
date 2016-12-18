/*!
** \file filteredsuperpixellation.cpp
** \brief Implementation of the FilteredSuperpixellation class.
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

#include "filteredsuperpixellation.h"

FilteredSuperpixellation::FilteredSuperpixellation(
        ImageData *& i,
        pxind *& sL,
        Superpixel **& s,
        const pxind &nS,
        bool *& sS,
        bool *& sP
    ) : Superpixellation(i, sL, s, nS),
    selectedSuperpixels(sS),
    selectedPixels(sP)

{
    sS = 0;
    sP = 0;
}

FilteredSuperpixellation::FilteredSuperpixellation(
        ImageData *&i,
        Superpixellation *& superpixellation,
        bool *& sS,
        bool *& sP
    )  : Superpixellation(*superpixellation),
    selectedSuperpixels(sS),
    selectedPixels(sP)
{
    Q_ASSERT(i == img);
    i = 0;
    delete superpixellation;
    superpixellation = 0;
    sS = 0;
    sP = 0;
}

FilteredSuperpixellation::~FilteredSuperpixellation(void) {
    if(!shareAll) {
        if(selectedSuperpixels != 0) {
            delete [] selectedSuperpixels;
        }
        if(selectedPixels != 0) {
            delete [] selectedPixels;
        }
    }
}
