/* This file is part of the KDE project
   Copyright (C) 2002, The Karbon Developers
*/

#include <qdom.h>
#include <qcstring.h>
#include <qfile.h>
#include <qstring.h>
#include <qcolor.h>

#include <kgenericfactory.h>
#include <koFilter.h>
#include <koFilterChain.h>
#include <koStore.h>

#include <kdebug.h>

#include <stdio.h>

#include "svgexport.h"


class SvgExportFactory : KGenericFactory<SvgExport, KoFilter>
{
public:
	SvgExportFactory( void )
		: KGenericFactory<SvgExport, KoFilter>( "karbonsvgexport" )
	{}

protected:
	virtual void setupTranslations( void )
	{
		KGlobal::locale()->insertCatalogue( "karbonsvgfilter" );
	}
};

K_EXPORT_COMPONENT_FACTORY( libkarbonsvgexport, SvgExportFactory() );

SvgExport::SvgExport( KoFilter*, const char*, const QStringList& )
	: KoFilter()
{
}

KoFilter::ConversionStatus
SvgExport::convert( const QCString& from, const QCString& to )
{
	if ( to != "image/svg+xml" || from != "application/x-karbon" )
	{
		return KoFilter::NotImplemented;
	}

	KoStore storeIn( m_chain->inputFile(), KoStore::Read );
	if( !storeIn.open( "root" ) )
	{
		storeIn.close();
		return KoFilter::StupidError;
	}

	QFile fileOut( m_chain->outputFile() );
	if( !fileOut.open( IO_WriteOnly ) ) {
		storeIn.close();
		return KoFilter::StupidError;
	}

	QByteArray byteArrayIn = storeIn.read( storeIn.size() );
	storeIn.close();

	QDomDocument domIn;
	domIn.setContent( byteArrayIn );
	QDomElement docNode = domIn.documentElement();

	QTextStream s( &fileOut );


	s <<
		"<?xml version=\"1.0\" standalone=\"no\"?>\n" <<
		"<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20010904//EN\" \"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">" <<
	endl;
	
	s <<
		"<svg xmlns=\"http://www.w3.org/2000/svg\">" <<
	endl;

	// parse dom-tree:
	QDomNodeList list = domIn.childNodes();
	for( uint i = 0; i < list.count(); ++i )
	{
		if( list.item( i ).isElement() )
		{
			QDomElement e = list.item( i ).toElement();

			if( e.tagName() == "DOC" )
				exportDocument( s, e );
		}
	}
	
	s <<
		"</svg>" <<
	endl;

	fileOut.close();
	return KoFilter::OK;
}

void
SvgExport::exportDocument( QTextStream& s, const QDomElement& node )
{
	QDomNodeList list = node.childNodes();
	for( uint i = 0; i < list.count(); ++i )
	{
		if( list.item( i ).isElement() )
		{
			QDomElement e = list.item( i ).toElement();

			if( e.tagName() == "LAYER" )
				exportLayer( s, e );
		}
	}
}

void
SvgExport::exportLayer( QTextStream& s, const QDomElement& node )
{
	QDomNodeList list = node.childNodes();
	for( uint i = 0; i < list.count(); ++i )
	{
		if( list.item( i ).isElement() )
		{
			QDomElement e = list.item( i ).toElement();

			if( e.tagName() == "PATH" )
				exportPath( s, e );
			if( e.tagName() == "TEXT" )
				exportText( s, e );
		}
	}
}

void
SvgExport::exportPath( QTextStream& s, const QDomElement& node )
{

	s << "<path";
	
	fill_rule = node.attribute( "fillRule" ).toInt();

	QDomNodeList list = node.childNodes();
	for( uint i = 0; i < list.count(); ++i )
	{
		if( list.item( i ).isElement() )
		{
			QDomElement e = list.item( i ).toElement();

			if( e.tagName() == "SEGMENTS" )
				exportSegments( s, e );
			if( e.tagName() == "FILL" )
				exportFill( s, e );
			if( e.tagName() == "STROKE" )
				exportStroke( s, e );
		}
	}

	s << " />" << endl;

}

void
SvgExport::exportSegments( QTextStream& s, const QDomElement& node )
{

	s << " d=\"";

	QDomNodeList list = node.childNodes();
	for( uint i = 0; i < list.count(); ++i )
	{
		if( list.item( i ).isElement() )
		{
			QDomElement e = list.item( i ).toElement();

			// TODO: CURVE1 CURVE2

			if( e.tagName() == "CURVE" )
			{
				s <<	
					"C" <<
					e.attribute( "x1" ) << "," <<
					e.attribute( "y1" ) << " " <<
					e.attribute( "x2" ) << "," <<
					e.attribute( "y2" ) << " " <<
					e.attribute( "x3" ) << "," <<
					e.attribute( "y3" ) << " ";
			}
			else if( e.tagName() == "LINE" )
			{
				s <<	
					"L" <<
					e.attribute( "x" ) << " " <<
					e.attribute( "y" ) << " ";
			}
			else if( e.tagName() == "MOVE" )
			{
				s <<	
					"M" <<
					e.attribute( "x" ) << " " <<
					e.attribute( "y" ) << " ";
			}
		}
	}
	
	if( node.attribute( "isClosed" ).toInt() == 1);
		s << "Z";

	s << "\"";
}

void
SvgExport::exportFill( QTextStream& s, const QDomElement& node )
{

	if( fill_rule == 1 )
	{
		s << " fill-rule=\"evenodd\"";
	}

	QDomNodeList list = node.childNodes();
	for( uint i = 0; i < list.count(); ++i )
	{
		if( list.item( i ).isElement() )
		{
			QDomElement e = list.item( i ).toElement();

			if( e.tagName() == "COLOR" )
			{

				if( !e.attribute( "colorSpace" ).isNull() )	// make sure getHexColor returns something
				{						// shouldn't be needed really
					s << " fill=\"";
					getHexColor( s, e );
					s << "\"";
				}

				if( !e.attribute( "opacity" ).isNull() )
				{
					s << " fill-opacity=\"" << e.attribute( "opacity" ) << "\"";
				}
			}
		}
	}

}

void
SvgExport::exportStroke( QTextStream& s, const QDomElement& node )
{

	// TODO: DASHPATTERN

	if( !node.attribute( "lineWidth" ).isNull() )
	{
		s << " stroke-width=\"" << node.attribute( "lineWidth" ) << "\"";
	}

	if( !node.attribute( "lineCap" ).isNull() )
	{
		if( node.attribute( "lineCap" ).toInt() == 0 )
		{
			s << " stroke-linecap=\"butt\"";
		}
		else if( node.attribute( "lineCap" ).toInt() == 1 )
		{
			s << " stroke-linecap=\"round\"";
		}
		else if( node.attribute( "lineCap" ).toInt() == 2 )
		{
			s << " stroke-linecap=\"square\"";
		}
	}

	if( !node.attribute( "lineJoin" ).isNull() )
	{
		if( node.attribute( "lineJoin" ).toInt() == 0 )
		{
			s << " stroke-linejoin=\"miter\"";
		}
		else if( node.attribute( "lineJoin" ).toInt() == 1 )
		{
			s << " stroke-linejoin=\"round\"";
		}
		else if( node.attribute( "lineJoin" ).toInt() == 2 )
		{
			s << " stroke-linejoin=\"bevel\"";
		}
	}

	if( !node.attribute( "miterLimit" ).isNull() )
	{
		s << " stroke-miterlimit=\"" << node.attribute( "miterLimit" ) << "\"";
	}

	QDomNodeList list = node.childNodes();
	for( uint i = 0; i < list.count(); ++i )
	{
		if( list.item( i ).isElement() )
		{
			QDomElement e = list.item( i ).toElement();

			if( e.tagName() == "COLOR" )
			{
				if( !e.attribute( "colorSpace" ).isNull() )	// make sure getHexColor returns something
				{						// shouldn't be needed really
					s << " stroke=\"";
					getHexColor( s, e );
					s << "\"";
				}

				if( !e.attribute( "opacity" ).isNull() )
				{
					s << " stroke-opacity=\"" << e.attribute( "opacity" ) << "\"";
				}
			}
		}
	}

}

void
SvgExport::getHexColor( QTextStream& s, const QDomElement& node )
{

	QString Output;

	if( node.attribute( "colorSpace" ).toInt() == 0 ) // rgb
	{
		Output.sprintf( "#%02x%02x%02x", int( node.attribute( "v1" ).toFloat() * 255 ), int( node.attribute( "v2" ).toFloat() * 255 ), int( node.attribute( "v3" ).toFloat() * 255 ) );
	}
	else if( node.attribute( "colorSpace" ).toInt() == 1 ) // cmyk
	{
		Output.sprintf( "#%02x%02x%02x", int( ( 1 - node.attribute( "v1" ).toFloat() - node.attribute( "v4" ).toFloat() ) * 255 ), int( ( 1 - node.attribute( "v2" ).toFloat() - node.attribute( "v4" ).toFloat() ) * 255 ), int( ( 1 - node.attribute( "v3" ).toFloat() - node.attribute( "v4" ).toFloat() ) * 255 ) );
	}
	else if( node.attribute( "colorSpace" ).toInt() == 2 ) // hsb
	{
		// maybe do this manually - or could it stay like this?
		QColor hsvColor;
		int rComponent;
		int gComponent;
		int bComponent;
		hsvColor.setHsv( int( node.attribute( "v1" ).toFloat() * 359 ), int( node.attribute( "v2" ).toFloat() * 255 ), int( node.attribute( "v3" ).toFloat() * 255 ) );
		hsvColor.rgb(&rComponent, &gComponent, &bComponent);

		Output.sprintf( "#%02x%02x%02x", rComponent, gComponent, bComponent );
	}
	else if( node.attribute( "colorSpace" ).toInt() == 3 ) // grey
	{
		Output.sprintf( "#%02x%02x%02x", int( node.attribute( "v" ).toFloat() * 255 ), int( node.attribute( "v" ).toFloat() * 255 ), int( node.attribute( "v" ).toFloat() * 255 ) );
	}
	
	s << Output;

}

void
SvgExport::exportText( QTextStream& s, const QDomElement& node )
{

	s << "<text";
	
	if( !node.attribute( "size" ).isNull() )
	{
		s << " font-size=\"" << node.attribute( "size" ) << "\"";
	}
	
	if( !node.attribute( "family" ).isNull() )
	{
		s << " font-family=\"" << node.attribute( "family" ) << "\"";
	}
	
	if( !node.attribute( "bold" ).isNull() )
	{
		s << " font-weight=\"bold\"";
	}
	
	if( !node.attribute( "italic" ).isNull() )
	{
		s << " font-style=\"italic\"";
	}

	s << ">";
	

	if( !node.attribute( "text" ).isNull() )
	{
		s << node.attribute( "text" );
	}
	
	s << "</text>" << endl;

}

#include "svgexport.moc"

