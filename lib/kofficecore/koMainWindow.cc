/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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

#include <config.h>

#include "koMainWindow.h"
#include "koDocument.h"
#include "koFilterManager.h"
#include "koIcons.h"

#include <qkeycode.h>
#include <qfile.h>
#include <qaction.h>
#include <qwhatsthis.h>
#include <qmime.h>
#include <qmessagebox.h>

#include <kapp.h>
#include <kstdaccel.h>
#include <klocale.h>
#include <kglobal.h>
#include <kmimetype.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kaction.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>

QList<KoMainWindow>* KoMainWindow::s_lstMainWindows = 0;

KoMainWindow::KoMainWindow( QWidget* parent, const char* name )
    : Shell( parent, name )
{
    if ( !s_lstMainWindows )
	s_lstMainWindows = new QList<KoMainWindow>;
    s_lstMainWindows->append( this );

    KAction* fnew = new KAction( i18n("New"), KofficeBarIcon( "filenew" ), CTRL + Key_N, this, SLOT( slotFileNew() ),
			  actionCollection(), "filenew" );
    KAction* open = new KAction( i18n("Open ..."), KofficeBarIcon( "fileopen" ), CTRL + Key_O, this, SLOT( slotFileOpen() ),
			  actionCollection(), "fileopen" );
    KAction* save = new KAction( i18n("Save"), KofficeBarIcon( "filefloppy" ), CTRL + Key_S, this, SLOT( slotFileSave() ),
			  actionCollection(), "filesave" );
    KAction* saveAs = new KAction( i18n("Save as..."), 0, this, SLOT( slotFileSaveAs() ),
			    actionCollection(), "filesaveas" );
    KAction* print = new KAction( i18n("Print..."), KofficeBarIcon( "fileprint" ), CTRL + Key_P, this, SLOT( slotFilePrint() ),
			  actionCollection(), "fileprint" );
    KAction* close = new KAction( i18n("Close"), 0, this, SLOT( slotFileClose() ),
			  actionCollection(), "fileclose" );
    KAction* quit = new KAction( i18n("Quit"), 0, this, SLOT( slotFileQuit() ),
			  actionCollection(), "quit" );
    KAction* helpAbout = new KAction( i18n("About"), 0, this, SLOT( slotFileQuit() ),
			  actionCollection(), "about" );

    KToolBar* fileTools = new KToolBar( this, "file operations" );
    fnew->plug( fileTools );
    open->plug( fileTools );
    save->plug( fileTools );
    print->plug( fileTools );
    (void)QWhatsThis::whatsThisButton( fileTools );
    fileTools->show();
}

KoMainWindow::~KoMainWindow()
{
    if ( s_lstMainWindows )
	s_lstMainWindows->removeRef( this );
}

KoMainWindow* KoMainWindow::firstMainWindow()
{
    if ( !s_lstMainWindows )
	return 0;

    return s_lstMainWindows->first();
}

KoMainWindow* KoMainWindow::nextMainWindow()
{
    if ( !s_lstMainWindows )
	return 0;

    return s_lstMainWindows->next();
}

bool KoMainWindow::openDocument( const char* _url )
{
    KoDocument* doc = document();
	
    KoDocument* newdoc = createDoc();
    if ( !newdoc->loadFromURL( _url ) )
    {
	delete newdoc;
	return FALSE;
    }

    if ( doc && doc->isEmpty() )
    {
	setRootPart( newdoc );
	delete doc;
	return TRUE;
    }
    else if ( doc && !doc->isEmpty() )
    {
        Shell *s = newdoc->createShell();
        s->show();
	return TRUE;
    }

    setRootPart( newdoc );
    return TRUE;
}

bool KoMainWindow::saveDocument( const char* _native_format, const char* _native_pattern, const char* _native_name, bool saveas )
{
    KoDocument* pDoc = document();

    QString url = pDoc->url();
    QString outputMimeType ( _native_format );

    if ( url.isEmpty() || saveas )
    {
	    QString filter = KoFilterManager::self()->fileSelectorList( KoFilterManager::Export,
	    _native_format, _native_pattern, _native_name, TRUE );
        QString file;

        bool bOk = true;
        do {
            file = KFileDialog::getSaveFileName( getenv( "HOME" ), filter );
            if ( file.isNull() )
                return false;

            if ( QFile::exists( file ) ) { // this file exists => ask for confirmation
                bOk = KMessageBox::questionYesNo( this,
                                                  i18n("A document with this name already exists\n"\
                                                  "Do you want to overwrite it ?"),
                                                  i18n("Warning") ) == KMessageBox::Yes;
            }
        } while ( !bOk );
	    KMimeType::Ptr t = KMimeType::findByURL( KURL( file ), 0, TRUE );
        outputMimeType = t->mimeType();

        url = file;
        pDoc->setURL( url );
    }

    KURL u( url );
    if ( !u.isLocalFile() ) return false; // only local files
    if ( QFile::exists( u.path() ) ) { // this file exists => backup
        // TODO : make this configurable ?
	system( QString( "rm -rf %1~" ).arg( u.path() ).latin1() );
	QString cmd = "cp %1 %2~";
	cmd = cmd.arg( u.path() ).arg( u.path() );
	system( cmd.latin1() );
    }

    // Not native format : save using export filter
    if ( outputMimeType != _native_format ) {
        QString nativeFile=KoFilterManager::self()->prepareExport(url, _native_format);
        if ( pDoc->saveToURL( nativeFile, _native_format ) && KoFilterManager::self()->export_() )
            return true;
        else
            return false;
    }

    // Native format => normal save
    if ( !pDoc->saveToURL( url, _native_format ) ) {
        QString tmp;
        tmp.sprintf( i18n( "Could not save\n%s" ), url.ascii() );
        KMessageBox::error( this, i18n( "IO Error" ) );
        return false;
    }
    return true;
}

bool KoMainWindow::closeDocument()
{
    if ( document() == 0 )
	return TRUE;

    if ( document()->isModified() )
    {
	int res = QMessageBox::warning( 0L, i18n( "Warning" ), i18n( "The document has been modified\nDo you want to save it ?" ),
					i18n( "Yes" ), i18n( "No" ), i18n( "Cancel" ) );

	if ( res == 0 )
	    return saveDocument( nativeFormatMimeType(), nativeFormatPattern(), nativeFormatName() );

	KoDocument* doc = document();
	setRootPart( 0 );
	delete doc;
    }

    return TRUE;
}

bool KoMainWindow::closeAllDocuments()
{
    KoMainWindow* win = firstMainWindow();
    for( ; win; win = nextMainWindow() )
    {
	if ( !win->closeDocument() )
	    return FALSE;
    }

    return TRUE;
}

void KoMainWindow::slotFileNew()
{
    KoDocument* doc = document();
	
    KoDocument* newdoc = createDoc();
    if ( !newdoc->initDoc() )
    {
	delete newdoc;
	return;
    }

    if ( doc && doc->isEmpty() )
    {
	setRootPart( newdoc );
	delete doc;
	return;
    }
    else if ( doc && !doc->isEmpty() )
    {
        Shell *s = newdoc->createShell();
        s->show();
	return;
    }

    setRootPart( newdoc );
    return;
}

void KoMainWindow::slotFileOpen()
{
    QString filter = KoFilterManager::self()->fileSelectorList( KoFilterManager::Import,
								nativeFormatMimeType(), nativeFormatPattern(),
								nativeFormatName(), TRUE );

    QString file = KFileDialog::getOpenFileName( getenv( "HOME" ), filter );
    if ( file.isNull() )
	return;

    file = KoFilterManager::self()->import( file, nativeFormatMimeType() );
    if ( file.isNull() )
	return;

    if ( !openDocument( file ) )
    {
        QString tmp;
        tmp.sprintf( i18n( "Could not open\n%s" ), file.data() );
        QMessageBox::critical( this, i18n( "IO Error" ), tmp, i18n( "OK" ) );
    }
}

void KoMainWindow::slotFileSave()
{
    saveDocument( nativeFormatMimeType(), nativeFormatPattern(), nativeFormatName() );
}

void KoMainWindow::slotFileSaveAs()
{
    saveDocument( nativeFormatMimeType(), nativeFormatPattern(), nativeFormatName(), TRUE );
}

void KoMainWindow::slotFileClose()
{
    if ( closeDocument() )
	delete this;
}

void KoMainWindow::slotFilePrint()
{
}

void KoMainWindow::slotFileQuit()
{
    if ( closeAllDocuments() )
	kapp->exit();
}

void KoMainWindow::slotHelpAbout()
{
}


#include "koMainWindow.moc"
