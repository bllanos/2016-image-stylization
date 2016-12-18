/*!
** \file imagemanager.cpp
** \brief Implementation of the ImageManager class.
**
** ### About
** Created for: COMP4905A Honours Project\n
** Fall 2016\n
** Bernard Llanos, ID 100793648\n
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
*/

#include <QtWidgets>
#include <QSvgRenderer>
#include <QSvgGenerator>
#include <QColor>
#include "imagemanager.h"
#include "algorithmmanager.h"
#include "imageviewer.h"

/*!
  \brief The MIME type filter for image file open/save dialogs that permit
  arbitrary file formats

  "application/octet-stream" means "All Files".
  */
#define IMAGEMANAGER_ALL_MIMETYPE "application/octet-stream"

/*!
  \brief The MIME type filter for SVG files
  */
#define IMAGEMANAGER_SVG_MIMETYPE "image/svg+xml"

/*!
 * \brief The file extension initially set in raster image file save dialogs
 *
 * If the user types an extension, the file type corresponding to the
 * extension will be used instead.
 */
#define IMAGEMANAGER_INITIAL_SAVE_EXTENSION_RASTER "png"

/*!
 * \brief The file extension initially set in vector image file save dialogs
 */
#define IMAGEMANAGER_INITIAL_SAVE_EXTENSION_VECTOR "svg"

/*!
 * \brief The format of a SVG image as returned by QImageReader::format()
 */
#define IMAGEMANAGER_SVG_IMAGE_FORMAT "svg"

/*!
 * \brief The background fill colour for displayed vector images
 */
#define IMAGEMANAGER_DEFAULT_VECTOR_IMAGE_BACKGROUND Qt::white

ImageManager::ImageManager(ImageViewer *v, AlgorithmManager* a) :
    QObject(v), viewer(v), algManager(a), svgData(0)
{

}

void ImageManager::cleanupSVGFile() {
    if(svgData != 0) {
        delete svgData;
        svgData = 0;
    }
    viewer->setExportSVGActionStatus(false);
}

void ImageManager::setSVGFile(QByteArray const* const &newSvgData) {
    cleanupSVGFile();
    if(newSvgData != 0) {
        svgData = newSvgData;
        viewer->setExportSVGActionStatus(true);
    }
}

ImageManager::~ImageManager() {
    cleanupSVGFile();
}

bool ImageManager::loadFile(const QString &fileName)
{
    QImageReader reader(fileName);
    QImage newImage;
    QByteArray* newSvgData = 0;
    bool result = false;
    if(reader.format() != IMAGEMANAGER_SVG_IMAGE_FORMAT){
        result = loadRasterImageFile(reader, newImage);
    } else {
        result = loadSVGImageFile(fileName, newImage, newSvgData);
    }

    if(result) {
        image = newImage;
        setSVGFile(newSvgData);

        const QString message = tr("Opened \"%1\", %2x%3, Depth: %4")
            .arg(QDir::toNativeSeparators(fileName))
            .arg(image.width())
            .arg(image.height())
            .arg(image.depth());

        viewer->setImage(newImage, message);
        algManager->enableAlgorithms();
    }

    return result;
}

void ImageManager::prepareSVGOutputBuffer(
            QPainter* &painter,
            QSvgGenerator* &svgGenerator,
            QBuffer* &svgFileIOWrapper,
            QByteArray* &svgFile,
            const QSize &size,
            const QString &title,
            const QString &description
        ) {
    // Error checking
    Q_ASSERT(painter == 0);
    Q_ASSERT(svgGenerator == 0);
    Q_ASSERT(svgFileIOWrapper == 0);
    Q_ASSERT(svgFile == 0);

    svgFile = new QByteArray();
    svgFileIOWrapper = new QBuffer(svgFile);
    svgGenerator = new QSvgGenerator();
    svgGenerator->setOutputDevice(svgFileIOWrapper);
    svgGenerator->setSize(size);
    svgGenerator->setViewBox(QRect(QPoint(), size));
    svgGenerator->setTitle(title);
    svgGenerator->setDescription(description);
    painter = new QPainter(svgGenerator);
}

void ImageManager::getImage(QImage &im) {
    im = image;
}

void ImageManager::setImage(const QImage &newImage, const QByteArray *& newSvgData)
{
    Q_ASSERT(!newImage.isNull());
    if(image.isNull()) {
        algManager->enableAlgorithms();
    }
    image = newImage;
    setSVGFile(newSvgData);
    newSvgData = 0;

    const QString message = tr("Result image, %2x%3, Depth: %4")
        .arg(image.width())
        .arg(image.height())
        .arg(image.depth());

    viewer->setImage(newImage, message);
}

bool ImageManager::browseForImage(QImage &im, const QString &title)
{
    QFileDialog dialog(viewer, title);
    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen, ImageFormat::ANY);

    bool result = false;
    while(dialog.exec() == QDialog::Accepted) {
        if(loadFile(dialog.selectedFiles().first())) {
            result = true;
            im = image;
            break;
        }
    }
    return result;
}

bool ImageManager::loadRasterImageFile(QImageReader & fileReader, QImage &image) {
    fileReader.setAutoTransform(true);
    image = fileReader.read();
    if (image.isNull()) {
        QMessageBox::information(
                viewer,
                QGuiApplication::applicationDisplayName(),
                tr("Cannot load %1: %2").arg(
                        QDir::toNativeSeparators(fileReader.fileName()),
                        fileReader.errorString()));
        return false;
    }
    return true;
}

bool ImageManager::loadSVGImageFile(const QString & fileName, QImage &image, QByteArray* &imageFile) {
    Q_ASSERT(imageFile == 0);
    QSvgRenderer renderer(fileName);
    if (!renderer.isValid()) {
        QMessageBox::information(viewer, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot load %1")
                                 .arg(QDir::toNativeSeparators(fileName)));
        return false;
    } else {
        // Produce a displayable image
        image = QImage(renderer.defaultSize(), QImage::Format_ARGB32_Premultiplied);
        image.fill(IMAGEMANAGER_DEFAULT_VECTOR_IMAGE_BACKGROUND);
        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing);
        renderer.render(&painter);
        /* Cleanup now, in case the remaining operations take time.
         * end() will be called by the destructor anyway.
         */
        painter.end();

        // Produce file data corresponding to the displayable image
        QPainter* dataPainter = 0;
        QSvgGenerator* svgGenerator = 0;
        QBuffer* svgFileIOWrapper = 0;
        QString description = tr("%1, as displayed by the %2 image viewer").arg(
                    fileName, "COMP4905A"
                );
        prepareSVGOutputBuffer(
            dataPainter,
            svgGenerator,
            svgFileIOWrapper,
            imageFile,
            renderer.defaultSize(),
            fileName,
            description
        );
        dataPainter->fillRect(
                    QRect(QPoint(), renderer.defaultSize()),
                    IMAGEMANAGER_DEFAULT_VECTOR_IMAGE_BACKGROUND
                );
        renderer.render(dataPainter);
        delete dataPainter; // Implicit call to dataPainter->end()
        delete svgGenerator;
        delete svgFileIOWrapper;
    }
    return true;
}

bool ImageManager::saveRasterFile(const QString &fileName)
{
    Q_ASSERT(!image.isNull());
    QImageWriter writer(fileName);

    if (!writer.write(image)) {
        QMessageBox::information(viewer, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot write %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName)),
                                 writer.errorString());
        return false;
    }
    qDebug() << tr("Saved \"%1\"").arg(QDir::toNativeSeparators(fileName));
    return true;
}

bool ImageManager::saveSVGFile(const QString &fileName)
{
    Q_ASSERT(svgData != 0);
    QFile file(fileName);
    bool result = false;
    if (file.open(QIODevice::WriteOnly)) {
        qint64 n = file.write(*svgData);
        result = (n == svgData->size());
    }

    if (!result) {
        QMessageBox::information(viewer, QGuiApplication::applicationDisplayName(),
                                 tr("Error writing SVG file %1")
                                 .arg(QDir::toNativeSeparators(fileName)));
    } else {
        qDebug() << tr("Saved SVG file \"%1\"").arg(QDir::toNativeSeparators(fileName));
    }
    return result;
}

void ImageManager::initializeImageFileDialog(
        QFileDialog &dialog,
        QFileDialog::AcceptMode acceptMode,
        ImageManager::ImageFormat format
    )
{
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    }

    QStringList mimeTypeFilters;
    if(format != ImageFormat::VECTOR) {
        const QByteArrayList supportedMimeTypes =
                acceptMode == QFileDialog::AcceptOpen ?
                    QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
        foreach (const QByteArray &mimeTypeName, supportedMimeTypes)
            mimeTypeFilters.append(mimeTypeName);
        mimeTypeFilters.sort();
    }
    if(format != ImageFormat::RASTER) {
        if(!mimeTypeFilters.contains(IMAGEMANAGER_SVG_MIMETYPE, Qt::CaseInsensitive)) {
            mimeTypeFilters.prepend(IMAGEMANAGER_SVG_MIMETYPE);
        }
    }
    if(format == ImageFormat::ANY) {
        mimeTypeFilters.prepend(IMAGEMANAGER_ALL_MIMETYPE);
    }
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter(mimeTypeFilters.first());
    if (acceptMode == QFileDialog::AcceptSave) {
        if(format == ImageFormat::RASTER) {
            dialog.setDefaultSuffix(IMAGEMANAGER_INITIAL_SAVE_EXTENSION_RASTER);
        } else {
            dialog.setDefaultSuffix(IMAGEMANAGER_INITIAL_SAVE_EXTENSION_VECTOR);
        }
    }

    dialog.setAcceptMode(acceptMode);
}

void ImageManager::open()
{
    QFileDialog dialog(viewer, tr("Open Image"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen, ImageFormat::ANY);

    while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().first())) {}
}

void ImageManager::saveAsRasterFile()
{
    QFileDialog dialog(viewer, tr("Save File As"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptSave, ImageFormat::RASTER);

    while (dialog.exec() == QDialog::Accepted && !saveRasterFile(dialog.selectedFiles().first())) {}
}

void ImageManager::saveAsSVGFile()
{
    QFileDialog dialog(viewer, tr("Save File As"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptSave, ImageFormat::VECTOR);

    while (dialog.exec() == QDialog::Accepted && !saveSVGFile(dialog.selectedFiles().first())) {}
}


#ifndef QT_NO_CLIPBOARD
QImage ImageManager::clipboardImage()
{
    /* The contents of the clipboard are managed by the clipboard, essentially.
     * No need to deallocate.
     * See https://forum.qt.io/topic/41994/solved-is-there-a-method-to-clear-clipboard-data-within-the-application/10
     */
    if (const QMimeData *mimeData = QGuiApplication::clipboard()->mimeData()) {
        if (mimeData->hasImage()) {
            const QImage image = qvariant_cast<QImage>(mimeData->imageData());
            if (!image.isNull())
                return image;
        }
    }
    return QImage();
}

void ImageManager::paste()
{
    const QImage newImage = clipboardImage();
    if (newImage.isNull()) {
        viewer->setStatusBarMessage(tr("No image in clipboard"));
    } else {
        const QString message = tr("Obtained image from clipboard, %1x%2, Depth: %3")
            .arg(newImage.width()).arg(newImage.height()).arg(newImage.depth());

        image = newImage;
        viewer->setImage(newImage, message);
        cleanupSVGFile();
        algManager->enableAlgorithms();
    }
}
#endif // !QT_NO_CLIPBOARD
