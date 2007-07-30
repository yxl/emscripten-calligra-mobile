/*
 *  Copyright (c) 2007 Emanuele Tamponi <emanuele@valinor.it>
 *
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

#include <lcms.h>

#include <QString>

#include <KLocale>

#include "KoColorSpaceRegistry.h"
#include "KoColorProfile.h"

#include "compositeops/KoCompositeOpOver.h"
#include "compositeops/KoCompositeOpErase.h"
#include "compositeops/KoCompositeOpMultiply.h"
#include "compositeops/KoCompositeOpDivide.h"
#include "compositeops/KoCompositeOpBurn.h"

#include "kis_illuminant_profile.h"
#include "mathematics.h"

#include "kis_ks_colorspace.h"

KisKSColorSpace::KisKSColorSpace(KoColorProfile *p)
	: KoIncompleteColorSpace<KSTraits, KoRGB16Fallback>("kscolorspace", "", KoColorSpaceRegistry::instance())
{
	if (profileIsCompatible(p))
		m_profile = dynamic_cast<KisIlluminantProfile*>(p);

	const quint32 ncols = m_profile->matrix().ncols();

	for (quint32 i = 0; i < 2*ncols; i+=2) {
		addChannel(new KoChannelInfo(i18n("Absorption"),
				   i+0 * sizeof(float),
				   KoChannelInfo::COLOR,
				   KoChannelInfo::FLOAT32,
				   sizeof(float),
				   QColor(0,0,255)));

		addChannel(new KoChannelInfo(i18n("Scattering"),
				   i+1 * sizeof(float),
				   KoChannelInfo::COLOR,
				   KoChannelInfo::FLOAT32,
				   sizeof(float),
				   QColor(255,0,0)));
	}

	addChannel(new KoChannelInfo(i18n("Alpha"),
			   2 * ncols * sizeof(float),
			   KoChannelInfo::ALPHA,
			   KoChannelInfo::FLOAT32,
			   sizeof(float)));

	addCompositeOp( new KoCompositeOpOver<KSTraits>( this ) );
	addCompositeOp( new KoCompositeOpErase<KSTraits>( this ) );
	addCompositeOp( new KoCompositeOpMultiply<KSTraits>( this ) );
	addCompositeOp( new KoCompositeOpDivide<KSTraits>( this ) );
	addCompositeOp( new KoCompositeOpBurn<KSTraits>( this ) );

	hsRGB = cmsCreate_sRGBProfile();
	hXYZ  = cmsCreateXYZProfile();

	XYZ_BGR = cmsCreateTransform(hXYZ, TYPE_XYZ_DBL, hsRGB, TYPE_BGR_16,
								 INTENT_ABSOLUTE_COLORIMETRIC, cmsFLAGS_NOTPRECALC);
	BGR_XYZ = cmsCreateTransform(hsRGB, TYPE_BGR_16, hXYZ, TYPE_XYZ_DBL,
								 INTENT_ABSOLUTE_COLORIMETRIC, cmsFLAGS_NOTPRECALC);
}

bool KisKSColorSpace::profileIsCompatible(KoColorProfile* profile) const
{
	if (!dynamic_cast<KisIlluminantProfile*>(profile))
		return false;

	return true;
}

void KisKSColorSpace::fromRgbA16(const quint8 * srcU8, quint8 * dstU8, quint32 nPixels) const
{
	const quint32 ncols = _to_decide_;
	const quint16 *src16 = reinterpret_cast<const quint16 *>(srcU8);
	float *dstf = reinterpret_cast<float *>(dstU8);

	double XYZ[3], REV[ncols];

	for (quint32 i = 0; i < nPixels; i++) {
		cmsDoTransform(BGR_XYZ, const_cast<quint16*>(src16), XYZ, 1);

		simplex(3, ncols, m_profile->matrix(), REF, XYZ);

		maths::computeKS(ncols, REF, dstf);

		dstf[2*ncols] = convert2f(src16[3]);

		dstf += 2*ncols + 1;
		src16 += 4;
	}
}

void KisKSColorSpace::toRgbA16(const quint8 * srcU8, quint8 * dstU8, quint32 nPixels) const
{
	const quint32 ncols = _to_decide_;
	const float *srcf = reinterpret_cast<const float *>(srcU8);
	quint16 *dst16 = reinterpret_cast<quint16 *>(dstU8);

	double XYZ[3], REF[ncols];

	for (quint32 i = 0; i < nPixels; i++) {
		computeReflectance(ncols, srcf, REF);

		maths::mult(3, ncols, m_profile->matrix(), REF, XYZ);

		cmsDoTransform(XYZ_BGR, XYZ, dst16, 1);

		dst16[3] = convert2i(srcf[2*ncols]);

		srcf += 2*ncols + 1;
		dst16 += 4;
	}
}
