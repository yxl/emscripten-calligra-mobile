/*
** A program to convert the XML rendered by KWord into LATEX.
**
** Copyright (C) 2000 Robert JACOLIN
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** To receive a copy of the GNU Library General Public License, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
**
*/

#include <kdebug.h>

#include "document.h"
#include "texte.h"

/*******************************************/
/* Constructor                             */
/*******************************************/
Document::Document()
{
}

/*******************************************/
/* Destructor                              */
/*******************************************/
Document::~Document()
{
	kdDebug() << "Corps Destructor" << endl;
}

/*******************************************/
/* Analyse                                 */
/*******************************************/
void Document::analyse(const Markup * balise_initiale)
{
	Markup *balise = 0;
		
	setTokenCurrent(balise_initiale->pContent);
	while((balise = getNextMarkup()) != 0)
	{
		kdDebug() << balise << endl;
		kdDebug() << balise->token.zText << endl;
		if(strcmp(balise->token.zText, "FRAMESET")== 0)
		{
			Element *elt = 0;
			kdDebug() <<"ANALYSE OF A FRAMESET" << endl;
			switch(getTypeFrameset(balise))
			{
				case ST_AUCUN: 
					kdDebug() << "NONE" << endl;
					break;
				case ST_IMAGE:
					kdDebug() << "PICTURE" << endl;
					// elt = new Image;
					// elt->analyse(balise);
					break;
				case ST_TEXTE: 
					kdDebug() << "TEXT" << endl;
					elt = new Texte;
					elt->analyse(balise);
					break;
				case ST_PARTS:
					break;
				default:
					kdDebug() << "error " << elt->getType() << " " << ST_TEXTE << endl;
			}
			
			/* 3. Add the Element in one of the lists */
			kdDebug() << "INFO : " << elt->getSection();
			switch(elt->getSection())
			{
				case SS_FOOTERS: kdDebug() << " FOOTER" <<endl;
					       _footer.add(elt);
					       break;
				case SS_HEADERS: kdDebug() << " HEADER" << endl;
						_header.add(elt);
					break;
				case SS_BODY: 	_corps.add(elt);
						kdDebug() << " BODY" << endl;
					break;
				case SS_FOOTNOTES: /* Just for the new kwd file version */
					break;
				default: kdDebug() << "UNKNOWN" << endl;
					break;
			}
		}
		kdDebug() << "END OF ANALYSE OF A FRAMESET" << endl;
	}
}

/*******************************************/
/* getTypeFrameset                         */
/*******************************************/
SType Document::getTypeFrameset(const Markup *balise)
{
	Arg*  arg  = 0;
	SType type = ST_AUCUN;

	for(arg= balise->pArg; arg!= 0; arg= arg->pNext)
	{
		kdDebug() << "param : " << arg->zName << " " << arg->zValue << endl;
		if(strcmp(arg->zName, "FRAMETYPE")== 0)
		{
			// A FINIR
			kdDebug() << "TYPE : TEXT" << endl;
			type = ST_TEXTE;
		}
	}
	kdDebug() << "END TYPE" << endl;
	return type;
}

/*******************************************/
/* Generate                                */
/*******************************************/
void Document::generate(QTextStream &out)
{
	ElementIter iter1;
	ElementIter iter2;
	ElementIter iter3;

	kdDebug() << "DOC. GENERATION." << endl;
	
	/* For each header */
	if(getFileHeader()->hasHeader())
	{
		kdDebug() << "header : " << _header.getSize() << endl;
		iter1.setList(&_header);

		while(!iter1.isTerminate())
		{
			generateTypeHeader(out, iter1.getCourant());
			iter1.next();
		}
	}
	
	/* For each footer */
	if(getFileHeader()->hasFooter())
	{
		kdDebug() << "footer : " << _header.getSize() << endl;
		iter2.setList(&_footer);

		while(!iter2.isTerminate())
		{
			generateTypeFooter(out, iter2.getCourant());
			iter2.next();
		}
	}
	if(getFileHeader()->hasHeader() || getFileHeader()->hasFooter())
		out << "\\pagestyle{fancy}" << endl;

	/* Body */
	kdDebug() << endl << "body : " << _corps.getSize() << endl;

	out << "\\begin{document}" << endl;
	if(_corps.getFirst() != 0)
		_corps.getFirst()->generate(out);
	out << "\\end{document}" << endl;
}

/*******************************************/
/* GenerateTypeHeader                      */
/*******************************************/
void Document::generateTypeHeader(QTextStream &out, Element *header)
{
	kdDebug() << "generate header" << endl;
	if(_fileHeader->getHeadType() == TH_ALL && header->getInfo() == SI_EVEN)
	{
		out << "\\lhead{}" << endl;
		out << "\\chead{";
		header->generate(out);
		out << "}" << endl;
		out << "\\rhead{}" << endl;
	}
	if(_fileHeader->getHeadType() == TH_EVODD)
	{
		switch(header->getInfo())
		{
			case SI_NONE:
			case SI_FIRST:
				break;
			case SI_ODD:
				out << "\\fancyhead[RO]{}" << endl;
				out << "\\fancyhead[CO]{";
				header->generate(out);
				out << "}" << endl;
				out << "\\fancyhead[LO]{}" << endl;
				break;
			case SI_EVEN:
				out << "\\fancyhead[RE]{}" << endl;
				out << "\\fancyhead[CE]{";
				header->generate(out);
				out << "}" << endl;
				out << "\\fancyhead[LE]{}" << endl;
				break;
		}
	}
	//if(_fileHeader->getHeadType() == TH_FIRST && header->getInfo() == SI_FIRST)
	if(header->getInfo() == SI_FIRST)
	{
		out << "\\markright{";
		header->generate(out);
		out << "}" << endl;
		out << "\\thispagestyle{fancy}" << endl;
	}
}

/*******************************************/
/* GenerateTypeFooter                      */
/*******************************************/
void Document::generateTypeFooter(QTextStream &out, Element *footer)
{
	if(_fileHeader->getFootType() == TH_ALL && footer->getInfo() == SI_EVEN)
	{
		out << "\\lfoot{}" << endl;
		out << "\\cfoot{";
		footer->generate(out);
		out << "}" << endl;
		out << "\\rfoot{}" << endl;
	}
	else if(_fileHeader->getFootType() == TH_EVODD)
	{
		switch(footer->getInfo())
		{
			case SI_NONE:
			case SI_FIRST:
				break;
			case SI_ODD:
				out << "\\fancyfoot[CO]{";
				footer->generate(out);
				out << "}";;
				break;
			case SI_EVEN:
				out << "\\fancyfoot[CE]{";
				footer->generate(out);
				out << "}";
				break;
		}
	}
	else if(_fileHeader->getFootType() == TH_FIRST && footer->getInfo() == SI_FIRST)
	{
		//out << "\\markright{}" << endl;
		//out << "\\thispagestyle{heading}" << endl;
	}
}
