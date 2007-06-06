/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_meta_data_schema.h"

#include <QString>

using namespace KisMetaData;

QString Schema::UriTIFF = "http://ns.adobe.com/tiff/1.0/";
QString Schema::UriEXIF = "http://ns.adobe.com/exif/1.0/";
QString Schema::UriDublinCore = "http://purl.org/dc/elements/1.1/";
QString Schema::UriXMP = "http://ns.adobe.com/xap/1.0/";


struct Schema::Private {
    QString uri;
    QString prefix;
};

Schema::Schema(QString _uri, QString _ns) : d(new Private)
{
    d->uri = _uri;
    d->prefix = _ns;
}

QString Schema::uri() const
{
    return d->uri;
}

QString Schema::prefix() const
{
    return d->prefix;
}
