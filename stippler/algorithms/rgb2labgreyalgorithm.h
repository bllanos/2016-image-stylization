#ifndef RGB2LABGREYALGORITHM_H
#define RGB2LABGREYALGORITHM_H

/*!
** \file rgb2labgreyalgorithm.h
** \brief Definition of the Rgb2LabGreyAlgorithm class.
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

#include <QtGlobal>
#include "algorithm.h"

/*!
 * \brief Convert an image to greyscale
 *
 * The grey values are the CIE L*a*b* colour space L* values of the image.
 */
class Rgb2LabGreyAlgorithm : public Algorithm
{
public:
    /*!
     * \brief Construct an empty instance
     */
    Rgb2LabGreyAlgorithm();

    virtual ~Rgb2LabGreyAlgorithm();

    /*!
     * \brief Convert the image data to the CIE L*a*b* colour space,
     * and extract only the lightness channel
     * \param [out] finished Whether or not processing has completed or failed (true),
     * or is incomplete (false).
     * \param [out] status A string describing the current progress towards completion
     * \return Success (true), or failure to process (false). In the latter case,
     * this object should be destroyed.
     */
    virtual bool increment(bool & finished, QString & status) Q_DECL_OVERRIDE;

    /*!
     * \brief Collect the results of processing
     * \param [out] image Raster image output, which must be deallocated by the caller
     * \param [out] svgData A null pointer, as this algorithm does not output
     * vector image data
     * \return Success (true) or failure (false)
     */
    virtual bool output(QImage *&image, QByteArray *& svgData) Q_DECL_OVERRIDE;

protected:
    /*!
     * \brief The effective destructor
     *
     * This function is called by the destructor as well as by initialize().
     */
    virtual void cleanup(void) Q_DECL_OVERRIDE;

protected:
    /*!
     * \brief The current stage of processing
     */
    int progress;
    /*!
     * \brief A reference to the lightness channel of the input image.
     */
    const qreal* l;
    /*!
     * \brief A copy of the lightness channel of the input image, used to create
     * the output image
     */
    qreal* lCopy;
    /*!
     * \brief The output image, which is a greyscale version of the input image
     *
     * The output image is constructed from the CIE L*a*b* colour space L* values
     * of the input image.
     */
    ImageData* outputData;
};

#endif // RGB2LABGREYALGORITHM_H
