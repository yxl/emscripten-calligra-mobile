/* This file is part of the KDE project
   Copyright (C) 2004 Jaroslaw Staniek <js@iidea.pl>

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

#ifndef KEXIGUIMSGHANDLER_H
#define KEXIGUIMSGHANDLER_H

#include <core/kexi.h>
#include <kexidb/msghandler.h>

class KEXICORE_EXPORT KexiGUIMessageHandler : public KexiDB::MessageHandler
{
	public:
		KexiGUIMessageHandler(QWidget *parent = 0);
		virtual ~KexiGUIMessageHandler();
		virtual void showErrorMessage(const QString &title, const QString &details = QString::null);
		virtual void showErrorMessage(KexiDB::Object *obj, const QString& msg = QString::null);

		void showErrorMessage(const QString&,const QString&,KexiDB::Object *obj);
		void showErrorMessage(Kexi::ObjectStatus *status);
		void showErrorMessage(const QString &message, Kexi::ObjectStatus *status);
		void showSorryMessage(const QString &title, const QString &details = QString::null);
		void showMessage(MessageType type, const QString &title, const QString &details);
};

#endif
