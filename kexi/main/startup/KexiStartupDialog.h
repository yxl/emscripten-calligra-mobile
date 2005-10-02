/* This file is part of the KDE project
   Copyright (C) 2003 Jaroslaw Staniek <js@iidea.pl>

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

#ifndef KexiStartupDialog_h
#define KexiStartupDialog_h

#include <kdialogbase.h>
#include <kicondialog.h>
#include <kiconview.h>
#include <kfileiconview.h>
#include <kfiledialog.h>

#include <qlabel.h>
#include <qsplitter.h>

#include <kexidb/connectiondata.h>

class KTextBrowser;

/*! Helper class for displaying templates set with description. */
class KEXIMAIN_EXPORT TemplatesPage : public QSplitter
{
	Q_OBJECT
	
	public:
		TemplatesPage( Orientation o, QWidget * parent = 0, const char * name = 0 );
		~TemplatesPage(); 
		void addItem(const QString& key, const QString& name, 
			const QString& description, const QPixmap& icon);
	private slots:
		void itemClicked(QIconViewItem *item);
	
	public:
		KIconView *templates;
		KTextBrowser *info;
};


class KexiStartupDialogPrivate;
class KexiProjectData;
class KexiProjectSet;
class KexiDBConnectionSet;
class ConnectionDataLVItem;

/*!
   This class is used to show the template/open-existing/open-recent tabbed dialog
   on Kexi startup. If only one page is shown, tab is no displayed, so dialog 
   becomes a normal "plain" type dialog.
 */
class KEXIMAIN_EXPORT KexiStartupDialog : public KDialogBase
{
    Q_OBJECT

public:
	/*! The Dialog returns one of these values depending 
	 on the input of the user.
	 CancelResult The user pressed 'Cancel'
	 TemplateResult The user selected a template
	 OpenExistingResult The user has chosen an existing connection or db file
	 OpenRecentResult The user selected one of recently used databases
	 */
	enum Result { CancelResult=0, TemplateResult=1, OpenExistingResult=2, OpenRecentResult=3 };
	/*!
	 To configure the dialog you have to use this enum 
	  (any !=0 or'ed value is ok)
	  - Templates Show "Templates" tab
	  - OpenExisting Show "Open existing" tab
	  - OpenRecent Show "Recent" tab
	  - Everything Show everything above
	 */
	enum DialogType { Templates = 1, OpenExisting = 2, OpenRecent = 4, Everything = (1+2+4) };

	/*! Options for a dialog
	  (any or'ed value or 0 is ok)
	  - CheckBoxDoNotShowAgain Adds "do not show this window" checkbox at the bottom
	*/
	enum DialogOptions { CheckBoxDoNotShowAgain = 1 };
	
	/*! Creates a dialog.
	 @param dialogType see DialogType description
	 @param dialogOptions see dialogOptions description
	 @param recentProjects a set of recent projects' info, used for "Open recent" tab
	 @param connSet conenction set used to present available conenctions
	  in "Open Existing" tab. Pass an empty object is this tab is not used.
	 @param parent parent widget, if any.
	 @param name name of this object.
	 */
	KexiStartupDialog(
		int dialogType, 
		int dialogOptions,
		KexiDBConnectionSet& connSet,
		KexiProjectSet& recentProjects,
		QWidget *parent = 0, const char *name = 0 );
	~KexiStartupDialog();

	/*! \return true if startup dialog should be shown (info is taken from kexi config)
	*/
	static bool shouldBeShown();

	/*! Executes dialog. 
	 \return one of Result values. Use this after dialog is closed. */
	int result() const;

	/*! \return key string of selected database template if result() is TemplateResult,
		otherwise null string. The key is of form: "<group>/<name>",
		for example: "business/address_book".
		For blank databases "blank" key is used - then additional information 
		about selected connection should be used with appropriate methods. */
	QString selectedTemplateKey() const;

	/*! \return data of selected Kexi project (if "Open Recent" tab was selected).
		Returns NULL if no selection has been made or other tab was selected.
	*/
	KexiProjectData* selectedProjectData() const;
	
	/*! \return name of selected Kexi project file 
		(if "Open Existing" tab was selected and this file was clicked).
		Returns empty string if no such selection has been made or other tab was selected.
	*/
	QString selectedExistingFile() const;

	/*! \return a pointer to selected Kexi connection data.
		(if "Open Existing" tab was selected and this connection data was clicked).
		Returns NULL if no such selection has been made or other tab was selected.
	*/
	KexiDB::ConnectionData* selectedExistingConnection() const;

	/*! Reimplemented for internal reasons */	
	virtual void show();

public slots:

protected slots:
	virtual void done(int r);
	virtual void reject();
	virtual void slotOk();
	
	//! slot activated when one of page in templates window is shown
	void templatesPageShown(QWidget *page);

	//! Any icon view item has been executed (dblclicked)
	void templateItemExecuted(QIconViewItem *item);

	//! Any icon view item has been selected
	void templateItemSelected(QIconViewItem *item);

	//! Any tab has been selected
	void tabShown(QWidget *w);

	//! helper
	void recentProjectItemExecuted(KexiProjectData *data);
	void existingFileSelected(const QString &f);
	void showSimpleConnForOpenExisting();
	void showAdvancedConnForOpenExisting();
	void connectionItemForOpenExistingExecuted(ConnectionDataLVItem *item);
	void connectionItemForOpenExistingHighlighted(ConnectionDataLVItem *item);

protected:
	virtual bool eventFilter( QObject *o, QEvent *e );
	
	//! helper: updates a state of dialog's OK button
	void updateDialogOKButton(QWidget *w);

	//! internal reimplementation
	int activePageIndex() const;
private:
	void setupPageTemplates();
	void setupPageOpenExisting();
	void setupPageOpenRecent();
	
	//! used internally on accepting templates selection
	void updateSelectedTemplateKeyInfo();

	KexiStartupDialogPrivate *d;
};

#endif

