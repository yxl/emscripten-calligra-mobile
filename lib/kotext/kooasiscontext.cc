/* This file is part of the KDE project
   Copyright (C) 2004 David Faure <faure@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "kooasiscontext.h"
#include <koOasisStyles.h>
#include <koOasisStore.h>
#include <koxmlns.h>
#include <koxmlwriter.h>
#include <kdebug.h>
#include <kodom.h>

KoOasisContext::KoOasisContext( KoDocument* doc, KoVariableCollection& varColl,
                                KoOasisStyles& styles, KoStore* store )
    : m_doc( doc ), m_store( store ), m_varColl( varColl ), m_styles( styles ),
      m_cursorTextParagraph( 0 )
{
    // Ideally this should be done by KoDocument and passed as argument here...
    KoOasisStore oasisStore( store );
    QString dummy;
    (void)oasisStore.loadAndParse( "tar:/META-INF/manifest.xml", m_manifestDoc, dummy );
}

void KoOasisContext::fillStyleStack( const QDomElement& object, const char* nsURI, const char* attrName )
{
    // find all styles associated with an object and push them on the stack
    // OoImpressImport has more tests here, but I don't think they're relevant to OoWriterImport
    // ### TODO check the above comment, now that things are in kotext...
    if ( object.hasAttributeNS( nsURI, attrName ) ) {
        const QString styleName = object.attributeNS( nsURI, attrName, QString::null );
        const QDomElement* style = m_styles.styles()[styleName];
        if ( style )
            addStyles( style );
        else
            kdWarning(32500) << "fillStyleStack: no style named " << styleName << " found." << endl;
    }
}

void KoOasisContext::addStyles( const QDomElement* style )
{
    Q_ASSERT( style );
    if ( !style ) return;
    // this recursive function is necessary as parent styles can have parents themselves
    if ( style->hasAttributeNS( KoXmlNS::style, "parent-style-name" ) ) {
        const QString parentStyleName = style->attributeNS( KoXmlNS::style, "parent-style-name", QString::null );
        QDomElement* parentStyle = m_styles.styles()[ parentStyleName ];
        if ( parentStyle )
            addStyles( parentStyle );
        else
            kdWarning(32500) << "Parent style not found: " << parentStyleName << endl;
    }
    else {
        QString family = style->attributeNS( KoXmlNS::style, "family", QString::null );
        if ( !family.isEmpty() ) {
            QDomElement* def = m_styles.defaultStyle( family );
            if ( def ) { // on top of all, the default style for this family
                //kdDebug(32500) << "pushing default style " << style->attributeNS( KoXmlNS::style, "name", QString::null ) << endl;
                m_styleStack.push( *def );
            }
        }
    }

    //kdDebug(32500) << "pushing style " << style->attributeNS( KoXmlNS::style, "name", QString::null ) << endl;
    m_styleStack.push( *style );
}

static QDomElement findListLevelStyle( const QDomElement& fullListStyle, int level )
{
    for ( QDomNode n = fullListStyle.firstChild(); !n.isNull(); n = n.nextSibling() )
    {
       const QDomElement listLevelItem = n.toElement();
       if ( listLevelItem.attributeNS( KoXmlNS::text, "level", QString::null ).toInt() == level )
           return listLevelItem;
    }
    return QDomElement();
}

bool KoOasisContext::pushListLevelStyle( const QString& listStyleName, int level )
{
    QDomElement* fullListStyle = m_styles.listStyles()[listStyleName];
    if ( !fullListStyle ) {
        kdWarning(32500) << "List style " << listStyleName << " not found!" << endl;
        return false;
    }
    else
        return pushListLevelStyle( listStyleName, *fullListStyle, level );
}

bool KoOasisContext::pushOutlineListLevelStyle( int level )
{
    QDomElement outlineStyle = KoDom::namedItemNS( m_styles.officeStyle(), KoXmlNS::text, "outline-style" );
    Q_ASSERT( !outlineStyle.isNull() );
    return pushListLevelStyle( "<outline-style>", outlineStyle, level );
}

bool KoOasisContext::pushListLevelStyle( const QString& listStyleName, // for debug only
                                         const QDomElement& fullListStyle, int level )
{
    // Find applicable list-level-style for level
    int i = level;
    QDomElement listLevelStyle;
    while ( i > 0 && listLevelStyle.isNull() ) {
        listLevelStyle = findListLevelStyle( fullListStyle, i );
        --i;
    }
    if ( listLevelStyle.isNull() ) {
        kdWarning(32500) << "List level style for level " << level << " in list style " << listStyleName << " not found!" << endl;
        return false;
    }
    //kdDebug(32500) << "Pushing list-level-style from list-style " << listStyleName << " level " << level << endl;
    m_listStyleStack.push( listLevelStyle );
    return true;
}

void KoOasisContext::setCursorPosition( KoTextParag* cursorTextParagraph,
                                        int cursorTextIndex )
{
    m_cursorTextParagraph = cursorTextParagraph;
    m_cursorTextIndex = cursorTextIndex;
}

KoOasisContext::~KoOasisContext()
{
}

////

KoSavingContext::KoSavingContext( KoGenStyles& mainStyles, KoVariableSettings* settings, bool hasColumns, SavingMode savingMode )
    : m_mainStyles( mainStyles ),
      m_savingMode( savingMode ),
      m_cursorTextParagraph( 0 ),
      m_variableSettings( settings ),
      m_hasColumns( hasColumns )
{
}


KoSavingContext::~KoSavingContext()
{
}

void KoSavingContext::setCursorPosition( KoTextParag* cursorTextParagraph,
                                         int cursorTextIndex )
{
    m_cursorTextParagraph = cursorTextParagraph;
    m_cursorTextIndex = cursorTextIndex;
}

void KoSavingContext::addFontFace( const QString& fontName )
{
    m_fontFaces[fontName] = true;
}

void KoSavingContext::writeFontFaces( KoXmlWriter& writer )
{
    writer.startElement( "office:font-face-decls" );
    const QStringList fontFaces = m_fontFaces.keys();
    for ( QStringList::const_iterator ffit = fontFaces.begin(), ffend = fontFaces.end() ; ffit != ffend ; ++ffit ) {
        writer.startElement( "style:font-face" );
        writer.addAttribute( "style:name", *ffit );
        writer.addAttribute( "svg:font-family", *ffit );
        // TODO style:font-family-generic
        // TODO style:font-pitch
        writer.endElement(); // style:font-face
    }
    writer.endElement(); // office:font-face-decls
}
