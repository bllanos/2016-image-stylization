#ifndef FILTEREDSUPERPIXELLATION_H
#define FILTEREDSUPERPIXELLATION_H

/*!
** \file filteredsuperpixellation.h
** \brief Definition of the FilteredSuperpixellation class.
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

#include "algorithms/superpixels/superpixellation.h"

/*!
 * \brief A class representing a set of superpixels and storing information about
 * which superpixels "passed" according to some criteria.
 */
class FilteredSuperpixellation : public Superpixellation
{
public:
    /*!
     * \brief Assemble a FilteredSuperpixellation from base information
     *
     * This object takes ownership of all input arguments, and the constructor sets
     * the pointers to null so that they are not deallocated by the caller.
     * \param [in] img The image corresponding to the superpixellation
     * \param [in] superpixelLabels An array with the same number of elements as
     * `img.pixelCount()` (ImageData::pixelCount()), where the element at index
     * `k` stores the superpixel ID of the pixel at 1D coordinate `k`.
     * \param [in] superpixels An array of superpixels corresponding to the data
     * in `superpixelLabels`.
     * \param [in] nSuperpixels The number of superpixels in the `superpixels` array
     * \param [in] selectedSuperpixels An array of the same size as `superpixels`
     * storing the selected/rejected status of each superpixel in `superpixels`.
     * \param [in] selectedPixels An array with the same number of elements as
     * `img.pixelCount()` (ImageData::pixelCount()), where the elements
     * indicate the selected/rejected status of the superpixels corresponding
     * to the image locations.
     * \see Superpixellation::Superpixellation(ImageData*&,pxind*&,Superpixel**&,const pxind&,const bool=false)
     */
    FilteredSuperpixellation(ImageData *& img,
                             pxind *& superpixelLabels,
                             Superpixel **& superpixels,
                             const pxind &nSuperpixels,
                             bool *& selectedSuperpixels,
                             bool *& selectedPixels
                         );

    /*!
     * \brief Assemble a FilteredSuperpixellation from an existing Superpixellation
     *
     * This object takes ownership of all input arguments, and the constructor sets
     * the pointers to null so that they are not deallocated by the caller.
     * \param [in] img The image corresponding to `superpixellation`.
     * `img` is asserted to refer to the same object as `superpixellation->img`.
     * This parameter is only present to prevent the caller from storing a dangling
     * pointer, as it is likely that the caller has stored a separate pointer to the
     * ImageData object outside of `superpixellation`.
     * \param [in] superpixellation A superpixel representation of `img`.
     * \param [in] selectedSuperpixels An array of the same size as
     * `superpixellation->superpixels` storing the selected/rejected status of
     * each superpixel in `superpixellation->superpixels`.
     * \param [in] selectedPixels An array with the same number of elements as
     * `img.pixelCount()` (ImageData::pixelCount()), where the elements
     * indicate the selected/rejected status of the superpixels corresponding
     * to the image locations.
     */
    FilteredSuperpixellation(ImageData *&img,
                             Superpixellation *& superpixellation,
                             bool *& selectedSuperpixels,
                             bool *& selectedPixels
                         );

    virtual ~FilteredSuperpixellation(void);

    // Data members
public:
    /*!
     * \brief An array of the same size as
     * Superpixellation::superpixels storing the selected/rejected status of
     * each superpixel in Superpixellation::superpixels.
     */
    const bool* const selectedSuperpixels;
    /*!
     * \brief An array with the same number of elements as the number of pixels
     * in the image Superpixellation::img. Each element
     * indicates the selected/rejected status of the superpixel corresponding
     * to the pixel location.
     */
    const bool* const selectedPixels;
};

#endif // FILTEREDSUPERPIXELLATION_H
