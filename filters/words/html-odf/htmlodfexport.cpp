/* This file is part of the Calligra project
   Copyright (C) 2010 Pramod S G <pramod.xyle@gmail.com>
   Copyright (C) 2010 Srihari Prasad G V <sri-hari@live.com>
   Copyright (C) 2012 Stuart Dickson <stuart@kogmbh.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the Library GNU General Public
   version 2 of the License, or (at your option) version 3 or,
   at the discretion of KDE e.V (which shall act as a proxy as in
   section 14 of the GPLv3), any later version..

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "htmlodfexport.h"

#include <QDomDocument>
#include <QFontInfo>
#include <QFile>
#include <QString>
#include <QBuffer>
#include <QByteArray>
#include <QFileInfo>
#include <QDir>
#include <QtXmlPatterns>

#include <kdebug.h>
#include <kpluginfactory.h>
#include <KStandardDirs>

#include <KoFilterChain.h>
#include <KoFilterManager.h>
#include <KoOdfWriteStore.h>
#include <KoStoreDevice.h>
#include <KoXmlWriter.h>

#include <document.h>
#include <exportdialog.h>
#include <KoDocument.h>

#include <iostream>

#include "ManifestParser.h"

K_PLUGIN_FACTORY(HTMLOdfExportFactory, registerPlugin<HTMLOdfExport>();)
K_EXPORT_STATIC_PLUGIN(HTMLOdfExportFactory("calligrafilters"), HTMLOdfExportFactory)


HTMLOdfExport::HTMLOdfExport(QObject* parent, const QVariantList&) :
    KoFilter(parent), m_dialog(new ExportDialog())
{
}

HTMLOdfExport::~HTMLOdfExport()
{
}

KoFilter::ConversionStatus HTMLOdfExport::convert(const QByteArray &from, const QByteArray &to)
{
    // check for proper conversion
    if (to != "text/html"
        || from != "application/vnd.oasis.opendocument.text") {
        return KoFilter::NotImplemented;
    }

    kDebug(30503) << "######################## HTMLOdfExport::convert ########################";

    QString inputFile = m_chain->inputFile();
    QString outputFile = m_chain->outputFile();

//    QFile::copy(inputFile, inputFile + ".odt");
//    qDebug() << "inputFile" << inputFile << "outputFile" << outputFile;

    /* TODO Reimplement export dialog when future functionality could benefit from user input.
     * The options of encoding and stylesheets have no effect on the export process at present.
     * The dialog was been disabled in the interest of reducing user confusion, avoiding
     * frustration should they try and use functionality which is not actually present.
     *
     * Styles are currently read from the ODT file's styles.xml and content.xml files and embedded
     * in the output html.
     */
    /*
    if (!m_chain->manager()->getBatchMode() ) {
        if (m_dialog->exec() == QDialog::Rejected) {
            return KoFilter::UserCancelled;
        }
    }
    */

    // Create output files
    QFile out(outputFile);
    if (!out.open(QIODevice::WriteOnly)) {
        kError(30501) << "Unable to open output file!";
        out.close();
        return KoFilter::FileNotFound;
    }

    QString filenameWithoutExtension = outputFile.left(outputFile.lastIndexOf('.'));

    KoFilter::ConversionStatus error;
    error = transformXml(inputFile, &out, filenameWithoutExtension+"/");

    if (error != KoFilter::OK) {
        return error;
    }

    QDir dir(outputFile);
    dir.mkdir(filenameWithoutExtension);

    error = extractImages(inputFile, filenameWithoutExtension+"/");
    if (error != KoFilter::OK) {
        return error;
    }

    out.close();

   return KoFilter::OK;
}


/*
 * Applies the converter.xsl stylesheet tranform on the combination
 * of the meta, styles and content xml files within the ODT file.
 */
KoFilter::ConversionStatus HTMLOdfExport::transformXml(const QString &inputFileName, QFile *outputFile, const QString &resourcesPath)
{
    KoFilter::ConversionStatus result = KoFilter::OK;
    bool success;

    // Create single xml file from ODT meta, styles and content components
    QByteArray contall("<?xml version='1.0' encoding='UTF-8'?>");
    contall.append("<office:document xmlns:office='urn:oasis:names:tc:opendocument:xmlns:office:1.0'>");

    QByteArray cont;
    QByteArray sty;
    QByteArray met;

    KoStore *storecont = KoStore::createStore(inputFileName, KoStore::Read);
    storecont->extractFile("meta.xml",met);
    met.remove(0,38); // remove xml file header
    contall.append(met);

    storecont->extractFile("styles.xml",sty);
    sty.remove(0,38); // remove xml file header
    contall.append(sty);

    storecont->extractFile("content.xml",cont);
    cont.remove(0,38); // remove xml file header
    contall.append(cont);

    contall.append("</office:document>");

    QFile temp1(KStandardDirs::locate("data","words/html-odf/converter.xsl"));
    temp1.open(QIODevice::ReadOnly);


    // Execute XML transformation

    QXmlQuery myQuery(QXmlQuery::XSLT20);
    // bind variables for root of resources and output filename
    myQuery.bindVariable(QString("html-odf-resourcesPath"), QVariant(resourcesPath));
    myQuery.bindVariable(QString("html-odf-fileName"), QVariant(outputFile->fileName()));
    myQuery.setFocus(contall);
    myQuery.setQuery(temp1.readAll());
    success = myQuery.evaluateTo(outputFile);

    if (!success) {
        result = KoFilter::ParsingError;
    }

    temp1.close();
    contall.clear();
    met.clear();
    sty.clear();
    cont.clear();

    delete storecont;

    return result;
}

/*
 * Given inputFile, extracts all recognised image types and exports
 * these and their containing folders to resourcesPath.
 * e.g.
 *      Picturess/myImage.png would be written to resourcesPath/Pictures/myImage.png
 *
 * ManifestParser determines which file types should be extracted.
 */
KoFilter::ConversionStatus HTMLOdfExport::extractImages(const QString &inputFile, const QString &resourcesPath)
{
    QDir dir(resourcesPath);
    QByteArray manifest;

    KoStore *storecont = KoStore::createStore(inputFile, KoStore::Read);
    storecont->extractFile("META-INF/manifest.xml",manifest);

    // parse manifest for file-entry elements.
    // The ManifestParser extracts just those files which are suppported
    ManifestParser manifestParser;
    QXmlInputSource source;
    source.setData(manifest);
    QXmlSimpleReader reader;
    reader.setContentHandler(&manifestParser);
    reader.parse(source);

    QString sourceImage;
    QString destImage;
    QString outputPath;

    // Extract each file
    QStringListIterator fileListIt = QStringListIterator(manifestParser.fileList());
    while (fileListIt.hasNext()) {
        sourceImage = fileListIt.next();
        destImage = resourcesPath + sourceImage;

        // Create the target directory
        outputPath = resourcesPath+sourceImage.left(sourceImage.lastIndexOf('/'));
        dir.mkpath(outputPath);

        storecont->extractFile(sourceImage,destImage);
    }

    return KoFilter::OK;
}

#include <htmlodfexport.moc>
