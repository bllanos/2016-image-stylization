#ifndef ISUPERPIXELGENERATOR_H
#define ISUPERPIXELGENERATOR_H

/*!
** \file isuperpixelgenerator.h
** \brief Definition of the ISuperpixelGenerator interface class.
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

#include "algorithms/algorithm.h"
#include "algorithms/superpixels/superpixellation.h"

/*!
 * \brief An interface class representing an algorithm which produces superpixels
 * for an image
 */
class ISuperpixelGenerator: public Algorithm
{
protected:
    /*!
     * \brief An empty constructor
     */
    ISuperpixelGenerator(void) {}

public:
    virtual ~ISuperpixelGenerator(void) {}

    /*!
     * \brief Output superpixel data
     * \param [out] superpixellation A superpixellation of an image.
     * The caller is expected to take ownership of this object. A null pointer
     * is expected to be passed in.
     * \return Success (true) or failure (false). For instance, a failure
     * result is returned if the object is not ready to produce superpixel data.
     */
    virtual bool outputSuperpixellation(Superpixellation *& superpixellation) = 0;

    // Currently not implemented - will cause linker errors if called
private:
    ISuperpixelGenerator(const ISuperpixelGenerator& other);
    ISuperpixelGenerator& operator=(const ISuperpixelGenerator& other);
};

#endif // ISUPERPIXELGENERATOR_H
