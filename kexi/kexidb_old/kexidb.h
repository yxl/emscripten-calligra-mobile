/* This file is part of the KDE project
Copyright (C) 2002   Lucijan Busch <lucijan@gmx.at>
Daniel Molkentin <molkentin@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#ifndef KEXIDB_H
#define KEXIDB_H

#include <qobject.h>

#include "kexidbresult.h"

class KexiDBInterfaceManager;

class KexiDB : public QObject
{
	Q_OBJECT
	
	public:
		KexiDB(QObject *parent=0, const char *name=0);
		~KexiDB();

		KexiDB *add(QString driver);

		QStringList getDrivers() const;

		//now driver related functions

		/*! connect to database hope that is ansi-compatible */
		int connect(QString host, QString user, QString password);
		/*! connect method for file-based databases*/
		int load(QString file);

		/*! hope thats ansi-compatilbe too */
		QStringList getDatabases() const;
		QStringList getTables() const;

		int query(QString statement);
		QString escape(QString &str);
		
		KexiDBResult	*storeResult();
		KexiDBResult	*useResult();

		unsigned long	affectedRows();


	protected:
		KexiDBInterfaceManager	*m_manager;
};

#endif
