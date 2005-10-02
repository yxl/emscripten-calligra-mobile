/* This file is part of the KDE project
   Copyright (C) 2004 Jaroslaw Staniek <js@iidea.pl>

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

#ifndef KEXIDBTITLEPAGE_H
#define KEXIDBTITLEPAGE_H

#include "KexiDBTitlePageBase.h"

class KEXIMAIN_EXPORT KexiDBTitlePage : public KexiDBTitlePageBase
{
    Q_OBJECT

public:
    KexiDBTitlePage( QWidget* parent = 0, const char* name = 0 );
    ~KexiDBTitlePage();

protected slots:
    virtual void languageChange() { KexiDBTitlePageBase::languageChange(); }

};

#endif // KEXIDBTITLEPAGE_H

