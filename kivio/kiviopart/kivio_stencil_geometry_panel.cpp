#include "kivio_stencil_geometry_panel.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qpixmap.h>

#include <koUnitWidgets.h>

static const char* width_xpm[] = {
  "13 11 3 1",
  "  c Gray0",
  ". c #808080",
  "X c None",
  "XXXXXXXXXXXXX",
  "XXXXXXXXXXXXX",
  ".XXXXXXXXXXX.",
  ".XX XXXXX XX.",
  ".X  XXXXX  X.",
  ".           .",
  ".X  XXXXX  X.",
  ".XX XXXXX XX.",
  ".XXXXXXXXXXX.",
  "XXXXXXXXXXXXX",
  "XXXXXXXXXXXXX"
};

static const char* height_xpm[] = {
  "13 11 3 1",
  "  c Gray0",
  ". c #808080",
  "X c None",
  "XXX.......XXX",
  "XXXXXX XXXXXX",
  "XXXXX   XXXXX",
  "XXXX     XXXX",
  "XXXXXX XXXXXX",
  "XXXXXX XXXXXX",
  "XXXXXX XXXXXX",
  "XXXX     XXXX",
  "XXXXX   XXXXX",
  "XXXXXX XXXXXX",
  "XXX.......XXX"
};

static const char* xpos_xpm[] = {
  "13 11 2 1",
  "  c Gray0",
  ". c None",
  ".. .. .......",
  ".. .. .......",
  "...  ........",
  "...  ........",
  ".. .. .......",
  ".. .. .     .",
  "....... ... .",
  "....... ... .",
  "....... ... .",
  "....... ... .",
  ".......     ."
};

static const char* ypos_xpm[] = {
  "13 11 2 1",
  "  c Gray0",
  ". c None",
  "... . .......",
  "... . .......",
  "... . .......",
  ".... ........",
  ".... ........",
  "..  ...     .",
  "....... ... .",
  "....... ... .",
  "....... ... .",
  "....... ... .",
  ".......     ."
};


KivioStencilGeometryPanel::KivioStencilGeometryPanel(QWidget* parent)
: QWidget(parent,"KivioStencilGeometryPanel")
{
    QGridLayout* grid = new QGridLayout( this, 4, 2, 3, 3 );

    QLabel *lx = new QLabel(this);
    QLabel *ly = new QLabel(this);
    QLabel *lw = new QLabel(this);
    QLabel *lh = new QLabel(this);

    lx->setPixmap( QPixmap((const char **)xpos_xpm) );
    ly->setPixmap( QPixmap((const char **)ypos_xpm) );
    lw->setPixmap( QPixmap((const char **)width_xpm) );
    lh->setPixmap( QPixmap((const char **)height_xpm) );

    m_pX = new KoUnitDoubleSpinBox(this, -1000.0, 1000.0, 0.5, 0.0);
    m_pY = new KoUnitDoubleSpinBox(this, -1000.0, 1000.0, 0.5, 0.0);
    m_pW = new KoUnitDoubleSpinBox(this, -1000.0, 1000.0, 0.5, 0.0);
    m_pH = new KoUnitDoubleSpinBox(this, -1000.0, 1000.0, 0.5, 0.0);

    QObject::connect( m_pX, SIGNAL(valueChanged(double)), this, SLOT(xChange(double)) );
    QObject::connect( m_pY, SIGNAL(valueChanged(double)), this, SLOT(yChange(double)) );
    QObject::connect( m_pW, SIGNAL(valueChanged(double)), this, SLOT(wChange(double)) );
    QObject::connect( m_pH, SIGNAL(valueChanged(double)), this, SLOT(hChange(double)) );

    grid->addWidget( lx, 0, 0 );
    grid->addWidget( m_pX, 0, 1 );

    grid->addWidget( ly, 1, 0 );
    grid->addWidget( m_pY, 1, 1 );

    grid->addWidget( lw, 2, 0 );
    grid->addWidget( m_pW, 2, 1 );

    grid->addWidget( lh, 3, 0 );
    grid->addWidget( m_pH, 3, 1 );

    m_unit = KoUnit::U_PT;
}

KivioStencilGeometryPanel::~KivioStencilGeometryPanel()
{
}

void KivioStencilGeometryPanel::setUnit( KoUnit::Unit m )
{
    m_pX->setUnit(m);
    m_pY->setUnit(m);
    m_pW->setUnit(m);
    m_pH->setUnit(m);
    m_unit = m;
}

void KivioStencilGeometryPanel::xChange( double d )
{
    emit positionChanged( KoUnit::ptFromUnit(d, m_unit), KoUnit::ptFromUnit(m_pY->value(), m_unit) );
}

void KivioStencilGeometryPanel::yChange( double d )
{
    emit positionChanged( KoUnit::ptFromUnit(m_pX->value(), m_unit), KoUnit::ptFromUnit(d, m_unit) );
}

void KivioStencilGeometryPanel::wChange( double d )
{
    emit sizeChanged( KoUnit::ptFromUnit(d, m_unit), KoUnit::ptFromUnit(m_pH->value(), m_unit) );
}

void KivioStencilGeometryPanel::hChange( double d )
{
    emit sizeChanged( KoUnit::ptFromUnit(m_pW->value(), m_unit), KoUnit::ptFromUnit(d, m_unit) );
}

void KivioStencilGeometryPanel::setPosition( double x, double y )
{
    m_pX->setValue(KoUnit::ptToUnit(x, m_unit));
    m_pY->setValue(KoUnit::ptToUnit(y, m_unit));
}

void KivioStencilGeometryPanel::setSize( double w, double h )
{
    m_pW->setValue(KoUnit::ptToUnit(w, m_unit));
    m_pH->setValue(KoUnit::ptToUnit(h, m_unit));
}
#include "kivio_stencil_geometry_panel.moc"
