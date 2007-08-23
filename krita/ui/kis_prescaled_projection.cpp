/*
 * Copyright (C) 2007, Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_prescaled_projection.h"

#include <QImage>
#include <QPixmap>
#include <QColor>
#include <QRect>
#include <QPoint>
#include <QSize>
#include <QPainter>

#include <blitz.h>

#include <KoColorProfile.h>
#include <KoViewConverter.h>

#include "kis_config.h"
#include "kis_config_notifier.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_mask.h"
#include "kis_node.h"
#include "kis_paint_device.h"
#include "kis_paint_layer.h"
#include "kis_selection.h"
#include "kis_selection_mask.h"
#include "kis_types.h"


// Casper's version that's fixed for jitter.
QImage sampleImage(const QImage& image, int columns, int rows, const QRect &dstRect)
{
    int *x_offset;
    int *y_offset;

    long j;
    long y;

    uchar *pixels;

    register const uchar *p;

    register long x;

    register uchar *q;

    /*
      Initialize sampled image attributes.
    */
    if ((columns == image.width()) && (rows == image.height()))
        return image.copy( dstRect );

    const int d = image.depth() / 8;

    QImage sample_image( dstRect.width(), dstRect.height(), image.depth());
    sample_image.setAlphaBuffer( image.hasAlphaBuffer());
    /*
      Allocate scan line buffer and column offset buffers.
    */
    pixels= new uchar[ image.width() * d ];
    x_offset= new int[ sample_image.width() ];
    y_offset= new int[ sample_image.height() ];
    /*
      Initialize pixel offsets.
    */
// In the following several code 0.5 needs to be added, otherwise the image
// would be moved by half a pixel to bottom-right, just like
// with Qt's QImage::scale()
    for (x=0; x < (long) sample_image.width(); x++)
    {
        x_offset[x] = int((x + dstRect.left()) * image.width() / columns);
    }
    for (y=0; y < (long) sample_image.height(); y++)
    {
        y_offset[y] = int((y + dstRect.top()) * image.height() / rows);
    }
    /*
      Sample each row.
    */
    j=(-1);
    for (y=0; y < (long) sample_image.height(); y++)
    {
        q= sample_image.scanLine( y );
        if (j != y_offset[y] )
        {
            /*
              Read a scan line.
            */
            j= y_offset[y];
            p= image.scanLine( j );
            (void) memcpy(pixels,p,image.width()*d);
        }
        /*
          Sample each column.
        */
        switch( d )
        {
        case 1: // 8bit
            for (x=0; x < (long) sample_image.width(); x++)
            {
                *q++=pixels[ x_offset[x] ];
            }
            break;
        case 4: // 32bit
            for (x=0; x < (long) sample_image.width(); x++)
            {
                *(QRgb*)q=((QRgb*)pixels)[ x_offset[x] ];
                q += d;
            }
            break;
        default:
            for (x=0; x < (long) sample_image.width(); x++)
            {
                memcpy( q, pixels + x_offset[x] * d, d );
                q += d;
            }
            break;
        }
    }
    if( d != 4 ) // != 32bit
    {
        sample_image.setNumColors( image.numColors());
        for( int i = 0; i < image.numColors(); ++i )
            sample_image.setColor( i, image.color( i ));
    }
    delete[] y_offset;
    delete[] x_offset;
    delete[] pixels;
    return sample_image;
}



struct KisPrescaledProjection::Private
{
    Private()
        : updateAllOfQPainterCanvas( false )
        , useDeferredSmoothing( false )
        , useNearestNeighbour( false )
        , useQtScaling( false )
        , useSampling( false )
        , useSmoothScaling( true ) // Default
        , drawCheckers( false )
        , scrollCheckers( false )
        , drawMaskVisualisationOnUnscaledCanvasCache( false )
        , cacheKisImageAsQImage( true )
        , showMask( true )
        , checkSize( 32 )
        , documentOffset( 0, 0 )
        , canvasSize( 0, 0 )
        , imageSize( 0, 0 )
        , viewConverter( 0 )
        , monitorProfile( 0 )
        , exposure( 0.0 )
        {
        }
    bool updateAllOfQPainterCanvas;
    bool useDeferredSmoothing;
    bool useNearestNeighbour;
    bool useQtScaling;
    bool useSampling;
    bool useSmoothScaling;
    bool drawCheckers;
    bool scrollCheckers;
    bool drawMaskVisualisationOnUnscaledCanvasCache;
    bool cacheKisImageAsQImage;
    bool showMask;
    QColor checkersColor;
    qint32 checkSize;
    QImage unscaledCache;
    QImage prescaledQImage;
    QPixmap prescaledPixmap;
    QPoint documentOffset;
    QSize canvasSize;
    QSize imageSize;
    KisImageSP image;
    KoViewConverter * viewConverter;
    KoColorProfile * monitorProfile;
    float exposure;
    KisNodeSP currentNode;
};

KisPrescaledProjection::KisPrescaledProjection()
    : QObject( 0 )
    , m_d( new Private() )
{
    updateSettings();
    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(updateSettings()));
}

KisPrescaledProjection::~KisPrescaledProjection()
{
    delete m_d;
}


void KisPrescaledProjection::setImage( KisImageSP image )
{
    m_d->image = image;
}

bool KisPrescaledProjection::drawCheckers() const
{
    return m_d->drawCheckers;
}


void KisPrescaledProjection::setDrawCheckers( bool drawCheckers )
{
    m_d->drawCheckers = drawCheckers;
}

QPixmap KisPrescaledProjection::prescaledPixmap() const
{
    return m_d->prescaledPixmap;
}

QImage KisPrescaledProjection::prescaledQImage() const
{
    return m_d->prescaledQImage;
}

void KisPrescaledProjection::setViewConverter( KoViewConverter * viewConverter )
{
    m_d->viewConverter = viewConverter;
}

void KisPrescaledProjection::updateSettings()
{
    KisConfig cfg;
    m_d->updateAllOfQPainterCanvas = cfg.updateAllOfQPainterCanvas();
    m_d->useDeferredSmoothing = cfg.useDeferredSmoothing();
    m_d->useNearestNeighbour = cfg.fastZoom();
    m_d->useQtScaling = cfg.useQtSmoothScaling();
    m_d->useSampling = cfg.useSampling();
    // If any of the above are true, we don't use our own smooth scaling
    m_d->useSmoothScaling = !( m_d->useNearestNeighbour ||
                               m_d->useSampling ||
                               m_d->useQtScaling ||
                               m_d->useDeferredSmoothing );
    m_d->scrollCheckers = cfg.scrollCheckers();
    m_d->checkSize = cfg.checkSize();
    m_d->checkersColor = cfg.checkersColor();
    m_d->cacheKisImageAsQImage = cfg.cacheKisImageAsQImage();
    m_d->drawMaskVisualisationOnUnscaledCanvasCache = cfg.drawMaskVisualisationOnUnscaledCanvasCache();
}

void KisPrescaledProjection::documentOffsetMoved( const QPoint &documentOffset )
{
    m_d->documentOffset = documentOffset;
}

void KisPrescaledProjection::updateCanvasProjection( const QRect & rc )
{

    // We cache the KisImage as a QImage
    if ( !m_d->useNearestNeighbour ) {

        if ( m_d->cacheKisImageAsQImage ) {

            QPainter p( &m_d->unscaledCache );
            p.setCompositionMode( QPainter::CompositionMode_Source );

            QImage updateImage = m_d->image->convertToQImage(rc.x(), rc.y(), rc.width(), rc.height(),
                                                             m_d->monitorProfile,
                                                             m_d->exposure);

            if ( m_d->showMask && m_d->drawMaskVisualisationOnUnscaledCanvasCache ) {

                // XXX: Also visualize the global selection

                KisSelectionSP selection = 0;
                if ( m_d->currentNode->inherits( "KisMask" ) ) {
                    selection = dynamic_cast<const KisMask*>( m_d->currentNode.data() )->selection();
                }
                else if ( m_d->currentNode->inherits( "KisLayer" ) ) {

                    KisLayerSP layer = dynamic_cast<KisLayer*>( m_d->currentNode.data() );
                    if ( KisSelectionMaskSP selectionMask = layer->selectionMask() ) {
                        selection = selectionMask->selection();
                    }

                    // XXX: transitional! Remove when we use
                    // KisSelectionMask instead of the selection in the
                    // paintdevice. That way we can also select on groups etc.
                    if ( !selection && m_d->currentNode->inherits( "KisPaintLayer" ) ) {
                        KisPaintDeviceSP dev = ( dynamic_cast<KisPaintLayer*>( m_d->currentNode.data() ) )->paintDevice();
                        if ( dev ) {
                            selection = dev->selection();
                        }
                    }
                }

                QTime t;
                t.start();
                selection->paint(&updateImage, rc);
                kDebug(41010) << "Mask visualisation rendering took: " << t.elapsed();
            }

            p.drawImage( rc.x(), rc.y(), updateImage, 0, 0, rc.width(), rc.height() );
            p.end();
        }
    }

    QRect vRect;

    if ( m_d->updateAllOfQPainterCanvas ) {
        vRect = QRect( m_d->documentOffset, m_d->canvasSize );
    }
    else {
        QRect vRect = viewRectFromImagePixels( rc );
    }

    if ( !vRect.isEmpty() ) {
        preScale( vRect );
    }

}

void KisPrescaledProjection::setImageSize(qint32 w, qint32 h)
{
    m_d->imageSize = QSize( w, h );
    if ( !m_d->useNearestNeighbour || !m_d->cacheKisImageAsQImage ) {
        m_d->unscaledCache = QImage( w, h, QImage::Format_ARGB32 );
    }
}

void KisPrescaledProjection::setMonitorProfile( KoColorProfile * profile )
{
    m_d->monitorProfile = profile;
}

void KisPrescaledProjection::setHDRExposure( float exposure )
{
    m_d->exposure = exposure;
}

void KisPrescaledProjection::setCurrentNode( const KisNodeSP node )
{
    m_d->currentNode = node;
}

void KisPrescaledProjection::showCurrentMask( bool showMask )
{
    m_d->showMask = showMask;
}


void KisPrescaledProjection::preScale()
{
    preScale( QRect( QPoint( 0, 0 ), m_d->canvasSize ) );
}

void KisPrescaledProjection::preScale( const QRect & rc )
{
    if ( !rc.isEmpty() ) {
        QTime t;
        t.start();
        QPainter gc( &m_d->prescaledQImage );
        gc.setCompositionMode( QPainter::CompositionMode_Source );
        drawScaledImage( rc, gc);
        kDebug(41010) <<"Prescaling took" << t.elapsed();
    }

}

void KisPrescaledProjection::resizePrescaledImage( QSize newSize, QSize oldSize )
{

    QTime t;
    t.start();

    QImage img = QImage(newSize, QImage::Format_ARGB32);
    QPainter gc( &img );
    gc.setCompositionMode( QPainter::CompositionMode_Source );

    if ( newSize.width() > oldSize.width() || newSize.height() > oldSize.height() ) {

        gc.drawImage( 0, 0,
                      m_d->prescaledQImage,
                      0, 0, m_d->prescaledQImage.width(), m_d->prescaledQImage.height() );

        QRegion r( QRect( 0, 0, newSize.width(), newSize.height() ) );
        r -= QRegion( QRect( 0, 0, m_d->prescaledQImage.width(), m_d->prescaledQImage.height() ) );

        foreach( QRect rc, r.rects() ) {
            drawScaledImage( rc, gc );
        }
    }
    else {
        gc.drawImage( 0, 0, m_d->prescaledQImage,
                      0, 0, m_d->prescaledQImage.width(), m_d->prescaledQImage.height() );
    }
    m_d->prescaledQImage = img;
    m_d->canvasSize = newSize;

    kDebug(41010) <<"Resize event:" << t.elapsed();

}

void KisPrescaledProjection::drawScaledImage( const QRect & rc,  QPainter & gc )
{
    if ( !m_d->image )
        return;

#if 0
    // When we're drawing pixels 1:1, don't try to scale at all
    if () {

    }
    else {
        // Use nearest neighbour interpolation from the raw KisImage
        if ( m_d->useNearestNeighbour || m_d->useDeferredSmoothing ) {
            if ( m_d->useDeferredSmoothing ) {
                // Start smoothing job. The job will be replaced by
                // the next smoothing job if it is added before this
                // one is done.
            }
        }
        else {

            QImage unscaled = m_d->unscaledCache;

            // If we don't cache the image as an unscaled QImage, get
            // an unscaled QImage for this rect from KisImage.
            if ( !m_d->cacheKisImageAsQImage ) {

            }

            if ( m_d->useQtScaling ) {
            }
            else if ( m_d->useSampling ) {

            }
            else { // Smooth scaling using blitz
            }
        }
    }
#endif
}

QRect KisPrescaledProjection::viewRectFromImagePixels( const QRect & rc )
{
    double pppx,pppy;
    pppx = m_d->image->xRes();
    pppy = m_d->image->yRes();

    QRectF docRect;
    docRect.setCoords((rc.left() - 2) / pppx, (rc.top() - 2) / pppy, (rc.right() + 2) / pppx, (rc.bottom() + 2) / pppy);

    QRect viewRect = m_d->viewConverter->documentToView(docRect).toAlignedRect();
    viewRect = viewRect.translated( -m_d->documentOffset );
    viewRect = viewRect.intersected( QRect( 0, 0, m_d->canvasSize.width(), m_d->canvasSize.width() ) );

    return viewRect;

}

#include "kis_prescaled_projection.moc"
