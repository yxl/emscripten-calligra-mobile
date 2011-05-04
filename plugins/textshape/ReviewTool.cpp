/* This file is part of the KDE project
 * Copyright (C) 2009-2010 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
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

#include "ReviewTool.h"

#include <KoCanvasBase.h>
#include <KoTextLayoutRootArea.h>
#include <KoChangeTracker.h>
#include <KoPointerEvent.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoTextDocument.h>
#include <KoTextDocumentLayout.h>
#include <KoTextEditor.h>
#include <KoTextShapeData.h>
#include <KoViewConverter.h>
#include "TextShape.h"

#include "commands/AcceptChangeCommand.h"
#include "commands/RejectChangeCommand.h"
#include "commands/ShowChangesCommand.h"
#include "dialogs/TrackedChangeModel.h"
#include "dialogs/TrackedChangeManager.h"
#include "dialogs/AcceptRejectChangeDialog.h"

#include <KLocale>
#include <KAction>

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QModelIndex>
#include <QPainter>
#include <QPushButton>
#include <QTextBlock>
#include <QTreeView>
#include <QVBoxLayout>
#include <QVector>
#include <QLabel>

ReviewTool::ReviewTool(KoCanvasBase* canvas): KoToolBase(canvas),
    m_disableShowChangesOnExit(false),
    m_textEditor(0),
    m_textShapeData(0),
    m_canvas(canvas),
    m_textShape(0),
    m_model(0),
    m_trackedChangeManager(0),
    m_changesTreeView(0)
{
    KAction *action;
    action = new KAction(i18n("Tracked change manager"), this);
    action->setShortcut(Qt::ALT + Qt::CTRL + Qt::Key_T);
    addAction("show_changeManager", action);
    connect(action, SIGNAL(triggered()), this, SLOT(showTrackedChangeManager()));
}

ReviewTool::~ReviewTool()
{
    delete m_trackedChangeManager;
    delete m_model;
}

void ReviewTool::mouseReleaseEvent(KoPointerEvent* event)
{
    event->ignore();
}

void ReviewTool::mouseMoveEvent(KoPointerEvent* event)
{
    updateSelectedShape(event->point);
    int position = pointToPosition(event->point);
    QTextCursor cursor(m_textShapeData->document());
    cursor.setPosition(position);

    int changeId = cursor.charFormat().property(KoCharacterStyle::ChangeTrackerId).toInt();
    if (changeId) {
        m_canvas->setCursor(QCursor(Qt::PointingHandCursor));
        QModelIndex index = m_model->indexForChangeId(changeId);
        m_changesTreeView->setCurrentIndex(index);
    } else {
        m_canvas->setCursor(QCursor(Qt::ArrowCursor));
        m_changesTreeView->setCurrentIndex(QModelIndex());
    }
}

void ReviewTool::mousePressEvent(KoPointerEvent* event)
{
    int position = pointToPosition(event->point);
    QTextCursor cursor(m_textShapeData->document());
    cursor.setPosition(position);

    int changeId = cursor.charFormat().property(KoCharacterStyle::ChangeTrackerId).toInt();
    if (changeId) {
        AcceptRejectChangeDialog acceptDialog(KoTextDocument(m_textShapeData->document()).changeTracker(), changeId);
        if (int result = acceptDialog.exec()) {
            if (result == (int)(AcceptRejectChangeDialog::eChangeAccepted)) {
                acceptChange();
            } else if (result == (int)(AcceptRejectChangeDialog::eChangeRejected)) {
                rejectChange();
            }
        }
    }
}

void ReviewTool::updateSelectedShape(const QPointF &point)
{
    if (! m_textShape->boundingRect().contains(point)) {
        QRectF area(point, QSizeF(1, 1));
        foreach(KoShape *shape, canvas()->shapeManager()->shapesAt(area, true)) {
            TextShape *textShape = dynamic_cast<TextShape*>(shape);
            if (textShape) {
                KoTextShapeData *d = static_cast<KoTextShapeData*>(textShape->userData());
                const bool sameDocument = d->document() == m_textShapeData->document();
                    m_textShape = textShape;
                if (sameDocument)
                    break; // stop looking.
            }
        }
        setShapeData(static_cast<KoTextShapeData*>(m_textShape->userData()));
    }
}

int ReviewTool::pointToPosition(const QPointF & point) const
{
    QPointF p = m_textShape->convertScreenPos(point);
    int caretPos = m_textShapeData->rootArea()->hitTest(p, Qt::FuzzyHit);
    return caretPos;
}

void ReviewTool::paint(QPainter& painter, const KoViewConverter& converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
    QTextBlock block = m_textEditor->block();
    if (! block.layout()) // not layouted yet.  The Shape paint method will trigger a layout
        return;
    if (m_textShapeData == 0)
        return;

    if (!m_changesTreeView->currentIndex().isValid())
        return;

    QList<QPair<int, int> > changeRanges = m_model->changeItemData(m_changesTreeView->currentIndex()).changeRanges;

    for (int i = 0; i < changeRanges.size(); ++i) {
        int start = changeRanges.at(i).first;
        int end = changeRanges.at(i).second;
        if (end < start)
            qSwap(start, end);
        QTextCursor cursor;
        cursor.setPosition(start);
        cursor.setPosition(end, QTextCursor::KeepAnchor);
        QList<TextShape *> shapesToPaint;
        KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(m_textShapeData->document()->documentLayout());
        if (lay) {
            foreach(KoShape *shape, lay->shapes()) {
                TextShape *ts = dynamic_cast<TextShape*>(shape);
                if (! ts)
                    continue;
                KoTextShapeData *data = ts->textShapeData();
                // check if shape contains some of the selection, if not, skip
                if (!data->isCursorVisible(&cursor))
                    continue;
                if (painter.hasClipping()) {
                    QRect rect = converter.documentToView(ts->boundingRect()).toRect();
                    if (painter.clipRegion().intersect(QRegion(rect)).isEmpty())
                        continue;
                }
                shapesToPaint << ts;
            }
        }
        if (shapesToPaint.isEmpty()) // quite unlikely, though ;)
            return;

        qreal zoomX, zoomY;
        converter.zoom(&zoomX, &zoomY);

        foreach(TextShape *ts, shapesToPaint) {
            KoTextShapeData *data = ts->textShapeData();
            Q_ASSERT(data);
            if (data->isDirty())
                continue;

            painter.save();
            QTransform shapeMatrix = ts->absoluteTransformation(&converter);
            shapeMatrix.scale(zoomX, zoomY);
            painter.setTransform(shapeMatrix * painter.transform());
            painter.setClipRect(QRectF(QPointF(), ts->size()), Qt::IntersectClip);
            painter.translate(0, -data->documentOffset());
#if 0 //FIXME refactor to new textlayout
            if (data->isCursorVisible(&cursor)) {
                QVector<QRectF> *clipVec = textRect(cursor);
                QRectF clip;
                foreach(clip, *clipVec) {
                    painter.save();
                    painter.setClipRect(clip, Qt::IntersectClip);
                    painter.fillRect(clip, QBrush(QColor(0,0,0,90)));
                    painter.restore();
                }
                delete clipVec;
            }
#endif

            painter.restore();
        }
    }
}


QRectF ReviewTool::textRect(QTextCursor &cursor) const
{
    if (!m_textShapeData)
        return QRectF();
    KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(m_textShapeData->document()->documentLayout());
    return lay->selectionBoundingBox(cursor);
}

void ReviewTool::keyPressEvent(QKeyEvent* event)
{
    KoToolBase::keyPressEvent(event);
}

void ReviewTool::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    Q_UNUSED(toolActivation);
    foreach(KoShape *shape, shapes) {
        m_textShape = dynamic_cast<TextShape*>(shape);
        if (m_textShape)
            break;
    }
    if (m_textShape == 0) { // none found
        emit done();
        return;
    }
    setShapeData(static_cast<KoTextShapeData*>(m_textShape->userData()));
    useCursor(Qt::ArrowCursor);


    m_textShape->update();
}

void ReviewTool::setShapeData(KoTextShapeData *data)
{
    bool docChanged = data == 0 || m_textShapeData == 0 || m_textShapeData->document() != data->document();
/*
    if (m_textShapeData) {
//        disconnect(m_textShapeData, SIGNAL(destroyed (QObject*)), this, SLOT(shapeDataRemoved()));
        KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(m_textShapeData->document()->documentLayout());
//        if (lay)
//            disconnect(lay, SIGNAL(shapeAdded(KoShape*)), this, SLOT(shapeAddedToDoc(KoShape*)));
    }
*/
    if (!data) {
        if (m_disableShowChangesOnExit) {
            ShowChangesCommand *command = new ShowChangesCommand(false, m_textShapeData->document(), m_canvas);
            m_textEditor->addCommand(command);
        }
    }
    m_textShapeData = data;
    if (m_textShapeData == 0)
        return;
//    connect(m_textShapeData, SIGNAL(destroyed (QObject*)), this, SLOT(shapeDataRemoved()));
    if (docChanged) {
//        if (m_textEditor)
//            disconnect(m_textEditor, SIGNAL(isBidiUpdated()), this, SLOT(isBidiUpdated()));
        m_textEditor = KoTextDocument(m_textShapeData->document()).textEditor();
        Q_ASSERT(m_textEditor);
//        connect(m_textEditor, SIGNAL(isBidiUpdated()), this, SLOT(isBidiUpdated()));

        KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(m_textShapeData->document()->documentLayout());
        if (lay) {
//            connect(lay, SIGNAL(shapeAdded(KoShape*)), this, SLOT(shapeAddedToDoc(KoShape*)));
        }
    }
    m_textEditor->updateDefaultTextDirection(m_textShapeData->pageDirection());
    if (!KoTextDocument(m_textShapeData->document()).changeTracker()->displayChanges()) {
        m_disableShowChangesOnExit = true;
        ShowChangesCommand *command = new ShowChangesCommand(true, m_textShapeData->document(), m_canvas);
        m_textEditor->addCommand(command);
    }
    if (m_model) {
        disconnect(m_changesTreeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(selectedChangeChanged(QModelIndex,QModelIndex)));
        delete m_model;
    }
    m_model = new TrackedChangeModel(m_textShapeData->document());
    if (m_changesTreeView) {
        QItemSelectionModel *m = m_changesTreeView->selectionModel();
        m_changesTreeView->setModel(m_model);
        delete m;
        connect(m_changesTreeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(selectedChangeChanged(QModelIndex,QModelIndex)));
        m_changesTreeView->reset();
    }
}

void ReviewTool::deactivate()
{
    m_textShape = 0;
    setShapeData(0);
    canvas()->canvasWidget()->setFocus();
}

QMap<QString, QWidget*> ReviewTool::createOptionWidgets()
{
    QMap<QString, QWidget *> widgets;
    QWidget *widget = new QWidget();
    widget->setObjectName("hmm");

    m_changesTreeView = new QTreeView(widget);
    m_changesTreeView->setModel(m_model);
    connect(m_changesTreeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(selectedChangeChanged(QModelIndex,QModelIndex)));

    QVBoxLayout *vLayout = new QVBoxLayout(widget);
    vLayout->addWidget(m_changesTreeView);
    QHBoxLayout *hLayout = new QHBoxLayout(widget);
    QPushButton *accept = new QPushButton(i18n("Accept"));
    QPushButton *reject = new QPushButton(i18n("Reject"));
    hLayout->addWidget(accept);
    hLayout->addWidget(reject);
    vLayout->addLayout(hLayout);
    widget->setLayout(vLayout);

    connect(accept, SIGNAL(clicked(bool)), this, SLOT(acceptChange()));
    connect(reject, SIGNAL(clicked(bool)), this, SLOT(rejectChange()));

    widgets.insert(i18n("Changes"), widget);
    QWidget *dummy = new QWidget();
    dummy->setObjectName("dummy1");
    widgets.insert(i18n("Spell checking"), dummy);
    dummy = new QWidget();
    dummy->setObjectName("dummy2");
    widgets.insert(i18n("Comments"), dummy);
    dummy = new QWidget();
    dummy->setObjectName("dummy3");
    widgets.insert(i18n("Statistics"), dummy);
    return widgets;
}

void ReviewTool::acceptChange()
{
    if (m_changesTreeView->currentIndex().isValid()) {
        AcceptChangeCommand *command = new AcceptChangeCommand(m_model->changeItemData(m_changesTreeView->currentIndex()).changeId,
                                                               m_model->changeItemData(m_changesTreeView->currentIndex()).changeRanges,
                                                                m_textShapeData->document());
        connect(command, SIGNAL(acceptRejectChange()), m_model, SLOT(setupModel()));
        m_textEditor->addCommand(command);
    }
}

void ReviewTool::rejectChange()
{
    if (m_changesTreeView->currentIndex().isValid()) {
        RejectChangeCommand *command = new RejectChangeCommand(m_model->changeItemData(m_changesTreeView->currentIndex()).changeId,
                                                               m_model->changeItemData(m_changesTreeView->currentIndex()).changeRanges,
                                                               m_textShapeData->document());
        connect(command, SIGNAL(acceptRejectChange()), m_model, SLOT(setupModel()));
        m_textEditor->addCommand(command);
    }
}

void ReviewTool::selectedChangeChanged(QModelIndex newItem, QModelIndex previousItem)
{
    Q_UNUSED(newItem);
    Q_UNUSED(previousItem);
    canvas()->updateCanvas(m_textShape->boundingRect());
}

void ReviewTool::showTrackedChangeManager()
{
/*    Q_ASSERT(m_model);
    m_trackedChangeManager = new TrackedChangeManager();
    m_trackedChangeManager->setModel(m_model);
    connect(m_trackedChangeManager, SIGNAL(currentChanged(QModelIndex)), this, SLOT(selectedChangeChanged(QModelIndex)));
    m_trackedChangeManager->show();
*/    //    view.setModel(&model);
    //    view.setWindowTitle("testTracked");
    //    view.show();
    //    TrackedChangeManager *dia = new TrackedChangeManager(m_textShapeData->document());
    //    dia->show();
}

#include <ReviewTool.moc>