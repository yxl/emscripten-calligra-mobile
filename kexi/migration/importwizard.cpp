/* This file is part of the KDE project
   Copyright (C) 2004 Adam Pigg <adam@piggz.co.uk>
   Copyright (C) 2004-2005 Jaroslaw Staniek <js@iidea.pl>
   Copyright (C) 2005 Martin Ellis <martin.ellis@kdemail.net>

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

#include "importwizard.h"
#include "keximigrate.h"
#include "migratemanager.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qvbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>

#include <kcombobox.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kdebug.h>
#include <klineedit.h>
#include <kiconloader.h>
#include <kactivelabel.h>

#include <kexidb/drivermanager.h>
#include <kexidb/driver.h>
#include <kexidb/connectiondata.h>
#include <core/kexidbconnectionset.h>
#include <core/kexi.h>
#include <KexiConnSelector.h>
#include <KexiProjectSelector.h>
#include <KexiOpenExistingFile.h>
#include <KexiDBTitlePage.h>
#include <kexiutils/utils.h>
#include <kexidbdrivercombobox.h>
#include <kexitextmsghandler.h>


using namespace KexiMigration;

//===========================================================
//
ImportWizard::ImportWizard(QWidget *parent, QMap<QString,QString>* args)
 : KWizard(parent)
 , m_args(args)
{
	parseArguments();
	setCaption(i18n("Database Importing"));
	setIcon(DesktopIcon("database_import"));
//	finishButton()->setText(i18n("&Import"));
	m_prjSet = 0;
	m_fileBasedDstWasPresented = false;
	m_setupFileBasedSrcNeeded = true;
	m_acceptImportExecuted = false;

	setMinimumSize(400, 400);
	setupIntro();
//	setupSrcType();
	setupSrcConn();
	setupSrcDB();
	setupDstType();
	setupDstTitle();
	setupDst();
	setupImportType();
	setupImporting();
	setupFinish();

	connect(this, SIGNAL(selected(const QString &)), this, SLOT(pageSelected(const QString &)));
	connect(this, SIGNAL(helpClicked()), this, SLOT(helpClicked()));

	if (!m_predefinedFileName.isEmpty()) {
		// setup wizard for predefined selections 
		// (used when external project type was opened in Kexi, e.g. mdb file)
//		MigrateManager manager;
//		QString driverName = manager.driverForMimeType( m_predefinedMimeType );
//		m_srcTypeCombo->setCurrentText( driverName );

//		showPage( m_srcConnPage );
		m_srcConn->showSimpleConn();
		m_srcConn->setSelectedFileName(m_predefinedFileName);

		//disable all prev pages except "welcome" page
		for (int i=0; i<indexOf(m_dstTypePage); i++) {
			if (page(i)!=m_introPage)
				setAppropriate( page(i), false );
		}
	}
//	setBackEnabled(m_finishPage, false);
}

//===========================================================
//
ImportWizard::~ImportWizard()
{
	delete m_prjSet;
}

//===========================================================
//
void ImportWizard::parseArguments()
{
	if (!m_args)
		return;

	if (!(*m_args)["fileName"].isEmpty() && !(*m_args)["mimeType"].isEmpty()) {
		m_predefinedFileName = (*m_args)["fileName"];
		m_predefinedMimeType = (*m_args)["mimeType"];
	}
	m_args->clear();
}

//===========================================================
//
void ImportWizard::setupIntro()
{
	m_introPage = new QWidget(this);
	QVBoxLayout *vbox = new QVBoxLayout(m_introPage, KDialog::marginHint());
	
	QLabel *lblIntro = new QLabel(m_introPage);
	lblIntro->setAlignment( Qt::AlignTop | Qt::AlignLeft | Qt::WordBreak );
	QString msg;
	if (!m_predefinedFileName.isEmpty()) { //predefined import
//! @todo this message is currently ok for files only
		KMimeType::Ptr mimeTypePtr = KMimeType::mimeType(m_predefinedMimeType);
		msg = i18n("Database Importing wizard is about to import \"%1\" file "
		"of type \"%2\" into a Kexi database.").arg(m_predefinedFileName).arg(mimeTypePtr->comment());
	}
	else {
		msg = i18n("Database Importing wizard allows you to import an existing database "
			"into a Kexi database.");
	}
	lblIntro->setText(msg+"\n\n"
		+i18n("Click \"Next\" button to continue or \"Cancel\" button to exit this wizard."));
	vbox->addWidget( lblIntro );
	addPage(m_introPage, i18n("Welcome to the Database Importing Wizard"));
}

//===========================================================
//
/*
void ImportWizard::setupSrcType()
{
	m_srcTypePage = new QWidget(this);

//! @todo Would be good if KexiDBDriverComboBox worked for migration drivers 
	QVBoxLayout *vbox = new QVBoxLayout(m_srcTypePage, KDialog::marginHint());

	QHBoxLayout *hbox = new QHBoxLayout(vbox);
	QLabel *lbl = new QLabel(i18n("Source database type:")+" ", m_srcTypePage);
	hbox->addWidget(lbl);

	m_srcTypeCombo = new KComboBox(m_srcTypePage);
	hbox->addWidget(m_srcTypeCombo);
	hbox->addStretch(1);
	vbox->addStretch(1);
	lbl->setBuddy(m_srcTypeCombo);

	MigrateManager manager;
	QStringList names = manager.driverNames();

	m_srcTypeCombo->insertStringList(names);
	addPage(m_srcTypePage, i18n("Select Source Database Type"));
}
*/
//===========================================================
//
void ImportWizard::setupSrcConn()
{
	m_srcConnPage = new QWidget(this);
	QVBoxLayout *vbox = new QVBoxLayout(m_srcConnPage, KDialog::marginHint());

	m_srcConn = new KexiConnSelectorWidget(Kexi::connset(), m_srcConnPage, "m_srcConnSelector");
	m_srcConn->hideConnectonIcon();

//	m_srcConn->hideHelpers();
	vbox->addWidget(m_srcConn);
	addPage(m_srcConnPage, i18n("Select Location for Source Database"));
}

//===========================================================
//
void ImportWizard::setupSrcDB()
{
//	arrivesrcdbPage creates widgets on that page
	m_srcDBPage = new QWidget(this);
	m_srcDBName = NULL;
	addPage(m_srcDBPage, i18n("Select Source Database"));
}

//===========================================================
//
void ImportWizard::setupDstType()
{
	m_dstTypePage = new QWidget(this);

	KexiDB::DriverManager manager;
	KexiDB::Driver::InfoMap drvs = manager.driversInfo();

	QVBoxLayout *vbox = new QVBoxLayout(m_dstTypePage, KDialog::marginHint());

	QHBoxLayout *hbox = new QHBoxLayout(vbox);
	QLabel *lbl = new QLabel(i18n("Destination database type:")+" ", m_dstTypePage);
	hbox->addWidget(lbl);
	m_dstTypeCombo = new KexiDBDriverComboBox(drvs, true, m_dstTypePage);

	hbox->addWidget(m_dstTypeCombo);
	hbox->addStretch(1);
	vbox->addStretch(1);
	lbl->setBuddy(m_dstTypeCombo);

//! @todo hardcoded: find a way to preselect default engine item
	m_dstTypeCombo->setCurrentText("SQLite3");
	addPage(m_dstTypePage, i18n("Select Destination Database Type"));
}

//===========================================================
//
void ImportWizard::setupDstTitle()
{
	m_dstTitlePage = new KexiDBTitlePage(i18n("Destination project's caption:"), 
		this, "KexiDBTitlePage");
	m_dstTitlePage->layout()->setMargin( KDialog::marginHint() );
	m_dstTitlePage->updateGeometry();
	m_dstNewDBNameLineEdit = m_dstTitlePage->le_caption;
	addPage(m_dstTitlePage, i18n("Select Destination Database Project's Caption"));
}

//===========================================================
//
void ImportWizard::setupDst()
{
	m_dstPage = new QWidget(this);
	QVBoxLayout *vbox = new QVBoxLayout(m_dstPage, KDialog::marginHint());

	m_dstConn = new KexiConnSelectorWidget(Kexi::connset(), m_dstPage, "m_dstConnSelector");
	m_dstConn->hideHelpers();
	//me: Can't connect m_dstConn->m_fileDlg here, it doesn't exist yet
	//connect(this, SLOT(next()), m_dstConn->m_fileDlg, SIGNAL(accepted()));

	vbox->addWidget( m_dstConn );
	connect(m_dstConn,SIGNAL(connectionItemExecuted(ConnectionDataLVItem*)),
		this,SLOT(next()));

//	m_dstConn->hideHelpers();
	m_dstConn->showSimpleConn();
	//anyway, db files will be _saved_
	m_dstConn->m_fileDlg->setMode( KexiStartupFileDialog::SavingFileBasedDB );
//	m_dstConn->hideHelpers();
//	m_dstConn->m_file->btn_advanced->hide();
//	m_dstConn->m_file->label->hide();
//	m_dstConn->m_file->lbl->hide();
	//m_dstConn->m_file->spacer7->hide();


	//js dstNewDBName = new KLineEdit(dstControls);
	//   dstNewDBName->setText(i18n("Enter new database name here"));
	addPage(m_dstPage, i18n("Select Location for Destination Database"));
}

//===========================================================
//
void ImportWizard::setupImportType()
{
	m_importTypePage = new QWidget(this);
	QVBoxLayout *vbox = new QVBoxLayout(m_importTypePage, KDialog::marginHint());
	m_importTypeButtonGroup = new QVButtonGroup(m_importTypePage);
	m_importTypeButtonGroup->setLineWidth(0);
	vbox->addWidget( m_importTypeButtonGroup );

	(void)new QRadioButton(i18n("Structure and data"), m_importTypeButtonGroup);
	(void)new QRadioButton(i18n("Structure only"), m_importTypeButtonGroup);

	m_importTypeButtonGroup->setExclusive( true );
	m_importTypeButtonGroup->setButton( 0 );
	addPage(m_importTypePage, i18n("Select Type of Import"));
}

//===========================================================
//
void ImportWizard::setupImporting()
{
	m_importingPage = new QWidget(this);
	m_importingPage->hide();
	QVBoxLayout *vbox = new QVBoxLayout(m_importingPage, KDialog::marginHint());
	m_lblImportingTxt = new QLabel(m_importingPage);
	m_lblImportingTxt->setAlignment( Qt::AlignTop | Qt::AlignLeft | Qt::WordBreak );

	m_lblImportingErrTxt = new KActiveLabel(m_importingPage);
	m_lblImportingErrTxt->setAlignment( Qt::AlignTop | Qt::AlignLeft | Qt::WordBreak );

	m_progress = new KProgress(100, m_importingPage);
	m_progress->hide();

	vbox->addWidget( m_lblImportingTxt );
	vbox->addWidget( m_lblImportingErrTxt );
	vbox->addStretch(1);
	vbox->addWidget( m_progress );
	vbox->addStretch(1);

	m_importingPage->show();

	addPage(m_importingPage, i18n("Importing"));
}

//===========================================================
//
void ImportWizard::setupFinish()
{
	m_finishPage = new QWidget(this);
	m_finishPage->hide();
	QVBoxLayout *vbox = new QVBoxLayout(m_finishPage, KDialog::marginHint());
	m_finishLbl = new KActiveLabel(m_finishPage);
	m_finishLbl->setAlignment( Qt::AlignTop | Qt::AlignLeft | Qt::WordBreak );

	vbox->addWidget( m_finishLbl );
	m_openImportedProjectCheckBox = new QCheckBox(i18n("Open imported project"), 
		m_finishPage, "openImportedProjectCheckBox");
	m_openImportedProjectCheckBox->setChecked(true);
	vbox->addSpacing( KDialog::spacingHint() );
	vbox->addWidget( m_openImportedProjectCheckBox );
	vbox->addStretch(1);

	addPage(m_finishPage, i18n("Success"));
}

//===========================================================
//
bool ImportWizard::checkUserInput()
{
	QString finishtxt;
	bool problem;

	problem = false;
//	if ((dstNewDBName->text() == "Enter new database name here" || dstNewDBName->text().isEmpty()))
	if (m_dstNewDBNameLineEdit->text().isEmpty())
	{
		problem = true;
		finishtxt = finishtxt + "\n" + i18n("No new database name was entered.");
	}

	if (problem)
	{
		finishtxt = i18n("Following problems were found with the data you entered:") +
					"\n\n" +
					finishtxt +
					"\n\n" +
					i18n("Please click 'Back' button and correct these errors.");
		m_lblImportingErrTxt->setText(finishtxt);
	}
//	else
//	{
//it was weird		finishtxt = i18n("No problems were found with the data you entered.");
//	}

	return !problem;
}

void ImportWizard::arriveSrcConnPage()
{
	m_srcConnPage->hide();

//	checkIfSrcTypeFileBased(m_srcTypeCombo->currentText());
//	if (fileBasedSrcSelected()) {
		m_srcConn->showSimpleConn();
		/*! @todo KexiStartupFileDialog needs "open file" and "open server" modes
		in addition to just "open" */
		if (m_setupFileBasedSrcNeeded) {
			m_setupFileBasedSrcNeeded = false;
			QStringList additionalMimeTypes;
	/* moved
			if (m_srcTypeCombo->currentText().contains("Access")) {
	//! @todo tmp: hardcoded!
				additionalMimeTypes << "application/x-msaccess";
			}*/
			m_srcConn->m_fileDlg->setMode(KexiStartupFileDialog::Opening, additionalMimeTypes);
/*moved			if (m_srcTypeCombo->currentText().contains("Access")) {
	//! @todo tmp: hardcoded!
	#ifdef Q_WS_WIN
				m_srcConn->m_fileDlg->setSelectedFilter("*.mdb");
	#else
				m_srcConn->m_fileDlg->setFilter("*.mdb");
	#endif
			}*/
			//m_srcConn->m_file->label->hide();
			//m_srcConn->m_file->btn_advanced->hide();
			//m_srcConn->m_file->label->parentWidget()->hide();
		}
//	} else {
//		m_srcConn->showAdvancedConn();
//	}
	/*! @todo Support different file extensions based on MigrationDriver */
	m_srcConnPage->show();
}

void ImportWizard::arriveSrcDBPage()
{
	if (fileBasedSrcSelected()) {
		//! @todo Back button doesn't work after selecting a file to import
		//moved	showPage(m_dstTypePage);
	}
	else if (!m_srcDBName) {
		m_srcDBPage->hide();
		kdDebug() << "Looks like we need a project selector widget!" << endl;

		KexiDB::ConnectionData* condata = m_srcConn->selectedConnectionData();
		if(condata) {
			m_prjSet = new KexiProjectSet(*condata);
			QVBoxLayout *vbox = new QVBoxLayout(m_srcDBPage, KDialog::marginHint());
			m_srcDBName = new KexiProjectSelectorWidget(m_srcDBPage,
				"KexiMigrationProjectSelector", m_prjSet);
			vbox->addWidget( m_srcDBName );
			m_srcDBName->label->setText(i18n("Select source database to you wish to import:"));
		}
		m_srcDBPage->show();
	}
}

void ImportWizard::arriveDstTitlePage()
{
	if(fileBasedSrcSelected()) {
		QString suggestedDBName( QFileInfo(m_srcConn->selectedFileName()).fileName() );
		const QFileInfo fi( suggestedDBName );
		suggestedDBName = suggestedDBName.left(suggestedDBName.length() 
			- (fi.extension().length() ? (fi.extension().length()+1) : 0));
		m_dstNewDBNameLineEdit->setText( suggestedDBName );
	} else {
		if (!m_srcDBName || !m_srcDBName->selectedProjectData()) {
			back(); //todo!
			return;
		}
		m_dstNewDBNameLineEdit->setText( m_srcDBName->selectedProjectData()->databaseName() );
	}
}

void ImportWizard::arriveDstPage()
{
	m_dstPage->hide();

//	checkIfDstTypeFileBased(m_dstTypeCombo->currentText());
	if(fileBasedDstSelected()) {
		m_dstConn->showSimpleConn();
		m_dstConn->m_fileDlg->setMode( KexiStartupFileDialog::SavingFileBasedDB );
		if (!m_fileBasedDstWasPresented) {
			//without extension - it will be added automatically
			m_dstConn->m_fileDlg->setLocationText(m_dstNewDBNameLineEdit->text());
		}
		m_fileBasedDstWasPresented = true;
	} else {
		m_dstConn->showAdvancedConn();
	}
	m_dstPage->show();
}

void ImportWizard::arriveImportingPage() {
//	checkIfDstTypeFileBased(m_dstTypeCombo->currentText());
/*moved	if (m_fileBasedDstWasPresented) {
		if (!m_dstConn->m_fileDlg->checkFileName()) {
			back();
			return;
		}
	}*/
	if (checkUserInput()) {
		setNextEnabled(m_importingPage, true);
	}
	else {
		setNextEnabled(m_importingPage, false);
	}

	m_lblImportingTxt->setText(i18n(
				 "All required information has now "
				 "been gathered. Click \"Next\" button to start importing.\n\n"
				 "Depending on size of the database this may take some time."
				 /*"Note: You may be asked for extra "
				 "information such as field types if "
				 "the wizard could not automatically "
				 "determine this for you."*/));
}

void ImportWizard::arriveFinishPage() {
//	backButton()->hide();
//	cancelButton()->setEnabled(false);
//	m_finishLbl->setText(	m_successText.arg(m_dstNewDBNameLineEdit->text()) );
}

bool ImportWizard::fileBasedSrcSelected() const {
	return m_srcConn->selectedConnectionType()==KexiConnSelectorWidget::FileBased;
/*	QString srcType(m_srcTypeCombo->currentText());

	//! @todo Use MigrateManager to get src type property
	if ((srcType == "PostgreSQL") || (srcType == "MySQL")) {
		return false;
	} else {
		return true;
	}*/
}

bool ImportWizard::fileBasedDstSelected() const {
	QString dstType(m_dstTypeCombo->currentText());

	//! @todo Use DriverManager to get dst type property
	KexiDB::DriverManager manager;
	return manager.driverInfo(dstType).fileBased;

/*	if ((dstType == "PostgreSQL") || (dstType == "MySQL")) {
		return false;
	} else {
		return true;
	}*/
}


void ImportWizard::progressUpdated(int percent) {
	m_progress->setProgress(percent);
	KApplication::kApplication()->processEvents();
}

//===========================================================
//
QString ImportWizard::driverNameForSelectedSource()
{
	if (fileBasedSrcSelected()) {
		KMimeType::Ptr ptr = KMimeType::findByFileContent( m_srcConn->selectedFileName() );
		if (!ptr || ptr.data()->name()=="application/octet-stream" || ptr.data()->name()=="text/plain") {
			//try by URL:
			ptr = KMimeType::findByURL( m_srcConn->selectedFileName() );
		}
		MigrateManager manager;
		return ptr ? manager.driverForMimeType( ptr.data()->name() ) : QString::null;
	}

	//server-based
	return m_srcConn->selectedConnectionData() 
		? m_srcConn->selectedConnectionData()->driverName : QString::null;
}

//===========================================================
//
void ImportWizard::accept()
{
	/*moved
	backButton()->setEnabled(false);
	finishButton()->setEnabled(false);
//	cancelButton()->setEnabled(false);
	acceptImport();
	backButton()->setEnabled(true);
	finishButton()->setEnabled(true);
//	cancelButton()->setEnabled(true);
*/
	if (m_args) {
		if (!fileBasedDstSelected() || !m_openImportedProjectCheckBox->isChecked()) {
			//do not opend dest db if used didn't want it
//! @todo also remove result when using server connection as target
			m_args->remove("destinationFileName");
		}
	}
	KWizard::accept();
}

bool ImportWizard::acceptImport()
{
	m_acceptImportExecuted = true;
	KexiUtils::WaitCursor wait;
	
	kdDebug() << "Creating managers..." << endl;
	// Start with a driver manager
	KexiDB::DriverManager manager;

	kdDebug() << "Creating destination driver..." << endl;
	// Get a driver to the destination database
	KexiDB::Driver *driver = manager.driver(m_dstTypeCombo->currentText());

	//Check for errors
	if (!driver || manager.error())
	{
		kdDebug() << "Manager error..." << endl;
		manager.debugError();
		return false;
	}

	KexiDB::ConnectionData *cdata;
	bool cdataOwned = false;
	QString dbname;
	if (m_dstConn->selectedConnectionData())
	{
		//server-based project
		kdDebug() << "Server destination..." << endl;
		cdata = m_dstConn->selectedConnectionData();
		dbname = m_dstNewDBNameLineEdit->text();
	}
	else if (m_dstTypeCombo->currentText().lower() == KexiDB::Driver::defaultFileBasedDriverName()) 
	{
		//file-based project
		kdDebug() << "File Destination..." << endl;
		cdata = new KexiDB::ConnectionData();
		cdataOwned = true;
		cdata->caption = m_dstNewDBNameLineEdit->text();
		cdata->driverName = KexiDB::Driver::defaultFileBasedDriverName();
		dbname = m_dstConn->selectedFileName();
		cdata->setFileName( dbname );
		kdDebug() << "Current file name: " << dbname << endl;
	}
	else
	{
		//TODO This needs a better message
		KMessageBox::error(this, 
			i18n("No connection data is available. You did not select an destination filename."));
		return false;
	}

	kdDebug() << "Creating connection to destination..." << endl;
/*moved to KexiMigrate
	//Create connections to the kexi database
	KexiDB::Connection *kexi_conn = driver->createConnection(*cdata);
	if(!kexi_conn || driver->error()) {
		kdDebug() << "Creating destination connection error..." << endl;
		KMessageBox::error(this, driver->errorMsg());
		delete kexi_conn;
		return;
	}
*/
	kdDebug() << "Creating source driver..." << endl;
	MigrateManager migrateManager;

//	KexiMigrate* import = migrateManager.driver(m_srcTypeCombo->currentText());
	KexiMigrate* import = migrateManager.driver( driverNameForSelectedSource() );
	if(!import || migrateManager.error()) {
		kdDebug() << "Import migrate driver error..." << endl;
		KMessageBox::error(this, migrateManager.errorMsg());
//		delete kexi_conn;
		return false;
	}

	if(import->progressSupported()) {
		m_progress->show();
		m_progress->updateGeometry();
		connect(import, SIGNAL(progressPercent(int)),
			this, SLOT(progressUpdated(int)));
		progressUpdated(0);
	}

	kdDebug() << "Setting import data.." << endl;
	bool keepData;
	if (m_importTypeButtonGroup->selectedId() == 0)
	{
		kdDebug() << "Structure and data selected" << endl;
		keepData = true;
	}
	else if (m_importTypeButtonGroup->selectedId() == 1)
	{
		kdDebug() << "structure only selected" << endl;
		keepData = false;
	}
	else
	{
		kdDebug() << "Neither radio button is selected (not possible?) presume keep data" << endl;
		keepData = true;
	}
	
	KexiMigration::Data* md = new KexiMigration::Data();
//	delete md->destination;
	md->destination = new KexiProjectData(*cdata, dbname);
	if(fileBasedSrcSelected()) {
		KexiDB::ConnectionData* conn_data = new KexiDB::ConnectionData();
		conn_data->setFileName(m_srcConn->selectedFileName());
		md->source = conn_data;
		md->sourceName = "";
		//	  md->destName = dbname;
		//	  md->keepData = keepData;
		//	  import->setData(md);
	}
	else 
	{
		md->source = m_srcConn->selectedConnectionData();
		md->sourceName = m_srcDBName->selectedProjectData()->databaseName();
//	  md->destName = dbname;
//	  md->keepData = keepData;
//	  import->setData(md);
//! @todo Aah, this is so C-like. Move to performImport().
	}
	md->keepData = keepData;
	import->setData(md);
	kdDebug() << "Performing import..." << endl;
	KexiUtils::removeWaitCursor();
	Kexi::ObjectStatus result;
	if (import->performImport(&result)) {
//		KWizard::accept(); //tmp, before adding "final page"
//		KMessageBox::information(this, i18n("Import Succeeded."), i18n("Success"));
		if (m_args) {
			if (fileBasedDstSelected()) {
//! @todo also pass result when using server connection as target
				m_args->insert("destinationFileName", dbname);
			}
		}
		setTitle(m_finishPage, i18n("Success"));
		return true;
	}
	else
	{
//??		KWizard::reject(); //tmp, before adding "final page"
		m_progress->setProgress(0);
		m_progress->hide();

		QString msg, details;
		KexiTextMessageHandler handler(msg, details);
		handler.showErrorMessage(&result);

//		KMessageBox::error(this, i18n("Import failed."), i18n("Failure"));

		kdDebug() << msg << "\n" << details << endl;
		setTitle(m_finishPage, i18n("Failure"));
		m_finishLbl->setText(	
			i18n("<p>Import failed.</p>%1<p>%2</p><p>You can click \"Back\" button and try again.</p>")
			.arg(msg).arg(details));
		return false;
	}
//	delete kexi_conn;
} 

void ImportWizard::reject()
{
	KWizard::reject();
}

//===========================================================
//
void ImportWizard::next()
{
	if (currentPage() == m_srcConnPage) {
		if (fileBasedSrcSelected()
			&& /*! @todo use KURL? */!QFileInfo(m_srcConn->selectedFileName()).isFile()) {

			KMessageBox::sorry(this,i18n("Select source database filename."));
			return;
		}

		if ( (! fileBasedSrcSelected()) && (! m_srcConn->selectedConnectionData()) ) {
			KMessageBox::sorry(this,i18n("Select source database."));
			return;
		}

		MigrateManager migrateManager;
		KexiMigrate* import = migrateManager.driver( driverNameForSelectedSource() );
		if(!import || migrateManager.error()) {
			QString dbname;
			if (fileBasedSrcSelected())
				dbname = m_srcConn->selectedFileName();
			else
				dbname = m_srcConn->selectedConnectionData() 
					? m_srcConn->selectedConnectionData()->serverInfoString() : QString::null;
				if (!dbname.isEmpty())
					dbname = QString(" \"%1\"").arg(dbname);
			KMessageBox::error(this, i18n("Could not import database%1. This type is not supported.")
				.arg(dbname));
			return;
		}
	}
	else if (currentPage() == m_dstPage) {
		if (m_fileBasedDstWasPresented) {
			if (!m_dstConn->m_fileDlg->checkFileName())
				return;
		}
	}
	else if (currentPage() == m_importingPage) {
		if (!m_acceptImportExecuted) {
			nextButton()->setEnabled(false);
			finishButton()->setEnabled(false);
			backButton()->setEnabled(false);
			m_lblImportingTxt->setText(i18n("Importing in progress..."));
			if (acceptImport()) {
				m_finishLbl->setText( 
					i18n("Database has been imported into Kexi database project \"%1\".")
					.arg(m_dstNewDBNameLineEdit->text()) );
				cancelButton()->setEnabled(false);
				setBackEnabled(m_finishPage, false);
				setFinishEnabled(m_finishPage, true);
				m_openImportedProjectCheckBox->show();
				next();
			}
			else {
				cancelButton()->setEnabled(true);
				setBackEnabled(m_finishPage, true);
				setFinishEnabled(m_finishPage, false);
				m_openImportedProjectCheckBox->hide();
				next();
				m_acceptImportExecuted = false;
			}
			return;
		}
	}

	setAppropriate( m_srcDBPage, !fileBasedSrcSelected() ); //skip m_srcDBPage
	KWizard::next();
}

void ImportWizard::back()
{
	setAppropriate( m_srcDBPage, !fileBasedSrcSelected() ); //skip m_srcDBPage
	KWizard::back();
}

void ImportWizard::pageSelected(const QString &)
{
	if (currentPage() == m_introPage) {
	}
//	else if (currentPage() == m_srcTypePage) {
//	}
	else if (currentPage() == m_srcConnPage) {
		arriveSrcConnPage();
	}
	else if (currentPage() == m_srcDBPage) {
		arriveSrcDBPage();
	}
	else if (currentPage() == m_dstTypePage) {
	}
	else if (currentPage() == m_dstTitlePage) {
		arriveDstTitlePage();
	}
	else if (currentPage() == m_dstPage) {
		arriveDstPage();
	}
	else if (currentPage() == m_importingPage) {
		arriveImportingPage();
	}
	else if (currentPage() == m_finishPage) {
		arriveFinishPage();
	}
}

void ImportWizard::helpClicked()
{
	if (currentPage() == m_introPage)
	{
		KMessageBox::information(this, i18n("No help available for this page"), i18n("Help"));
	}
/*	else if (currentPage() == m_srcTypePage)
	{
		KMessageBox::information(this, i18n("Here you can choose the type of data to import data from"), i18n("Help"));
	}*/
	else if (currentPage() == m_srcConnPage)
	{
		KMessageBox::information(this, i18n("Here you can choose the location to import data from"), i18n("Help"));
	}
	else if (currentPage() == m_srcDBPage)
	{
		KMessageBox::information(this, i18n("Here you can choose the actual database to import data from"), i18n("Help"));
	}
	else if (currentPage() == m_dstTypePage)
	{
		KMessageBox::information(this, i18n("Here you can choose the location to save the data"), i18n("Help"));
	}
	else if (currentPage() == m_dstPage)
	{
		KMessageBox::information(this, i18n("Here you can choose the location to save the data in and the new database name"), i18n("Help"));
	}
	else if (currentPage() == m_finishPage || currentPage() == m_importingPage)
	{
		KMessageBox::information(this, i18n("No help available for this page"), i18n("Help"));
	}
}

#include "importwizard.moc"
