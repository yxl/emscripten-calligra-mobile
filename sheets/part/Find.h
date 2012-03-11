/* This file is part of the KDE project
 *
 * Copyright (c) 2011 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef CALLIGRA_TABLES_FIND_H
#define CALLIGRA_TABLES_FIND_H

#include "calligra_tables_export.h"
#include "Cell.h"

#include <KoFindBase.h>

namespace Calligra
{
namespace Sheets
{

class Sheet;
class SheetView;
/**
 * Searching implementation for searching through spreadsheets.
 *
 * This class implements a KoFind-backend for searching in spreadsheets.
 *
 * Matches found by this class will use Calligra::Sheets::Sheet* as container
 * and Calligra::Sheets::Cell as location.
 *
 * TODO: Add support for searching in notes/comments.
 * TODO: Support searching through all Sheets in a document.
 * TODO: Search within the displayed text or the user input.
 */
class CALLIGRA_TABLES_COMMON_EXPORT Find : public KoFindBase
{
    Q_OBJECT
public:
    /**
     * Constructor.
     */
    explicit Find(QObject* parent = 0);

public Q_SLOTS:
    /**
     * Set the current active sheet.
     *
     * Currently this class only searches within the active sheet.
     */
    void setCurrentSheet(Sheet *sheet, SheetView *view);

protected:
    /**
     * Overridden from KoFindBase
     */
    virtual void replaceImplementation(const KoFindMatch &match, const QVariant &value);
    /**
     * Overridden from KoFindBase
     */
    virtual void findImplementation(const QString &pattern, KoFindBase::KoFindMatchList &matchList);

    /**
     * Overridden from KoFindBase
     */
    virtual void clearMatches();

private:
    class Private;
    Private * const d;

private Q_SLOTS:
    void setActiveMatch(const KoFindMatch &match);
};

} //namespace Sheets
} //namespace Calligra

Q_DECLARE_METATYPE(Calligra::Sheets::Cell)
Q_DECLARE_METATYPE(Calligra::Sheets::Sheet*)

#endif // CALLIGRA_TABLES_FIND_H