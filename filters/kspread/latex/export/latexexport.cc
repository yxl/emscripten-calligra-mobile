/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#include <latexexport.h>
#include <latexexport.moc>
#include <kdebug.h>
#include <koFilterChain.h>
#include <kgenericfactory.h>
#include <kglobal.h>
#include <klocale.h>
#include <qtextcodec.h>
#include "kspreadlatexexportdiaImpl.h"

typedef KGenericFactory<LATEXExport, KoFilter> LATEXExportFactory;
K_EXPORT_COMPONENT_FACTORY( libkspreadlatexexport, LATEXExportFactory( "kofficefilters" ) )


LATEXExport::LATEXExport(KoFilter *, const char *, const QStringList&) :
                     KoFilter() {
}

KoFilter::ConversionStatus LATEXExport::convert( const QCString& from, const QCString& to )
{
    QString config;

    if(to != "text/x-tex" || from != "application/x-kspread")
        return KoFilter::NotImplemented;

    KoStore* in = KoStore::createStore(m_chain->inputFile(), KoStore::Read);
    if(!in || !in->open("root")) {
        kdError(30503) << "Unable to open input file!" << endl;
        delete in;
        return KoFilter::FileNotFound;
    }
		kdDebug(30522) << "In the kspread latex export filter..." << endl;
    /* input file Reading */
    in->close();

		KSpreadLatexExportDiaImpl* dialog = new KSpreadLatexExportDiaImpl(in);
    dialog->setOutputFile(m_chain->outputFile());

    dialog->exec();
    delete dialog;
    delete in;

    return KoFilter::OK;
}
