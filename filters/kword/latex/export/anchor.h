/* A PARA IS A TITLE, A SET OF WORDS OR A LIST. TO KNOW ITS TYPE,
 * YOU MUST LOOK AT IN THE LAYOUT CLASS.
 */
/*
** Header file for inclusion with kword_xml2latex.c
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

#ifndef __KWORD_LATEX_ANCHOR_H__
#define __KWORD_LATEX_ANCHOR_H__

#include <qstring.h>
#include "format.h"

/***********************************************************************/
/* Class: Anchor                                                       */
/***********************************************************************/

/**
 * This class hold an anchor. This class will be a lot modified because
 * it doesn't correspond whith the dtd 1.0.
 */
class Anchor: public Format
{
	QString _type;
	QString _instance;

	public:

		/**
		 * Constructors
		 */
		
		/**
		 * Creates a new instances of Anchor.
		 *
		 * @param para the parent class.
		 */
		Anchor(Para* para = 0);
		//Anchor(TextZone);

		/**
		 * Destructor
		 *
		 * Nothing to do
		 */
		virtual ~Anchor();

		/**
		 * Accessors
		 */
		QString getType    () const { return _type; }
		QString getInstance() const { return _instance; }

		/**
		 * Modifiers
		 */
		void setType    (QString type) { _type   = type; }
		void setInstance(QString inst) { _instance = inst; }

		/**
		 * Helpfull functions
		 */

		void analyse (const QDomNode);

		void generate(QTextStream&);

	//private:
};


#endif /* __KWORD_LATEX_ANCHOR_H__ */

