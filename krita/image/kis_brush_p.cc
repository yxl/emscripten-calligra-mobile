/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#include <cfloat>
#include <QImage>
#include <QVector>

#include <kdebug.h>

#include <KoColorSpace.h>

#include "kis_global.h"
#include "kis_brush_p.h"

KisBrush::ScaledBrush::ScaledBrush()
{
    m_mask = 0;
    m_image = QImage();
    m_scale = 1;
    m_xScale = 1;
    m_yScale = 1;
}

KisBrush::ScaledBrush::ScaledBrush(KisQImagemaskSP scaledMask, const QImage& scaledImage, double scale, double xScale, double yScale)
{
    m_mask = scaledMask;
    m_image = scaledImage;
    m_scale = scale;
    m_xScale = xScale;
    m_yScale = yScale;

    if (!m_image.isNull()) {
        // Convert image to pre-multiplied by alpha.

        m_image.detach();

        for (int y = 0; y < m_image.height(); y++) {
            for (int x = 0; x < m_image.width(); x++) {

                QRgb pixel = m_image.pixel(x, y);

                int red = qRed(pixel);
                int green = qGreen(pixel);
                int blue = qBlue(pixel);
                int alpha = qAlpha(pixel);

                red = (red * alpha) / 255;
                green = (green * alpha) / 255;
                blue = (blue * alpha) / 255;

                m_image.setPixel(x, y, qRgba(red, green, blue, alpha));
            }
        }
    }
}

KisQImagemask::KisQImagemask(const QImage& img, bool hasColor)
{
    m_width = img.width();
    m_height = img.height();

    if (hasColor) {
        copyAlpha(img);
    }
    else {
        computeAlpha(img);
    }
}

KisQImagemask::KisQImagemask(const QImage& img)
{
    m_width = img.width();
    m_height = img.height();

    if (!img.allGray()) {
        copyAlpha(img);
    }
    else {
        computeAlpha(img);
    }
}

KisQImagemask::KisQImagemask(qint32 width, qint32 height)
{
    m_width = width;
    m_height = height;

    m_data.clear();
    m_data.insert(0, width * height, OPACITY_TRANSPARENT);
}

KisQImagemask::~KisQImagemask()
{
}

qint32 KisQImagemask::width() const
{
    return m_width;
}

qint32 KisQImagemask::height() const
{
    return m_height;
}



void KisQImagemask::copyAlpha(const QImage& img)
{
    for (int y = 0; y < img.height(); y++) {
        for (int x = 0; x < img.width(); x++) {
                        QRgb c = img.pixel(x,y);
                        quint8 a = (qGray(c) * qAlpha(c)) / 255;
            m_data.push_back(a);

        }
    }
}

void KisQImagemask::computeAlpha(const QImage& img)
{
    // The brushes are mostly grayscale on a white background,
    // although some do have a colors. The alpha channel is seldom
    // used, so we take the average gray value of this pixel of
    // the brush as the setting for the opacitiy. We need to
    // invert it, because 255, 255, 255 is white, which is
    // completely transparent, but 255 corresponds to
    // OPACITY_OPAQUE.

    for (int y = 0; y < img.height(); y++) {
        for (int x = 0; x < img.width(); x++) {
            m_data.push_back(255 - qRed(img.pixel(x, y)));
        }
    }
}

KisQImagemaskSP KisQImagemask::interpolate(KisQImagemaskSP mask1, KisQImagemaskSP mask2, double t)
{
    Q_ASSERT((mask1->width() == mask2->width()) && (mask1->height() == mask2->height()));
    Q_ASSERT(t > -DBL_EPSILON && t < 1 + DBL_EPSILON);

    int width = mask1->width();
    int height = mask1->height();
    KisQImagemaskSP outputMask = KisQImagemaskSP(new KisQImagemask(width, height));
    Q_CHECK_PTR(outputMask);

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            quint8 d = static_cast<quint8>((1 - t) * mask1->alphaAt(x, y) + t * mask2->alphaAt(x, y));
            outputMask->setAlphaAt(x, y, d);
        }
    }

    return outputMask;
}


