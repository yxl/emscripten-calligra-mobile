/* This file is part of the KDE project
   Copyright (C) 2003 Lucijan Busch <lucijan@kde.org>
   Copyright (C) 2003 Jaroslaw Staniek <js@iidea.pl>

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

#include "kexipart.h"

#include "kexipartinfo.h"
#include "kexipartitem.h"
#include "kexidialogbase.h"

#include "kexipartguiclient.h"
#include "keximainwindow.h"

#include <kiconloader.h>

using namespace KexiPart;

Part::Part(QObject *parent, const char *name, const QStringList &)
: QObject(parent, name)
{
	m_info = 0;
}

Part::~Part()
{
}

GUIClient*
Part::createGUIClient(KexiMainWindow *win) {
	return new GUIClient(win, this, instanceName());
}

KexiDialogBase* Part::execute(KexiMainWindow *win, const KexiPart::Item &item)
{
	KexiDialogBase *dlg = createInstance(win,item);
	if (!dlg)
		return 0;
	dlg->resize(dlg->sizeHint());
	dlg->setMinimumSize(dlg->minimumSizeHint().width(),dlg->minimumSizeHint().height());

//js TODO: apply settings for caption displaying menthod; there can be option for
//- displaying item.caption() as caption, if not empty, without instanceName
//- displaying the same as above in tabCaption (or not)
	dlg->setCaption( QString("%1 : %2").arg(item.name()).arg(instanceName()) );
	dlg->setTabCaption( dlg->caption() );
	dlg->setIcon( SmallIcon( info()->itemIcon() ) );
	dlg->setDocID(item.identifier());
	dlg->registerDialog();
	return dlg;
}


//-------------------------------------------------------------------------


GUIClient::GUIClient(KexiMainWindow *win, Part* part, const QString &i18nInstanceName)
 : QObject(part, part->info()->objectName().latin1()), KXMLGUIClient(win)
{
	new KAction(i18nInstanceName+"...", part->info()->itemIcon(), 0, this, 
		SLOT(create()), actionCollection(), (part->info()->objectName()+"part_create").latin1());
		setXMLFile(QString("kexi")+part->info()->objectName()+"partui.rc");

	win->guiFactory()->addClient(this);
}


#include "kexipart.moc"

