/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
             (C) 1999-2002 The KSpread Team
                           www.koffice.org/kspread

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

#ifndef __kspread_table_h__
#define __kspread_table_h__

class KSpreadSheet;

class ColumnFormat;
class RowFormat;
class KSpreadCell;
class KSpreadView;
class KSpreadPoint;
class KSpreadMap;
class KSpreadCanvas;
class KSpreadDoc;
class KoDocumentEntry;

class QStingList;
class QWidget;
class QPainter;
class QDomElement;

class DCOPObject;
class KPrinter;

#include <koDocument.h>
#include <koDocumentChild.h>

#include <qpen.h>
#include <qptrlist.h>
#include <qintdict.h>
#include <qmemarray.h>
#include <qrect.h>
#include <qwidget.h>
#include <qdragobject.h>

#define BORDER_SPACE 1

#include "kspread_autofill.h"
#include "kspread_format.h"
#include "kspread_cell.h"
#include "kspread_global.h"
#include "kspread_cluster.h"
#include "kspread_selection.h"

/********************************************************************
 *
 * CellBinding
 *
 ********************************************************************/

/**
 * @short This is an abstract base class only.
 */
class CellBinding : public QObject
{
    Q_OBJECT
public:
    CellBinding( KSpreadSheet *_table, const QRect& _area );
    virtual ~CellBinding();

    bool contains( int _x, int _y );
    /**
     * Call this function if one of the cells covered by this binding ( @see #rect )
     * has changed. This will in turn force for example a chart to update.
     *
     * @param _obj may by 0L. In this case all cells may have changed.
     */
    virtual void cellChanged( KSpreadCell *_obj );

    virtual void setIgnoreChanges( bool _ignore ) { m_bIgnoreChanges = _ignore; }

    virtual QRect& dataArea() { return m_rctDataArea; }
    virtual void setDataArea( const QRect _rect ) { m_rctDataArea = _rect; }

    KSpreadSheet* table()const { return m_pTable; }

signals:
    void changed( KSpreadCell *_obj );

protected:
    QRect m_rctDataArea;
    KSpreadSheet *m_pTable;
    bool m_bIgnoreChanges;
};

/********************************************************************
 *
 * KSpreadChild
 *
 ********************************************************************/

/**
 * Holds an embedded object.
 */
class KSpreadChild : public KoDocumentChild
{
public:
  KSpreadChild( KSpreadDoc *parent, KSpreadSheet *_table, KoDocument* doc, const QRect& geometry );
  KSpreadChild( KSpreadDoc *parent, KSpreadSheet *_table );
  ~KSpreadChild();

  KSpreadDoc* parent()const { return (KSpreadDoc*)parent(); }
  KSpreadSheet* table()const { return m_pTable; }

protected:
  KSpreadSheet *m_pTable;
};

/********************************************************************
 *
 * Charts
 *
 ********************************************************************/

class ChartChild;
namespace KoChart { class Part; }

class ChartBinding : public CellBinding
{
    Q_OBJECT
public:

    ChartBinding( KSpreadSheet *_table, const QRect& _area, ChartChild *_child );
    virtual ~ChartBinding();

    virtual void cellChanged( KSpreadCell *_obj );

private:
    ChartChild* m_child;
};

class ChartChild : public KSpreadChild
{
    Q_OBJECT
public:
    ChartChild( KSpreadDoc *_spread, KSpreadSheet *_table, KoDocument* doc, const QRect& _rect );
    ChartChild( KSpreadDoc *_spread, KSpreadSheet *_table );
    ~ChartChild();

    void setDataArea( const QRect& _data );
    void update();

    /**
     * @reimp
     */
    bool load( const QDomElement& element );
    /**
     * @reimp
     */
    QDomElement save( QDomDocument& doc );

    /**
     * @reimp
     */
    bool loadDocument( KoStore* _store );

    KoChart::Part* chart();

private:
    ChartBinding *m_pBinding;
};

/********************************************************************
 *
 * KSpreadTextDrag
 *
 ********************************************************************/

/**
 * @short This is a class for handling clipboard data
 */

class KSpreadTextDrag : public QTextDrag
{
    Q_OBJECT

public:
    KSpreadTextDrag( QWidget * dragSource = 0L, const char * name = 0L );
    virtual ~KSpreadTextDrag();

    void setPlain( QString const & _plain ) { setText( _plain ); }
    void setKSpread( QByteArray const & _kspread ) { m_kspread = _kspread; }

    virtual QByteArray encodedData( const char * mime ) const;
    virtual const char* format( int i ) const;

    static bool canDecode( QMimeSource * e );

    static const char * selectionMimeType();

protected:
    QByteArray m_kspread;
};


/********************************************************************
 *
 * Table
 *
 ********************************************************************/

/**
 */
class KSpreadSheet : public QObject
{
    friend class KSpreadCell;

    Q_OBJECT
public:
    enum SortingOrder{ Increase, Decrease };
    enum ChangeRef { ColumnInsert, ColumnRemove, RowInsert, RowRemove };
    enum TestType { Text, Validity, Comment, ConditionalCellAttribute };

    KSpreadSheet( KSpreadMap *_map, const QString &tableName, const char *_name=0L );
    ~KSpreadSheet();

    virtual bool isEmpty( unsigned long int x, unsigned long int y ) const;

    /**
     * @return the name of this table.
     *
     * @see #setTableName
     */
    QString tableName()const { return m_strName; }
    /**
     * Renames a table. This will automatically adapt all formulas
     * in all tables and all cells to reflect the new name.
     *
     * If the name really changed then @ref #sig_nameChanged is emitted
     * and the GUI will reflect the change. In addition a @ref KSpreadUndoSetTableName
     * object will be created to implement undo.
     *
     * @param init If set to TRUE then no formula will be changed and no signal
     *             will be emitted and no undo action created. Usually you dont
     *             want to do that.
     *
     *
     * @return FALSE if the table could not be renamed. Usually the reason is
     *         that this name is already used.
     *
     * @see #changeCellTabName
     * @see KSpreadTabBar::renameTab
     * @see #tableName
     */
    bool setTableName( const QString& name, bool init = FALSE, bool makeUndo=true );

    /**
     * Saves the table and all it's children in XML format
     */
    virtual QDomElement saveXML( QDomDocument& );
    /**
     * Loads the table and all it's children in XML format
     */
    virtual bool loadXML( const QDomElement& );

    /**
     * Saves a children
     */
    virtual bool saveChildren( KoStore* _store, const QString &_path );
    /**
     * Loads a children
     */
    virtual bool loadChildren( KoStore* _store );

    bool isLoading();

    void password( QCString & passwd ) const { passwd = m_strPassword; }
    bool isProtected() const { return !m_strPassword.isNull(); }
    void setProtected( QCString const & passwd );
    bool checkPassword( QCString const & passwd ) const { return ( passwd == m_strPassword ); }

    void setDefaultHeight( double height );
    void setDefaultWidth( double width );

    const ColumnFormat* columnFormat( int _column ) const;
    ColumnFormat* columnFormat( int _column );
    /**
     * If no special @ref ColumnFormat exists for this column, then a new one is created.
     *
     * @return a non default ColumnFormat for this column.
     */
    ColumnFormat* nonDefaultColumnFormat( int _column, bool force_creation = TRUE );

    const RowFormat* rowFormat( int _row ) const;
    RowFormat* rowFormat( int _row );
    /**
     * If no special @ref RowFormat exists for this row, then a new one is created.
     *
     * @return a non default RowFormat for this row.
     */
    RowFormat* nonDefaultRowFormat( int _row, bool force_creation = TRUE );

    /**
     * @return the first cell of this table. Next cells can
     * be retrieved by calling @ref KSpreadCell::nextCell.
     */
    KSpreadCell* firstCell() const;

    RowFormat* firstRow() const;

    ColumnFormat* firstCol() const;

    KSpreadCell* cellAt( int _column, int _row ) const;
    /**
     * @param _scrollbar_update will change the scrollbar if set to true disregarding
     *                          whether _column/_row are bigger than
     *                          m_iMaxRow/m_iMaxColumn. May be overruled by
     *                          @ref #m_bScrollbarUpdates.
     */
    KSpreadCell* cellAt( int _column, int _row, bool _scrollbar_update = false );
    /**
     * A convenience function.
     */
    KSpreadCell* cellAt( const QPoint& _point, bool _scrollbar_update = false )
      { return cellAt( _point.x(), _point.y(), _scrollbar_update ); }
    /**
     * @returns the pointer to the cell that is visible at a certain position. That means If the cell
     *          at this position is obscured then the obscuring cell is returned.
     *
     * @param _scrollbar_update will change the scrollbar if set to true disregarding
     *                          whether _column/_row are bigger than
     *                          m_iMaxRow/m_iMaxColumn. May be overruled by
     *                          @ref #m_bScrollbarUpdates.
     */
    KSpreadCell* visibleCellAt( int _column, int _row, bool _scrollbar_update = false );
    /**
     * If no special KSpreadCell exists for this position then a new one is created.
     *
     * @param _scrollbar_update will change the scrollbar if set to true disregarding
     *                          whether _column/_row are bigger than
     *                          m_iMaxRow/m_iMaxColumn. May be overruled by
     *                          @ref #m_bScrollbarUpdates.
     *
     * @return a non default KSpreadCell for the position.
     */
    KSpreadCell* nonDefaultCell( int _column, int _row, bool _scrollbar_update = false );
    KSpreadCell* nonDefaultCell( QPoint cellRef, bool scroll = false )
      { return nonDefaultCell( cellRef.x(), cellRef.y(), scroll ); }

    KSpreadCell* defaultCell()const { return m_pDefaultCell; }

    KSpreadFormat* defaultFormat() { return m_defaultFormat; };
    const KSpreadFormat* defaultFormat() const { return m_defaultFormat; }

    int topRow( int _ypos, double &_top, const KSpreadCanvas *_canvas = 0L ) const;
    int bottomRow( int _ypos, const KSpreadCanvas *_canvas = 0L ) const;
    int leftColumn( int _xpos, double &_left, const KSpreadCanvas *_canvas = 0L ) const;
    int rightColumn( int _xpos, const KSpreadCanvas *_canvas = 0L ) const;

    /**
     * @return the left corner of the column as int.
     *
     * @param _canvas If not 0 then the returned position is in screen
     *                coordinates. Otherwise the point (0|0) is in the upper
     *                left corner of the table.
     */
    int columnPos( int _col, const KSpreadCanvas *_canvas = 0L ) const;
    /**
     * @return the left corner of the column as double.
     * Use this method, when you later calculate other positions depending on this one
     * to avoid rounding problems
     *
     * @param _canvas If not 0 then the returned position is in screen
     *                coordinates. Otherwise the point (0|0) is in the upper
     *                left corner of the table.
     */
    double dblColumnPos( int _col, const KSpreadCanvas *_canvas = 0L ) const;
    /**
     * @return the top corner of the row as int.
     *
     * @param _canvas If not 0 then the returned position is in screen
     *                coordinates. Otherwise the point (0|0) is in the upper
     *                top corner of the table.
     */
    int rowPos( int _row, const KSpreadCanvas *_canvas = 0L ) const;
    /**
     * @return the top corner of the row as double.
     * Use this method, when you later calculate other positions depending on this one
     * to avoid rounding problems
     *
     * @param _canvas If not 0 then the returned position is in screen
     *                coordinates. Otherwise the point (0|0) is in the upper
     *                top corner of the table.
     */
    double dblRowPos( int _row, const KSpreadCanvas *_canvas = 0L ) const;

    /**
     * @return the maximum size of the column range
     */
    double sizeMaxX()const { return m_dSizeMaxX; }
    /**
     * @return the maximum size of the row range
     */
    double sizeMaxY()const { return m_dSizeMaxY; }

    /**
     * Adjusts the internal reference of the sum of the widths of all columns.
     * Used in resizing of columns.
     */
    void adjustSizeMaxX ( double _x );

    /**
     * Adjusts the internal reference of the sum of the heights of all rows.
     * Used in resizing of rows.
     */
    void adjustSizeMaxY ( double _y );

    /**
     * Sets the @ref KSpreadCell::layoutDirtyFlag in all cells.
     */
    void setLayoutDirtyFlag();
    /**
     * Sets the @ref KSpreadCell::calcDirtyFlag in all cells.
     * That means that the cells are marked dirty and will recalculate
     * if requested. This function does only MARK, it does NOT actually calculate.
     * Use @ref #recalc to recaculate dirty values.
     */
    void setCalcDirtyFlag();

    /**
     * Calculates all cells in the table with the CalcDirtyFlag.
     */
  //why on earth would we want to do this?
//    void calc();

    /**
     * Recalculates the current table. If you want to recalculate EVERYTHING, then
     * call @ref Table::setCalcDirtyFlag for all tables in the @ref #m_pMap to make
     * sure that no invalid values in other tables make you trouble.
     */
    void recalc();

    /**
     * Sets the contents of the cell at row,column to text
     * @param updateDepends set to false to disable updating the dependencies
     */
    void setText( int row, int column, const QString& text, bool updateDepends = true );



    void setSelectionFont( KSpreadSelection* selectionInfo,
                           const char *_font = 0L, int _size = -1,
                           signed char _bold = -1, signed char _italic = -1,
                           signed char _underline = -1,
                           signed char _strike = -1 );

    void setSelectionMoneyFormat( KSpreadSelection* selectionInfo, bool b );
    void setSelectionAlign( KSpreadSelection* selectionInfo,
                            KSpreadFormat::Align _align );
    void setSelectionAlignY( KSpreadSelection* selectionInfo,
                             KSpreadFormat::AlignY _alignY );
    void setSelectionPrecision( KSpreadSelection* selectionInfo, int _delta );
    void setSelectionPercent( KSpreadSelection* selectionInfo, bool b );
    void setSelectionMultiRow( KSpreadSelection* selectionInfo, bool enable );

    /**
    * setSelectionSize increase or decrease font size
    */
    void setSelectionSize( KSpreadSelection* selectionInfo, int _size );

    /**
     *change string to upper case if _type equals 1
     * or change string to lower if _type equals -1
     */
    void setSelectionUpperLower( KSpreadSelection* selectionInfo, int _type );

    void setSelectionfirstLetterUpper( KSpreadSelection* selectionInfo);

    void setSelectionVerticalText( KSpreadSelection* selectionInfo, bool _b);

    void setSelectionComment( KSpreadSelection* selectionInfo,
                              const QString &_comment);
    void setSelectionRemoveComment(KSpreadSelection* selectionInfo);

    void setSelectionAngle(KSpreadSelection* selectionInfo, int _value );

    void setSelectionTextColor( KSpreadSelection* selectionInfo,
                                const QColor &tbColor );
    void setSelectionbgColor( KSpreadSelection* selectionInfo,
                              const QColor &bg_Color );
    void setSelectionBorderColor( KSpreadSelection* selectionInfo,
                                  const QColor &bd_Color );

    /**
     * @param _marker is used if there is no selection currently.
     *                In this case the cell on which the marker is will
     *                be deleted.
     */
    void deleteSelection( KSpreadSelection* selectionInfo );
    /**
     * @param _marker is used if there is no selection currently.
     *                In this case the cell on which the marker is will
     *                be copied.
     */
    void copySelection( KSpreadSelection* selectionInfo );
    /**
     * @param _marker is used if there is no selection currently.
     *                In this case the cell on which the marker is will
     *                be cut.
     */
    void cutSelection( KSpreadSelection* selectionInfo );
    /**
     * @param _marker is used if there is no selection currently.
     *                In this case the cell on which the marker is will
     *                be cleared.
     */
    void clearTextSelection( KSpreadSelection* selectionInfo );

    void clearValiditySelection(KSpreadSelection* selectionInfo );

    void clearConditionalSelection(KSpreadSelection* selectionInfo );


    void setWordSpelling(KSpreadSelection* selectionInfo,const QString _listWord );

    QString getWordSpelling(KSpreadSelection* selectionInfo );

    /**
     * A convenience function which retrieves the data to be pasted
     * from the clipboard.
     */
    void paste( const QRect &pasteArea, bool makeUndo=true, PasteMode=Normal,
                Operation=OverWrite,bool insert=false,int insertTo=0 );
    void paste( const QByteArray& data, const QRect &pasteArea,
                bool makeUndo=false, PasteMode=Normal, Operation=OverWrite,
                bool insert=false, int insertTo=0 );
    void defaultSelection( KSpreadSelection* selectionInfo );

    /**
     * A function which allows to paste a text plain from the clipboard
     */
    void pasteTextPlain( QString &_text, QRect pasteArea);

    void sortByRow( const QRect &area, int ref_row, SortingOrder );
    void sortByRow( const QRect &area, int key1, int key2, int key3,
                    SortingOrder order1, SortingOrder order2, SortingOrder order3,
                    QStringList const * firstKey, bool copyFormat, bool headerRow,
                    KSpreadPoint const & outputPoint );
    void sortByColumn( const QRect &area, int ref_column, SortingOrder );
    void sortByColumn( const QRect &area, int key1, int key2, int key3,
                       SortingOrder order1, SortingOrder order2, SortingOrder order3,
                       QStringList const * firstKey, bool copyFormat, bool headerRow,
                       KSpreadPoint const & outputPoint );
    void swapCells( int x1, int y1, int x2, int y2, bool cpFormat );

    /**
     * @param x1, y1: values from source cell,
     * @param x2, y2: values from target cell
     * @param cpFormat: if true: cell format gets copied, too
     */
  void copyCells( int x1, int y1, int x2, int y2, bool cpFormat );
    void setSeries( const QPoint &_marker, double start, double end, double step, Series mode, Series type );

    /**
     * Moves all cells of the row _marker.y() which are in
     * the column _marker.x() or right hand of that one position
     * to the right.
     *
     * @return TRUE if the shift was possible, or false otherwise.
     *         A reason for returning FALSE is that there was a cell
     *         in the right most position.
     */
    bool shiftRow( const QRect &_rect, bool makeUndo=true );
    bool shiftColumn( const QRect& rect, bool makeUndo=true );

    void unshiftColumn( const QRect& rect, bool makeUndo=true );
    void unshiftRow( const QRect& rect, bool makeUndo=true );

    /**
     * Moves all columns which are >= @p col one position to the right and
     * inserts a new and empty column. After this the table is redrawn.
     * nbCol is the number of column which are installing
     */
    bool insertColumn( int col, int nbCol=0, bool makeUndo=true );
    /**
     * Moves all rows which are >= @p row one position down and
     * inserts a new and empty row. After this the table is redrawn.
     */
    bool insertRow( int row, int nbRow=0, bool makeUndo=true );

    /**
     * Deletes the column @p col and redraws the table.
     */
    void removeColumn( int col, int nbCol=0, bool makeUndo=true );
    /**
     * Deletes the row @p row and redraws the table.
     */
    void removeRow( int row, int nbRow=0, bool makeUndo=true );

    /**
    * hide row
    */
    void hideRow( int row, int nbRow=0, QValueList<int>list=QValueList<int>() );
    void emitHideRow();
    void showRow( int row, int NbRow=0, QValueList<int>list=QValueList<int>() );

    /**
    * hide column
    */
    void hideColumn( int col, int NbCol=0, QValueList<int>list=QValueList<int>() );
    void emitHideColumn();
    void showColumn( int col, int NbCol=0, QValueList<int>list=QValueList<int>() );

    int adjustColumn( KSpreadSelection* selectionInfo, int _col = -1 );
    int adjustRow( KSpreadSelection* selectionInfo, int _row = -1 );

    /**
     * Install borders
     */
    void borderLeft( KSpreadSelection* selectionInfo, const QColor &_color );
    void borderTop( KSpreadSelection* selectionInfo, const QColor &_color );
    void borderOutline( KSpreadSelection* selectionInfo, const QColor &_color );
    void borderAll( KSpreadSelection* selectionInfo, const QColor &_color );
    void borderRemove( KSpreadSelection* selectionInfo );
    void borderBottom( KSpreadSelection* selectionInfo, const QColor &_color );
    void borderRight( KSpreadSelection* selectionInfo, const QColor &_color );

    void setConditional( KSpreadSelection* selectionInfo,
			 QValueList<KSpreadConditional> const & newConditions );

    void setValidity( KSpreadSelection* selectionInfo,KSpreadValidity tmp );

    /**
     * Returns, if the grid shall be shown on the screen
     */
    bool getShowGrid() const { return m_bShowGrid; }

    /**
     * Sets, if the grid shall be shown on the screen
     */
    void setShowGrid( bool _showGrid ) { m_bShowGrid=_showGrid; }

    /**
     * Sets, if formula shall be shown instead of the result
     */
    bool getShowFormula() const {return m_bShowFormula;}

    void setShowFormula(bool _showFormula) {m_bShowFormula=_showFormula;}

    /**
     * Sets, if indicator must be shown when the cell holds a formula
     */
    bool getShowFormulaIndicator() const {return m_bShowFormulaIndicator;}

    void setShowFormulaIndicator(bool _showFormulaIndicator)
        {m_bShowFormulaIndicator=_showFormulaIndicator;}

    bool getLcMode() const {return m_bLcMode;}

    void setLcMode(bool _lcMode) {m_bLcMode=_lcMode;}

    bool getAutoCalc() const {return m_bAutoCalc;}

    void setAutoCalc(bool _AutoCalc) {m_bAutoCalc=_AutoCalc;}

    bool getShowColumnNumber() const {return m_bShowColumnNumber;}

    void setShowColumnNumber(bool _showColumnNumber) {m_bShowColumnNumber=_showColumnNumber;}

    bool getHideZero() const {return m_bHideZero;}

    void setHideZero(bool _hideZero) {m_bHideZero=_hideZero;}

    bool getFirstLetterUpper() const {return m_bFirstLetterUpper;}

    void setFirstLetterUpper(bool _firstUpper) {m_bFirstLetterUpper=_firstUpper;}

    void mergeCells( const QRect &area, bool makeUndo=true );
    void dissociateCell( const QPoint &cellRef, bool makeUndo=true );
    void changeMergedCell( int m_iCol, int m_iRow, int m_iExtraX, int m_iExtraY);

    void increaseIndent( KSpreadSelection* selectionInfo );
    void decreaseIndent( KSpreadSelection* selectionInfo );

    bool areaIsEmpty(const QRect &area, TestType _type = Text) ;

    void refreshPreference() ;

    void hideTable(bool _hide);

    void removeTable();

    QRect selectionCellMerged(const QRect &_sel);
    /**
     * Change name of reference when the user inserts or removes a column,
     * a row or a cell (= insertion of a row [or column] on a single column [or row]).
     * For example the formula =Table1!A1 is changed into =Table1!B1 if a Column
     * is inserted before A.
     *
     * @param pos the point of insertion (only one coordinate may be used, depending
     * on the other paramaters).
     * @param fullRowOrColumn if true, a whole row or column has been inserted/removed.
     *                        if false, we inserted or removed a cell
     * @param ref see ChangeRef
     * @param tabname completes the pos specification by giving the table name
     */
    void changeNameCellRef(const QPoint & pos, bool fullRowOrColumn, ChangeRef ref,QString tabname,int NbCol=1);


    void refreshRemoveAreaName(const QString &_areaName);
    void refreshChangeAreaName(const QString &_areaName);


    /**
     * Update chart when you insert or remove row or column
     *
     * @param pos the point of insertion (only one coordinate may be used, depending
     * on the other paramaters).
     * @param fullRowOrColumn if true, a whole row or column has been inserted/removed.
     *                        if false, we inserted or removed a cell
     * @param ref see ChangeRef
     * @param tabname completes the pos specification by giving the table name
     */
    void refreshChart(const QPoint & pos, bool fullRowOrColumn, ChangeRef ref);
    /**
     * Refresh merged cell when you insert or remove row or column
     */
    void refreshMergedCell();

    /**
     * @return true if this table is hidden
     */
    bool isHidden()const { return m_bTableHide; }
    /**
     * Hides or shows this tables
     */
    void setHidden( bool hidden ) { m_bTableHide=hidden; }

    /**
     * For internal use only.
     */
    void setMap( KSpreadMap* _map ) { m_pMap = _map; }

    KSpreadDoc* doc()const { return m_pDoc; }
    KSpreadMap* map()const { return m_pMap; }

    /**
     * @return a painter for the hidden widget ( @ref #widget ).
     *
     * This function is useful while making formats where you
     * need some QPainter related functions.
     */
    QPainter& painter() { return *m_pPainter; }
    /**
     * @return a hidden widget.
     *
     * @see #painter
     */
    QWidget* widget()const { return m_pWidget; }

    /**
     * @return a flag that indicates whether the table should paint the page breaks.
     *
     * @see #setShowPageBorders
     * @see #bShowPageBorders
     */
    bool isShowPageBorders()const { return m_bShowPageBorders; }

    /**
     * Turns the page break lines on or off.
     *
     * @see #isShowPageBorders
     * @see #bShowPageBorders
     */
    void setShowPageBorders( bool _b );

    /**
     * Tests whether _column is the first column of a new page. In this
     * case the left border of this column may be drawn highlighted to show
     * that this is a page break.
     */
    bool isOnNewPageX( int _column );

    /**
     * Tests whether _row is the first row of a new page. In this
     * case the top border of this row may be drawn highlighted to show
     * that this is a page break.
     */
    bool isOnNewPageY( int _row );

    void addCellBinding( CellBinding *_bind );
    void removeCellBinding( CellBinding *_bind );
    CellBinding* firstCellBinding() { return m_lstCellBindings.first(); }
    CellBinding* nextCellBinding() { return m_lstCellBindings.next(); }

    /**
     * Used by the 'chart' to get the table on which the chart is build.
     * The cells we are interested in are in the rectangle '_range'.
     * The cells are stored row after row in '_list'.
     */
    bool getCellRectangle( const QRect &_range, QPtrList<KSpreadCell> &_list );

    /**
     * A convenience function that finds a table by its name.
     */
    KSpreadSheet *findTable( const QString & _name );

    /**
     * Inserts the @p _cell into the table.
     * All cells depending on this cell will be actualized.
     * The border range will be actualized, when the cell is out of current range.
     */
    void insertCell( KSpreadCell *_cell );
    /**
     * Used by Undo.
     *
     * @see KSpreadUndoDeleteColumn
     */
    void insertColumnFormat( ColumnFormat *_l );
    /**
     * Used by Undo.
     *
     * @see KSpreadUndoDeleteRow
     */
    void insertRowFormat( RowFormat *_l );

    /**
     * @see #copy
     */
    QDomDocument saveCellRect( const QRect& );

    /**
    * insertTo defined if you insert to the bottom or right
    * insert to bottom if insertTo==1
    * insert to right if insertTo ==-1
    * insertTo used just for insert/paste an area
     * @see #paste
     */
    bool loadSelection( const QDomDocument& doc, const QRect &pasteArea, int _xshift, int _yshift,bool makeUndo,PasteMode = Normal, Operation = OverWrite,bool insert=false,int insertTo=0 );

    void loadSelectionUndo( const QDomDocument & doc, const QRect &loadArea,
                            int _xshift, int _yshift,bool insert,int insertTo);

    /**
    *  Used when you insert and paste cell
    * return true if it's a area
    * false if it's a column/row
    * it's used to select if you want to insert at the bottom or right
    * @see #paste
     */
    bool testAreaPasteInsert()const;

    /**
     * Deletes all cells in the given rectangle.
     * The display is NOT updated by this function.
     * This function can be used to clear an area before you paste something from the clipboard
     * in this area.
     *
     * @see #loadCells
     */
    void deleteCells( const QRect& rect );


    /**
     * Return TRUE if there are text value in cell
     * so you can create list selection
     */
    bool testListChoose(KSpreadSelection* selectionInfo);

    /**
     * returns the text to be copied to the clipboard
     */
    QString copyAsText(KSpreadSelection* selection);

    /**
     * Assume that the retangle 'src' was already selected. Then the user clicked on the
     * lower right corner of the marker and resized the area ( left mouse button ).
     * Once he releases the mouse we have to autofill the region 'dest'. Mention that
     * src.left() == dest.left() and src.top() == dest.top().
     *
     * @see #mouseReleaseEvent
     */
    void autofill( QRect &src, QRect &dest );

    void print( QPainter &painter, KPrinter *_printer );

    /**
     * Deletes a child object. That will cause all views to update
     * accordingly. Do not use this child object afterwards.
     *
     * @ref #insertChild
     */
    void deleteChild( KSpreadChild *_child );
    /**
     * @ref #deleteChild
     */
    void insertChild( const QRect& _geometry, KoDocumentEntry& );
    /**
     * A convenience function around @ref #insertChild.
     */
    void insertChart( const QRect& _geometry, KoDocumentEntry&, const QRect& _data );
    void changeChildGeometry( KSpreadChild *_child, const QRect& _geometry );

    const QColorGroup& colorGroup() { return m_pWidget->colorGroup(); }

    int id() const { return m_id; }

    /**
     * Return the currently maximum defined column of the horizontal scrollbar.
     * It's always 10 times higher than the maximum access column.
     * In an empty table it starts with 256.
     */
    int maxColumn()const { return m_iMaxColumn; }

    /**
     * Checks if the argument _column is out of the current maximum range of the vertical border
     * If this is the case, the current maximum value m_iMaxColumn is adjusted and the vertical border
     * is resized.
     * Use this function with care, as it involves a repaint of the border, when it is out of range.
     */
    void checkRangeHBorder ( int _column );

    /**
     * Return the currently maximum defined row of the vertical scrollbar.
     * It's always 10 times higher than the maximum access row.
     * In an empty table it starts with 256.
     */
    int maxRow()const { return m_iMaxRow; }

    /**
     * Checks if the argument _row is out of the current maximum range of the horizontal border
     * If this is the case, the current maximum value m_iMaxRow is adjusted and the horizontal border
     * is resized.
     * Use this function with care, as it involves a repaint of the border, when it is out of range.
     */
    void checkRangeVBorder ( int _row );


    void enableScrollBarUpdates( bool _enable );

    virtual DCOPObject* dcopObject();

    static KSpreadSheet* find( int _id );

#ifndef NDEBUG
    void printDebug();
#endif

    /**
     * Calculates the cell if necessary, makes its layout if necessary,
     * and force redraw.
     * Then it sets the cell's @ref KSpreadCell::m_bDisplayDirtyFlag to false.
     */
    void updateCell( KSpreadCell* _cell, int _col, int _row );

    /**
     * Like updateCell except it works on a range of cells.  Use this function
     * rather than calling updateCell(..) on several adjacent cells so there
     * will be one paint event instead of several
     */
    void updateCellArea(const QRect &cellArea);

  /**
     * Updates every cell on the table
     */
    void update();

    /**
     * used to refresh cells when you make redodelete
     */
    void refreshView(const QRect& rect);

    void emit_updateRow( RowFormat *_format, int _row );
    void emit_updateColumn( ColumnFormat *_format, int _column );

    /**
     * Needed for @ref KSpreadCell::leftBorderPen and friends, since we can not
     * have a static pen object.
     *
     * The returned pen has pen style NoPen set.
     */
    const QPen& emptyPen() const { return m_emptyPen; }
    const QBrush& emptyBrush() const { return m_emptyBrush; }
    const QColor& emptyColor() const { return m_emptyColor; }

    void updateLocale();


  /**
   * set a region of the spreadsheet to be 'paint dirty' meaning it
   * needs repainted.  This is not a flag on the cell itself since quite
   * often this needs set on a default cell
   */
  void setRegionPaintDirty(QRect region);

  /**
   * Remove all records of 'paint dirty' cells
   */
  void clearPaintDirtyData();

  /**
   * Test whether a cell needs repainted
   */
  bool cellIsPaintDirty(QPoint cell);

  /**
   * Retrieve the first used cell in a given column.  Can be used in conjunction
   * with getNextCellDown to loop through a column.
   *
   * @param col The column to get the first cell from
   *
   * @return Returns a pointer to the cell, or NULL if there are no used cells
   *         in this column
   */
  KSpreadCell* getFirstCellColumn(int col) const;

  /**
   * Retrieve the last used cell in a given column.  Can be used in conjunction
   * with getNextCellUp to loop through a column.
   *
   * @param col The column to get the cell from
   *
   * @return Returns a pointer to the cell, or NULL if there are no used cells
   *         in this column
   */
  KSpreadCell* getLastCellColumn(int col) const;

  /**
   * Retrieve the first used cell in a given row.  Can be used in conjunction
   * with getNextCellRight to loop through a row.
   *
   * @param row The row to get the first cell from
   *
   * @return Returns a pointer to the cell, or NULL if there are no used cells
   *         in this row
   */
  KSpreadCell* getFirstCellRow(int row) const;

  /**
   * Retrieve the last used cell in a given row.  Can be used in conjunction
   * with getNextCellLeft to loop through a row.
   *
   * @param row The row to get the last cell from
   *
   * @return Returns a pointer to the cell, or NULL if there are no used cells
   *         in this row
   */
  KSpreadCell* getLastCellRow(int row) const;

  /**
   * Retrieves the next used cell above the given col/row pair.  The given
   * col/row pair does not need to reference a used cell.
   *
   * @param col column to start looking through
   * @param row the row above which to start looking.
   *
   * @return Returns the next used cell above this one, or NULL if there are none
   */
  KSpreadCell* getNextCellUp(int col, int row) const;

  /**
   * Retrieves the next used cell below the given col/row pair.  The given
   * col/row pair does not need to reference a used cell.
   *
   * @param col column to start looking through
   * @param row the row below which to start looking.
   *
   * @return Returns the next used cell below this one, or NULL if there are none
   */
  KSpreadCell* getNextCellDown(int col, int row) const;

  /**
   * Retrieves the next used cell to the right of the given col/row pair.
   * The given col/row pair does not need to reference a used cell.
   *
   * @param col the column after which should be searched
   * @param row the row to search through
   *
   * @return Returns the next used cell to the right of this one, or NULL if
   * there are none
   */
  KSpreadCell* getNextCellLeft(int col, int row) const;

  /**
   * Retrieves the next used cell to the left of the given col/row pair.
   * The given col/row pair does not need to reference a used cell.
   *
   * @param col the column before which should be searched
   * @param row the row to search through
   *
   * @return Returns the next used cell to the left of this one, or NULL if
   * there are none
   */
  KSpreadCell* getNextCellRight(int col, int row) const;

signals:
    void sig_updateView( KSpreadSheet *_table );
    void sig_updateView( KSpreadSheet *_table, const QRect& );
    void sig_updateHBorder( KSpreadSheet *_table );
    void sig_updateVBorder( KSpreadSheet *_table );
    void sig_updateChildGeometry( KSpreadChild *_child );
    void sig_removeChild( KSpreadChild *_child );
    void sig_maxColumn( int _max_column );
    void sig_maxRow( int _max_row );
    /**
     * @see #setTableName
     */
    void sig_nameChanged( KSpreadSheet* table, const QString& old_name );
    /**
     * Emitted if a certain area of some table has to be redrawn.
     * That is for example the case when a new child is inserted.
     */
    void sig_polygonInvalidated( const QPointArray& );

    void sig_TableHidden( KSpreadSheet* table);
    void sig_TableShown( KSpreadSheet* table);
    void sig_TableRemoved( KSpreadSheet* table);
    void sig_TableActivated( KSpreadSheet* );
    void sig_RefreshView( KSpreadSheet* );

protected:
    /**
     * Change the name of a table in all formulas.
     * When you change name table Table1 -> Price
     * for all cell which refere to Table1, this function changes the name.
     */
    void changeCellTabName(QString old_name,QString new_name);

    void insertChild( KSpreadChild *_child );

    /**
     * Prints the page specified by 'page_range'.
     *
     * @return the last vertical line which was printed plus one.
     *
     * @param _page_range QRect defines a rectangle of cells which should be painted
     *                    to the device 'prn'.
     * @view KoRect defines the sourrounding rectangle which is the printing frame.
     *
     * @param _childOffset KoPoint used to calculate the correct position of children,
     *                    if there are repeated columns/rows
     */
    void printPage( QPainter &_painter, const QRect& page_range, const KoRect& view, const KoPoint _childOffset );

    /**
     * @see #autofill
     */
    void fillSequence( QPtrList<KSpreadCell>& _srcList, QPtrList<KSpreadCell>& _destList, QPtrList<AutoFillSequence>& _seqList, bool down = true );

    KSpreadCluster m_cells;
    KSpreadRowCluster m_rows;
    KSpreadColumnCluster m_columns;

    KSpreadCell* m_pDefaultCell;
    RowFormat* m_pDefaultRowFormat;
    ColumnFormat* m_pDefaultColumnFormat;

    /**
     * The name of the table. This name shows in the tab bar on the bottom of the window.
     */
    QString m_strName;

    /**
     * Indicates whether the table should paint the page breaks.
     * Doing so costs some time, so by default it should be turned off.
     */
    bool m_bShowPageBorders;

    /**
     * List of all cell bindings. For example charts use bindings to get
     * informed about changing cell contents.
     *
     * @see #addCellBinding
     * @see #removeCellBinding
     */
    QPtrList<CellBinding> m_lstCellBindings;

    /**
     * The label returned by @ref #columnLabel
     */
    char m_arrColumnLabel[20];

    /**
     * The map to which this table belongs.
     */
    KSpreadMap *m_pMap;
    KSpreadDoc *m_pDoc;

    /**
     * Password for protecting this sheet
     */
    QCString m_strPassword;

    /**
     * Needed to get infos about font metrics.
     */
    QPainter *m_pPainter;
    /**
     * Used for @ref #m_pPainter
     */
    QWidget *m_pWidget;

    /**
     * List of all embedded objects.
     */
    // QPtrList<KSpreadChild> m_lstChildren;

    int m_id;

    /**
     * The highest row ever accessed by the user.
     */
    int m_iMaxRow;
    /**
     * The highest column ever accessed by the user.
     */
    int m_iMaxColumn;

    /**
     * Max range of canvas in x direction.
     * Depends on KS_colMax and the width of all columns
     */
    double m_dSizeMaxX;

    /**
     * Max range of canvas in y direction.
     * Depends on KS_rowMax and the heigth of all rows
     */
    double m_dSizeMaxY;


    bool m_bScrollbarUpdates;

    DCOPObject* m_dcop;
    bool m_bTableHide;

    static int s_id;
    static QIntDict<KSpreadSheet>* s_mapTables;

    /**
     * Show the grid on the screen
     */
    bool m_bShowGrid;
    bool m_bShowFormula;
    bool m_bShowFormulaIndicator;
    bool m_bAutoCalc;
    bool m_bLcMode;
    bool m_bShowColumnNumber;
    bool m_bHideZero;
    bool m_bFirstLetterUpper;

    KSpreadFormat* m_defaultFormat;

    /**
     * @see #emptyPen
     */
    QPen m_emptyPen;
    QBrush m_emptyBrush;
    QColor m_emptyColor;

    int m_iScrollPosX;
    int m_iScrollPosY;

    /**********************/
    /* Printout functions */
    /**********************/

public:

    /**
     * @return the printable width of the paper in millimeters.
     */
    float printableWidth()const { return m_paperWidth - m_leftBorder - m_rightBorder; }

    /**
     * @return the printable height of the paper in millimeters.
     */
    float printableHeight()const { return m_paperHeight - m_topBorder - m_bottomBorder; }

    float paperHeight()const { return m_paperHeight; }
    float paperWidth()const { return m_paperWidth; }

    void setPaperHeight(float _val) { m_paperHeight=_val; }
    void setPaperWidth(float _val) { m_paperWidth=_val; }

    /**
     * @return the left border in millimeters
     */
    float leftBorder()const { return m_leftBorder; }
    /**
     * @return the right border in millimeters
     */
    float rightBorder()const { return m_rightBorder; }
    /**
     * @return the top border in millimeters
     */
    float topBorder()const { return m_topBorder; }
    /**
     * @return the bottom border in millimeters
     */
    float bottomBorder()const { return m_bottomBorder; }

    /**
     * @return the orientation of the paper.
     */
    KoOrientation orientation()const { return m_orientation; }
    /**
     * @return the ascii name of the paper orientation ( like Portrait, Landscape )
     */
    const char* orientationString() const;

    /**
     * @return the paper format.
     */
    KoFormat paperFormat()const { return m_paperFormat; }
    /**
     * @return the ascii name of the paper format ( like A4, Letter etc. )
     */
    QString paperFormatString()const;

    void setPaperFormat(KoFormat _format) {m_paperFormat=_format;}

    void setPaperOrientation(KoOrientation _orient);

    /**
     * Returns the page layout
     */
    KoPageLayout getPaperLayout() const;

     /**
     * Changes the paper layout and repaints the currently displayed KSpreadSheet.
     */
    void setPaperLayout( float _leftBorder, float _topBorder, float _rightBorder, float _bottomBoder,
                         KoFormat _paper, KoOrientation orientation );
    /**
     * A convenience function using a QString as paper format and orientation.
     */
    void setPaperLayout( float _leftBorder, float _topBorder, float _rightBorder, float _bottomBoder,
                         const QString& _paper, const QString& _orientation );

    QString headLeft( int _p, const QString &_t  )const { if ( m_headLeft.isNull() ) return "";
    return completeHeading( m_headLeft, _p, _t ); }
    QString headMid( int _p, const QString &_t )const { if ( m_headMid.isNull() ) return "";
    return completeHeading( m_headMid, _p, _t ); }
    QString headRight( int _p, const QString &_t )const { if ( m_headRight.isNull() ) return "";
    return completeHeading( m_headRight, _p, _t ); }
    QString footLeft( int _p, const QString &_t )const { if ( m_footLeft.isNull() ) return "";
    return completeHeading( m_footLeft, _p, _t ); }
    QString footMid( int _p, const QString &_t )const { if ( m_footMid.isNull() ) return "";
    return completeHeading( m_footMid, _p, _t ); }
    QString footRight( int _p, const QString &_t )const { if ( m_footRight.isNull() ) return "";
    return completeHeading( m_footRight, _p, _t ); }

    QString headLeft()const { if ( m_headLeft.isNull() ) return ""; return m_headLeft; }
    QString headMid()const { if ( m_headMid.isNull() ) return ""; return m_headMid; }
    QString headRight()const { if ( m_headRight.isNull() ) return ""; return m_headRight; }
    QString footLeft()const { if ( m_footLeft.isNull() ) return ""; return m_footLeft; }
    QString footMid()const { if ( m_footMid.isNull() ) return ""; return m_footMid; }
    QString footRight()const { if ( m_footRight.isNull() ) return ""; return m_footRight; }

    /**
     * Returns the print range.
     * Returns ( QPoint (1, 1), QPoint(KS_colMax, KS_rowMax) ) if nothing is defined
     */
    QRect printRange() const { return m_printRange; }
    /**
     * Sets the print range.
     * Set it to ( QPoint (1, 1), QPoint(KS_colMax, KS_rowMax) ) to undefine it
     */
    void setPrintRange( QRect _printRange );

    /**
     * Returns the columns, which are printed on each page.
     * Returns QPair (0, 0) if nothing is defined
     */
    QPair<int, int> printRepeatColumns() const { return m_printRepeatColumns; }
    /**
     * Sets the columns to be printed on each page.
     * Only the x-values of the points are used
     * Set it to QPair (0, 0) to undefine it
     */
    void setPrintRepeatColumns( QPair<int, int> _printRepeatColumns );

    /**
     * Returns the rows, which are printed on each page.
     * Returns QPair (0, 0) if nothing is defined
     */
    QPair<int, int> printRepeatRows() const { return m_printRepeatRows; }
    /**
     * Sets the rows to be printed on each page.
     * Only the y-values of the points are used
     * Set it to QPair (0, 0) to undefine it
     */
    void setPrintRepeatRows( QPair<int, int> _printRepeatRows );

    /**
     * Updates the new page list for columns starting at column @arg _col
     */
    void updateNewPageListX( int _col );

    /**
     * Updates the new page list for rows starting at row @arg _row
     */
    void updateNewPageListY( int _row );

    /**
     * Replaces in _text all _search text parts by _replace text parts.
     * Included is a test to not change if _search == _replace.
     * The arguments should not include neither the beginning "<" nor the leading ">", this is already
     * included internally.
     */
    void replaceHeadFootLineMacro ( QString &_text, const QString &_search, const QString &_replace );
    /**
     * Replaces in _text all page macros by the i18n-version of the macros
     */
    QString localizeHeadFootLine ( const QString &_text );
    /**
     * Replaces in _text all i18n-versions of the page macros by the internal version of the macros
     */
    QString delocalizeHeadFootLine ( const QString &_text );

    /**
     * Returns the head and foot line of the print out
     */
    KoHeadFoot getHeadFootLine() const;

    /**
     * Sets the head and foot line of the print out
     */
    void setHeadFootLine( const QString &_headl, const QString &_headm, const QString &_headr,
                          const QString &_footl, const QString &_footm, const QString &_footr );

    /**
     * Returns, if the grid shall be shown on printouts
     */
    bool getPrintGrid() const { return m_bPrintGrid; }

    /**
     * Sets, if the grid shall be shown on printouts
     */
    void setPrintGrid( bool _printGrid );

    /**
     * Returns, if the comment rect shall be shown on printouts
     */
    bool getPrintCommentIndicator() const { return m_bPrintCommentIndicator; }

    /**
     * Sets, if the comment rect shall be shown on printouts
     */
    void setPrintCommentIndicator( bool _printCommentIndicator );

    /**
     * Returns, if the formula rect shall be shown on printouts
     */
    bool getPrintFormulaIndicator() const { return m_bPrintFormulaIndicator; }

    /**
     * Sets, if the formula Rect shall be shown on printouts
     */
    void setPrintFormulaIndicator( bool _printFormulaIndicator );

    /**
     * Updates m_dPrintRepeatColumnsWidth according to the new settings
     */
    void updatePrintRepeatColumnsWidth();

    /**
     * Updates m_dPrintRepeatColumnsWidth according to the new settings
     */
    void updatePrintRepeatRowsHeight();

    /**
     * Define the print range with the current selection
     */
    void definePrintRange(KSpreadSelection* selectionInfo);
    /**
     * Reset the print range to the standard definition (whole sheet)
     */
    void resetPrintRange();

protected:

    /**
     * Looks at @ref #m_paperFormat and calculates @ref #m_paperWidth and @ref #m_paperHeight.
     */
    void calcPaperSize();

    /**
     * Replaces macros like <name>, <file>, <date> etc. in the string and
     * returns the modified one.
     *
     * @param _page is the page number for which the heading is produced.
     * @param _KSpreadSheet is the name of the KSpreadSheet for which we generate the headings.
     */
    QString completeHeading( const QString &_data, int _page, const QString &_table ) const ;

    /**
     * The orientation of the paper.
     */
    KoOrientation m_orientation;
    /**
     * Tells about the currently seleced paper size.
     */
    KoFormat m_paperFormat;
    /**
     * The paper width in millimeters. Dont change this value, it is calculated by
     * @ref #calcPaperSize from the value @ref #m_paperFormat.
     */
    float m_paperWidth;
    /**
     * The paper height in millimeters. Dont change this value, it is calculated by
     * @ref #calcPaperSize from the value @ref #m_paperFormat.
     */
    float m_paperHeight;
    /**
     * The left border in millimeters.
     */
    float m_leftBorder;
    /**
     * The right border in millimeters.
     */
    float m_rightBorder;
    /**
     * The top border in millimeters.
     */
    float m_topBorder;
    /**
     * The right border in millimeters.
     */
    float m_bottomBorder;

    /**
     * Header string. The string may contains makros. That means
     * it has to be processed before printing.
     */
    QString m_headLeft;
    /**
     * Header string. The string may contains makros. That means
     * it has to be processed before printing.
     */
    QString m_headRight;
    /**
     * Header string. The string may contains makros. That means
     * it has to be processed before printing.
     */
    QString m_headMid;
    /**
     * Footer string. The string may contains makros. That means
     * it has to be processed before printing.
     */
    QString m_footLeft;
    /**
     * Footer string. The string may contains makros. That means
     * it has to be processed before printing.
     */
    QString m_footRight;
    /**
     * Footer string. The string may contains makros. That means
     * it has to be processed before printing.
     */
    QString m_footMid;

    /**
     * Number of total pages, only calculated during printing
     */
    uint m_uprintPages;

    /**
     * Defined printable area
     */
    QRect m_printRange;

    /**
     * Repeated columns on printout
     */
    QPair<int, int> m_printRepeatColumns;

    /**
     * Repeated rows on printout
     */
    QPair<int, int> m_printRepeatRows;

    /**
     * Show the grid when making printout
     */
    bool m_bPrintGrid;

    /**
     * Show the formula rect when making printout
     */
    bool m_bPrintFormulaIndicator;

    /**
     * Show the comment rect when making printout
     */
    bool m_bPrintCommentIndicator;

    /**
     * Width of repeated columns in mm, stored for perfomance reasons
     */
    double m_dPrintRepeatColumnsWidth;
    /**
     * Height of repeated rows in mm, stored for perfomance reasons
     */
    double m_dPrintRepeatRowsHeight;

    /**
     * Stores the new page columns
     */
     QValueList<int> m_lnewPageListX;

    /**
     * Stores the new page columns
     */
     QValueList<int> m_lnewPageListY;

  /**
   * Stores cells that need repainting
   */
  QValueList<QRect> m_paintDirtyList;

public:
    // see kspread_sheet.cc for an explanation of this
    // this is for type B and also for type A (better use CellWorkerTypeA for that)
    struct CellWorker {
	const bool create_if_default;
	const bool emit_signal;
	const bool type_B;

	CellWorker( bool cid=true, bool es=true, bool tb=true ) : create_if_default( cid ), emit_signal( es ), type_B( tb ) { }
	virtual ~CellWorker() { }

	virtual class KSpreadUndoAction* createUndoAction( KSpreadDoc* doc, KSpreadSheet* table, QRect& r ) =0;

	// these are only needed for type A
	virtual bool testCondition( RowFormat* ) { return false; }
	virtual void doWork( RowFormat* ) { }
	virtual void doWork( ColumnFormat* ) { }
	virtual void prepareCell( KSpreadCell* ) { }

	// these are needed in all CellWorkers
	virtual bool testCondition( KSpreadCell* cell ) =0;
	virtual void doWork( KSpreadCell* cell, bool cellRegion, int x, int y ) =0;
    };

    // this is for type A (surprise :))
    struct CellWorkerTypeA : public CellWorker {
	CellWorkerTypeA( ) : CellWorker( true, true, false ) { }
	virtual QString getUndoTitle( ) =0;
	class KSpreadUndoAction* createUndoAction( KSpreadDoc* doc, KSpreadSheet* table, QRect& r );
    };

protected:
    typedef enum { CompleteRows, CompleteColumns, CellRegion } SelectionType;
    SelectionType workOnCells( KSpreadSelection* selectionInfo,
                               CellWorker& worker );

private:
    bool FillSequenceWithInterval (QPtrList<KSpreadCell>& _srcList,
				   QPtrList<KSpreadCell>& _destList,
				   QPtrList<AutoFillSequence>& _seqList,
                                   bool down);

    void FillSequenceWithCopy (QPtrList<KSpreadCell>& _srcList,
			       QPtrList<KSpreadCell>& _destList,
                               bool down);

    void convertObscuringBorders();
    void checkCellContent(KSpreadCell * cell1, KSpreadCell * cell2, int & ret);
    int adjustColumnHelper( KSpreadCell * c, int _col, int _row );

};

typedef KSpreadSheet KSpreadTable;

#endif
