/* This file is part of the KDE project
  Copyright (C) 2006 - 2007 Dag Andersen <kplato@kde.org>

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

#ifndef KPTITEMMODELBASE_H
#define KPTITEMMODELBASE_H

#include "kplatomodels_export.h"

#include "kptglobal.h"

#include <QAbstractItemModel>
#include <QItemDelegate>
#include <QSplitter>
#include <QScrollBar>
#include <QTreeView>

#include <KoXmlReaderForward.h>

class KoDocument;
class QDomElement;

namespace KPlato
{

namespace Delegate
{
    enum EditorType { EnumEditor, TimeEditor };
}

class Project;

class KPLATOMODELS_EXPORT SelectorDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    SelectorDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class KPLATOMODELS_EXPORT EnumDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    EnumDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class KPLATOMODELS_EXPORT DurationDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    DurationDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class KPLATOMODELS_EXPORT DurationSpinBoxDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    DurationSpinBoxDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class KPLATOMODELS_EXPORT SpinBoxDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    SpinBoxDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class KPLATOMODELS_EXPORT DoubleSpinBoxDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    DoubleSpinBoxDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class KPLATOMODELS_EXPORT MoneyDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    MoneyDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class KPLATOMODELS_EXPORT TimeDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    TimeDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class KPLATOMODELS_EXPORT ItemModelBase : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit ItemModelBase( KoDocument *part, QObject *parent = 0 );
    ~ItemModelBase();

    Project *project() const { return m_project; }
    virtual void setProject( Project *project );
    virtual void setReadWrite( bool rw ) { m_readWrite = rw; }
    bool isReadWrite() { return m_readWrite; }

    /**
     * Check if the @p data is allowed to be dropped on @p index,
     * @p dropIndicatorPosition indicates position relative @p index.
     *
     * Base implementation checks flags and mimetypes.
     */
    virtual bool dropAllowed( const QModelIndex &index, int dropIndicatorPosition, const QMimeData *data );
    
protected slots:
    virtual void slotLayoutToBeChanged();
    virtual void slotLayoutChanged();

protected:
    KoDocument *m_part;
    Project *m_project;
    bool m_readWrite;
};

class KPLATOMODELS_EXPORT TreeViewBase : public QTreeView
{
    Q_OBJECT
public:
    // Copy from protected enum in QAbstractItemView
    enum DropIndicatorPosition {
        OnItem = QAbstractItemView::OnItem,  /// The item will be dropped on the index.
        AboveItem = QAbstractItemView::AboveItem, /// The item will be dropped above the index.
        BelowItem = QAbstractItemView::BelowItem,  /// The item will be dropped below the index.
        OnViewport = QAbstractItemView::OnViewport /// The item will be dropped onto a region of the viewport with no items if acceptDropsOnView is set.
    };
    
    explicit TreeViewBase( QWidget *parent );

    void setReadWrite( bool rw );
    
    void setArrowKeyNavigation( bool on ) { m_arrowKeyNavigation = on; }
    bool arrowKeyNavigation() const { return m_arrowKeyNavigation; }

    QModelIndex nextColumn( const QModelIndex &current ) const;
    QModelIndex previousColumn( const QModelIndex &current ) const;
    
    void setAcceptDropsOnView( bool mode ) { m_acceptDropsOnView = mode; }

    ItemModelBase *itemModel() const;
    virtual void setModel( QAbstractItemModel *model );
    
    virtual void setSelectionModel( QItemSelectionModel *model );
    
    void setStretchLastSection( bool );
    
    void mapToSection( int column, int section );
    int section( int col ) const;

    void setColumnsHidden( const QList<int> &list );

    /// Loads context info into this view. Reimplement.
    virtual bool loadContext( const KoXmlElement &/*context*/ );
    /// Save context info from this view. Reimplement.
    virtual void saveContext( QDomElement &/*context*/ ) const;

signals:
    /// Context menu requested from viewport at global position @p pos
    void contextMenuRequested( QModelIndex, const QPoint &pos );
    /// Context menu requested from header at global position @p pos
    void headerContextMenuRequested( const QPoint &pos );
    
    void moveAfterLastColumn( const QModelIndex & );
    void moveBeforeFirstColumn( const QModelIndex & );
    
public slots:
    void updateColumnsHidden();
    
protected:
    void keyPressEvent(QKeyEvent *event);
    QItemSelectionModel::SelectionFlags selectionCommand(const QModelIndex &index, const QEvent *event) const;
    
    void contextMenuEvent ( QContextMenuEvent * event );
    
    void dragMoveEvent(QDragMoveEvent *event);
    
protected slots:
    virtual void slotCurrentChanged ( const QModelIndex & current, const QModelIndex & previous );
    void slotHeaderContextMenuRequested( const QPoint& );
    
protected:
    bool m_arrowKeyNavigation;
    bool m_acceptDropsOnView;
    QList<int> m_hideList;
    bool m_readWrite;
};


class KPLATOMODELS_EXPORT DoubleTreeViewBase : public QSplitter
{
    Q_OBJECT
public:
    explicit DoubleTreeViewBase( QWidget *parent );
    DoubleTreeViewBase( bool mode, QWidget *parent );
    ~DoubleTreeViewBase();
    
    void setReadWrite( bool rw );
    
    void setModel( ItemModelBase *model );
    ItemModelBase *model() const;
    
    void setArrowKeyNavigation( bool on ) { m_arrowKeyNavigation = on; }
    bool arrowKeyNavigation() const { return m_arrowKeyNavigation; }

    QItemSelectionModel *selectionModel() const { return m_selectionmodel; }
    void setSelectionModel( QItemSelectionModel *model );
    void setSelectionMode( QAbstractItemView::SelectionMode mode );
    void setSelectionBehavior( QAbstractItemView::SelectionBehavior mode );
    void setItemDelegateForColumn( int col, QAbstractItemDelegate * delegate );
    void setEditTriggers ( QAbstractItemView::EditTriggers );
    QAbstractItemView::EditTriggers editTriggers() const;
    
    void setAcceptDrops( bool );
    void setAcceptDropsOnView( bool );
    void setDropIndicatorShown( bool );
    void setDragDropMode( QAbstractItemView::DragDropMode mode );
    void setDragEnabled ( bool mode );

    void setStretchLastSection( bool );
    
    /// Hide columns in the @p hideList, show all other columns.
    /// If the hideList.last() == -1, the rest of the columns are hidden.
    void hideColumns( TreeViewBase *view, const QList<int> &hideList );
    void hideColumns( const QList<int> &masterList, const QList<int> List = QList<int>() );
    void hideColumn( int col ) {
        m_leftview->hideColumn( col ); 
        if ( m_rightview ) m_rightview->hideColumn( col );
    }
    void showColumn( int col ) { 
        if ( col == 0 || m_rightview == 0 ) m_leftview->showColumn( col );
        else m_rightview->showColumn( col );
    }
    bool isColumnHidden( int col ) const {
        return m_rightview ? m_rightview->isColumnHidden( col ) : m_leftview->isColumnHidden( col );
    }
    
    TreeViewBase *masterView() const { return m_leftview; }
    TreeViewBase *slaveView() const { return m_rightview; }

    /// Loads context info into this view. Reimplement.
    virtual bool loadContext( const KoXmlElement &/*context*/ );
    /// Save context info from this view. Reimplement.
    virtual void saveContext( QDomElement &/*context*/ ) const;
    

signals:
    /// Context menu requested from the viewport, pointer over @p index at global position @p pos
    void contextMenuRequested( QModelIndex index, const QPoint& pos );
    /// Context menu requested from master- or slave header at global position @p pos
    void headerContextMenuRequested( const QPoint &pos );
    /// Context menu requested from master header at global position @p pos
    void masterHeaderContextMenuRequested( const QPoint &pos );
    /// Context menu requested from slave header at global position @p pos
    void slaveHeaderContextMenuRequested( const QPoint &pos );
    
    void currentChanged ( const QModelIndex & current, const QModelIndex & previous );
    void selectionChanged( const QModelIndexList );
    
public slots:
    void edit( const QModelIndex &index );

protected slots:
    void slotSelectionChanged( const QItemSelection &sel, const QItemSelection & );
    void slotToRightView( const QModelIndex &index );
    void slotToLeftView( const QModelIndex &index );
    
    void slotRightHeaderContextMenuRequested( const QPoint &pos );
    void slotLeftHeaderContextMenuRequested( const QPoint &pos );

    void updateColumnsHidden();

protected:
    void init( bool mode );
    
protected:
    TreeViewBase *m_leftview;
    TreeViewBase *m_rightview;
    ItemModelBase *m_model;
    QItemSelectionModel *m_selectionmodel;
    bool m_arrowKeyNavigation;
    bool m_readWrite;
};


} // namespace KPlato

#endif
