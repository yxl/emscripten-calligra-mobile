/*
 *  Copyright (c) 2009,2011 Cyrille Berger <cberger@cberger.net>
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

#include "kis_play_info.h"

#include "kis_image.h"
#include "kis_node.h"
#include <kis_paint_device.h>

struct KisPlayInfo::Private {
    KisImageWSP image;
    KisNodeSP currentNode;
};

KisPlayInfo::KisPlayInfo(KisImageWSP image, KisNodeSP currentNode)
        : d(new Private)
{
    d->image = image;
    d->currentNode = currentNode;
}

KisPlayInfo::KisPlayInfo(const KisPlayInfo& _rhs) : d(new Private(*_rhs.d))
{
}

KisPlayInfo& KisPlayInfo::operator=(const KisPlayInfo& _rhs)
{
    *d = *_rhs.d;
    return *this;
}

KisPlayInfo::~KisPlayInfo()
{
    delete d;
}

KisUndoAdapter* KisPlayInfo::undoAdapter() const
{
    return d->image->undoAdapter();
}

KisImageWSP KisPlayInfo::image() const
{
    return d->image;
}

KisNodeSP KisPlayInfo::currentNode() const
{
    return d->currentNode;
}
