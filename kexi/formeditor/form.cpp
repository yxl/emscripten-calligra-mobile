/* This file is part of the KDE project
   Copyright (C) 2003 Lucijan Busch <lucijan@gmx.at>

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

#include <qwidget.h>
#include <qtabwidget.h>

#include <kdebug.h>

#include "form.h"
#include "container.h"
#include "objpropbuffer.h"
#include "formIO.h"

namespace KFormDesigner {

Form::Form(QObject *parent, const char *name, WidgetLibrary *lib, ObjectPropertyBuffer *buffer)
:QObject(parent, name)
{
	if(!lib)
		m_widgetLib = new WidgetLibrary(this);
	else
		m_widgetLib = lib;

	m_toplevel = 0;
	m_selWidget = 0;
	m_topTree = 0;
	m_buffer = buffer;
	m_inter = true;

	connect(buffer, SIGNAL(nameChanged(const char*, const QString&)), this, SLOT(changeName(const char*, const QString&)));
}

void
Form::createToplevel(QWidget *container)
{
	kdDebug() << "Form::createToplevel()" << endl;

	m_toplevel = new Container(0, container, this, "form1");
	m_topTree = new ObjectTree(container->className(), container->name(), container);
	m_toplevel->setObjectTree(m_topTree);
	m_toplevel->setForm(this);
	
	m_topTree->setWidget(container);
	m_topTree->addModProperty("caption");

	connect(m_widgetLib, SIGNAL(prepareInsert(const QString&)), this,
	 SLOT(insertWidget(const QString&)));
//	connect(m_widgetLib, SIGNAL(insertWidget(WidgetLibrary *, const QString &)), m_toplevel,
//	 SLOT(slotInsertWidget(WidgetLibrary *, const QString &)));

	kdDebug() << "Form::createToplevel(): m_toplevel=" << m_toplevel << endl;
}

void
Form::insertWidget(const QString &c)
{
	kdDebug() << "Form::insertWidget()" << endl;
	if(m_toplevel)
		m_toplevel->emitPrepareInsert( c);
}

QWidget*
Form::createInstance(QWidget *parent, bool e)
{
    return NULL;//TMP!
}

QWidget*
Form::createEmptyInstance(const QString &c, QWidget *parent)
{
	kdDebug() << "Form::createEmptyInstance()" << endl;

	QWidget *m = m_widgetLib->createWidget(c, parent, "form1", 0);
	if(!m)
		return 0;

	kdDebug() << "Form::createEmptyInstance() m=" << m << endl;
	createToplevel(m);

	m_topTree = new ObjectTree(c, m->name(), m);
	m_toplevel->setObjectTree(m_topTree);
	return m;
}

Actions
Form::createActions(KActionCollection *parent)
{
	return m_widgetLib->createActions(parent, this, SLOT(insertWidget(const QString &)));
}

void
Form::setSelectedWidget(QWidget *w)
{
	m_selWidget = w;
	if(w)
		m_buffer->setObject(w);
	else
		m_buffer->setObject(m_topTree->widget());
}

void
Form::changeName(const char *oldname, const QString &newname)
{
	m_topTree->rename(oldname, newname);
}

void
Form::saveForm()
{
	if(m_buffer)
		m_buffer->checkModifiedProp();
	FormIO::saveForm(this);
}


Form::~Form()
{
}

}

#include "form.moc"
