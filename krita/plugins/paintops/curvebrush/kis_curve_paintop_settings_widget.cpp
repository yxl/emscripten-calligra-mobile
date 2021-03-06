/*
 *  Copyright (c) 2008,2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include <kis_curve_paintop_settings_widget.h>
#include <kis_properties_configuration.h>
#include <kis_curve_paintop_settings.h>

#include "kis_curve_line_option.h"
#include <kis_compositeop_option.h>
#include <kis_paint_action_type_option.h>
#include <kis_curve_option_widget.h>
#include <kis_pressure_opacity_option.h>
#include <kis_linewidth_option.h>
#include "kis_curves_opacity_option.h"

KisCurvePaintOpSettingsWidget:: KisCurvePaintOpSettingsWidget(QWidget* parent)
    : KisPaintOpOptionsWidget(parent)
{
    addPaintOpOption(new KisCurveOpOption());
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureOpacityOption()));
    addPaintOpOption(new KisCurveOptionWidget(new KisLineWidthOption()));
    addPaintOpOption(new KisCurveOptionWidget(new KisCurvesOpacityOption()));
    addPaintOpOption(new KisCompositeOpOption(true));
    addPaintOpOption(new KisPaintActionTypeOption());
}

KisCurvePaintOpSettingsWidget::~ KisCurvePaintOpSettingsWidget()
{
}


KisPropertiesConfiguration*  KisCurvePaintOpSettingsWidget::configuration() const
{
    KisCurvePaintOpSettings* config = new KisCurvePaintOpSettings();
    config->setOptionsWidget(const_cast<KisCurvePaintOpSettingsWidget*>(this));
    config->setProperty("paintop", "chalkbrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}

