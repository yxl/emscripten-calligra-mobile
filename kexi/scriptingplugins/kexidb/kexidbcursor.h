/***************************************************************************
 * kexidbcursor.h
 * This file is part of the KDE project
 * copyright (C)2004-2005 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef KROSS_KEXIDB_KEXIDBCURSOR_H
#define KROSS_KEXIDB_KEXIDBCURSOR_H

#include <qstring.h>

#include <api/object.h>
#include <api/variant.h>
#include <api/list.h>
#include <api/class.h>

#include <kexidb/cursor.h>

namespace Kross { namespace KexiDB {

    // Forward declaration.
    class KexiDBConnection;

    /**
     * The KexiDBCursor class is a wrapper around the
     * \a ::KexiDB::Cursor class to provide database
     * cursor functionality.
     */
    class KexiDBCursor : public Kross::Api::Class<KexiDBCursor>
    {
        public:

            /**
             * Constructor.
             *
             * \param connection The parent \a KexiDBConnection
             *        instance this KexiDBCursor belongs to.
             * \param cursor The \a ::KexiDB::Cursor this class
             *        wraps.
             */
            KexiDBCursor(KexiDBConnection* connection, ::KexiDB::Cursor* cursor);

            /**
             * Destructor.
             */
            virtual ~KexiDBCursor();

            /// See \see Kross::Api::Object::getClassName
            virtual const QString getClassName() const;

        private:
            ::KexiDB::Cursor* cursor();
            ::KexiDB::Cursor* m_cursor;

            /// Moves current position to the first record and retrieves it.
            Kross::Api::Object::Ptr moveFirst(Kross::Api::List::Ptr);
            /// Moves current position to the last record and retrieves it.
            Kross::Api::Object::Ptr moveLast(Kross::Api::List::Ptr);
            /// Moves current position to the next record and retrieves it.
            Kross::Api::Object::Ptr moveNext(Kross::Api::List::Ptr);
            /// Moves current position to the next record and retrieves it.
            Kross::Api::Object::Ptr movePrev(Kross::Api::List::Ptr);

            /// Returns true if current position is after last record.
            Kross::Api::Object::Ptr eof(Kross::Api::List::Ptr);
            /// Returns true if current position is before first record.
            Kross::Api::Object::Ptr bof(Kross::Api::List::Ptr);

            /** Returns current internal position of the cursor's query. Records 
            are numbered from 0; the value -1 means that the cursor does not 
            point to a valid record. */
            Kross::Api::Object::Ptr at(Kross::Api::List::Ptr);
            /// Returns the number of fields available for this cursor.
            Kross::Api::Object::Ptr fieldCount(Kross::Api::List::Ptr);
            /// Returns the value stored in column number \a i (counting from 0).
            Kross::Api::Object::Ptr value(Kross::Api::List::Ptr);
    };

}}

#endif

