/*
 * shearimage.cc -- Part of Krita
 *
 * Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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

#include "shearimage.h"

#include <klocale.h>
#include <kpluginfactory.h>

#include <kis_image.h>
#include <kis_view2.h>
#include <kis_node_manager.h>
#include <kis_image_manager.h>
#include <kis_action.h>

#include "dlg_shearimage.h"

K_PLUGIN_FACTORY(ShearImageFactory, registerPlugin<ShearImage>();)
K_EXPORT_PLUGIN(ShearImageFactory("krita"))

ShearImage::ShearImage(QObject *parent, const QVariantList &)
        : KisViewPlugin(parent, "kritaplugins/shearimage.rc")
{
    KisAction *action  = new KisAction(i18n("&Shear Image..."), this);
    addAction("shearimage", action);
    connect(action,  SIGNAL(triggered()), this, SLOT(slotShearImage()));

    action = new KisAction(i18n("&Shear Layer..."), this);
    action->setActivationFlags(KisAction::ACTIVE_LAYER);
    action->setActivationConditions(KisAction::ACTIVE_NODE_EDITABLE);
    addAction("shearlayer", action);
    connect(action,  SIGNAL(triggered()), this, SLOT(slotShearLayer()));
}

ShearImage::~ShearImage()
{
}

void ShearImage::slotShearImage()
{
    KisImageWSP image = m_view->image();

    if (!image) return;

    DlgShearImage * dlgShearImage = new DlgShearImage(m_view, "ShearImage");
    Q_CHECK_PTR(dlgShearImage);

    dlgShearImage->setCaption(i18n("Shear Image"));

    if (dlgShearImage->exec() == QDialog::Accepted) {
        qint32 angleX = dlgShearImage->angleX();
        qint32 angleY = dlgShearImage->angleY();
        m_view->imageManager()->shearCurrentImage(angleX, angleY);
    }
    delete dlgShearImage;
}

void ShearImage::slotShearLayer()
{
    KisImageWSP image = m_view->image();

    if (!image) return;

    DlgShearImage * dlgShearImage = new DlgShearImage(m_view, "ShearLayer");
    Q_CHECK_PTR(dlgShearImage);

    dlgShearImage->setCaption(i18n("Shear Layer"));

    if (dlgShearImage->exec() == QDialog::Accepted) {
        qint32 angleX = dlgShearImage->angleX();
        qint32 angleY = dlgShearImage->angleY();
        m_view->nodeManager()->shear(angleX, angleY);

    }
    delete dlgShearImage;
}

#include "shearimage.moc"
