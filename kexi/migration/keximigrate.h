/* This file is part of the KDE project
   Copyright (C) 2004 Adam Pigg <adam@piggz.co.uk>
   Copyright (C) 2004-2005 Jaroslaw Staniek <js@iidea.pl>
   Copyright (C) 2005 Martin Ellis <martin.ellis@kdemail.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KEXI_MIGRATE_H
#define KEXI_MIGRATE_H


#include "kexidb/tableschema.h"
#include "keximigratedata.h"

#include <qstringlist.h>

class KexiProject;
namespace Kexi
{
	class ObjectStatus;
}

/*!
 * \namespace KexiMigration
 * \brief Framework for importing databases into native KexiDB databases.
 */
namespace KexiMigration 
{
  //! Imports non-native databases into Kexi projects.
	/*! A generic API for importing data from an existing
	database into a new Kexi project.

	Basic idea is this:
	-# User selects an existing DB and new project (file or server based)
	-# User specifies whether to import structure and data or structure only.
	-# Import tool connects to db
	-# Checks if it is already a kexi project (not implemented yet)
	-# If not, then read structure and construct new project
	-# Ask user what to do if column type is not supported

	See kexi/doc/dev/kexi_import.txt for more info.
	*/
	class KEXIMIGR_EXPORT KexiMigrate : public QObject, public KexiDB::Object
	{
		Q_OBJECT

		public:
			//KexiMigrate();
			KexiMigrate(QObject *parent, const char *name, const QStringList &args = QStringList());
			virtual ~KexiMigrate();

			//! Data Setup.  Requires two connection objects, a name and a bool
			void setData(KexiMigration::Data*);

			//! Perform an import operation
			bool performImport(Kexi::ObjectStatus* result = 0);

			//! Perform an export operation
			bool performExport(Kexi::ObjectStatus* result = 0);

			//! Returns true if the migration driver supports progress updates.
			inline bool progressSupported() { return drv_progressSupported(); }

		signals:
			void progressPercent(int percent);

		public:
			virtual int versionMajor() const = 0;
			virtual int versionMinor() const = 0;
		protected:

			//! Connect to source database (driver specific)
			virtual bool drv_connect() = 0;
			//! Disconnect from source database (driver specific)
			virtual bool drv_disconnect() = 0;

			//! Get table names in source database (driver specific)
			virtual bool drv_tableNames(QStringList& tablenames) = 0;

			//! Read schema for a given table (driver specific)
			virtual bool drv_readTableSchema(
				const QString& originalName, KexiDB::TableSchema& tableSchema) = 0;

			//! Copy a table from source DB to target DB (driver specific)
			virtual bool drv_copyTable(const QString& srcTable, KexiDB::Connection *destConn, 
				KexiDB::TableSchema* dstTable) = 0;

			virtual bool drv_progressSupported() { return false; }

			//! Return the size of a table to be imported, or 0 if not supported
			/*! Finds the size of the named table, in order to provide feedback on
			    migration progress.

			    The units of the return type are deliberately unspecified.  Migration
			    drivers may return the number of records in the table, or the size in
			    bytes, etc.  Units should be chosen in order that the driver can 
			    return the size in the fastest way possible (e.g. migration from CSV
			    files should use file size to avoid counting the number of rows, and
			    migration from MDB files should return the number of rows as this is
			    stored within the file).

			    Obviously, the driver should use the same units when reporting
			    migration progress.

			    \return size of the specified table
			*/
			virtual bool drv_getTableSize(const QString&, Q_ULLONG&)
			{ return false; }

			void updateProgress(Q_ULLONG step = 1ULL);

			//! Prompt user to select a field type for unrecognised fields
			KexiDB::Field::Type userType(const QString& fname);

			// Protected data members
			//! Migrate Options
			KexiMigration::Data* m_migrateData;

			// Temporary values used during import (set by driver specific methods)
			KexiDB::Field* m_f;

		private:
			//! Get the list of tables
			bool tableNames(QStringList& tablenames);

			//! Create the target database project
			KexiProject* createProject(Kexi::ObjectStatus* result);

			//Private data members
			//! Flag indicating whether data should be copied
			bool m_keepData;

			//! Table schemas from source DB
			QPtrList<KexiDB::TableSchema> m_tableSchemas;

			//! Estimate size of migration job
			/*! Calls drv_getTableSize for each table to be copied.
			    \return sum of the size of all tables to be copied.
			*/
			bool progressInitialise();

			KexiProject *m_destPrj;

			//! Size of migration job
			Q_ULLONG m_progressTotal;
			//! Amount of migration job complete
			Q_ULLONG m_progressDone;
			//! Don't recalculate progress done until this value is reached.
			Q_ULLONG m_progressNextReport;
	};
} //namespace KexiMigration

#include <kgenericfactory.h>
#define KEXIMIGRATE_DRIVER_INFO( class_name, internal_name ) \
	int class_name::versionMajor() const { return 0; } \
	int class_name::versionMinor() const { return 0; } \
	K_EXPORT_COMPONENT_FACTORY(keximigrate_ ## internal_name, \
	  KGenericFactory<KexiMigration::class_name>( "keximigrate_" #internal_name ))
#endif

