/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef KIS_SKETCH_PAINTOP_SETTINGS_H_
#define KIS_SKETCH_PAINTOP_SETTINGS_H_

#include <kis_paintop_settings.h>
#include <kis_types.h>

#include "kis_sketch_paintop_settings_widget.h"

#include <kis_pressure_opacity_option.h>

#include <opengl/kis_opengl.h>

#if defined(_WIN32) || defined(_WIN64)
# include <windows.h>
#endif

class QWidget;
class QString;

class QDomElement;
class QDomDocument;


class KisSketchPaintOpSettings : public KisPaintOpSettings
{

public:
    KisSketchPaintOpSettings();
    virtual ~KisSketchPaintOpSettings() {}

    virtual void paintOutline(const QPointF& pos, KisImageWSP image, QPainter& painter, OutlineMode _mode) const;
    virtual QRectF paintOutlineRect(const QPointF& pos, KisImageWSP image, OutlineMode _mode) const;
    
    bool paintIncremental();
    bool isAirbrushing() const;
    int rate() const;
    
#if defined(HAVE_OPENGL)
    QString modelName() const;
#endif

};

#endif