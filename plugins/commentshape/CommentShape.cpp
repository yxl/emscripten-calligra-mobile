/* This file is part of the KDE project
 * Copyright (C) 2010 Carlos Licea <carlos@kdab.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "CommentShape.h"

#include <KoXmlReader.h>
#include <KoXmlNS.h>
#include <KoShapeSavingContext.h>
#include <KoXmlWriter.h>

#include <QPainter>
#include <QGradient>
#include <QBrush>
#include <QRectF>
#include <fcntl.h>
#include <KoShapeLoadingContext.h>

CommentShape::CommentShape()
: KoShape()
{
    KoShape::setSize(QSizeF(20,20));
}

CommentShape::~CommentShape()
{
}

bool CommentShape::loadOdf(const KoXmlElement& element, KoShapeLoadingContext& context)
{
    loadOdfAttributes(element, context, OdfPosition);

    KoXmlElement child;
    forEachElement(child, element)
    {
        if(child.namespaceURI() == KoXmlNS::dc) {
            if(child.localName() == "creator") {
                m_creator = child.text();
                QStringList creatorNames = m_creator.split(' ');
                //TODO fix for RTL
                foreach(QString name, creatorNames) {
                    m_initials += name.left(1);
                }
            }
            else if(child.localName() == "date") {
                m_date = QDate::fromString(child.text(), Qt::ISODate);
            }
        }
        else if(child.namespaceURI() == KoXmlNS::text && child.localName() == "p") {
            m_comment = child.text();
        }
    }

    return true;
}

void CommentShape::paint(QPainter& painter, const KoViewConverter& converter)
{
    applyConversion(painter, converter);

    QLinearGradient gradient(0, 0, 0, 20);
    gradient.setColorAt(0.0, QColor(Qt::yellow));
    gradient.setColorAt(1.0, QColor(254, 201, 7));
    const QBrush brush(gradient);
    painter.setBrush(brush);

    painter.setPen(Qt::black);

    const QSizeF size = this->size();
    painter.drawRect(QRectF(0, 0, size.width(), size.height()));

    painter.drawText( QRectF(QPoint(0,0), size), Qt::AlignCenter, m_initials);
}

void CommentShape::saveOdf(KoShapeSavingContext& context) const
{
    KoXmlWriter& writer = context.xmlWriter();

    writer.startElement("officeooo:annotation"); //TODO replace with standarized element name
    saveOdfAttributes(context, OdfPosition);

    writer.startElement("dc:creator");
    writer.addTextSpan(m_creator);
    writer.endElement();//dc:creator

    writer.startElement("dc:date");
    writer.addTextSpan(m_date.toString(Qt::ISODate));
    writer.endElement();//dc:date

    writer.startElement("text:p");
    writer.addTextSpan(m_comment);
    writer.endElement();//text:p

    writer.endElement();//officeooo:annotation
}

void CommentShape::setSize(const QSizeF& size)
{
    KoShape::setSize(QSizeF(20,20));
}