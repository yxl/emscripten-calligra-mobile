/*
 *  Copyright (c) 2008 Lukas Tvrdy <lukast.dev@gmail.com>
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

#ifndef KIS_SOFT_PAINTOP_SETTINGS_H_
#define KIS_SOFT_PAINTOP_SETTINGS_H_

#include <kis_paintop_settings.h>
#include <kis_types.h>

#include "kis_soft_paintop_settings_widget.h"

#include <config-opengl.h>

#if defined(_WIN32) || defined(_WIN64)
# include <windows.h>
#endif

class QWidget;
class QString;

class QDomElement;
class QDomDocument;


class KisSoftPaintOpSettings : public KisPaintOpSettings
{

public:


    KisSoftPaintOpSettings();
    virtual ~KisSoftPaintOpSettings() {}

    bool paintIncremental();

    virtual void changePaintOpSize ( qreal x, qreal y ) const;
    virtual void paintOutline ( const QPointF& pos, KisImageWSP image, QPainter& painter, const KoViewConverter& converter, OutlineMode _mode ) const;
    virtual QRectF paintOutlineRect ( const QPointF& pos, KisImageWSP image, OutlineMode _mode ) const;

    KisPaintOpSettingsSP clone() const;

    int diameter() const;
    qreal spacing() const;
    qreal sigma() const;

    qreal start() const;
    qreal end() const;
    
#if defined(HAVE_OPENGL)
    QString modelName() const { return "3d-pencil"; }
#endif


// XXX: Hack!
void setOptionsWidget(KisPaintOpSettingsWidget* widget)
{
    if (m_options != 0 && m_options->property("owned by settings").toBool()) {
        delete m_options;
    }
    if (!widget) {
        m_options = 0;
    }
    else {
        m_options = qobject_cast<KisSoftPaintOpSettingsWidget*>(widget);
        m_options->writeConfiguration( this );
    }
}

private:

    KisSoftPaintOpSettingsWidget* m_options;
};

#endif
