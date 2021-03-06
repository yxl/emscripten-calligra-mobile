/* This file is part of the KDE project
   Copyright 2006-2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 1999-2002,2004 Laurent Montel <montel@kde.org>
   Copyright 2002-2005 Ariya Hidayat <ariya@kde.org>
   Copyright 1999-2001,2003 David Faure <faure@kde.org>
   Copyright 2001-2003 Philipp Mueller <philipp.mueller@gmx.de>
   Copyright 2002-2003 Norbert Andres <nandres@web.de>
   Copyright 2000-2001 Werner Trobin <trobin@kde.org>
   Copyright 2002 Harri Porten <porten@kde.org>
   Copyright 2002 John Dailey <dailey@vt.edu>
   Copyright 1999-2000 Torben Weis <weis@kde.org>
   Copyright 2000 Wilco Greven <greven@kde.org>
   Copyright 1999 Boris Wedl <boris.wedl@kfunigraz.ac.at>
   Copyright 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 2010 Boudewijn Rempt <boud@kogmbh.com>

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

#ifndef CALLIGRA_SHEETS_CANVAS_ITEM
#define CALLIGRA_SHEETS_CANVAS_ITEM

#include <QList>
#include <QGraphicsWidget>

#include <KoCanvasBase.h>

#include "calligra_sheets_export.h"

#include "Global.h"
#include "CanvasBase.h"

class QDragLeaveEvent;
class QDragMoveEvent;
class QDropEvent;
class QEvent;
class QFocusEvent;
class QKeyEvent;
class QMouseEvent;
class QPainter;
class QPaintEvent;
class QPen;
class QResizeEvent;
class QScrollBar;

namespace Calligra
{
namespace Sheets
{
class Cell;
class CellEditor;
class CanvasItem;
class ColumnHeader;
class Damage;
class Doc;
class Sheet;
class RowHeader;
class Selection;
class View;


/**
 * The scrollable area showing the cells.
 */
class CALLIGRA_SHEETS_COMMON_EXPORT CanvasItem : public QGraphicsWidget, public CanvasBase
{
    friend class ColumnHeaderItem;
    friend class RowHeaderItem;
    friend class View;
    friend class CellTool;

    Q_OBJECT

public:
    explicit CanvasItem(Doc* doc, QGraphicsItem *parent = 0);
    ~CanvasItem();

    virtual QWidget* canvasWidget() {
        return 0;
    }
    virtual const QWidget* canvasWidget() const {
        return 0;
    }

    /**
     * Return the widget that will be added to the scrollArea.
     */
    virtual QGraphicsObject *canvasItem() { return this; }

    /**
     * Return the widget that will be added to the scrollArea.
     */
    virtual const QGraphicsObject *canvasItem() const{ return this; }


public Q_SLOTS:
    void setDocumentOffset(const QPoint& offset) {
        CanvasBase::setDocumentOffset(offset);
    }
    void setDocumentSize(const QSizeF& size) {
        CanvasBase::setDocumentSize(size);
    }

    void refreshSheetViews();
    void setActiveSheet(Sheet* sheet);

    void setObscuredRange(const QSize& size);

    /**
     * \ingroup Damages
     * Handles damages that need visual updates.
     */
    void handleDamages(const QList<Damage*>& damages);

    void updateAccessedCellRange(Sheet* sheet, const QPoint& location);

Q_SIGNALS:
    /* virtual */ void documentSizeChanged(const QSize&);
    void obscuredRangeChanged(const Calligra::Sheets::Sheet* sheet, const QSize&);

public:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* _ev);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* _ev);
protected:
    virtual void keyPressEvent(QKeyEvent* _ev) {
        CanvasBase::keyPressed(_ev);
    }
    virtual void paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* _ev);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent*);
    virtual void focusInEvent(QFocusEvent* _ev) {
        CanvasBase::focusIn(_ev);
        QGraphicsWidget::focusInEvent(_ev);
    }
    virtual void dragEnterEvent(QGraphicsSceneDragDropEvent*);
    virtual void dragMoveEvent(QGraphicsSceneDragDropEvent*);
    virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent*);
    virtual void dropEvent(QGraphicsSceneDragDropEvent*);
    /// reimplemented method from superclass
    virtual QVariant inputMethodQuery(Qt::InputMethodQuery query) const {
        return CanvasBase::inputMethodQuery(query);
    }
    /// reimplemented method from superclass
    virtual void inputMethodEvent(QInputMethodEvent *event) {
        CanvasBase::inputMethodEvent(event);
    }
    /// reimplemented method from superclass
    virtual void tabletEvent(QTabletEvent *e) {
        CanvasBase::tabletEvent(e);
    }

private:
    virtual bool eventFilter(QObject *o, QEvent *e) {
        return CanvasBase::eventFilter(o, e);
    }

public:
    virtual Selection* selection() const;
    virtual Sheet* activeSheet() const;
    virtual KoZoomHandler* zoomHandler() const;
    virtual SheetView* sheetView(const Sheet* sheet) const;

    virtual bool isViewLoading() const { return false; }
    virtual void enableAutoScroll() {}
    virtual void disableAutoScroll() {}
    virtual void setVertScrollBarPos(qreal) {}
    virtual void setHorizScrollBarPos(qreal) {}

    virtual void update() { QGraphicsWidget::update(); }
    virtual void update(const QRectF& rect) { QGraphicsWidget::update(rect); }
    virtual Qt::LayoutDirection layoutDirection() const { return QGraphicsWidget::layoutDirection(); }
    virtual QRectF rect() const { return QGraphicsWidget::rect(); }
    virtual QSizeF size() const { return QGraphicsWidget::size(); }
    virtual QPoint mapToGlobal(const QPointF& point) const { return QGraphicsWidget::mapToScene(point).toPoint(); /* TODO */ }
    virtual void updateMicroFocus() { /*QGraphicsWidget::updateMicroFocus();*/ }

    virtual ColumnHeader* columnHeader() const;
    virtual RowHeader* rowHeader() const;
    virtual void setCursor(const QCursor &cursor);

    virtual void showContextMenu(const QPoint& globalPos) { Q_UNUSED(globalPos); }
private:
    Q_DISABLE_COPY(CanvasItem)

    class Private;
    Private * const d;
};

} // namespace Sheets
} // namespace Calligra

#endif // CALLIGRA_SHEETS_CANVAS_ITEM
