/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2004-2005 Jaroslaw Staniek <js@iidea.pl>

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

#include "kexiformscrollview.h"
//#include "kexiformview.h"

#include <formeditor/form.h>
#include <formeditor/formmanager.h>
#include <formeditor/objecttree.h>
#include <formeditor/commands.h>
#include <widget/utils/kexirecordmarker.h>

#include <kpopupmenu.h>

KexiFormScrollView::KexiFormScrollView(QWidget *parent, bool preview)
 : KexiScrollView(parent, preview)
 , KexiRecordNavigatorHandler()
 , KexiSharedActionClient()
 , KexiDataAwareObjectInterface()
 , KexiFormDataProvider()
 , KexiFormEventHandler()
{
	m_currentLocalSortColumn = -1; /* no column */
	m_localSortingOrder = -1; /* no sorting */
	m_previousItem = 0;
	m_navPanel = m_scrollViewNavPanel; //copy this pointer from KexiScrollView
	if (preview) {
		setRecordNavigatorVisible(true);
//tmp
//		recordNavigator()->setEditingIndicatorEnabled(true);
//		recordNavigator()->showEditingIndicator(true);
	}

	connect(this, SIGNAL(resizingStarted()), this, SLOT(slotResizingStarted()));

	m_popup = new KPopupMenu(this, "contextMenu");

//	setFocusPolicy(NoFocus);
}

KexiFormScrollView::~KexiFormScrollView()
{
}

void
KexiFormScrollView::show()
{
	KexiScrollView::show();

#if 0 //moved to KexiFormView, OK?
	//now get resize mode settings for entire form
	if (m_preview) {
		KexiFormView* fv = dynamic_cast<KexiFormView*>(parent());
		int resizeMode = fv ? fv->resizeMode() : KexiFormView::ResizeAuto;
		if (resizeMode == KexiFormView::ResizeAuto)
			setResizePolicy(AutoOneFit);
	}
#endif
}

void
KexiFormScrollView::slotResizingStarted()
{
	if(m_form && m_form->manager())
		setSnapToGrid(m_form->manager()->snapWidgetsToGrid(), m_form->gridSize());
	else
		setSnapToGrid(false);
}

int KexiFormScrollView::rowsPerPage() const
{
	//! @todo
	return 10;
}

void KexiFormScrollView::selectCellInternal()
{
	//m_currentItem is already set by KexiDataAwareObjectInterface::setCursorPosition()
	if (m_currentItem) {
		if (m_currentItem!=m_previousItem) {
			fillDataItems(*m_currentItem);
			m_previousItem = m_currentItem;
		}
	}
	else
			m_previousItem = 0;
}

void KexiFormScrollView::ensureCellVisible(int row, int col/*=-1*/)
{
	//! @todo
//	if (m_currentItem)
		//fillDataItems(*m_currentItem);

//	if (m_form->tabStops()->first() && m_form->tabStops()->first()->widget())
//		m_form->tabStops()->first()->widget()->setFocus();
}

void KexiFormScrollView::moveToRecordRequested(uint r)
{
	//! @todo
}

void KexiFormScrollView::moveToLastRecordRequested()
{
	//! @todo
	selectLastRow();
}

void KexiFormScrollView::moveToPreviousRecordRequested()
{
	//! @todo
	selectPrevRow();
}

void KexiFormScrollView::moveToNextRecordRequested()
{
	//! @todo
	selectNextRow();
}

void KexiFormScrollView::moveToFirstRecordRequested()
{
	//! @todo
	selectFirstRow();
}

/*
void KexiFormScrollView::addNewRecordRequested()
{
	//! @todo
}*/

void KexiFormScrollView::clearColumnsInternal(bool repaint)
{
	//! @todo
}

void KexiFormScrollView::addHeaderColumn(const QString& caption, const QString& description, int width)
{
	//! @todo
}

int KexiFormScrollView::currentLocalSortingOrder() const
{
	//! @todo
	return m_localSortingOrder;
}

int KexiFormScrollView::currentLocalSortColumn() const
{
	return m_currentLocalSortColumn;
}

void KexiFormScrollView::setLocalSortingOrder(int col, int order)
{
	//! @todo
	m_currentLocalSortColumn = col;
	m_localSortingOrder = order;
}

void KexiFormScrollView::sortColumnInternal(int col, int order)
{
	//! @todo
}

void KexiFormScrollView::updateGUIAfterSorting()
{
	//! @todo
}

void KexiFormScrollView::createEditor(int row, int col, const QString& addText, 
	bool removeOld)
{
	if (isReadOnly()) {
		kexipluginsdbg << "KexiFormScrollView::createEditor(): DATA IS READ ONLY!"<<endl;
		return;
	}
	if (column( col )->readOnly()) {
		kexipluginsdbg << "KexiFormScrollView::createEditor(): COL IS READ ONLY!"<<endl;
		return;
	}

	//! @todo
	const bool startRowEdit = !m_rowEditing; //remember if we're starting row edit

	if (!m_rowEditing) {
		//we're starting row editing session
		m_data->clearRowEditBuffer();
		
		m_rowEditing = true;
		//indicate on the vheader that we are editing:
		if (m_verticalHeader)
			m_verticalHeader->setEditRow(m_curRow);
		if (isInsertingEnabled() && m_currentItem==m_insertItem) {
			//we should know that we are in state "new row editing"
			m_newRowEditing = true;
			//'insert' row editing: show another row after that:
			m_data->append( m_insertItem );
			//new empty insert item
			m_insertItem = m_data->createItem(); //new KexiTableItem(dataColumns());
//			updateContents();
			if (m_verticalHeader)
				m_verticalHeader->addLabel();
//			m_verticalHeaderAlreadyAdded = true;
			updateWidgetContentsSize();
			//refr. current and next row
//			updateContents(columnPos(0), rowPos(row), viewport()->width(), d->rowHeight*2);
//js: warning this breaks behaviour (cursor is skipping, etc.): qApp->processEvents(500);
//			ensureVisible(columnPos(m_curCol), rowPos(row+1)+d->rowHeight-1, columnWidth(m_curCol), d->rowHeight);

//			m_verticalHeader->setOffset(contentsY());
		}
	}	

	m_editor = editor(col); //m_dataItems.at(col);
	if (!m_editor)
		return;

	if (startRowEdit) {
		recordNavigator()->showEditingIndicator(true);
//		recordNavigator()->updateButtons(); //refresh 'next btn'

		emit rowEditStarted(m_curRow);
	}
}

KexiDataItemInterface *KexiFormScrollView::editor( int col, bool ignoreMissingEditor )
{
	if (!m_data || col<0 || col>=columns())
		return 0;

	return dynamic_cast<KexiFormDataItemInterface*>(dbFormWidget()->orderedDataAwareWidgets()->at( col ));
//	KexiFormDataItemInterface *item = m_dataItems.at(col);
	//return item;

/*
	KexiTableViewColumn *tvcol = m_data->column(col);
//	int t = tvcol->field->type();

	//find the editor for this column
	KexiDataItemInterface *editor = d->editors[ tvcol ];
	if (editor)
		return editor;

	//not found: create
//	editor = KexiCellEditorFactory::createEditor(*m_data->column(col)->field, this);
	editor = KexiCellEditorFactory::createEditor(*m_data->column(col), this);
	if (!editor) {//create error!
		if (!ignoreMissingEditor) {
			//js TODO: show error???
			cancelRowEdit();
		}
		return 0;
	}
	editor->hide();
	connect(editor,SIGNAL(editRequested()),this,SLOT(slotEditRequested()));
	connect(editor,SIGNAL(cancelRequested()),this,SLOT(cancelEditor()));
	connect(editor,SIGNAL(acceptRequested()),this,SLOT(acceptEditor()));

	editor->resize(columnWidth(col)-1, rowHeight()-1);
	editor->installEventFilter(this);
	if (editor->widget())
		editor->widget()->installEventFilter(this);
	//store
	d->editors.insert( tvcol, editor );
	return editor;*/
}

void KexiFormScrollView::editorShowFocus( int row, int col )
{
	//! @todo
//	if (m_currentItem)
//		m_provider->fillDataItems(*m_currentItem);
}

void KexiFormScrollView::updateCell(int row, int col)
{
	//! @todo
}

void KexiFormScrollView::updateRow(int row)
{
	//! @todo
}

void KexiFormScrollView::updateWidgetContents()
{
	//! @todo
}

void KexiFormScrollView::updateWidgetContentsSize()
{
	//! @todo
}

void KexiFormScrollView::updateWidgetScrollBars()
{
	//! @todo
}

void KexiFormScrollView::slotRowRepaintRequested(KexiTableItem& item)
{
	//! @todo
}

/*void KexiFormScrollView::slotAboutToDeleteRow(KexiTableItem& item, 
	KexiDB::ResultInfo* result, bool repaint)
{
	//! @todo
}*/

/*void KexiFormScrollView::slotRowDeleted()
{
	//! @todo
}*/

void KexiFormScrollView::slotRowInserted(KexiTableItem *item, bool repaint)
{
	//! @todo
}

void KexiFormScrollView::slotRowInserted(KexiTableItem *item, uint row, bool repaint)
{
	//! @todo
}

void KexiFormScrollView::slotRowsDeleted( const QValueList<int> & )
{
	//! @todo
}

KexiDBForm* KexiFormScrollView::dbFormWidget() const
{
	return dynamic_cast<KexiDBForm*>(m_widget);
}

int KexiFormScrollView::columns() const
{
	return dbFormWidget()->orderedDataAwareWidgets()->count(); //m_dataItems.count();
}

/*uint KexiFormScrollView::fieldNumberForColumn(int col)
{
	KexiFormDataItemInterface *item = dynamic_cast<KexiFormDataItemInterface*>(dbFormWidget()->orderedDataAwareWidgets()->at( col ));
	if (!item)
		return -1;
	KexiFormDataItemInterfaceToIntMap::ConstIterator it(m_fieldNumbersForDataItems.find( item ));
	return it!=m_fieldNumbersForDataItems.constEnd() ? it.data() : -1;
}*/

bool KexiFormScrollView::columnEditable(int col)
{
	kexipluginsdbg << "KexiFormScrollView::columnEditable(" << col << ")" << endl;
	foreach_list (QPtrListIterator<KexiFormDataItemInterface>, it, m_dataItems) {
		kexipluginsdbg << (dynamic_cast<QWidget*>(it.current()) ? dynamic_cast<QWidget*>(it.current())->name() : "" ) 
			<< " " << it.current()->dataSource() << endl;
	}
	kexipluginsdbg << "-- focus widgets --" << endl;
	foreach_list (QPtrListIterator<QWidget>, it, *dbFormWidget()->orderedFocusWidgets()) {
		kexipluginsdbg << it.current()->name() << endl;
	}
	kexipluginsdbg << "-- data-aware widgets --" << endl;
	foreach_list (QPtrListIterator<QWidget>, it, *dbFormWidget()->orderedDataAwareWidgets()) {
		kexipluginsdbg << it.current()->name() << endl;
	}

	//int index = dbFormWidget()->indexForDataItem( item );
//	KexiFormDataItemInterface *item1 = dynamic_cast<KexiFormDataItemInterface*>(dbFormWidget()->orderedFocusWidgets()->at( col ));
	KexiFormDataItemInterface *item = dynamic_cast<KexiFormDataItemInterface*>(dbFormWidget()->orderedDataAwareWidgets()->at( col ));

	if (!item || item->isReadOnly())
		return false;

//	KexiFormDataItemInterfaceToIntMap::ConstIterator it(m_fieldNumbersForDataItems.find( item ));
//	return KexiDataAwareObjectInterface::columnEditable( it!=m_fieldNumbersForDataItems.constEnd() ? it.data() : -1 );
	return KexiDataAwareObjectInterface::columnEditable( col );
}

void KexiFormScrollView::valueChanged(KexiDataItemInterface* item)
{
	if (!item)
		return;
	//only signal start editing when no row editing was started already
	kexipluginsdbg << "** KexiFormScrollView::valueChanged(): editedItem=" 
		<< (dbFormWidget()->editedItem ? dbFormWidget()->editedItem->value().toString() : QString::null)
		<< ", "
		<< (item ? item->value().toString() : QString::null)
		<< endl;
	if (dbFormWidget()->editedItem!=item) {
		kexipluginsdbg << "**>>>	dbFormWidget()->editedItem = dynamic_cast<KexiFormDataItemInterface*>(item)" << endl;
		dbFormWidget()->editedItem = dynamic_cast<KexiFormDataItemInterface*>(item);
		startEditCurrentCell();
	}
	fillDuplicatedDataItems(dynamic_cast<KexiFormDataItemInterface*>(item), item->value());
}

bool KexiFormScrollView::cursorAtNewRow()
{
	return isInsertingEnabled() && ( m_currentItem==m_insertItem || m_newRowEditing );
}

void KexiFormScrollView::initDataContents()
{
	KexiDataAwareObjectInterface::initDataContents();

	if (m_preview) {
//! @todo here we can react if user wanted to show the navigator
		setRecordNavigatorVisible(m_data);
		recordNavigator()->setEnabled(m_data);
		if (m_data) {
			recordNavigator()->setEditingIndicatorEnabled( !isReadOnly() );
			recordNavigator()->showEditingIndicator(false);
		}
	}
}

KexiTableViewColumn* KexiFormScrollView::column(int col)
{
	const int id = fieldNumberForColumn(col);
	return (id >= 0) ? m_data->column( id ) : 0;
}

void KexiFormScrollView::cancelEditor()
{
	if (!dynamic_cast<KexiFormDataItemInterface*>(m_editor))
		return;

	dynamic_cast<KexiFormDataItemInterface*>(m_editor)->undoChanges();
	m_editor = 0;
}

void KexiFormScrollView::updateAfterCancelRowEdit()
{
	for (QPtrListIterator<KexiFormDataItemInterface> it(m_dataItems); it.current(); ++it) {
		it.current()->undoChanges();
	}
	recordNavigator()->showEditingIndicator(false);
	dbFormWidget()->editedItem = 0;
}

void KexiFormScrollView::updateAfterAcceptRowEdit()
{
	if (!m_currentItem)
		return;
	recordNavigator()->showEditingIndicator(false);
	dbFormWidget()->editedItem = 0;
	//update visible data because there could be auto-filled (eg. autonumber) fields
	fillDataItems(*m_currentItem);
	m_previousItem = m_currentItem;
}

void KexiFormScrollView::beforeSwitchView()
{
	m_editor = 0;
}

void KexiFormScrollView::refreshContentsSize()
{
	KexiScrollView::refreshContentsSize();
	//only clear cmd history when KexiScrollView::refreshContentsSizeLater() has been called
	if (!m_preview && sender()==&m_delayedResize) {
		if (m_form)
			m_form->clearCommandHistory();
	}
}

#include "kexiformscrollview.moc"
