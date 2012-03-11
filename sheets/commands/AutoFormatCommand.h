/* This file is part of the KDE project
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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
   Boston, MA 02110-1301, USA.
*/

#ifndef CALLIGRA_TABLES_AUTO_FORMAT_COMMAND
#define CALLIGRA_TABLES_AUTO_FORMAT_COMMAND

#include "AbstractRegionCommand.h"

#include <QList>

namespace Calligra
{
namespace Sheets
{
class Style;

/**
 * \ingroup Commands
 * \brief Formats a cell range using a pre-defined style for the table layout.
 */
class AutoFormatCommand : public AbstractRegionCommand
{
public:
    /**
     * Constructor.
     */
    AutoFormatCommand();

    /**
     * Destructor.
     */
    virtual ~AutoFormatCommand();

    void setStyles(const QList<Style>& styles);

protected:
    virtual bool process(Element* element);
    virtual bool preProcessing();
    virtual bool mainProcessing();

private:
    QList<Style> m_styles;
};

} // namespace Sheets
} // namespace Calligra

#endif // CALLIGRA_TABLES_AUTO_FORMAT_COMMAND