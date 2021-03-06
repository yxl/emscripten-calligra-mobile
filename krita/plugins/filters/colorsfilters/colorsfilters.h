/*
 * This file is part of Krita
 *
 * Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. */

#ifndef COLORSFILTERS_H
#define COLORSFILTERS_H

#include <QObject>
#include <QVariant>
#include "kis_perchannel_filter.h"
#include "filter/kis_color_transformation_filter.h"

class KoColorSpace;
class KoColorTransformation;

class ColorsFilters : public QObject
{
    Q_OBJECT
public:
    ColorsFilters(QObject *parent, const QVariantList &);
    virtual ~ColorsFilters();
};

class KisAutoContrast : public KisFilter
{
public:
    KisAutoContrast();
public:

    void processImpl(KisPaintDeviceSP device,
                     const QRect& applyRect,
                     const KisFilterConfiguration* config,
                     KoUpdater* progressUpdater
                     ) const;
    static inline KoID id() {
        return KoID("autocontrast", i18n("Auto Contrast"));
    }

};


#endif
