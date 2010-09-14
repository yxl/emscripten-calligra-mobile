/*
 *  Copyright (c) 2006 Boudewijn Rempt  <boud@valdyas.org>
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
#include "kis_canvas_resource_provider.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <fixx11h.h>
#include <QX11Info>
#endif

#include <QImage>
#include <QPainter>

#include <KoCanvasBase.h>
#include <KoID.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>
#include <KoColorProfile.h>
#include <KoAbstractGradient.h>
#include <KoCompositeOp.h>
#include <KoResourceServerProvider.h>
#include <KoStopGradient.h>

#include <kis_pattern.h>
#include <kis_paint_device.h>
#include <filter/kis_filter_configuration.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_paintop_preset.h>
#include <kis_paintop_settings.h>
#include "ko_favorite_resource_manager.h"

#include "kis_exposure_visitor.h"
#include "kis_config.h"
#include "kis_view2.h"
#include "canvas/kis_canvas2.h"

KisCanvasResourceProvider::KisCanvasResourceProvider(KisView2 * view)
        : m_view(view)
{
    m_fGChanged = true;  
    m_enablefGChange = true;    // default to true, so that colour history is working without popup palette
}

KisCanvasResourceProvider::~KisCanvasResourceProvider()
{
}

void KisCanvasResourceProvider::setResourceManager(KoResourceManager *resourceManager)
{
    m_resourceManager = resourceManager;

    QVariant v;
    v.setValue(KoColor(Qt::black, m_view->image()->colorSpace()));
    m_resourceManager->setResource(KoCanvasResource::ForegroundColor, v);

    v.setValue(KoColor(Qt::white, m_view->image()->colorSpace()));
    m_resourceManager->setResource(KoCanvasResource::BackgroundColor, v);

    setCurrentCompositeOp(COMPOSITE_OVER);
    resetDisplayProfile();

    connect(m_resourceManager, SIGNAL(resourceChanged(int, const QVariant &)),
            this, SLOT(slotResourceChanged(int, const QVariant&)));

}


KoCanvasBase * KisCanvasResourceProvider::canvas() const
{
    return m_view->canvasBase();
}

KoColor KisCanvasResourceProvider::bgColor() const
{
    return m_resourceManager->resource(KoCanvasResource::BackgroundColor).value<KoColor>();
}

KoColor KisCanvasResourceProvider::fgColor() const
{
    return m_resourceManager->resource(KoCanvasResource::ForegroundColor).value<KoColor>();
}

float KisCanvasResourceProvider::HDRExposure() const
{
    return static_cast<float>(m_resourceManager->resource(HdrExposure).toDouble());
}

void KisCanvasResourceProvider::setHDRExposure(float exposure)
{
    m_resourceManager->setResource(HdrExposure, static_cast<double>(exposure));
    KisExposureVisitor eV(exposure);
    m_view->image()->projection()->colorSpace()->profile()->setProperty("exposure", exposure);
    m_view->image()->rootLayer()->accept(eV);
    m_view->canvasBase()->updateCanvas();
    m_view->canvasBase()->startUpdateCanvasProjection(m_view->image()->bounds());
}


KisPattern * KisCanvasResourceProvider::currentPattern() const
{
    return static_cast<KisPattern*>(m_resourceManager->resource(CurrentPattern).value<void *>());
}

KisFilterConfiguration * KisCanvasResourceProvider::currentGeneratorConfiguration() const
{
    return static_cast<KisFilterConfiguration*>(m_resourceManager->
            resource(CurrentGeneratorConfiguration).value<void *>());
}


KoAbstractGradient* KisCanvasResourceProvider::currentGradient() const
{
    return static_cast<KoAbstractGradient*>(m_resourceManager->resource(CurrentGradient).value<void *>());
}


void KisCanvasResourceProvider::resetDisplayProfile()
{
    // XXX: The X11 monitor profile overrides the settings
    m_displayProfile = KisCanvasResourceProvider::getScreenProfile();

    if (m_displayProfile == 0) {
        KisConfig cfg;
        QString monitorProfileName = cfg.monitorProfile();
        m_displayProfile = KoColorSpaceRegistry::instance()->profileByName(monitorProfileName);
    }
    emit sigDisplayProfileChanged(m_displayProfile);
}

const KoColorProfile * KisCanvasResourceProvider::currentDisplayProfile() const
{
    return m_displayProfile;

}

KisImageWSP KisCanvasResourceProvider::currentImage() const
{
    return m_view->image();
}

KisNodeSP KisCanvasResourceProvider::currentNode() const
{
    return m_view->activeNode();
}

KisPaintOpPresetSP KisCanvasResourceProvider::currentPreset() const
{
    KisPaintOpPresetSP preset = m_resourceManager->resource(CurrentPaintOpPreset).value<KisPaintOpPresetSP>();
    return preset;
}


void KisCanvasResourceProvider::slotPatternActivated(KoResource * res)
{
    KisPattern * pattern = dynamic_cast<KisPattern*>(res);
    QVariant v = qVariantFromValue((void *) pattern);
    m_resourceManager->setResource(CurrentPattern, v);
    emit sigPatternChanged(pattern);
}

void KisCanvasResourceProvider::slotGeneratorConfigurationActivated(KisFilterConfiguration * res)
{
    KisFilterConfiguration * generatorConfiguration = dynamic_cast<KisFilterConfiguration*>(res);
    QVariant v = qVariantFromValue((void *) generatorConfiguration);
    m_resourceManager->setResource(CurrentGeneratorConfiguration, v);
    emit sigGeneratorConfigurationChanged(generatorConfiguration);
}

void KisCanvasResourceProvider::slotGradientActivated(KoResource *res)
{

    KoAbstractGradient * gradient = dynamic_cast<KoAbstractGradient*>(res);
    QVariant v = qVariantFromValue((void *) gradient);
    m_resourceManager->setResource(CurrentGradient, v);
    emit sigGradientChanged(gradient);
}

void KisCanvasResourceProvider::slotPaintOpPresetActivated(const KisPaintOpPresetSP preset)
{

    Q_ASSERT(preset->valid());
    Q_ASSERT(!preset->paintOp().id().isEmpty());
    Q_ASSERT(preset->settings());
    if (!preset) return;

    dbgUI << "slotPaintOpPresetActivated" << preset->paintOp();

    QVariant v;
    v.setValue(preset);
    m_resourceManager->setResource(CurrentPaintOpPreset, v);
    emit sigPaintOpPresetChanged(preset);
}

void KisCanvasResourceProvider::setBGColor(const KoColor& c)
{

    QVariant v;
    v.setValue(c);
    m_resourceManager->setResource(KoCanvasResource::BackgroundColor, v);
    emit sigBGColorChanged(c);
}

void KisCanvasResourceProvider::setFGColor(const KoColor& c)
{
    m_fGChanged = true;

    QVariant v;
    v.setValue(c);
    m_resourceManager->setResource(KoCanvasResource::ForegroundColor, v);
    emit sigFGColorChanged(c);
}

void KisCanvasResourceProvider::slotSetFGColor(const KoColor& c)
{
    setFGColor(c);
}

void KisCanvasResourceProvider::slotSetBGColor(const KoColor& c)
{
    setBGColor(c);
}

void KisCanvasResourceProvider::slotNodeActivated(const KisNodeSP node)
{
    QVariant v;
    v.setValue(node);
    m_resourceManager->setResource(CurrentKritaNode, v);
    emit sigNodeChanged(currentNode());
}


void KisCanvasResourceProvider::slotImageSizeChanged()
{
    if (KisImageWSP image = m_view->image()) {
        float fw = image->width() / image->xRes();
        float fh = image->height() / image->yRes();

        QSizeF postscriptSize(fw, fh);
        m_resourceManager->setResource(KoCanvasResource::PageSize, postscriptSize);
    }
}

void KisCanvasResourceProvider::slotSetDisplayProfile(const KoColorProfile * profile)
{
    m_displayProfile = const_cast<KoColorProfile*>(profile);
    emit sigDisplayProfileChanged(profile);
}

void KisCanvasResourceProvider::slotResourceChanged(int key, const QVariant & res)
{
    if(key == KoCanvasResource::ForegroundColor || key == KoCanvasResource::BackgroundColor) {
        KoAbstractGradient* resource = KoResourceServerProvider::instance()->gradientServer()->resources()[0];
        KoStopGradient* stopGradient = dynamic_cast<KoStopGradient*>(resource);
        if(stopGradient) {
            QList<KoGradientStop> stops;
            stops << KoGradientStop(0.0, fgColor()) << KoGradientStop(1.0, bgColor());
            stopGradient->setStops(stops);
            KoResourceServerProvider::instance()->gradientServer()->updateResource(resource);
        }
        resource = KoResourceServerProvider::instance()->gradientServer()->resources()[1];
        stopGradient = dynamic_cast<KoStopGradient*>(resource);
        if(stopGradient) {
            QList<KoGradientStop> stops;
            stops << KoGradientStop(0.0, fgColor()) << KoGradientStop(1.0,  KoColor(QColor(0, 0, 0, 0), fgColor().colorSpace()));
            stopGradient->setStops(stops);
            KoResourceServerProvider::instance()->gradientServer()->updateResource(resource);
        }
    }
    switch (key) {
    case(KoCanvasResource::ForegroundColor):
        m_fGChanged = true;
        emit sigFGColorChanged(res.value<KoColor>());
        break;
    case(KoCanvasResource::BackgroundColor):
        emit sigBGColorChanged(res.value<KoColor>());
        break;
    case(CurrentPattern):
        emit sigPatternChanged(static_cast<KisPattern *>(res.value<void *>()));
        break;
    case(CurrentGeneratorConfiguration):
        emit sigGeneratorConfigurationChanged(static_cast<KisFilterConfiguration*>(res.value<void*>()));
    case(CurrentGradient):
        emit sigGradientChanged(static_cast<KoAbstractGradient *>(res.value<void *>()));
        break;
    case(CurrentPaintOpPreset):
        emit sigPaintOpPresetChanged(currentPreset());
        break;
    case(CurrentKritaNode) :
        emit sigNodeChanged(currentNode());
        break;
    case(CurrentCompositeOp) :
        emit sigCompositeOpChanged(currentCompositeOp());
    default:
        ;
        // Do nothing
    };
}

const KoColorProfile *KisCanvasResourceProvider::getScreenProfile(int screen)
{
#ifdef Q_WS_X11

    Atom type;
    int format;
    unsigned long nitems;
    unsigned long bytes_after;
    quint8 * str;

    static Atom icc_atom = XInternAtom(QX11Info::display(), "_ICC_PROFILE", True);

    if (XGetWindowProperty(QX11Info::display(),
                           QX11Info::appRootWindow(screen),
                           icc_atom,
                           0,
                           INT_MAX,
                           False,
                           XA_CARDINAL,
                           &type,
                           &format,
                           &nitems,
                           &bytes_after,
                           (unsigned char **) &str) == Success
       ) {
        QByteArray bytes(nitems, '\0');
        bytes = QByteArray::fromRawData((char*)str, (quint32)nitems);

        return KoColorSpaceRegistry::instance()->createColorProfile(RGBAColorModelID.id(), Integer8BitsColorDepthID.id(), bytes);
        return 0;
    } else {
        return 0;
    }
#else
    return 0;

#endif
}

void KisCanvasResourceProvider::setCurrentCompositeOp(const QString& compositeOp)
{
    QVariant v;
    v.setValue(compositeOp);
    m_resourceManager->setResource(CurrentCompositeOp, v);
    emit sigCompositeOpChanged(compositeOp);
}

QString KisCanvasResourceProvider::currentCompositeOp() const
{
    return m_resourceManager->resource(CurrentCompositeOp).value<QString>();
}

void KisCanvasResourceProvider::slotPainting()
{

    if (m_fGChanged && m_enablefGChange) {
        emit sigFGColorUsed(fgColor());
        m_fGChanged = false;
    }
}

void KisCanvasResourceProvider::slotResetEnableFGChange(bool b)
{
    m_enablefGChange = b;
}

#include "kis_canvas_resource_provider.moc"
