#ifndef ALGORITHMRESULTPAIR_H
#define ALGORITHMRESULTPAIR_H

/*!
** \file algorithmresultpair.h
** \brief Definition of the AlgorithmResultPair class.
**
** ### About
** Created for: COMP4905A Honours Project\n
** Fall 2016\n
** Bernard Llanos, ID 100793648\n
** Supervised by Dr. David Mould\n
** School of Computer Science, Carleton University
**
** ## References
** [Qt Queued Custom Type Example](http://doc.qt.io/qt-5/qtcore-threads-queuedcustomtype-example.html)
**
** The original license from Qt examples is provided below:
**
** --------------------------------------
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** --------------------------------------
**
*/

#include <QImage>
#include <QByteArray>

/*!
 * \brief A class to combine a QImage with a QByteArray for the purposes of
 * transferring data between threads.
 *
 * It appears that slots and signals can transfer single objects, not multiple
 * objects, between the GUI thread and the image processing thread.
 */
class AlgorithmResultPair
{
public:
    /*!
     * \brief Create an empty instance
     */
    AlgorithmResultPair();
    /*!
     * \brief Trivial copy constructor
     * \param [in] other The source object
     */
    AlgorithmResultPair(const AlgorithmResultPair &other);
    ~AlgorithmResultPair();

    /*!
     * \brief Construct an instance containing the given image data
     * \param [in] image Raster image data
     * \param [in] svgData Vector image data (SVG file contents)
     */
    AlgorithmResultPair(const QImage &image, const QByteArray &svgData);

    /*!
     * \brief The contained raster image data
     * \return Raster image data
     */
    QImage image() const;
    /*!
     * \brief The contained vector image data
     * \return Image data as an SVG file
     */
    QByteArray svgData() const;

private:
    QImage m_image;
    QByteArray m_svgData;
};

Q_DECLARE_METATYPE(AlgorithmResultPair)

#endif // ALGORITHMRESULTPAIR_H
