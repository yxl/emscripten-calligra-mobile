/* This file is part of the KDE project

   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "KoCanvasBase.h"
#include "KoCanvasResourceManager.h"
#include "KoShapeController.h"
#include "KoCanvasController.h"
#include "KoViewConverter.h"
#include "KoSnapGuide.h"
#include "SnapGuideConfigWidget.h"
#include "KoShapeManager.h"
#include "KoToolProxy.h"
#include "KoSelection.h"
#include <kglobal.h>
#include <kconfiggroup.h>
#include <ksharedptr.h>
#include <ksharedconfig.h>

class KoCanvasBase::Private
{
public:
    Private() : shapeController(0),
        resourceManager(0),
        controller(0),
        snapGuide(0)
    {
    }

    ~Private() {
        delete shapeController;
        delete resourceManager;
        delete snapGuide;
    }
    KoShapeController *shapeController;
    KoCanvasResourceManager *resourceManager;
    KoCanvasController *controller;
    KoSnapGuide *snapGuide;
};

KoCanvasBase::KoCanvasBase(KoShapeBasedDocumentBase *shapeBasedDocument)
        : d(new Private())
{
    d->resourceManager = new KoCanvasResourceManager();
    d->shapeController = new KoShapeController(this, shapeBasedDocument);
    d->snapGuide = new KoSnapGuide(this);
}

KoCanvasBase::~KoCanvasBase()
{
    delete d;
}

QPointF KoCanvasBase::viewToDocument(const QPointF &viewPoint) const
{
    return viewConverter()->viewToDocument(viewPoint - documentOrigin());;
}

KoShapeController *KoCanvasBase::shapeController() const
{
    return d->shapeController;
}

void KoCanvasBase::disconnectCanvasObserver(QObject *object)
{
    shapeManager()->selection()->disconnect(object);
    resourceManager()->disconnect(object);
    shapeManager()->disconnect(object);
    toolProxy()->disconnect(object);
}


KoCanvasResourceManager *KoCanvasBase::resourceManager() const
{
    return d->resourceManager;
}

void KoCanvasBase::ensureVisible(const QRectF &rect)
{
    if (d->controller && d->controller->canvas())
        d->controller->ensureVisible(
                d->controller->canvas()->viewConverter()->documentToView(rect));
}

void KoCanvasBase::setCanvasController(KoCanvasController *controller)
{
    d->controller = controller;
}

KoCanvasController *KoCanvasBase::canvasController() const
{
    return d->controller;
}

void KoCanvasBase::clipToDocument(const KoShape *, QPointF &) const
{
}

KoSnapGuide * KoCanvasBase::snapGuide() const
{
    return d->snapGuide;
}

KoGuidesData * KoCanvasBase::guidesData()
{
    return 0;
}

QWidget *KoCanvasBase::createSnapGuideConfigWidget() const
{
    return new SnapGuideConfigWidget(d->snapGuide);
}
