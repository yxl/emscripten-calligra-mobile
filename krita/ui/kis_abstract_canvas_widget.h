/*
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2006
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef _KIS_ABSTRACT_CANVAS_WIDGET_
#define _KIS_ABSTRACT_CANVAS_WIDGET_

class QWidget;
class KoToolProxy;

class KisAbstractCanvasWidget {

public:

    KisAbstractCanvasWidget() {}

    virtual ~KisAbstractCanvasWidget() {}

    virtual QWidget * widget() = 0;

    virtual KoToolProxy * toolProxy() = 0;

    virtual void parentSizeChanged( const QSize & ) {}
};

#endif // _KIS_ABSTRACT_CANVAS_WIDGET_
