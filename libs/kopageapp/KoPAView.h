/* This file is part of the KDE project

   Copyright (C) 2006-2009 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2009 Inge Wallin            <inge@lysator.liu.se>

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

#ifndef KOPAVIEW_H
#define KOPAVIEW_H

#include "KoPAViewBase.h"
#include "KoPageApp.h"
#include "kopageapp_export.h"

#include <KoView.h>
#include <KoZoomHandler.h>
#include <KoPart.h>

#include <QObject>

class KoCanvasController;
class KoFind;
class KoPart;
class KoPACanvasBase;
class KoPADocument;
class KoPAPageBase;
class KoPAViewMode;
class KoPADocumentStructureDocker;
class KoRuler;
class KoShapeManager;
class KoZoomAction;
class KoZoomController;
class KToggleAction;
class KUrl;

class QDragEnterEvent;
class QDropEvent;
class QTextDocument;
class QLabel;
class QTabBar;
class KoCopyController;
class KoCutController;

/// Creates a view with a KoPACanvasBase and rulers
class KOPAGEAPP_EXPORT KoPAView : public KoView, public KoPAViewBase
{
    Q_OBJECT
public:
    enum KoPAAction
    {
        ActionInsertPage = 1,
        ActionCopyPage   = 2,
        ActionDeletePage = 4,
        ActionViewShowMasterPages = 8,
        ActionFormatMasterPage = 16,
        AllActions       = 0xFF
    };

    enum KoPAFlags
    {
        NormalMode =1,
        ModeBox = 2
    };

    /**
     * Constructor
     * @param document the document of this view
     * @param parent the parent widget
     */
    explicit KoPAView(KoPart *part, KoPADocument *document, KoPAFlags withModeBox, QWidget *parent);

    virtual ~KoPAView();

    //  KoPAViewBase/KoView overrides

    void addImages(const QList<QImage> &imageList, const QPoint &insertAt);

    KoZoomController* zoomController() const;

    KoCopyController* copyController() const;

    KoCutController* cutController() const;

    KAction* deleteSelectionAction() const;

    void updateReadWrite( bool readwrite );

    KoRuler *horizontalRuler();
    KoRuler *verticalRuler();

    /// @return the canvas for the application
    KoPACanvasBase * kopaCanvas() const;
    /// @return the document for the application
    KoPADocument * kopaDocument() const;
    /// @return Page that is shown in the canvas
    KoPAPageBase* activePage() const;

    /// Set page shown in the canvas to @p page
    void setActivePage( KoPAPageBase * page );

    void navigatePage( KoPageApp::PageNavigation pageNavigation );

    /// @return the shape manager used for this view
    KoShapeManager* shapeManager() const;

    /// @return the master shape manager used for this view
    KoShapeManager* masterShapeManager() const;

    /**
     * @brief Enables/Disables the given actions
     *
     * The actions are of Type KoPAAction
     *
     * @param actions which should be enabled/disabled
     * @param enable new state of the actions
     */
    void setActionEnabled( int actions, bool enable );

    /**
     * @brief Set the view mode
     *
     * @param mode the new view mode
     */
    void setViewMode( KoPAViewMode* mode );

    /**
     * Set the active page and updates the UI
     */
    void doUpdateActivePage( KoPAPageBase * page );

    /**
     * Paste the page if everything is ok
     */
    void pagePaste();

    /// reimplemented
    virtual KoPrintJob * createPrintJob();

    /**
     * Get the thumbnail for the page.
     *
     * Us this method instead the on in the pages directly
     */
    QPixmap pageThumbnail(KoPAPageBase* page, const QSize& size);

    /**
     * Save thumbnail to an image file.
     *
     * Export a thumbnail to disk using a pixmap file like e.g. PNG
     * This method uses a QPixmap::save() call.
     *
     * @param page page to get thumbnail of
     * @param url the url of the image file to be created
     * @param size the desired image size in px
     * @param format the format of the image file (see QPixmap::save())
     * @param quality the quality of the image in [0,100] or -1 to use default (see QPixmap::save())
     *
     * @returns whether the image was successfully saved
     */
    bool exportPageThumbnail( KoPAPageBase * page, const KUrl& url, const QSize& size = QSize( 512, 512 ),
                              const char * format = 0, int quality = -1 );

    /// Update page navigation actions
    void updatePageNavigationActions();

    /// Shows/hides the rulers
    void setShowRulers(bool show);

    /// Insert a new page after the current one
    void insertPage();

    void centerPage();

    /// return a pointer to the tab bar (horizontal by default)
    QTabBar *tabBar() const;

    /// set view Tab Bar position (vertical / horizontal)
    void setTabBarPosition(Qt::Orientation orientation);

    /// Show a custom central widget and hides the standard one.
    void replaceCentralWidget(QWidget *newWidget);

    /// hides any custom central widget and shows the standard widget.
    void restoreCentralWidget();

signals:
    /// emitted when select All action is triggered and the view is not visible
    void selectAllRequested();
    /// emitted when deselect All action is triggered and the view is not visible
    void deselectAllRequested();

protected:

    /// creates the widgets (called from the constructor)
    void initGUI(KoPAFlags flags);
    /// creates the actions (called from the constructor)
    void initActions();

    /// Returns the document structure docker
    KoPADocumentStructureDocker* documentStructureDocker() const;

    bool isMasterUsed( KoPAPageBase * page );
    void editPaste();

    void hideCustomCentralWidget();

public slots:

    /// Copy Page
    void copyPage();

    /// Delete the current page
    void deletePage();

    /// Make sure the canvas size matches the content
    void updateCanvasSize(bool forceUpdate = false);

protected slots:

    void viewSnapToGrid(bool snap);
    void viewGuides(bool show);
    void slotZoomChanged( KoZoomMode::Mode mode, qreal zoom );

    void editDeleteSelection();
    void editSelectAll();
    void editDeselectAll();

    void formatMasterPage();

    void formatPageLayout();

    /// Change the current view mode to work on master pages
    void setMasterMode( bool master );

    // update the rulers
    void pageOffsetChanged();

    /// Called when the mouse position changes on the canvas
    virtual void updateMousePosition(const QPoint& position);

    /// Called when the selection changed
    virtual void selectionChanged();

    /// Called when the clipboard changed
    virtual void clipboardDataChanged();

    /// Go to the previous page
    void goToPreviousPage();
    /// Go to the next page
    void goToNextPage();
    /// Go to the first page
    void goToFirstPage();
    /// Go to the last page
    void goToLastPage();

    /**
     * Set the next document that should be used in find
     *
     * @param document The current document
     */
    void findDocumentSetNext( QTextDocument * document );

    /**
     * Set the previous document that should be used in find
     *
     * @param document The current document
     */
    void findDocumentSetPrevious( QTextDocument * document );

    /**
     * Re-initialize the document structure docker after active document in this
     * view has been changed
     */
    void reinitDocumentDocker();

    /**
     * Import slideshow
     */
    void importDocument();

    /**
     * Configure kopapage apps
     */
    void configure();

    /**
     * This is called when the unit of the document changes
     */
    void updateUnit(const KoUnit &unit);

private:
    class Private;
    Private * const d;
};

#endif /* KOPAVIEW_H */
