/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef PAGE_H
#define PAGE_H

#include <qwidget.h>
#include <qptrlist.h>
#include <qpicture.h>
#include <qvaluelist.h>

#include <koRuler.h>
#include <qpixmap.h>

#include <koQueryTrader.h>
#include <kprinter.h>

#include <global.h>

#define RAD_FACTOR 180.0 / M_PI

class KoTextFormatInterface;
class KPresenterView;
class KPresenterDoc;
class QPainter;
class KTextEdit;
class KPTextObject;
class KPObject;
class QPopupMenu;
class QResizeEvent;
class QPaintEvent;
class QDragEnterEvent;
class QDragLeaveEvent;
class QDragMoveEvent;
class QDropEvent;
class QFocusEvent;
class QMouseEvent;
class QWheelEvent;
class QKeyEvent;
class KPPixmapObject;
class KPBackGround;
class QRect;
class QSize;
class QPoint;
class KPresenterSoundPlayer;
class KoTextObject;

class KPTextView;

/**********************************************************************/
/* class Page - There is ONE instance of this class for a given view  */
/* It manages the graphical representation of all the objects         */
/**********************************************************************/
class Page : public QWidget
{
    Q_OBJECT

public:

    // constructor - destructor
    Page( QWidget *parent=0, const char *name=0, KPresenterView *_view=0 );
    ~Page();

    // public functions
    void selectAllObj();
    void deSelectAllObj();
    void selectObj( int num );
    void selectObj( KPObject* );
    void deSelectObj( int num );
    void deSelectObj( KPObject* );
    void setFont(const QFont &font, bool _subscript, bool _superscript, const QColor &col, const QColor &backGroundColor, int flags);
    void setTextColor( const QColor & );
    void setTextBackgroundColor( const QColor & );
    void setTextAlign( int );
    void setTextDepthPlus();
    void setTextDepthMinus();
    void setNewRightIndent(double _rightIndent);
    void setNewLeftIndent(double _leftIndent);
    void setNewFirstIndent(double _firstIndent);
    void setTabList( const KoTabulatorList & tabList );
    void setTextBold( bool b );
    void setTextItalic( bool b );
    void setTextUnderline( bool b );
    void setTextStrikeOut( bool b );
    void setTextFamily( const QString &f );
    void setTextPointSize( int s );
    void setTextSubScript( bool b );
    void setTextSuperScript( bool b );
    void setTextDefaultFormat( );
    void setIncreaseFontSize();
    void setDecreaseFontSize();
    KPTextObject* kpTxtObj();

    // Start a screen presentation
    // All pages if curPgNum is -1, otherwise just curPgNum (1-based)
    void startScreenPresentation( float presFakt, int curPgNum = -1);
    void stopScreenPresentation();
    bool pNext( bool );
    bool pPrev( bool );
    void setNextPageTimer( bool _nextPageTimer ) { nextPageTimer = _nextPageTimer; }

    unsigned int presPage() const { return currPresPage; } // 1 based
    int presStep() const { return currPresStep; }
    float presFakt() const { return _presFakt; }
    int numPresSteps() const { return presStepList.count(); }
    int numPresPages() const { return slideList.count(); }

    bool canAssignEffect( QPtrList<KPObject> &objs ) const;

    void print( QPainter*, KPrinter*, float, float );


    void setPartEntry( KoDocumentEntry& _e )
    { partEntry = _e; }
    void setToolEditMode( ToolEditMode _m, bool updateView = true );
    void setAutoForm( QString _autoform )
    { autoform = _autoform; }

    void drawPageInPix( QPixmap&, int );
    void drawPageInPix2( QPixmap&, int, int, float _zoom = 1.0 );

    void gotoPage( int pg );


    QPtrList<KoTextObject> objectText();
    bool oneObjectTextExist();

    bool isOneObjectSelected();
    /** Returns the list of selected text objects */
    QPtrList<KPTextObject> selectedTextObjs() const;
    /**
     * Returns the list of interfaces to use for changing the text format.
     * This can be either the currently edited text object,
     * or the list of text objects currently selected
     */
    QPtrList<KoTextFormatInterface> applicableTextInterfaces() const;
    /**
     * Returns the list of text objects, either selected or activated.
     */
    QPtrList<KPTextObject> applicableTextObjects() const;

    void setMouseSelectedObject(bool b);

    bool haveASelectedPictureObj();

    KPTextView *currentTextObjectView() const { return m_currentTextObjectView; }

    KPresenterView * getView(){return view;}
    QPtrList<KPObject> *objectList();

    void stopSound();

    int diffx() const;
    int diffy() const;

    //for KPTextView
    void dragStarted() { mousePressed = false; }

    void setXimPosition( int x, int y, int w, int h, QFont *f );

public slots:
    void exitEditMode();

    void clipCut();
    void clipCopy();
    void clipPaste();
    void deleteObjs();
    void rotateObjs();
    void shadowObjs();

    void chPic();
    void chClip();
    void picViewOrig640x480();
    void picViewOrig800x600();
    void picViewOrig1024x768();
    void picViewOrig1280x1024();
    void picViewOrig1600x1200();
    void picViewOrigFactor();


signals:

    // signals to notify of changes
    void fontChanged( const QFont & );
    void colorChanged( const QColor & );
    void alignChanged( int );
    void updateSideBarItem( int );
    void stopPres();
    void objectSelectedChanged();

    void selectionChanged( bool hasSelection );
    void sigMouseWheelEvent( QWheelEvent * );

protected:

    struct PicCache
    {
        QPicture pic;
        int num;
        int subPresStep;
    };

    // functions for displaying
    virtual void paintEvent( QPaintEvent* );
    void paintBackground( QPainter*, QRect );
    void drawBackground( QPainter*, QRect, bool ignoreSkip = false );
    void drawObjects( QPainter*, QRect, bool drawCursor, bool ignoreSkip = false );
    virtual void mousePressEvent( QMouseEvent *e );
    virtual void mouseReleaseEvent( QMouseEvent *e );
    virtual void mouseMoveEvent( QMouseEvent *e );
    virtual void mouseDoubleClickEvent( QMouseEvent *e );
    virtual void wheelEvent( QWheelEvent *e );
    virtual void resizeEvent( QResizeEvent *e );
    int getObjectAt( int x, int y );
    virtual void focusInEvent( QFocusEvent* ) {}
    virtual void focusOutEvent( QFocusEvent* ) {}
    virtual void enterEvent( QEvent *e );
    virtual void leaveEvent( QEvent *e );
    virtual void dragEnterEvent( QDragEnterEvent *e );
    virtual void dragLeaveEvent( QDragLeaveEvent *e );
    virtual void dragMoveEvent( QDragMoveEvent *e );
    virtual void dropEvent( QDropEvent *e );
    virtual void keyPressEvent( QKeyEvent *e );
    virtual void keyReleaseEvent( QKeyEvent *e );

    void eraseEmptySpace( QPainter * painter, const QRegion & emptySpaceRegion, const QBrush & brush );

    // setup popupmenus
    void setupMenus();

    // get - set data
    QPtrList<KPBackGround> *backgroundList();
    const QPtrList<KPBackGround> *backgroundList() const;
    const QPtrList<KPObject> *objectList() const;
    unsigned int objNums();
    unsigned int currPgNum();
    unsigned int rastX() const;
    unsigned int rastY() const;
    QColor txtBackCol() const;
    bool spInfinitLoop() const;
    bool spManualSwitch() const;
    QRect getPageRect( unsigned int p, float fakt=1.0, bool decBorders = true );
    unsigned int pageNums();
    int getPageOfObj( int i, float fakt = 1.0 );
    float objSpeedFakt();
    float pageSpeedFakt();
    bool calcRatio( int &dx, int &dy, KPObject *kpobject, double ratio ) const;

    void _repaint( bool erase=true );
    void _repaint( QRect r );
    void _repaint( KPObject *o );

    void printPage( QPainter*, int, QRect );
    void changePages( QPixmap, QPixmap, PageEffect );
    void doObjEffects();
    void drawObject( KPObject*, QPixmap*, int, int, int, int, int, int );

    void insertText( QRect );
    void insertLineH( QRect, bool );
    void insertLineV( QRect, bool );
    void insertLineD1( QRect, bool );
    void insertLineD2( QRect, bool );
    void insertRect( QRect );
    void insertEllipse( QRect );
    void insertPie( QRect );
    void insertObject( QRect );
    void insertAutoform( QRect, bool );
    void insertFreehand( const QPointArray &_pointArray );
    void insertPolyline( const QPointArray &_pointArray );
    void insertCubicBezierCurve( const QPointArray &_pointArray );
    void insertPolygon( const QPointArray &_pointArray );

    void selectNext();
    void selectPrev();

    void scalePixmapToBeOrigIn( const QSize &origSize, const QSize &currentSize,
                                const QSize &pgSize, const QSize &presSize, KPPixmapObject *obj );
    bool getPixmapOrigAndCurrentSize( KPPixmapObject *&obj, QSize *origSize, QSize *currentSize );
    void setTextBackground( KPTextObject *obj );
    virtual bool eventFilter( QObject *o, QEvent *e );
    virtual bool focusNextPrevChild( bool );

    void endDrawPolyline();

    void drawCubicBezierCurve( int _dx, int _dy );
    void endDrawCubicBezierCurve();
    double getAngle( QPoint p1, QPoint p2 );

private:
    // variables
    QPopupMenu *presMenu;
    bool mousePressed;
    ModifyType modType;
    unsigned int oldMx, oldMy;
    int resizeObjNum, editNum;
    bool fillBlack;
    KPresenterView *view;
    bool editMode, goingBack, drawMode;
    bool drawLineInDrawMode;
    bool mouseSelectedObject;
    unsigned int currPresPage, currPresStep, subPresStep;
    unsigned int oldPresPage, oldPresStep, oldSubPresStep;
    float _presFakt;
    int m_showOnlyPage; // 1-based (-1 = all)
    QValueList<int> presStepList, slideList;
    QValueList<int>::Iterator slideListIterator;
    int PM_DM, PM_SM;
    int firstX, firstY;
    int delPageId;
    bool drawRubber;
    QRect rubber, oldBoundingRect, oldRect;
    ToolEditMode toolEditMode;
    QRect insRect;
    KoDocumentEntry partEntry;
    // List of objects to draw, from the previous step in the presentation
    // (those that remain on screen between two steps)
    QPtrList <KPObject> tmpObjs;
    QString autoform;
    bool inEffect, keepRatio;
    double ratio;
    QPixmap buffer;

    KPTextView *m_currentTextObjectView;

    KPresenterSoundPlayer *soundPlayer;

    QPointArray m_pointArray, m_oldCubicBezierPointArray;
    QPoint m_dragStartPoint, m_dragEndPoint, m_dragSymmetricEndPoint;
    QPoint m_CubicBezierSecondPoint, m_CubicBezierThirdPoint;
    unsigned int m_indexPointArray;
    bool m_drawPolyline;
    bool m_drawCubicBezierCurve;
    bool m_drawLineWithCubicBezierCurve;

private:
    QValueList<int> pages(const QString &range);
    bool pagesHelper(const QString &chunk, QValueList<int> &list);
    void picViewOrigHelper(int x, int y);

    void moveObject( int x, int y, bool key );
    void resizeObject( ModifyType _modType, int _dx, int _dy );

    void raiseObject();
    void lowerObject();
    int selectedObjectPosition;

    bool nextPageTimer;

    void playSound( const QString &soundFileName );

    QRect getDrawRect( const QPointArray &_points );

    void drawPolygon( const QPoint &startPoint, const QPoint &endPoint );

private slots:
    void toFontChanged( const QFont &font ) { emit fontChanged( font ); }
    void toColorChanged( const QColor &color ) { emit colorChanged( color ); }
    void toAlignChanged( int a ) { emit alignChanged( a ); }
    void drawingMode();
    void switchingMode();
    void slotGotoPage();
    void slotExitPres();
    void terminateEditing( KPTextObject * );
};
#endif //PAGE_H
