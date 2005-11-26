/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
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

#include "krs_doc.h"

#include <kis_doc.h>
#include <kis_image.h>

#include "krs_image.h"

namespace Kross { namespace KritaCore {

Doc::Doc(::KisDoc* doc) : Kross::Api::Class<Doc>("KritaDocument", 0 ), m_doc(doc) {
    addFunction("getImage", &Doc::getImage);
    addFunction("notifyModification", &Doc::notifyModification);
}

Doc::~Doc() {
    
}

const QString Doc::getClassName() const {
    return "Kross::KritaCore::Doc";
}

Kross::Api::Object::Ptr Doc::getImage(Kross::Api::List::Ptr)
{
    return new Image(m_doc->currentImage());
}

Kross::Api::Object::Ptr Doc::notifyModification(Kross::Api::List::Ptr args)
{
    m_doc->setModified(true);
    m_doc->currentImage()->notify();
    return 0;
}

}
}
