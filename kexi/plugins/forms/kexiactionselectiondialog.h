/* This file is part of the KDE project
   Copyright (C) 2005 Jaroslaw Staniek <js@iidea.pl>

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

#ifndef KEXIACTIONSELECTIONDIALOG_H
#define KEXIACTIONSELECTIONDIALOG_H

#include <kdialogbase.h>

class KexiMainWindow;
class KListView;

class KEXIFORMUTILS_EXPORT KexiActionSelectionDialog : public KDialogBase
{
	Q_OBJECT

	public:
		KexiActionSelectionDialog(KexiMainWindow* mainWin, QWidget *parent, 
			const QCString& currentActionName, const QCString& actionWidgetName);
		~KexiActionSelectionDialog();

		/*! \return selected action name or empty string 
		 if dialog has been rejected (check it with QDialog::result()) 
		 or "<no action>" has been selected. */
		QCString selectedActionName() const;

	protected slots:
		void slotListViewExecuted(QListViewItem *);

	protected:
		KListView* m_listview;
};

#endif
