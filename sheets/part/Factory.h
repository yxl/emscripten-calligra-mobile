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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef CALLIGRA_SHEETS_FACTORY_H
#define CALLIGRA_SHEETS_FACTORY_H

#include <kpluginfactory.h>

#include "calligra_sheets_export.h"

class KAboutData;
class KIconLoader;

namespace Calligra
{
namespace Sheets
{

class CALLIGRA_SHEETS_COMMON_EXPORT Factory : public KPluginFactory
{
    Q_OBJECT
public:
    explicit Factory(QObject* parent = 0);
    ~Factory();

    virtual QObject* create(const char* iface, QWidget* parentWidget, QObject *parent, const QVariantList& args, const QString& keyword);

    static const KComponentData &global();

    // _Creates_ a KAboutData but doesn't keep ownership
    static KAboutData* aboutData();

private:
    static KComponentData* s_global;
    static KAboutData* s_aboutData;
};

} // namespace Sheets
} // namespace Calligra

#endif
