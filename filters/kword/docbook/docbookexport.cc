/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright (c) 2000 ID-PRO Deutschland GmbH. All rights reserved.
                      Contact: Wolf-Michael Bolle <Wolf-Michael.Bolle@GMX.de>
   Copyright (C) 2001 Nicolas GOUTTE
   Copyright (c) 2001 IABG mbH. All rights reserved.
                      Contact: Wolf-Michael Bolle <Bolle@IABG.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <docbookexport.h>
#include <docbookexport.moc>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <qregexp.h>
#include <qdir.h>
#include <qdom.h>

#include <KWEFStructures.h>
#include <KWEFUtil.h>
#include <KWEFKWordLeader.h>
#include <TagProcessing.h>
#include <ProcessDocument.h>
#include <KWEFBaseWorker.h>

typedef KGenericFactory<DocBookExport, KoFilter> DocBookExportFactory;
K_EXPORT_COMPONENT_FACTORY( libdocbookexport, DocBookExportFactory( "docbookexport" ) );

DocBookExport::DocBookExport ( KoFilter          *parent,
                               const char        *name,
                               const QStringList & ) : KoFilter (parent, name)
{
}


#define INSERT_TABLE_IN_PARA 1    // Do not change this!
#define TABLES_WITH_TITLES   0


struct DocData
{
    bool article;
    bool head1;
    bool head2;
    bool head3;
    bool head4;
    bool bulletList;
    bool enumeratedList;
    bool alphabeticalList;
};


class DocBookWorker : public KWEFBaseWorker
{
    public:
        DocBookWorker (void) {}

        bool doOpenDocument  ( void );
        bool doCloseDocument ( void );

        bool doOpenFile  ( const QString &, const QString & );
        bool doCloseFile ( void );

        bool doOpenBody  ( void );
        bool doCloseBody ( void );

        bool doFullDocumentInfo ( const KWEFDocumentInfo & );

        bool doFullDocument ( const QValueList<ParaData> &paraList,
                              QString                    &storeFileName,
                              QString                    &exportFileName );

    private:
        void ProcessPictureData ( const Picture  &picture,
                                  QString        &storeFileName,
                                  QString        &exportFileName );

        void ProcessTableData ( const Table &table,
                                QString     &storeFileName,
                                QString     &exportFileName );

        void ProcessParagraphData ( const ParaData &para,
                                    QString         tag,
                                    QString        &storeFileName,
                                    QString        &exportFileName );

        void CloseItemizedList     ( void );
        void CloseEnumeratedList   ( void );
        void CloseAlphabeticalList ( void );
        void CloseLists            ( void );

        void CloseHead4           ( void );
        void CloseHead3           ( void );
        void CloseHead2           ( void );
        void CloseHead1AndArticle ( void );

        void OpenArticleUnlessHead1 ( void );

        QString  outputText;
        DocData  docData;
        QFile   *fileOut;
};


// ProcessPictureData () takes the available picture data, makes a
// copy of the image file into *.sgml.d/pictures/*.* from KoStore
// pictures/*.*, and creates the necessary DocBook tags for it.

void DocBookWorker::ProcessPictureData ( const Picture  &picture,
                                         QString        &storeFileName,
                                         QString        &exportFileName )
{
#if 0
    kdError (30507) << "DEBUG: ProcessPictureData (): picture " << picture.key
                    << " is stored as " << picture.koStoreName << endl;
#endif

    KoStore store (storeFileName, KoStore::Read);

    if ( store.open (picture.koStoreName) )
    {
        QByteArray byteArray = store.read ( store.size () );
        store.close ();

        QFileInfo fileInfo (exportFileName);
        QDir dir ( fileInfo.dirPath () );
        QString subDirName = fileInfo.fileName () + ".d";

        if ( !dir.exists (subDirName) )
        {
            dir.mkdir (subDirName);
        }

        dir.cd (subDirName);

        if ( !dir.exists ("pictures") )
        {
            dir.mkdir ("pictures");
        }

        QString pictureFileName = dir.filePath (picture.koStoreName);

        QFile pictureFile (pictureFileName);

        if ( pictureFile.open (IO_WriteOnly) )
        {
            pictureFile.writeBlock ( byteArray, byteArray.size () );

            QString pictureText;

#if TABLES_WITH_TITLES
            pictureText += "<FIGURE>\n";

#if 1
            pictureText += "  <TITLE>" + picture.name + "</TITLE>\n";
#else
            pictureText += "  <TITLE></TITLE>\n";
#endif
#else
            pictureText += "<INFORMALFIGURE>\n";
#endif
            pictureText += "  <MEDIAOBJECT>\n";
            pictureText += "    <IMAGEOBJECT>\n";
            pictureText += "      <IMAGEDATA FILEREF=\"" + pictureFileName + "\">\n";
            pictureText += "    </IMAGEOBJECT>\n";
            pictureText += "  </MEDIAOBJECT>\n";
#if TABLES_WITH_TITLES
            pictureText += "</FIGURE>\n";
#else
            pictureText += "</INFORMALFIGURE>\n";
#endif

            outputText += pictureText;
        }
        else
        {
            kdError (30507) << "Unable to open picture file " << pictureFileName << "!" << endl;

            pictureFile.close ();
        }
    }
    else
    {
        kdError (30507) << "Unable to open KoStore file " << picture.koStoreName << "!" << endl;

        store.close ();
    }
}


// ProcessTableData () takes the table data and creates the necessary
// DocBook tags for it.

void DocBookWorker::ProcessTableData ( const Table &table,
                                       QString     &storeFileName,
                                       QString     &exportFileName )
{
#if 0
    kdError (30507) << "DEBUG: ProcessTableData ()" << endl;
#endif

    QString tableText;

#if TABLES_WITH_TITLES
    tableText += "<TABLE>\n";

#if 1
    tableText += "  <TITLE>" + table.name + "</TITLE>\n";
#else
    tableText += "  <TITLE></TITLE>\n";
#endif
#else
    tableText += "<INFORMALTABLE>\n";
#endif

    tableText += "  <TGROUP COLS=\"" + QString::number (table.cols) + "\">\n";
    tableText += "    <TBODY>\n";

    int currentRow = -1;

    QValueList<TableCell>::ConstIterator cellIt;

    for ( cellIt = table.cellList.begin ();
          cellIt != table.cellList.end ();
          cellIt++ )
    {
        if ( (*cellIt).row != currentRow )
        {
            if ( currentRow >= 0 )
            {
                tableText += "      </ROW>\n";
            }

            currentRow = (*cellIt).row;

            tableText += "      <ROW>\n";
        }

        QString tmpBuf;
        tmpBuf = outputText;
        outputText = "";

        doFullDocument ( *(*cellIt).paraList, storeFileName, exportFileName );

        tableText += "        <ENTRY>" + outputText.replace ( QRegExp ( "\n" ), "" ) + "</ENTRY>\n";

        outputText = tmpBuf;
    }

    if ( currentRow >= 0 )
    {
        tableText += "      </ROW>\n";
    }

    tableText += "    </TBODY>\n";
    tableText += "  </TGROUP>\n";

#if TABLES_WITH_TITLES
    tableText += "</TABLE>\n";
#else
    tableText += "</INFORMALTABLE>\n";
#endif

    outputText += tableText;

#if 0
    kdError (30507) << "DEBUG: ProcessTableData (): " << tableText << endl;
#endif
}


// ProcessParagraphData () mangles the pure text through the
// formatting information stored in the FormatData list and prints it
// out to the export file.

void DocBookWorker::ProcessParagraphData ( const ParaData &para,
                                           QString         tag,
                                           QString        &storeFileName,
                                           QString        &exportFileName )
{
#if !INSERT_TABLE_IN_PARA
    QValueList<AnchoredInsert> tmpAnchoredInsertList;
#endif

    outputText += "<" + tag + ">";

    if ( para.text.length () > 0 )
    {
        ValueListFormatData::ConstIterator  formattingIt;

        for ( formattingIt = para.formattingList.begin ();
              formattingIt != para.formattingList.end ();
              formattingIt++ )
        {
            switch ( (*formattingIt).id )
            {
                case 1:   // texts
                    {
                    if ( (*formattingIt).text.linkReference.length () )
                    {
                        kdError (30507) << "ProcessParagraphData: Link to "
                                        << (*formattingIt).text.linkReference
                                        << " not implemented, yet!" << endl;
                    }

                    bool fixedFont = false;

                    if ( (*formattingIt).text.fontName == "courier"     ||
                         (*formattingIt).text.fontName == "Courier"     ||
                         (*formattingIt).text.fontName == "Courier New"    )
                    {
                        fixedFont = true;
                    }

                    if ( (*formattingIt).text.italic && !para.layout.formatData.text.italic )
                    {
                        outputText += "<EMPHASIS>";
                    }

                    if ( (*formattingIt).text.weight > para.layout.formatData.text.weight )
                    {
                        outputText += "<EMPHASIS ROLE=bold>";
                    }

                    if ( fixedFont )
                    {
                        outputText += "<LITERAL>";
                    }

                    outputText += EscapeXmlText (para.text.mid ( (*formattingIt).pos, (*formattingIt).len ));

                    if ( fixedFont )
                    {
                        outputText += "</LITERAL>";
                    }

                    if ( (*formattingIt).text.weight > para.layout.formatData.text.weight )
                    {
                        outputText += "</EMPHASIS>";
                    }

                    if ( (*formattingIt).text.italic && !para.layout.formatData.text.italic )
                    {
                        outputText += "</EMPHASIS>";
                    }
                    }
                    break;

               case 6:   // anchors
#if 0
                  kdError (30507) << "Processing anchor " << (*formattingIt).frameAnchor.name << endl;
#endif

#if INSERT_TABLE_IN_PARA
                  outputText += "</" + tag + ">\n";

#if 0
                  anchoredInsertList.prepend ( AnchoredInsert ( (*formattingIt).frameAnchor.name,
                                                                outputText.length ()                  ) );
#endif

                  switch ( (*formattingIt).frameAnchor.type )
                  {
                      case 2:
                          ProcessPictureData ( (*formattingIt).frameAnchor.picture, storeFileName, exportFileName );
                          break;

                      case 6:
                          ProcessTableData ( (*formattingIt).frameAnchor.table, storeFileName, exportFileName );
                          break;

                      default:
                          kdError (30507) << "Unhandled anchor type "
                                          << (*formattingIt).frameAnchor.type << "!" << endl;
                  }
#else
                  tmpAnchoredInsertList << AnchoredInsert ( (*formattingIt).frameAnchor.name, 0 );
#endif

                  outputText += "<" + tag + ">";

                  break;

               default:
                  kdError (30507) << "Unhandled format id "
                                  << (*formattingIt).id << "!" << endl;
             }
        }
    }

    outputText += "</" + tag + ">\n";

#if !INSERT_TABLE_IN_PARA
    QValueList<AnchoredInsert>::Iterator anchoredInsert;

    for ( anchoredInsert = tmpAnchoredInsertList.begin ();
          anchoredInsert != tmpAnchoredInsertList.end ();
          anchoredInsert++ )
    {
        (*anchoredInsert).pos = outputText.length ();
        anchoredInsertList.prepend (*anchoredInsert);
    }
#endif
}


void DocBookWorker::CloseItemizedList ( void )
{
    if ( docData.bulletList )
    {
        outputText += "</ITEMIZEDLIST> <!-- End of Bullet List -->\n";
        docData.bulletList = false;
    }
}


void DocBookWorker::CloseEnumeratedList ( void )
{
    if ( docData.enumeratedList )
    {
        outputText += "</ORDEREDLIST> <!-- End of Enumerated List -->\n";
        docData.enumeratedList = false;
    }
}


void DocBookWorker::CloseAlphabeticalList ( void )
{
    if ( docData.alphabeticalList )
    {
        outputText += "</ORDEREDLIST> <!-- End of Alphabetical List -->\n";
        docData.alphabeticalList = false;
    }
}


void DocBookWorker::CloseLists ( void )
{
    CloseItemizedList ();
    CloseEnumeratedList ();
    CloseAlphabeticalList ();
}


void DocBookWorker::CloseHead4 ( void )
{
    CloseLists ();

    if ( docData.head4 )
    {
        outputText += "</SECTION> <!-- End of Head 4 -->\n";
        docData.head4 = false;
    }
}


void DocBookWorker::CloseHead3 ( void )
{
    CloseHead4 ();

    if ( docData.head3 )
    {
        outputText += "</SECTION> <!-- End of Head 3 -->\n";
        docData.head3 = false;
    }
}


void DocBookWorker::CloseHead2 ( void )
{
    CloseHead3 ();

    if ( docData.head2 )
    {
        outputText += "</SECTION> <!-- End of Head 2 -->\n";
        docData.head2 = false;
    }
}


void DocBookWorker::CloseHead1AndArticle ( void )
{
    CloseHead2 ();

    if ( docData.article )
    {
        outputText += "</ARTICLE>\n";
        docData.article = false;
    }

    if ( docData.head1 )
    {
        outputText += "</CHAPTER> <!-- End of Head 1 -->\n";
        docData.head1 = false;
    }
}


void DocBookWorker::OpenArticleUnlessHead1 ( void )
{
    if ( !docData.head1 && !docData.article )
    {
        outputText += "<ARTICLE> <!-- Begin of Article -->\n";
        docData.article = true;
    }
}


bool DocBookWorker::doFullDocument ( const QValueList<ParaData> &paraList,
                                     QString                    &storeFileName,
                                     QString                    &exportFileName )
{
#if 0
    kdError (30507) << "doFullDocument () - Begin" << endl;
#endif

    QValueList<ParaData>::ConstIterator paraIt;

    for ( paraIt = paraList.begin (); paraIt != paraList.end (); paraIt++ )
    {
        switch ( (*paraIt).layout.counter.numbering )
        {
            case CounterData::NUM_LIST:
                switch ( (*paraIt).layout.counter.style )
                {
                    case CounterData::STYLE_CUSTOMBULLET:
                    case CounterData::STYLE_CIRCLEBULLET:
                    case CounterData::STYLE_SQUAREBULLET:
                    case CounterData::STYLE_DISCBULLET:
                    case CounterData::STYLE_CUSTOM:
                    case CounterData::STYLE_NONE:
                        CloseEnumeratedList ();
                        CloseAlphabeticalList ();

                        OpenArticleUnlessHead1 ();

                        if ( !docData.bulletList )
                        {
                            outputText += "<ITEMIZEDLIST> <!-- Begin of Bullet List -->\n";
                            docData.bulletList = true;
                        }

                        outputText += "<LISTITEM>\n";
                        ProcessParagraphData (*paraIt, "PARA", storeFileName, exportFileName);
                        outputText += "</LISTITEM>\n";
                        break;

                    case CounterData::STYLE_NUM:
                    case CounterData::STYLE_ROM_NUM_L:
                    case CounterData::STYLE_ROM_NUM_U:
                        CloseItemizedList ();
                        CloseAlphabeticalList ();

                        OpenArticleUnlessHead1 ();

                        if ( !docData.enumeratedList )
                        {
                            outputText += "<ORDEREDLIST NUMERATION=\"Arabic\"> <!-- Begin of Enumerated List -->\n";
                            docData.enumeratedList = true;
                        }

                        outputText += "<LISTITEM>\n";
                        ProcessParagraphData (*paraIt, "PARA", storeFileName, exportFileName);
                        outputText += "</LISTITEM>\n";
                        break;

                    case CounterData::STYLE_ALPHAB_L:
                    case CounterData::STYLE_ALPHAB_U:
                        CloseItemizedList ();
                        CloseEnumeratedList ();

                        OpenArticleUnlessHead1 ();

                        if ( !docData.alphabeticalList )
                        {
                            outputText += "<ORDEREDLIST NUMERATION=\"Loweralpha\"> <!-- Begin of Alphabetical List -->\n";
                            docData.alphabeticalList = true;
                        }

                        outputText += "<LISTITEM>\n";
                        ProcessParagraphData (*paraIt, "PARA", storeFileName, exportFileName);
                        outputText += "</LISTITEM>\n";
                        break;

                    default:
                        kdError (30507) << "Unknown counter style " << (*paraIt).layout.counter.style << "!" << endl;
                        CloseLists ();
                        OpenArticleUnlessHead1 ();
                        ProcessParagraphData (*paraIt, "PARA", storeFileName, exportFileName);
                }

                break;

            case CounterData::NUM_CHAPTER:
                switch ( (*paraIt).layout.counter.depth )
                {
                    case 0:
                        CloseHead1AndArticle ();

                        outputText += "<CHAPTER> <!-- Begin of Head 1 -->\n";
                        docData.head1 = true;

                        ProcessParagraphData (*paraIt, "TITLE", storeFileName, exportFileName);
                        break;

                    case 1:
                        CloseHead2 ();

                        outputText += "<SECTION> <!-- Begin of Head 2 -->\n";
                        docData.head2 = true;

                        ProcessParagraphData (*paraIt, "TITLE", storeFileName, exportFileName);
                        break;

                    case 2:
                        CloseHead3 ();

                        outputText += "<SECTION> <!-- Begin of Head 3 -->\n";
                        docData.head3 = true;

                        ProcessParagraphData (*paraIt, "TITLE", storeFileName, exportFileName);
                        break;

                    case 3:
                        CloseHead4 ();

                        outputText += "<SECTION> <!-- Begin of Head 4 -->\n";
                        docData.head4 = true;

                        ProcessParagraphData (*paraIt, "TITLE", storeFileName, exportFileName);
                        break;

                    default:
                        kdError (30507) << "Unexpected chapter depth " << (*paraIt).layout.counter.depth << "!" << endl;
                        CloseLists ();
                        OpenArticleUnlessHead1 ();
                        ProcessParagraphData (*paraIt, "PARA", storeFileName, exportFileName);
                }

                break;

            default:
                CloseLists ();
                OpenArticleUnlessHead1 ();
                ProcessParagraphData (*paraIt, "PARA", storeFileName, exportFileName);
        }
    }

#if 0
    kdError (30507) << "doFullDocument () - End" << outputText << endl;
#endif
    return true;
}


bool DocBookWorker::doOpenDocument ( void )
{
    outputText += "<!DOCTYPE BOOK PUBLIC \"-//OASIS//DTD DocBook V3.1//EN\">\n";
    outputText += "<BOOK>\n";

    return true;
}


bool DocBookWorker::doOpenBody ( void )
{
    docData.article          = false;
    docData.head1            = false;
    docData.head2            = false;
    docData.head3            = false;
    docData.head4            = false;
    docData.bulletList       = false;
    docData.enumeratedList   = false;
    docData.alphabeticalList = false;

    return true;
}


bool DocBookWorker::doCloseBody ( void )
{
    CloseHead1AndArticle ();

    return true;
}


bool DocBookWorker::doCloseDocument ( void )
{
    outputText += "</BOOK>\n";

    return true;
}


bool DocBookWorker::doOpenFile ( const QString &filenameOut, const QString &to )
{
    fileOut = new QFile(filenameOut);

    if ( !fileOut )
    {
        kdError(30507) << "No output file! Aborting!" << endl;
        return false;
    }

    if ( !fileOut->open (IO_WriteOnly) )
    {
        kdError(30507) << "Unable to open output file!" << endl;

        fileOut->close ();
        delete fileOut;
        fileOut = NULL;

        return false;
    }

    return true;
}


bool DocBookWorker::doCloseFile ( void )
{
    if ( !fileOut ) return true;

    fileOut->writeBlock ( (const char *) outputText.local8Bit (), outputText.length () );
    fileOut->close ();
    delete fileOut;
    fileOut = NULL;

    return true;
}


// ProcessInfoData () creates a subtag to the current tag level with
// text that was created earlier by ProcessInfoData () and adds it to
// the current tag level. It is used by ProcessDocumentIntoTag () to
// assemble the diverse levels of information of the BOOKINFO tag.

static void ProcessInfoData ( QString tagName,
                              QString tagText,
                              QString &outputText)
{
    if ( tagText.length () )
    {
        outputText += "<" + tagName + ">" + tagText + "</" + tagName + ">\n";
    }
}


bool DocBookWorker::doFullDocumentInfo ( const KWEFDocumentInfo &docInfo )
{
    QString bookInfoText;
    QString abstractText;
    QString authorText;
    QString affiliationText;
    QString addressText;

    ProcessInfoData ( "TITLE",       docInfo.title,      bookInfoText    );
    ProcessInfoData ( "PARA",        docInfo.abstract,   abstractText    );
    ProcessInfoData ( "SURNAME",     docInfo.fullName,   authorText      );
    ProcessInfoData ( "JOBTITLE",    docInfo.jobTitle,   affiliationText );
    ProcessInfoData ( "ORGNAME",     docInfo.company,    affiliationText );
    ProcessInfoData ( "STREET",      docInfo.street,     addressText     );
    ProcessInfoData ( "CITY",        docInfo.city,       addressText     );
    ProcessInfoData ( "POSTCODE",    docInfo.postalCode, addressText     );
    ProcessInfoData ( "COUNTRY",     docInfo.country,    addressText     );
    ProcessInfoData ( "EMAIL",       docInfo.email,      addressText     );
    ProcessInfoData ( "PHONE",       docInfo.telephone,  addressText     );
    ProcessInfoData ( "FAX",         docInfo.fax,        addressText     );

    ProcessInfoData ( "ADDRESS",     addressText,        affiliationText );
    ProcessInfoData ( "AFFILIATION", affiliationText,    authorText      );
    ProcessInfoData ( "ABSTRACT",    abstractText,       bookInfoText    );
    ProcessInfoData ( "AUTHOR",      authorText,         bookInfoText    );
    ProcessInfoData ( "BOOKINFO",    bookInfoText,       outputText      );

    return true;
}


bool DocBookExport::filter ( const QString  &filenameIn,
                             const QString  &filenameOut,
                             const QString  &from,
                             const QString  &to,
                             const QString  &param        )
{
#if 0
    kdError (30507) << "to = " << to << ", from = " << from << endl;
#endif

    if ( to != "text/sgml" && to != "text/docbook" || from != "application/x-kword" )
    {
        return false;
    }

#if 1
    kdError (30507) << "let's get on with it" << endl;
#endif

    DocBookWorker worker;
    KWEFKWordLeader leader (&worker);
    leader.filter (filenameIn, filenameOut, from, to, param);

#if 1
    kdError (30507) << "done here" << endl;
#endif

    return true;
}
