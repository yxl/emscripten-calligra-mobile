/* This file is part of the KDE project
   Copyright (C) 2001 David Faure <faure@kde.org>

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

#ifndef kotextformatter_h
#define kotextformatter_h

#include "qrichtext_p.h"
using namespace Qt3;
class KoZoomHandler;

/**
 * We implement our own text formatter to implement WYSIWYG:
 * It is heavily based on QTextFormatterBreakWords, but stores the x position
 * of characters (and their width) in pixels, whereas all the rest is in L.U.
 * Having our own text formatter will also enable implementing word-breaking later.
 */
class KoTextFormatter : public QTextFormatter
{
public:
    KoTextFormatter() {}
    virtual ~KoTextFormatter() {}

    virtual int format( Qt3::QTextDocument *doc, Qt3::QTextParag *parag, int start, const QMap<int, QTextParagLineStart*> &oldLineStarts );

protected:
    QTextParagLineStart *formatLineKo(
        KoZoomHandler *zh,
        Qt3::QTextParag * /*parag*/, KoTextString *string, QTextParagLineStart *line,
        KoTextStringChar *startChar, KoTextStringChar *lastChar, int align, int space );
};

#endif
