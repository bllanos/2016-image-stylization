#ifndef IMAGEMANAGER_H
#define IMAGEMANAGER_H

/*!
** \file imagemanager.h
** \brief Definition of the ImageManager class.
**
** ### About
** Created for: COMP4905A Honours Project\n
** Fall 2016\n
** Bernard Llanos\n
** Supervised by Dr. David Mould\n
** School of Computer Science, Carleton University
**
** ## Primary basis
** [Qt Image Viewer Example](http://doc.qt.io/qt-5/qtwidgets-widgets-imageviewer-example.html)
**
** ## Other references
** - [Qt SVG Viewer Example](http://doc.qt.io/qt-5/qtsvg-svgviewer-example.html)
** - [Qt SVG Generator Example](http://doc.qt.io/qt-5/qtsvg-svggenerator-example.html)
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

#include <QFileDialog>
#include <QImage>

class ImageViewer;
class AlgorithmManager;
class QSvgGenerator;
class QBuffer;

/*!
 * \brief Image input, output and selection
 *
 * A class for managing the set of images that can be displayed in an ImageViewer.
 * Images can be loaded from files, saved to files, and input from the clipboard.
 * ImageManager also provides an interface for selecting which image is to be displayed.
 */
class ImageManager: public QObject
{
    Q_OBJECT

public:
    /*!
     * \brief Create an image manager
     * \param [in] viewer The image viewer which will display images managed by
     * this object, and which is also the parent of this object.
     * \param [in] algorithmManager The algorithms manager whose user interface
     * actions are to be enabled when an image is loaded
     */
    ImageManager(ImageViewer* viewer, AlgorithmManager *algorithmManager);

    ~ImageManager();

    /*!
     * \brief Load an image
     *
     * Load an image file and display the image on the parent ImageViewer.
     * \param [in] fileName Image filepath
     * \return `true`, if the file was loaded.
     */
    bool loadFile(const QString & fileName);

    /*!
     * \brief Connect the chain of objects responsible for serializing vector
     * graphics.
     *
     * New QPainter, QSvgGenerator, QBuffer, and QByteArray objects are linked
     * together so that the caller can call QPainter painting operations
     * to produce an SVG file in the svgFile QByteArray object. All of these
     * objects must be passed in as null pointers and deallocated by the caller.
     * \param [out] painter Painter generating the vector graphics. Note that
     * it is not necessary to call QPainter::begin() before painting,
     * as this function does the equivalent.
     * \param [out] svgGenerator Paint device receiving the vector graphics
     * \param [out] svgFileIOWrapper I/O device receiving data from the paint device
     * \param [out] svgFile Storage for data collected by the I/O device
     * \param [in] size Desired SVG file default dimensions
     * \param [in] title Title to use for SVG file metadata
     * \param [in] description Description to use for SVG file metadata
     */
    static void prepareSVGOutputBuffer(
            QPainter* &painter,
            QSvgGenerator* &svgGenerator,
            QBuffer* &svgFileIOWrapper,
            QByteArray* &svgFile,
            const QSize &size,
            const QString &title,
            const QString &description
        );

    /*!
     * \brief Retrieve the current image
     * \param [out] image The active image (which may be a null image, if
     * no image is loaded)
     */
    void getImage(QImage &image);

    /*!
     * \brief Set the current image
     *
     * This function is to be called with procedurally-generated image data.
     * \param [in] image A new image to display
     * \param [in] svgData Any SVG file data associated with the image (null
     * if there is no associated data). This object takes ownership of 'svgData',
     * and sets 'svgData' to a null pointer.
     */
    void setImage(const QImage &image, const QByteArray *&svgData);

    /*!
     * \brief Open an image file using a file browsing dialog, then output
     * the image to the caller.
     *
     * The image is also set as the current image.
     *
     * If no image was opened, the output parameter will be left unchanged.
     * \param [out] image The new image (unmodified if no image file was opened)
     * \param [in] title The title to give the file browsing dialog
     * \return Whether or not a new image was loaded
     */
    bool browseForImage(QImage &image, const QString &title);

private:
    enum class ImageFormat : unsigned int {
        ANY,
        RASTER,
        VECTOR
    };

    /*!
     * \brief Customize a file dialog for image loading.
     *
     * Adjusts the file browsing dialog to filter for image file types.
     * The first dialog will display the canonical "pictures" location,
     * whereas subsequent dialogs will open to the last-used location.
     * \param [in,out] dialog File browsing dialog to use for image filepath selection
     * \param [in] acceptMode Whether to open or save a file
     * \param [in] format Whether to filter for raster image formats, vector
     * image formats, or any filetype.
     */
    static void initializeImageFileDialog(
            QFileDialog &dialog,
            QFileDialog::AcceptMode acceptMode,
            ImageFormat format
        );

#ifndef QT_NO_CLIPBOARD
    /*!
     * \brief Helper function for pasting an image from the clipboard
     * \return An image obtained from the clipboard.
     */
    static QImage clipboardImage(void);
#endif // !QT_NO_CLIPBOARD

public slots:
    /*!
     * \brief Open an image file using a file browsing dialog.
     */
    void open();
#ifndef QT_NO_CLIPBOARD
    /*!
     * \brief Input an image from the clipboard.
     */
    void paste();
#endif // !QT_NO_CLIPBOARD
    /*!
     * \brief Save a raster format image file using a file browsing dialog.
     */
    void saveAsRasterFile();

    /*!
     * \brief Save a SVG image file using a file browsing dialog.
     */
    void saveAsSVGFile();

private:
    /*!
     * \brief Load a raster image
     *
     * Load a raster format image file, and display the image on the parent
     * ImageViewer. Actually, this function can also load SVG files, but
     * will give transparent SVG images a white background.
     * \param [in] fileReader Reader constructed with the filepath as an argument
     * \param [out] image The image loaded from the file
     * \return `true`, if the file was loaded.
     */
    bool loadRasterImageFile(QImageReader & fileReader, QImage &image);

    /*!
     * \brief Load a SVG file
     *
     * Load a SVG image file and display the image on the parent ImageViewer.
     * The background colour of the displayed SVG image is explicitly set
     * in this function, in contrast to loadRasterImageFile()
     * \param [in] fileName Image filepath
     * \param [out] image The image loaded from the file
     * \param [out] imageFile The same image in a serialized form that can be
     * directly saved to a file. Passed in as a null pointer.
     * \return `true`, if the file was loaded.
     */
    bool loadSVGImageFile(const QString & fileName, QImage &image, QByteArray* &imageFile);

    bool saveRasterFile(const QString &fileName);

    bool saveSVGFile(const QString &fileName);

    /*!
     * \brief Update the current SVG file data
     *
     * If the input parameter is null, the existing SVG file data is still
     * deallocated. If the input parameter is not null, then the parent ImageViewer's
     * ImageViewer::setExportSVGActionStatus() is called to enable SVG file export.
     * \param [in] newSvgData The new SVG file data
     */
    void setSVGFile(QByteArray const* const& newSvgData);

    /*!
     * \brief Delete the current SVG file data
     *
     * The parent ImageViewer's ImageViewer::setExportSVGActionStatus() is
     * called to enable SVG file export.
     * \param [in] newSvgData The new SVG file data
     */
    void cleanupSVGFile();

private:
    ImageViewer* viewer;
    AlgorithmManager* algManager;
    QImage image;
    const QByteArray* svgData;
};

#endif // IMAGEMANAGER_H
