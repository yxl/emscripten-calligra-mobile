/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *                1999 Michael Koch    <koch@kde.org>
 *                1999 Carsten Pfeiffer <pfeiffer@kde.org>
 *                2002 Patrick Julien <freak@codepimps.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

// Qt
#include <qapplication.h>
#include <qbutton.h>
#include <qclipboard.h>
#include <qcursor.h>
#include <qevent.h>
#include <qpainter.h>
#include <qscrollbar.h>
#include <qspinbox.h>

// KDE
#include <dcopobject.h>
#include <kaction.h>
#include <kcolordialog.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knotifyclient.h>
#include <kpushbutton.h>
#include <kruler.h>
#include <kstatusbar.h>
#include <kstdaction.h>

// KOffice
#include <koColor.h>
#include <koView.h>

// Local
#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_cursor.h"
#include "kis_doc.h"
#include "kis_canvas.h"
#include "kis_config.h"
#include "kis_channelview.h"
#include "kis_dlg_builder_progress.h"
#include "kis_dlg_dimension.h"
#include "kis_dlg_new_layer.h"
#include "kis_dlg_paintoffset.h"
#include "kis_image_magick_converter.h"
#include "kis_itemchooser.h"
#include "kis_factory.h"
#include "kis_painter.h"
#include "kis_layer.h"
#include "kis_listbox.h"
#include "kis_dlg_paint_properties.h"
#include "kis_resourceserver.h"
#include "kis_selection.h"
#include "kis_sidebar.h"
#include "kis_tabbar.h"
#include "kis_tool.h"
#include "kis_tool_factory.h"
#include "kis_tool_paste.h"
#include "kis_view.h"
#include "kis_util.h"
#include "kis_paint_device_visitor.h"
#include "labels/kis_label_builder_progress.h"
#include "labels/kis_label_io_progress.h"
#include "builder/kis_builder_subject.h"
#include "builder/kis_builder_monitor.h"
#include "strategy/kis_strategy_move.h"
#include "visitors/kis_flatten.h"
#include "visitors/kis_merge.h"

#define KISVIEW_MIN_ZOOM (1.0 / 16.0)
#define KISVIEW_MAX_ZOOM 16.0

KisView::KisView(KisDoc *doc, QWidget *parent, const char *name) : super(doc, parent, name)
{
	if (!doc -> isReadWrite())
		setXMLFile("krita_readonly.rc");
	else
		setXMLFile("krita.rc");

	m_doc = doc;
	m_canvas = 0;
	m_tabBar = 0;
	m_tabFirst = 0;
	m_tabLeft = 0;
	m_tabRight = 0;
	m_tabLast = 0;
	m_hRuler = 0;
	m_vRuler = 0;
	m_zoomIn = 0;
	m_zoomOut = 0;
	m_layerAdd = 0;
	m_layerRm = 0;
	m_layerDup = 0;
	m_layerLink = 0;
	m_layerHide = 0;
	m_layerProperties = 0;
	m_layerSaveAs = 0;
	m_layerResize = 0;
	m_layerResizeToImage = 0;
	m_layerScale = 0;
	m_layerRaise = 0;
	m_layerLower = 0;
	m_layerTop = 0;
	m_layerBottom = 0;
	m_selectionCut = 0;
	m_selectionCopy = 0;
	m_selectionPaste = 0;
	m_selectionCrop = 0;
	m_selectionFillBg = 0;
	m_selectionFillFg = 0;
	m_selectionRm = 0;
	m_selectionSelectAll = 0;
	m_selectionSelectNone = 0;
	m_imgRm = 0;
	m_imgResize = 0;
	m_imgDup = 0;
	m_imgImport = 0;
	m_imgExport = 0;
	m_imgScan = 0;
	m_imgMergeAll = 0;
	m_imgMergeVisible = 0;
	m_imgMergeLinked = 0;
	m_sidebarToggle = 0;
	m_floatsidebarToggle  = 0;
	m_lsidebarToggle = 0;
	m_dlgColorsToggle = 0;
       	m_dlgCrayonToggle = 0;
	m_dlgBrushToggle = 0;
	m_dlgPatternToggle = 0;
	m_dlgLayersToggle = 0;
	m_dlgChannelsToggle = 0;
	m_hScroll = 0;
	m_vScroll = 0;
	m_dcop = 0;
	m_xoff = 0;
	m_yoff = 0;
	m_fg = KoColor::black();
	m_bg = KoColor::white();
	m_sideBar = 0;
	m_brushChooser = 0;
	m_crayonChooser = 0;
	m_patternChooser = 0;
	m_paletteChooser = 0;
	m_gradientChooser = 0;
	m_imageChooser = 0;
	m_brush = 0;
	m_crayon = 0;
	m_pattern = 0;
	m_gradient = 0;
	m_layerBox = 0;
	m_imgBuilderMgr = new KisBuilderMonitor(this);
	m_buildProgress = 0;
	setInstance(KisFactory::global());
	setupActions();
	setupCanvas();
	setupRulers();
	setupScrollBars();
	setupSideBar();
	setupTabBar();
	setupStatusBar();
	dcopObject();
	connect(m_doc, SIGNAL(imageListUpdated()), SLOT(docImageListUpdate()));
	connect(m_doc, SIGNAL(layersUpdated(KisImageSP)), SLOT(layersUpdated(KisImageSP)));
	connect(m_doc, SIGNAL(projectionUpdated(KisImageSP)), SLOT(projectionUpdated(KisImageSP)));
	connect(this, SIGNAL(embeddImage(const QString&)), SLOT(slotEmbedImage(const QString&)));
	connect(m_imgBuilderMgr, SIGNAL(size(Q_INT32)), SLOT(nBuilders(Q_INT32)));
	setupTools();
	selectionUpdateGUI(false);
	setupClipboard();
	clipboardDataChanged();
}

KisView::~KisView()
{
	delete m_dcop;
}

DCOPObject* KisView::dcopObject()
{
#if 0
	if (!m_dcop)
		m_dcop = new KRayonViewIface(this);

	return m_dcop;
#endif
	return 0;
}

void KisView::setupSideBar()
{
	m_sideBar = new KisSideBar(this, "kis_sidebar");

	m_crayonChooser = new KisItemChooser(KisFactory::rServer() -> brushes(), false, m_sideBar -> dockFrame(), "crayon_chooser");
	m_crayonChooser -> addItem(KisFactory::rServer() -> patterns());
//	m_pKrayon = m_crayonChooser -> currentKrayon();
	QObject::connect(m_crayonChooser, SIGNAL(selected(KoIconItem*)), this, SLOT(setActiveCrayon(KoIconItem*)));
	m_crayonChooser -> setCaption(i18n("Crayons"));
	m_sideBar -> plug(m_crayonChooser);

	m_brushChooser = new KisItemChooser(KisFactory::rServer() -> brushes(), true, m_sideBar -> dockFrame(), "brush_chooser");
	m_brush = dynamic_cast<KisBrush*>(m_brushChooser -> currentItem());
	QObject::connect(m_brushChooser, SIGNAL(selected(KoIconItem*)), this, SLOT(setActiveBrush(KoIconItem*)));
	m_brushChooser -> setCaption(i18n("Brushes"));
	m_sideBar -> plug(m_brushChooser);

	m_patternChooser = new KisItemChooser(KisFactory::rServer() -> patterns(), true, m_sideBar -> dockFrame(), "pattern_chooser");
//	m_pPattern = m_patternChooser -> currentPattern();
	QObject::connect(m_patternChooser, SIGNAL(selected(KoIconItem*)), this, SLOT(setActivePattern(KoIconItem*)));
	m_patternChooser -> setCaption(i18n("Patterns"));
	m_sideBar -> plug(m_patternChooser);

	m_gradientChooser = new QWidget(this);
//	m_gradient = new KisGradient;
	m_gradientChooser -> setCaption(i18n("Gradients"));
	m_sideBar -> plug(m_gradientChooser);

	m_imageChooser = new QWidget(this);
	m_imageChooser -> setCaption(i18n("Images"));
	m_sideBar -> plug(m_imageChooser);

	m_paletteChooser = new QWidget(this);
	m_paletteChooser -> setCaption(i18n("Palettes"));
	m_sideBar -> plug(m_paletteChooser);

	m_layerBox = new KisListBox(i18n("layer"), KisListBox::SHOWALL, m_sideBar);
	m_layerBox -> setCaption(i18n("Layers"));

	connect(m_layerBox, SIGNAL(itemToggleVisible()), SLOT(layerToggleVisible()));
	connect(m_layerBox, SIGNAL(itemSelected(int)), SLOT(layerSelected(int)));
	connect(m_layerBox, SIGNAL(itemToggleLinked()), SLOT(layerToggleLinked()));
	connect(m_layerBox, SIGNAL(itemProperties()), SLOT(layerProperties()));
	connect(m_layerBox, SIGNAL(itemAdd()), SLOT(layerAdd()));
	connect(m_layerBox, SIGNAL(itemRemove()), SLOT(layerRemove()));
	connect(m_layerBox, SIGNAL(itemAddMask(int)), SLOT(layerAddMask(int)));
	connect(m_layerBox, SIGNAL(itemRmMask(int)), SLOT(layerRmMask(int)));
	connect(m_layerBox, SIGNAL(itemRaise()), SLOT(layerRaise()));
	connect(m_layerBox, SIGNAL(itemLower()), SLOT(layerLower()));
	connect(m_layerBox, SIGNAL(itemFront()), SLOT(layerFront()));
	connect(m_layerBox, SIGNAL(itemBack()), SLOT(layerBack()));
	connect(m_layerBox, SIGNAL(itemLevel(int)), SLOT(layerLevel(int)));

	m_sideBar -> plug(m_layerBox);
	layersUpdated();

	m_channelView = new KisChannelView(m_doc, this);
	m_channelView -> setCaption(i18n("Channels"));
	m_sideBar -> plug(m_channelView);

	m_sideBar -> slotActivateTab(i18n("Brushes"));

	if (m_brush)
		m_sideBar -> slotSetBrush(*m_brush);

	m_sideBar -> slotSetBGColor(m_bg);
	m_sideBar -> slotSetFGColor(m_fg);
	connect(m_sideBar, SIGNAL(fgColorChanged(const KoColor&)), this, SLOT(slotSetFGColor(const KoColor&)));
	connect(m_sideBar, SIGNAL(bgColorChanged(const KoColor&)), this, SLOT(slotSetBGColor(const KoColor&)));
	connect(this, SIGNAL(fgColorChanged(const KoColor&)), m_sideBar, SLOT(slotSetFGColor(const KoColor&)));
	connect(this, SIGNAL(bgColorChanged(const KoColor&)), m_sideBar, SLOT(slotSetBGColor(const KoColor&)));
	m_sidebarToggle -> setChecked(true);
}

void KisView::setupScrollBars()
{
	m_vScroll = new QScrollBar(QScrollBar::Vertical, this);
	m_hScroll = new QScrollBar(QScrollBar::Horizontal, this);
	m_vScroll -> setGeometry(width() - 16, 20, 16, height() - 36);
	m_hScroll -> setGeometry(20, height() - 16, width() - 36, 16);
	m_hScroll -> setValue(0);
	m_vScroll -> setValue(0);
	QObject::connect(m_vScroll, SIGNAL(valueChanged(int)), this, SLOT(scrollV(int)));
	QObject::connect(m_hScroll, SIGNAL(valueChanged(int)), this, SLOT(scrollH(int)));
}

void KisView::setupRulers()
{
	m_hRuler = new KRuler(Qt::Horizontal, this);
	m_vRuler = new KRuler(Qt::Vertical, this);
	m_hRuler -> setGeometry(20, 0, width() - 20, 20);
	m_vRuler -> setGeometry(0, 20, 20, height() - 20);
	m_vRuler -> setRulerMetricStyle(KRuler::Pixel);
	m_hRuler -> setRulerMetricStyle(KRuler::Pixel);
	m_vRuler -> setFrameStyle(QFrame::Panel | QFrame::Raised);
	m_hRuler -> setFrameStyle(QFrame::Panel | QFrame::Raised);
	m_hRuler -> setLineWidth(1);
	m_vRuler -> setLineWidth(1);
	m_hRuler -> setShowEndLabel(true);
	m_vRuler -> setShowEndLabel(true);
	m_hRuler -> setShowPointer(true);
	m_vRuler -> setShowPointer(true);
}

void KisView::setupTabBar()
{
	m_tabBar = new KisTabBar(this, m_doc);
	m_tabBar -> slotImageListUpdated();
	connect(m_tabBar, SIGNAL(tabSelected(const QString&)), SLOT(selectImage(const QString&)));
	QObject::connect(m_doc, SIGNAL(imageListUpdated()), m_tabBar, SLOT(slotImageListUpdated()));
	m_tabFirst = new KPushButton(this);
	m_tabLeft = new KPushButton(this);
	m_tabRight = new KPushButton(this);
	m_tabLast = new KPushButton(this);
	m_tabFirst -> setPixmap(QPixmap(BarIcon("tab_first")));
	m_tabLeft -> setPixmap(QPixmap(BarIcon("tab_left")));
	m_tabRight -> setPixmap(QPixmap(BarIcon("tab_right")));
	m_tabLast -> setPixmap(QPixmap(BarIcon("tab_last")));
	QObject::connect(m_tabFirst, SIGNAL(clicked()), m_tabBar, SLOT(slotScrollFirst()));
	QObject::connect(m_tabLeft, SIGNAL(clicked()), m_tabBar, SLOT(slotScrollLeft()));
	QObject::connect(m_tabRight, SIGNAL(clicked()), m_tabBar, SLOT(slotScrollRight()));
	QObject::connect(m_tabLast, SIGNAL(clicked()), m_tabBar, SLOT(slotScrollLast()));
}

void KisView::setupStatusBar()
{
	KoMainWindow *sh = shell();
	KStatusBar *sb = statusBar();

	Q_ASSERT(sh);
	Q_ASSERT(sb);

	if (sb) {
		KisLabelIOProgress *l;

		m_buildProgress = new KisLabelBuilderProgress(this);
		m_buildProgress -> setMaximumWidth(225);
		m_buildProgress -> setMaximumHeight(sb -> height());
		addStatusBarItem(m_buildProgress, 0);
		l = new KisLabelIOProgress(this);
		connect(m_doc, SIGNAL(ioProgress(Q_INT8)), l, SLOT(update(Q_INT8)));
		connect(m_doc, SIGNAL(ioSteps(Q_INT32)), l, SLOT(steps(Q_INT32)));
		connect(m_doc, SIGNAL(ioCompletedStep()), l, SLOT(completedStep()));
		connect(m_doc, SIGNAL(ioDone()), l, SLOT(done()));
		l -> setMaximumWidth(200);
		l -> setMaximumHeight(sb -> height());
		addStatusBarItem(l, 1);
		m_buildProgress -> hide();
		l -> hide();
	}
}

void KisView::setupActions()
{
	// navigation actions
	KStdAction::redisplay(this, SLOT(canvasRefresh()), actionCollection(), "refresh_canvas");
	(void)new KAction(i18n("Reset Button"), "stop", 0, this, SLOT(reset()), actionCollection(), "panic_button");

	// selection actions
	m_selectionCut = KStdAction::cut(this, SLOT(cut()), actionCollection(), "cut");
	m_selectionCopy = KStdAction::copy(this, SLOT(copy()), actionCollection(), "copy");
	m_selectionPaste = KStdAction::paste(this, SLOT(paste()), actionCollection(), "paste_special");
	m_selectionRm = new KAction(i18n("Remove Selection"), "remove", 0, this, SLOT(removeSelection()), actionCollection(), "remove");
	m_selectionCrop = new KAction(i18n("Copy Selection to New Layer"), "crop", 0,  this, SLOT(crop()), actionCollection(), "crop");
	m_selectionSelectAll = KStdAction::selectAll(this, SLOT(selectAll()), actionCollection(), "select_all");
	m_selectionSelectNone = KStdAction::deselect(this, SLOT(unSelectAll()), actionCollection(), "select_none");
	m_selectionFillFg = new KAction(i18n("Fill with Foreground Color"), 0, this, SLOT(fillSelectionFg()), actionCollection(), "fill_fgcolor");
	m_selectionFillBg = new KAction(i18n("Fill with Background Color"), 0, this, SLOT(fillSelectionBg()), actionCollection(), "fill_bgcolor");

	// import/export actions
	m_imgImport = new KAction(i18n("Import Image..."), "wizard", 0, this, SLOT(slotImportImage()), actionCollection(), "import_image");
	m_imgExport = new KAction(i18n("Export Image..."), "wizard", 0, this, SLOT(export_image()), actionCollection(), "export_image");
	m_imgScan = 0; // How the hell do I get a KAction to the scan plug-in?!?

	// view actions
	m_zoomIn = KStdAction::zoomIn(this, SLOT(zoomIn()), actionCollection(), "zoom_in");
	m_zoomOut = KStdAction::zoomOut(this, SLOT(zoomOut()), actionCollection(), "zoom_out");

	// tool settings actions
	(void)new KAction(i18n("&Gradient Dialog..."), "blend", 0, this, SLOT(dialog_gradient()), actionCollection(), "dialog_gradient");

	// tool actions
	(void)new KAction(i18n("&Current Tool Properties..."), "configure", 0, this, SLOT(tool_properties()), actionCollection(), "current_tool_properties");

	// layer actions
	m_layerAdd = new KAction(i18n("&Add Layer..."), 0, this, SLOT(layerAdd()), actionCollection(), "insert_layer");
	m_layerRm = new KAction(i18n("&Remove Layer"), 0, this, SLOT(layerRemove()), actionCollection(), "remove_layer");
	m_layerDup = new KAction(i18n("Duplicate Layer"), 0, this, SLOT(layerDuplicate()), actionCollection(), "duplicate_layer");
	m_layerLink = new KAction(i18n("&Link/Unlink Layer"), 0, this, SLOT(layerToggleLinked()), actionCollection(), "link_layer");
	m_layerHide = new KAction(i18n("&Hide/Show Layer"), 0, this, SLOT(layerToggleVisible()), actionCollection(), "hide_layer");
	m_layerRaise = new KAction(i18n("Raise Layer"), "raiselayer", 0, this, SLOT(layerRaise()), actionCollection(), "raiselayer");
	m_layerLower = new KAction(i18n("Lower Layer"), "lowerlayer", 0, this, SLOT(layerLower()), actionCollection(), "lowerlayer");
	m_layerTop = new KAction(i18n("Layer to Top"), 0, this, SLOT(layerFront()), actionCollection(), "toplayer");
	m_layerBottom = new KAction(i18n("Layer to Bottom"), 0, this, SLOT(layerBack()), actionCollection(), "bottomlayer");
	m_layerProperties = new KAction(i18n("Layer Properties..."), 0, this, SLOT(layerProperties()), actionCollection(), "layer_properties");
	(void)new KAction(i18n("I&nsert Image as Layer..."), 0, this, SLOT(slotInsertImageAsLayer()), actionCollection(), "insert_image_as_layer");
	m_layerSaveAs = new KAction(i18n("Save Layer as Image..."), 0, this, SLOT(save_layer_as_image()), actionCollection(), "save_layer_as_image");
	m_layerResize = new KAction(i18n("Resize Layer..."), 0, this, SLOT(layerResize()), actionCollection(), "resizelayer");
	m_layerResizeToImage = new KAction(i18n("Resize Layer to Image"), 0, this, SLOT(layerResizeToImage()), actionCollection(), "resizelayertoowner");

	// layer transformations - should be generic, for selection too
	m_layerScale = new KAction(i18n("Scale Layer..."), 0, this, SLOT(layerScale()), actionCollection(), "scalelayer");
	(void)new KAction(i18n("Rotate &180"), 0, this, SLOT(layer_rotate180()), actionCollection(), "layer_rotate180");
	(void)new KAction(i18n("Rotate &270"), 0, this, SLOT(layer_rotateleft90()), actionCollection(), "layer_rotateleft90");
	(void)new KAction(i18n("Rotate &90"), 0, this, SLOT(layer_rotateright90()), actionCollection(), "layer_rotateright90");
	(void)new KAction(i18n("Rotate &Custom..."), 0, this, SLOT(layer_rotate_custom()), actionCollection(), "layer_rotate_custom");
	(void)new KAction(i18n("Mirror &X"), 0, this, SLOT(layer_mirrorX()), actionCollection(), "layer_mirrorX");
	(void)new KAction(i18n("Mirror &Y"), 0, this, SLOT(layer_mirrorY()), actionCollection(), "layer_mirrorY");

	// color actions
	(void)new KAction(i18n("Select Foreground Color..."), 0, this, SLOT(selectFGColor()), actionCollection(), "select_fgColor");
	(void)new KAction(i18n("Select Background Color..."), 0, this, SLOT(selectBGColor()), actionCollection(), "select_bgColor");
	(void)new KAction(i18n("Reverse Foreground/Background Colors"), 0, this, SLOT(reverseFGAndBGColors()), actionCollection(), "reverse_fg_bg");

	// image actions
	(void)new KAction(i18n("Add New Image..."), 0, this, SLOT(add_new_image_tab()), actionCollection(), "add_new_image_tab");
	m_imgRm = new KAction(i18n("Remove Current Image"), 0, this, SLOT(remove_current_image_tab()), actionCollection(), "remove_current_image_tab");
	m_imgResize = new KAction(i18n("Resize Image..."), 0, this, SLOT(imageResize()), actionCollection(), "resize_image");
	m_imgDup = new KAction(i18n("Duplicate Image"), 0, this, SLOT(duplicateCurrentImg()), actionCollection(), "duplicate_image");
	m_imgMergeAll = new KAction(i18n("Merge &All Layers"), 0, this, SLOT(merge_all_layers()), actionCollection(), "merge_all_layers");
	m_imgMergeVisible = new KAction(i18n("Merge &Visible Layers"), 0, this, SLOT(merge_visible_layers()), actionCollection(), "merge_visible_layers");
	m_imgMergeLinked = new KAction(i18n("Merge &Linked Layers"), 0, this, SLOT(merge_linked_layers()), actionCollection(), "merge_linked_layers");

	// setting actions
	(void)new KAction(i18n("Paint Offset..."), "paint_offet", this, SLOT(setPaintOffset()), actionCollection(), "paint_offset");
	m_sidebarToggle = new KToggleAction(i18n("Show/Hide Sidebar"), "ok", 0, this, SLOT(showSidebar()), actionCollection(), "show_sidebar");
	m_floatsidebarToggle = new KToggleAction(i18n("Dock/Undock Sidebar"), "attach", 0, this, SLOT(floatSidebar()), actionCollection(), "float_sidebar");
	m_lsidebarToggle = new KToggleAction(i18n("Left/Right Sidebar"), "view_right", 0, this, SLOT(placeSidebarLeft()), actionCollection(), "left_sidebar");
	(void)KStdAction::saveOptions(this, SLOT(saveOptions()), actionCollection(), "save_options");
	KStdAction::preferences(this, SLOT(preferences()), actionCollection(), "preferences");

	// crayon box toolbar actions - these will be used only
	// to dock and undock wideget in the crayon box
	m_dlgColorsToggle = new KToggleAction(i18n("&Colors"), "color_dialog", 0, this, SLOT(dialog_colors()), actionCollection(), "colors_dialog");
	m_dlgCrayonToggle = new KToggleAction(i18n("C&rayons"), "krayon_box", 0, this, SLOT(dialog_crayons()), actionCollection(), "crayons_dialog");
	m_dlgBrushToggle = new KToggleAction(i18n("Brushes"), "brush_dialog", 0, this, SLOT(dialog_brushes()), actionCollection(), "brushes_dialog");
	m_dlgPatternToggle = new KToggleAction(i18n("Patterns"), "pattern_dialog", 0, this, SLOT(dialog_patterns()), actionCollection(), "patterns_dialog");
	m_dlgLayersToggle = new KToggleAction(i18n("Layers"), "layer_dialog", 0, this, SLOT(dialog_layers()), actionCollection(), "layers_dialog");
	m_dlgChannelsToggle = new KToggleAction(i18n("Channels"), "channel_dialog", 0, this, SLOT(dialog_channels()), actionCollection(), "channels_dialog");

	m_dlgBrushToggle -> setChecked(true);
	m_dlgPatternToggle -> setChecked(true);
	m_dlgLayersToggle -> setChecked(true);
	m_dlgChannelsToggle -> setChecked(true);
}

void KisView::reset()
{
	zoomUpdateGUI(0, 0, 1.0);
	canvasRefresh();
}

void KisView::resizeEvent(QResizeEvent *)
{
	Q_INT32 rsideW = 0;
	Q_INT32 lsideW = 0;
	Q_INT32 ruler = 20;
	Q_INT32 tbarOffset = 64;
	Q_INT32 tbarBtnH = 16;
	Q_INT32 tbarBtnW = 16;
	Q_INT32 drawH;
	Q_INT32 drawW;
	Q_INT32 docW;
	Q_INT32 docH;

	if (m_sideBar && m_sidebarToggle -> isChecked() && !m_floatsidebarToggle -> isChecked()) {
		if (m_lsidebarToggle -> isChecked()) {
			lsideW = m_sideBar -> width();
			m_sideBar -> setGeometry(0, 0, lsideW, height());
		} else {
			rsideW = m_sideBar -> width();
			m_sideBar -> setGeometry(width() - rsideW, 0, rsideW, height());
		}

		m_sideBar -> show();
	}

	m_hRuler -> setGeometry(ruler + lsideW, 0, width() - ruler - rsideW - lsideW, ruler);
	m_vRuler -> setGeometry(0 + lsideW, ruler, ruler, height() - (ruler + tbarBtnH));
	m_tabFirst -> setGeometry(0 + lsideW, height() - tbarBtnH, tbarBtnW, tbarBtnH);
	m_tabLeft -> setGeometry(tbarBtnW + lsideW, height() - tbarBtnH, tbarBtnW, tbarBtnH);
	m_tabRight -> setGeometry(2 * tbarBtnW + lsideW, height() - tbarBtnH, tbarBtnW, tbarBtnH);
	m_tabLast -> setGeometry(3 * tbarBtnW + lsideW, height() - tbarBtnH, tbarBtnW, tbarBtnH);
	m_tabFirst -> show();
	m_tabLeft -> show();
	m_tabRight -> show();
	m_tabLast -> show();
	drawH = height() - ruler - tbarBtnH - canvasYOffset();
	drawW = width() - ruler - lsideW - rsideW - canvasXOffset();
	docW = docWidth();
	docH = docHeight();
	docW = static_cast<Q_INT32>((zoom()) * docW);
	docH = static_cast<Q_INT32>((zoom()) * docH);
	m_vScroll -> setEnabled(docH > drawH);
	m_hScroll -> setEnabled(docW > drawW);

	if (docH <= drawH && docW <= drawW) {
		// we need no scrollbars
		m_vScroll -> hide();
		m_hScroll -> hide();
		m_vScroll -> setValue(0);
		m_hScroll -> setValue(0);
		m_canvas -> setGeometry(ruler + lsideW, ruler, drawW, drawH);
		m_tabBar -> setGeometry(tbarOffset + lsideW, height() - tbarBtnH, width() - rsideW - lsideW - tbarOffset, tbarBtnH);
		m_canvas -> show();
		m_tabBar -> show();
	} else if (docH <= drawH) {
		// we need a horizontal scrollbar only
		m_vScroll -> hide();
		m_vScroll -> setValue(0);
		m_hScroll -> setRange(0, static_cast<int>((docW - drawW) / zoom()));
		m_hScroll -> setGeometry(tbarOffset + lsideW + (width() - rsideW -lsideW - tbarOffset) / 2,
				height() - tbarBtnH,
				(width() - rsideW -lsideW - tbarOffset) / 2,
				tbarBtnH);
		m_canvas -> setGeometry(ruler + lsideW, ruler, drawW, drawH);
		m_tabBar -> setGeometry(tbarOffset + lsideW, height() - tbarBtnH, (width() - rsideW - lsideW - tbarOffset) / 2, tbarBtnH);
		m_hScroll -> show();
		m_canvas -> show();
		m_tabBar -> show();
	} else if(docW <= drawW) {
		// we need a vertical scrollbar only
		m_hScroll -> hide();
		m_hScroll -> setValue(0);
		m_vScroll -> setRange(0, static_cast<int>((docH - drawH) / zoom()));
		m_vScroll -> setGeometry(width() - tbarBtnW - rsideW, ruler, tbarBtnW, height() - (ruler + tbarBtnH));
		m_canvas -> setGeometry(ruler + lsideW, ruler, drawW - tbarBtnW, drawH);
		m_tabBar -> setGeometry(tbarOffset + lsideW, height() - tbarBtnH, width() - rsideW -lsideW - tbarOffset, tbarBtnH);
		m_vScroll -> show();
		m_canvas -> show();
		m_tabBar -> show();
	} else {
		// we need both scrollbars
		m_vScroll -> setRange(0, static_cast<int>((docH - drawH) / zoom()));
		m_vScroll -> setGeometry(width() - tbarBtnW - rsideW, ruler, tbarBtnW, height() - (ruler + tbarBtnH));
		m_hScroll -> setRange(0, static_cast<int>((docW - drawW) / zoom()));
		m_hScroll -> setGeometry(tbarOffset + lsideW + (width() - rsideW -lsideW - tbarOffset) / 2,
				height() - tbarBtnH,
				(width() - rsideW -lsideW - tbarOffset) / 2,
				tbarBtnH);
		m_canvas -> setGeometry(ruler + lsideW, ruler, drawW - tbarBtnW, drawH);
		m_tabBar -> setGeometry(tbarOffset + lsideW, height() - tbarBtnH, (width() - rsideW -lsideW - tbarOffset)/2, tbarBtnH);
		m_vScroll -> show();
		m_hScroll -> show();
		m_canvas -> show();
		m_tabBar -> show();
	}

	m_vRuler -> setRange(0, docH + static_cast<Q_INT32>(100 * zoom()));
	m_hRuler -> setRange(0, docW + static_cast<Q_INT32>(100 * zoom()));
	m_vScroll -> setLineStep(static_cast<Q_INT32>(100 * zoom()));
	m_hScroll -> setLineStep(static_cast<Q_INT32>(100 * zoom()));

	if (m_vScroll -> isVisible())
		m_vRuler -> setOffset(m_vScroll -> value());
	else
		m_vRuler -> setOffset(-canvasYOffset());

	if (m_hScroll -> isVisible())
		m_hRuler -> setOffset(m_hScroll -> value());
	else
		m_hRuler -> setOffset(-canvasXOffset());

	m_hRuler -> show();
	m_vRuler -> show();
}

void KisView::updateReadWrite(bool readwrite)
{
	layerUpdateGUI(readwrite);
	selectionUpdateGUI(readwrite);
}

void KisView::clearCanvas(const QRect& rc)
{
	QPainter gc(m_canvas);

	gc.eraseRect(rc);
}

void KisView::activateTool(KisToolSP tool)
{
	if (m_tool)
		m_tool -> clear();

	if (tool && qFind(m_toolSet.begin(), m_toolSet.end(), tool) != m_toolSet.end()) {
		m_tool = tool;
		m_tool -> cursor(m_canvas);
		m_tool -> activate();
	} else {
		m_tool = 0;
		m_canvas -> setCursor(KisCursor::arrowCursor());
	}
}

KisImageSP KisView::currentImg() const
{
	if (m_current && m_doc -> contains(m_current))
		return m_current;

	m_current = m_doc -> imageNum(m_doc -> nimages() - 1);
	connectCurrentImg();
	return m_current;
}

QString KisView::currentImgName() const
{
	if (currentImg())
		return currentImg() -> name();

	return QString::null;
}

Q_INT32 KisView::horzValue() const
{
	return m_hScroll -> value();
}

Q_INT32 KisView::vertValue() const
{
	return m_vScroll -> value();
}

void KisView::paintView(const QRect& rc)
{
	KisImageSP img = currentImg();

	if (img) {
		QPainter gc(m_canvas);
		QRect ur = rc;
		Q_INT32 xt;
		Q_INT32 yt;

		if (ur.x() < 0)
			ur.setLeft(0);

		if (ur.y() < 0)
			ur.setTop(0);

		if (canvasXOffset())
			gc.eraseRect(0, 0, canvasXOffset(), height());

		if (canvasYOffset())
			gc.eraseRect(static_cast<Q_INT32>(canvasXOffset() * zoom()), 0, width(), static_cast<Q_INT32>(canvasYOffset() * zoom()));

		gc.eraseRect(static_cast<Q_INT32>(canvasXOffset() * zoom()), static_cast<Q_INT32>(docHeight() * zoom()) - canvasYOffset(), width(), height());
		gc.eraseRect(static_cast<Q_INT32>(docWidth() * zoom()) - canvasXOffset() - 1, static_cast<Q_INT32>(canvasYOffset() * zoom()), width(), height());
		xt = -canvasXOffset() + horzValue();
		yt = -canvasYOffset() + vertValue();
		ur.moveBy(xt, yt);

		if (img -> width() * zoom() < ur.right())
			ur.setWidth(img -> width());

		if (img -> height() * zoom() < ur.bottom())
			ur.setHeight(img -> height());

		if (ur.x() > docWidth())
			return;

		if (ur.y() > docHeight())
			return;

		ur.setBottom(ur.bottom() + 1);
		ur.setRight(ur.right() + 1);
		xt = canvasXOffset() - horzValue();
		yt = canvasYOffset() - vertValue();

		if (zoom() < 1.0 || zoom() > 1.0)
			gc.setViewport(0, 0, static_cast<Q_INT32>(m_canvas -> width() * zoom()), static_cast<Q_INT32>(m_canvas -> height() * zoom()));

		if (xt || yt)
			gc.translate(xt, yt);

		m_doc -> setProjection(img);
		m_doc -> paintContent(gc, ur, false, 1.0, 1.0);

		if (m_tool)
			m_tool -> paint(gc, ur);
	} else {
		clearCanvas(rc);
	}
}

void KisView::updateCanvas()
{
	QRect rc(0, 0, m_canvas -> width(), m_canvas -> height());

	updateCanvas(rc);
}

void KisView::updateCanvas(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{
	QRect rc(x, y, w, h);

	updateCanvas(rc);
}

void KisView::updateCanvas(const QRect& rc)
{
	QRect ur = rc;

	ur.setX(ur.x() - static_cast<Q_INT32>(horzValue() * zoom()));
	ur.setY(ur.y() - static_cast<Q_INT32>(vertValue() * zoom()));
	paintView(ur);
}

/*
    tool_properties invokes the optionsDialog() method for the
    currentImg active tool.  There should be an options dialog for
    each tool, but these can also be consolidated into a master
    options dialog by reparenting the widgets for each tool to a
    tabbed properties dialog, each tool getting a tab  - later
*/

void KisView::tool_properties()
{
#if 0
	Q_ASSERT(m_pTool);
	m_pTool -> optionsDialog();
#endif
}

void KisView::layerUpdateGUI(bool enable)
{
	KisImageSP img = currentImg();
	KisLayerSP layer;
	Q_INT32 nlayers;
	Q_INT32 layerPos;

	if (img) {
		layer = img -> activeLayer();
		nlayers = img -> nlayers();
	}
	
	if (layer)
		layerPos = img -> index(layer);

	enable = enable && img && layer;
	m_layerDup -> setEnabled(enable);
	m_layerRm -> setEnabled(enable);
	m_layerLink -> setEnabled(enable);
	m_layerHide -> setEnabled(enable);
	m_layerProperties -> setEnabled(enable);
	m_layerSaveAs -> setEnabled(enable);
	m_layerResize -> setEnabled(enable);
	m_layerResizeToImage -> setEnabled(enable);
	m_layerScale -> setEnabled(enable);
	m_layerRaise -> setEnabled(enable && nlayers > 1 && layerPos);
	m_layerLower -> setEnabled(enable && nlayers > 1 && layerPos != nlayers - 1);
	m_layerTop -> setEnabled(enable && nlayers > 1 && layerPos);
	m_layerBottom -> setEnabled(enable && nlayers > 1 && layerPos != nlayers - 1);
	imgUpdateGUI();
}

void KisView::selectionUpdateGUI(bool enable)
{
	KisImageSP img = currentImg();

	enable = enable && img && img -> selection() && img -> selection() -> parent();
	m_selectionCut -> setEnabled(enable);
	m_selectionCopy -> setEnabled(enable);
	m_selectionCrop -> setEnabled(enable);
	m_selectionPaste -> setEnabled(img != 0 && m_clipboardHasImage);
	m_selectionRm -> setEnabled(enable);
	m_selectionFillBg -> setEnabled(enable);
	m_selectionFillFg -> setEnabled(enable);
	m_selectionSelectAll -> setEnabled(img != 0);
	m_selectionSelectNone -> setEnabled(enable);
}

void KisView::cut()
{
	copy();
	removeSelection();
}

void KisView::copy()
{
	KisImageSP img = currentImg();

	if (img) {
		KisSelectionSP selection = img -> selection();

		if (selection) {
			selection -> clearParentOnMove(false);
			m_doc -> setClipboardSelection(selection);
			imgSelectionChanged(currentImg());
		}
	}
}

void KisView::paste()
{
	Q_ASSERT(!QApplication::clipboard() -> image().isNull());
	activateTool(m_paste);
}

void KisView::removeSelection()
{
	KisImageSP img = currentImg();

	if (img) {
		KisSelectionSP selection = img -> selection();

		if (selection) {
			KisPaintDeviceSP parent = selection -> parent();

			if (parent) {
				QRect rc = selection -> bounds();
				QRect ur;
				KisPainter gc(parent);

				Q_ASSERT(!m_doc -> inMacro());
				m_doc -> beginMacro(i18n("Remove Selection"));
				img -> unsetSelection(true);
				ur = rc;
			
				if (parent -> x() || parent -> y())
					rc.moveBy(-parent -> x(), -parent -> y());

				gc.beginTransaction("remove selection on parent");
				gc.eraseRect(rc);
				m_doc -> addCommand(gc.end());
				m_doc -> endMacro();
				m_doc -> setModified(true);
				img -> invalidate(rc);
				updateCanvas(ur);
			}
		}
	}
}

void KisView::crop()
{
	KisImageSP img = currentImg();

	if (img) {
		KisSelectionSP selection = img -> selection();

		img -> unsetSelection(false);

		if (selection && m_doc -> layerAdd(img, img -> nextLayerName(), selection))
			layersUpdated();
		else
			img -> setSelection(selection);
	}
}


void KisView::imgUpdateGUI()
{
	const KisImageSP img = currentImg();
	Q_INT32 n = 0;
	Q_INT32 nvisible = 0;
	Q_INT32 nlinked = 0;

	m_imgRm -> setEnabled(img != 0);
	m_imgResize -> setEnabled(img != 0);
	m_imgDup -> setEnabled(img != 0);
	m_imgExport -> setEnabled(img != 0);
	m_layerAdd -> setEnabled(img != 0);

	if (img) {
		const vKisLayerSP& layers = img -> layers();

		n = layers.size();

		for (vKisLayerSP_cit it = layers.begin(); it != layers.end(); it++) {
			const KisLayerSP& layer = *it;

			if (layer -> linked())
				nlinked++;

			if (layer -> visible())
				nvisible++;

			if (nlinked > 1 && nvisible > 1)
				break;
		}
	}

	m_imgMergeAll -> setEnabled(n > 1);
	m_imgMergeVisible -> setEnabled(nvisible > 1);
	m_imgMergeLinked -> setEnabled(nlinked > 1);
}

void KisView::fillSelectionBg()
{
	fillSelection(bgColor(), OPACITY_OPAQUE);
}

void KisView::fillSelectionFg()
{
	fillSelection(fgColor(), OPACITY_OPAQUE);
}

void KisView::fillSelection(const KoColor& c, QUANTUM opacity)
{
	KisImageSP img = currentImg();

	if (img) {
		KisSelectionSP selection = img -> selection();

		if (selection) {
			QRect rc = selection -> bounds();
			QRect ur = rc;
			KisPainter gc(selection.data());

			rc.moveBy(-rc.x(), -rc.y());
			gc.beginTransaction(i18n("Fill Selection."));
			gc.fillRect(rc, c, opacity);
			m_doc -> addCommand(gc.endTransaction());
			gc.end();
			img -> invalidate(ur);
			m_doc -> setModified(true);
			updateCanvas(ur);
		}
	}
}

void KisView::selectAll()
{
	KisImageSP img = currentImg();

	if (img) {
		KisPaintDeviceSP dev;
	       
		img -> unsetSelection();
		dev = img -> activeDevice();

		if (dev) {
			KisSelectionSP selection = new KisSelection(dev, img, "Selection box from KisView", OPACITY_OPAQUE);

			selection -> setBounds(dev -> bounds());
			img -> setSelection(selection);
			updateCanvas();
		}
	}
}

void KisView::unSelectAll()
{
	KisImageSP img = currentImg();

	if (img) {
		img -> unsetSelection();
		updateCanvas();
	}
}

void KisView::zoomUpdateGUI(Q_INT32 x, Q_INT32 y, double zf)
{
	Q_ASSERT(m_zoomIn);
	Q_ASSERT(m_zoomOut);
	setZoom(zf);
	m_zoomIn -> setEnabled(zf <= KISVIEW_MAX_ZOOM);
	m_zoomOut -> setEnabled(zf >= KISVIEW_MIN_ZOOM);

	if (zf > 3.0) {
		m_hRuler -> setPixelPerMark(static_cast<Q_INT32>(zf * 1.0));
		m_vRuler -> setPixelPerMark(static_cast<Q_INT32>(zf * 1.0));
	} else {
		Q_INT32 mark = static_cast<Q_INT32>(zf * 10.0);

		if (!mark)
			mark = 1;

		m_hRuler -> setPixelPerMark(mark);
		m_vRuler -> setPixelPerMark(mark);
	}

	m_hRuler -> setShowTinyMarks(zf <= 0.4);
	m_vRuler -> setShowTinyMarks(zf <= 0.4);
	m_hRuler -> setShowLittleMarks(zf > 0.3);
	m_vRuler -> setShowLittleMarks(zf > 0.3);
	m_hRuler -> setShowMediumMarks(zf > 0.2);
	m_vRuler -> setShowMediumMarks(zf > 0.2);

	if (x < 0 || y < 0) {
		QRect ur(0, 0, m_canvas -> width(), m_canvas -> height());

		resizeEvent(0);
		ur.setX(ur.x() - static_cast<Q_INT32>(horzValue() * zoom()));
		ur.setY(ur.y() - static_cast<Q_INT32>(vertValue() * zoom()));

		if (zoom() > 1.0 || zoom() < 1.0) {
			Q_INT32 urL = ur.left();
			Q_INT32 urT = ur.top();
			Q_INT32 urW = ur.width();
			Q_INT32 urH = ur.height();

			urL = static_cast<int>(static_cast<double>(urL) / zoom());
			urT = static_cast<int>(static_cast<double>(urT) / zoom());
			urW = static_cast<int>(static_cast<double>(urW) / zoom());
			urH = static_cast<int>(static_cast<double>(urH) / zoom());
			ur.setLeft(urL);
			ur.setTop(urT);
			ur.setWidth(urW);
			ur.setHeight(urH);
		}

		updateCanvas(ur);
	} else {
		x = static_cast<Q_INT32>(x * zf - width() / 2);
		y = static_cast<Q_INT32>(y * zf - height() / 2);

		if (x < 0)
			x = 0;

		if (y < 0)
			y = 0;

		scrollTo(x, y);
	}
}

void KisView::zoomIn(Q_INT32 x, Q_INT32 y)
{
	if (zoom() <= KISVIEW_MAX_ZOOM)
		zoomUpdateGUI(x, y, zoom() * 2);
}

void KisView::zoomOut(Q_INT32 x, Q_INT32 y)
{
	if (zoom() >= KISVIEW_MIN_ZOOM)
		zoomUpdateGUI(x, y, zoom() / 2);
}

void KisView::zoomIn()
{
	if (zoom() <= KISVIEW_MAX_ZOOM)
		zoomUpdateGUI(-1, -1, zoom() * 2);
}

void KisView::zoomOut()
{
	if (zoom() >= KISVIEW_MIN_ZOOM)
		zoomUpdateGUI(-1, -1, zoom() / 2);
}

/*
    dialog_gradient - invokes a GradientDialog which is
    now an options dialog.  Gradients can be used by many tools
    and are not a tool in themselves.
*/

void KisView::dialog_gradient()
{
#if 0
	GradientDialog *pGradientDialog = new GradientDialog(m_pGradient);
	pGradientDialog->exec();

	if (pGradientDialog->result() == QDialog::Accepted) {
		/* set m_pGradient here and update gradientwidget in sidebar
		   to show sample of gradient selected. Also update effect for
		   the framebuffer's gradient, which is used in painting */

		int type = pGradientDialog->gradientTab()->gradientType();
		m_pGradient->setEffect(static_cast<KImageEffect::GradientType>(type));

		KisFrameBuffer *fb = m_doc->frameBuffer();
		fb->setGradientEffect(static_cast<KImageEffect::GradientType>(type));

		kdDebug() << "gradient type is " << type << endl;
	}
#endif
}

void KisView::dialog_colors()
{
}

void KisView::dialog_crayons()
{
}

void KisView::dialog_brushes()
{
	m_brushChooser -> setDocked(m_dlgBrushToggle -> isChecked());
}

void KisView::dialog_patterns()
{
#if 0
	if (m_dialog_patterns -> isChecked())
		m_sideBar -> plug(m_pPatternChooser);
	else
		m_sideBar -> unplug(m_pPatternChooser);
#endif
}

void KisView::dialog_layers()
{
#if 0
	if(m_dialog_layers -> isChecked())
		m_sideBar -> plug(m_layerBox);
	else
		m_sideBar -> unplug(m_layerBox);
#endif
}

void KisView::dialog_channels()
{
#if 0
	if (m_dialog_channels -> isChecked())
		m_sideBar -> plug(m_pChannelView);
	else
		m_sideBar -> unplug(m_pChannelView);
#endif
}

void KisView::next_layer()
{
	KisImageSP img = currentImg();
	KisLayerSP layer;

	if (!img)
		return;

	layer = img -> activeLayer();

	if (layer) {
		m_doc -> layerNext(img, layer);
		resizeEvent(0);
		updateCanvas();
	}
}

void KisView::previous_layer()
{
	KisImageSP img = currentImg();
	KisLayerSP layer;

	if (!img)
		return;

	layer = img -> activeLayer();

	if (layer) {
		m_doc -> layerPrev(img, layer);
		resizeEvent(0);
		updateCanvas();
	}
}

void KisView::slotImportImage()
{
	if (importImage(false) > 0)
		m_doc -> setModified(true);
}

void KisView::export_image()
{
	KURL url = KFileDialog::getSaveURL(QString::null, KisUtil::writeFilters(), this, i18n("Export Image"));
	KisImageSP img = currentImg();
	KisLayerSP dst;

	if (url.isEmpty())
		return;

	Q_ASSERT(img);

	if (img) {
		KisImageMagickConverter ib(m_doc);
		img = new KisImage(*img);

		if (img -> nlayers() == 1) {
			dst = img -> layer(0);
			Q_ASSERT(dst);
		} else {
			dst = new KisLayer(img, img -> width(), img -> height(), "layer for exporting", OPACITY_OPAQUE);

			KisPainter gc;
			KisMerge<flattenAll> visitor(img, true);
			vKisLayerSP layers = img -> layers();

			gc.begin(dst.data());
			visitor(gc, layers);
			gc.end();
		}

		m_imgBuilderMgr -> attach(&ib);
		m_buildProgress -> changeSubject(&ib);

		switch (ib.buildFile(url, dst)) {
			case KisImageBuilder_RESULT_UNSUPPORTED:
				KMessageBox::error(this, i18n("No coder for this type of file."), i18n("Error Saving file"));
				break;
			case KisImageBuilder_RESULT_INVALID_ARG:
				KMessageBox::error(this, i18n("Invalid argument."), i18n("Error Saving file"));
				break;
			case KisImageBuilder_RESULT_NO_URI:
			case KisImageBuilder_RESULT_NOT_LOCAL:
				KMessageBox::error(this, i18n("Unable to locate file."), i18n("Error Saving file"));
				break;
			case KisImageBuilder_RESULT_BAD_FETCH:
				KMessageBox::error(this, i18n("Unable to upload file."), i18n("Error Saving File"));
				break;
			case KisImageBuilder_RESULT_EMPTY:
				KMessageBox::error(this, i18n("Empty file."), i18n("Error Saving File"));
				break;
			case KisImageBuilder_RESULT_FAILURE:
				KMessageBox::error(this, i18n("Error Saving File."), i18n("Error Saving File"));
				break;
			case KisImageBuilder_RESULT_OK:
			default:
				break;
		}
	}
}

void KisView::slotInsertImageAsLayer()
{
	if (importImage(true) > 0)
		m_doc -> setModified(true);
}

void KisView::save_layer_as_image()
{
	KURL url = KFileDialog::getSaveURL(QString::null, KisUtil::writeFilters(), this, i18n("Export Layer"));
	KisImageSP img = currentImg();

	if (url.isEmpty())
		return;

	Q_ASSERT(img);

	if (img) {
		KisImageMagickConverter ib(m_doc);
		KisLayerSP dst = img -> activeLayer();

		Q_ASSERT(dst);

		switch (ib.buildFile(url, dst)) {
			case KisImageBuilder_RESULT_UNSUPPORTED:
				KMessageBox::error(this, i18n("No coder for this type of file."), i18n("Error Saving file"));
				break;
			case KisImageBuilder_RESULT_INVALID_ARG:
				KMessageBox::error(this, i18n("Invalid argument."), i18n("Error Saving file"));
				break;
			case KisImageBuilder_RESULT_NO_URI:
			case KisImageBuilder_RESULT_NOT_LOCAL:
				KMessageBox::error(this, i18n("Unable to locate file."), i18n("Error Saving file"));
				break;
			case KisImageBuilder_RESULT_BAD_FETCH:
				KMessageBox::error(this, i18n("Unable to upload file."), i18n("Error Saving File"));
				break;
			case KisImageBuilder_RESULT_EMPTY:
				KMessageBox::error(this, i18n("Empty file."), i18n("Error Saving File"));
				break;
			case KisImageBuilder_RESULT_FAILURE:
				KMessageBox::error(this, i18n("Error Saving File."), i18n("Error Saving File"));
				break;
			case KisImageBuilder_RESULT_OK:
			default:
				break;
		}
	}
}

void KisView::slotEmbedImage(const QString& filename)
{
	importImage(false, true, filename);
}

Q_INT32 KisView::importImage(bool createLayer, bool modal, const QString& filename)
{
	KURL::List urls;
	Q_INT32 rc = 0;

	if (filename.isEmpty())
		urls = KFileDialog::getOpenURLs(QString::null, KisUtil::readFilters(), 0, i18n("Import Image"));

	if (urls.empty())
		return 0;

	KisImageMagickConverter ib(m_doc);
	KisImageSP img;

	m_imgBuilderMgr -> attach(&ib);

	for (KURL::List::iterator it = urls.begin(); it != urls.end(); it++) {
		KURL url = *it;
		KisDlgBuilderProgress dlg(&ib);

		if (modal)
			dlg.show();
		else
			m_buildProgress -> changeSubject(&ib);

		switch (ib.buildImage(url)) {
			case KisImageBuilder_RESULT_UNSUPPORTED:
				KMessageBox::error(this, i18n("No coder for this type of file."), i18n("Error Loading file"));
				continue;
			case KisImageBuilder_RESULT_NO_URI:
			case KisImageBuilder_RESULT_NOT_LOCAL:
				KNotifyClient::event("cannotopenfile");
				continue;
			case KisImageBuilder_RESULT_NOT_EXIST:
				KMessageBox::error(this, i18n("File does not exist."), i18n("Error Loading File"));
				KNotifyClient::event("cannotopenfile");
				continue;
			case KisImageBuilder_RESULT_BAD_FETCH:
				KMessageBox::error(this, i18n("Unable to download file."), i18n("Error Loading File"));
				KNotifyClient::event("cannotopenfile");
				continue;
			case KisImageBuilder_RESULT_EMPTY:
				KMessageBox::error(this, i18n("Empty file."), i18n("Error Loading File"));
				KNotifyClient::event("cannotopenfile");
				continue;
			case KisImageBuilder_RESULT_FAILURE:
				KMessageBox::error(this, i18n("Error Loading File."), i18n("Error Loading File"));
				KNotifyClient::event("cannotopenfile");
				continue;
			case KisImageBuilder_RESULT_PROGRESS:
				break;
			case KisImageBuilder_RESULT_OK:
			default:
				break;
		}

		if (!(img = ib.image()))
			continue;

		if (createLayer && currentImg()) {
			vKisLayerSP v = img -> layers();
			KisImageSP current = currentImg();

			rc += v.size();
			current -> unsetSelection();

			for (vKisLayerSP_it it = v.begin(); it != v.end(); it++) {
				KisLayerSP layer = *it;

				layer -> setImage(current);
				layer -> setName(current -> nextLayerName());
				m_doc -> layerAdd(current, layer, 0);
				m_layerBox -> setCurrentItem(img -> index(layer));
			}

			current -> invalidate();
			resizeEvent(0);
			updateCanvas();
		} else {
			m_doc -> addImage(img);
			rc++;
		}
	}

	if (img)
		selectImage(img);

	return rc;
}

void KisView::layerScale(bool )
{
#if 0
    KisImageSP img = m_doc->currentImg();
    if (!img)  return;

    KisLayerSP lay = img->getCurrentLayer();
    if (!lay)  return;

    KisFrameBuffer *fb = m_doc->frameBuffer();
    if (!fb)  return;

    NewLayerDialog *pNewLayerDialog = new NewLayerDialog();
    pNewLayerDialog->exec();
    if(!pNewLayerDialog->result() == QDialog::Accepted)
        return;

    QRect srcR(lay->imageExtents());

    // only get the part of the layer which is inside the
    // image boundaries - layer can be bigger or can overlap
    srcR = srcR.intersect(img->imageExtents());

    bool ok;

    if(smooth)
        ok = fb->scaleSmooth(srcR,
            pNewLayerDialog->width(), pNewLayerDialog->height());
    else
        ok = fb->scaleRough(srcR,
            pNewLayerDialog->width(), pNewLayerDialog->height());

    if(!ok)
    {
        kdDebug() << "layer_scale() failed" << endl;
    }
    else
    {
        // bring new scaled layer to front
        uint indx = img->layerList().size() - 1;
        img->setCurrentLayer(indx);
        img->markDirty(img->getCurrentLayer()->imageExtents());
	layerSelected(indx);
	layersUpdated();

        m_doc->setModified(true);
    }
#endif
}

void KisView::layer_rotate180()
{

}

void KisView::layer_rotateleft90()
{

}

void KisView::layer_rotateright90()
{

}

void KisView::layer_rotate_custom()
{

}

void KisView::layer_mirrorX()
{
}

void KisView::layer_mirrorY()
{
}

void KisView::add_new_image_tab()
{
	m_doc -> slotNewImage();
}

void KisView::remove_current_image_tab()
{
	KisImageSP current = currentImg();

	if (current) {
		disconnectCurrentImg();
		m_current = 0;
		m_doc -> removeImage(current);
	}
}

void KisView::imageResize()
{
	KisImageSP img = currentImg();

	if (img) {
		KisConfig cfg;
		KisDlgDimension dlg(cfg.maxImgWidth(), img -> width(), cfg.maxImgHeight(), img -> height(), this);

		if (dlg.exec() == QDialog::Accepted) {
			QSize size = dlg.getSize();

			img -> resize(size.width(), size.height());
			img -> invalidate();
			resizeEvent(0);
			layersUpdated();
			canvasRefresh();
		}
	}
}

void KisView::merge_all_layers()
{
	KisImageSP img = currentImg();

	if (img) {
		KisLayerSP dst = new KisLayer(img, img -> width(), img -> height(), img -> nextLayerName(), OPACITY_OPAQUE);
		KisPainter gc(dst.data());
		KisMerge<flattenAll> visitor(img, false);
		vKisLayerSP layers = img -> layers();

		visitor(gc, layers);
		img -> add(dst, -1);
		layersUpdated();
		updateCanvas();
	}
}

void KisView::merge_visible_layers()
{
	KisImageSP img = currentImg();

	if (img) {
		KisLayerSP dst = new KisLayer(img, img -> width(), img -> height(), img -> nextLayerName(), OPACITY_OPAQUE);
		KisPainter gc(dst.data());
		KisMerge<flattenAllVisible> visitor(img, false);
		vKisLayerSP layers = img -> layers();

		visitor(gc, layers);
		img -> add(dst, -1);
		layersUpdated();
		updateCanvas();
	}
}

void KisView::merge_linked_layers()
{
	KisImageSP img = currentImg();

	if (img) {
		KisLayerSP dst = new KisLayer(img, img -> width(), img -> height(), img -> nextLayerName(), OPACITY_OPAQUE);
		KisPainter gc(dst.data());
		KisMerge<flattenAllLinked> visitor(img, false);
		vKisLayerSP layers = img -> layers();

		visitor(gc, layers);
		img -> add(dst, -1);
		layersUpdated();
		updateCanvas();
	}
}

void KisView::showMenubar()
{
}


void KisView::showToolbar()
{
}


void KisView::showStatusbar()
{
}


void KisView::showSidebar()
{
	if (m_sidebarToggle -> isChecked())
		m_sideBar -> show();
	else
		m_sideBar -> hide();

	resizeEvent(0);
}

void KisView::floatSidebar()
{
	m_sideBar -> setDocked(!m_floatsidebarToggle -> isChecked());
	resizeEvent(0);
}

void KisView::placeSidebarLeft()
{
	resizeEvent(0);
}

/*
    saveOptions - here we need to write entries to a congig
    file.
*/
void KisView::saveOptions()
{
}


/*
    preferences - the main Krayon preferences dialog - modeled
    after the konqueror prefs dialog - quite nice compound dialog
*/
void KisView::preferences()
{
 //   PreferencesDialog::editPreferences();
}

Q_INT32 KisView::docWidth() const
{
	return currentImg() ? currentImg() -> width() : 0;
}

Q_INT32 KisView::docHeight() const
{
	return currentImg() ? currentImg() -> height() : 0;
}

void KisView::scrollTo(Q_INT32 x, Q_INT32 y)
{
	kdDebug() << "scroll to " << x << "," << y << endl;

    // this needs to update the scrollbar values and
    // let resizeEvent() handle the repositioning
    // with showScollBars()
}

void KisView::setActiveBrush(KoIconItem *brush)
{
	m_brush = dynamic_cast<KisBrush*>(brush);
	m_sideBar -> slotSetBrush(*m_brush);
}

void KisView::setActiveCrayon(KoIconItem *crayon)
{
	m_crayon = dynamic_cast<KisKrayon*>(crayon);
	m_sideBar -> slotSetKrayon(*m_crayon);
}

void KisView::setActivePattern(KoIconItem *pattern)
{
	m_pattern = dynamic_cast<KisPattern*>(pattern);
	m_sideBar -> slotSetPattern(*m_pattern);
}

void KisView::setBGColor(const KoColor& c)
{
	emit bgColorChanged(c);
	m_bg = c;
}

void KisView::setFGColor(const KoColor& c)
{
	emit fgColorChanged(c);
	m_fg = c;
}

void KisView::slotSetFGColor(const KoColor& c)
{
	m_fg = c;
}

void KisView::slotSetBGColor(const KoColor& c)
{
	m_bg = c;
}

void KisView::setupPrinter(KPrinter& )
{
#if 0
    printer.setPageSelection(KPrinter::ApplicationSide);

    int count = 0;
    QStringList imageList = m_doc->images();
    for (QStringList::Iterator it = imageList.begin(); it != imageList.end(); ++it) {
        if (*it == currentImg()->name())
            break;
        ++count;
    }

    printer.setCurrentPage(1 + count);
    printer.setMinMax(1, m_doc->images().count());
    printer.setPageSize(KPrinter::A4);
    printer.setOrientation(KPrinter::Portrait);
#endif
}

void KisView::print(KPrinter &)
{
#if 0
    printer.setFullPage(true);
    QPainter paint;
    paint.begin(&printer);
    paint.setClipping(false);
    QValueList<int> imageList;
    int from = printer.fromPage();
    int to = printer.toPage();
    if(!from && !to)
    {
        from = printer.minPage();
        to = printer.maxPage();
    }
    for (int i = from; i <= to; i++)
        imageList.append(i);
    QString tmp_currentImageName = m_doc->currentImgName();
    QValueList<int>::Iterator it = imageList.begin();
    for (; it != imageList.end(); ++it)
    {
        int imageNumber = *it - 1;
        if (it != imageList.begin())
            printer.newPage();

        m_doc->setImage(*m_doc->images().at(imageNumber));
        m_doc->paintContent(paint, m_doc->getImageRect());
    }
    paint.end ();
    m_doc->setImage(tmp_currentImageName);
#endif
}

void KisView::setupTools()
{
	m_toolSet = toolFactory(this, m_doc);
	m_paste = new KisToolPaste(this, m_doc);
	m_toolSet.push_back(m_paste);
	m_paste -> setup();
}

void KisView::canvasGotPaintEvent(QPaintEvent *event)
{
	QRect ur = event -> rect();

	if (zoom() > 1.0 || zoom() < 1.0) {
		Q_INT32 urL = ur.left();
		Q_INT32 urT = ur.top();
		Q_INT32 urW = ur.width();
		Q_INT32 urH = ur.height();

		urL = static_cast<Q_INT32>(static_cast<double>(urL) / zoom());
		urT = static_cast<Q_INT32>(static_cast<double>(urT) / zoom());
		urW = static_cast<Q_INT32>(static_cast<double>(urW) / zoom());
		urH = static_cast<Q_INT32>(static_cast<double>(urH) / zoom());
		ur.setLeft(urL);
		ur.setTop(urT);
		ur.setWidth(urW);
		ur.setHeight(urH);
	}

	paintView(ur);
}

void KisView::canvasGotMousePressEvent(QMouseEvent *e)
{
	if (m_tool) {
		Q_INT32 x = static_cast<Q_INT32>((e -> pos().x() - canvasXOffset() + horzValue() * zoom()) / zoom());
		Q_INT32 y = static_cast<Q_INT32>((e -> pos().y() - canvasYOffset() + vertValue() * zoom()) / zoom());
		QMouseEvent ev(QEvent::MouseButtonPress, QPoint(x, y), e -> globalPos(), e -> button(), e -> state());

		m_tool -> mousePress(&ev);
	}
}

void KisView::canvasGotMouseMoveEvent(QMouseEvent *e)
{
	if (m_tool) {
		Q_INT32 x = static_cast<Q_INT32>((e -> pos().x() - canvasXOffset() + horzValue() * zoom()) / zoom());
		Q_INT32 y = static_cast<Q_INT32>((e -> pos().y() - canvasYOffset() + vertValue() * zoom()) / zoom());
		QMouseEvent ev(QEvent::MouseButtonPress, QPoint(x, y), e -> globalPos(), e -> button(), e -> state());

		if (zoom() >= 1.0 / 4.0) {
			m_hRuler -> setValue(e -> pos().x() - canvasXOffset());
			m_vRuler -> setValue(e -> pos().y() - canvasYOffset());
		}

		m_tool -> mouseMove(&ev);
	}
}

void KisView::canvasGotMouseReleaseEvent(QMouseEvent *e)
{
	if (m_tool) {
		Q_INT32 x = static_cast<Q_INT32>((e -> pos().x() - canvasXOffset() + horzValue() * zoom()) / zoom());
		Q_INT32 y = static_cast<Q_INT32>((e -> pos().y() - canvasYOffset() + vertValue() * zoom()) / zoom());
		QMouseEvent ev(QEvent::MouseButtonPress, QPoint(x, y), e -> globalPos(), e -> button(), e -> state());

		m_tool -> mouseRelease(&ev);
	}
}

void KisView::canvasGotEnterEvent(QEvent *e)
{
	if (m_tool)
		m_tool -> enter(e);
}

void KisView::canvasGotLeaveEvent (QEvent *e)
{
	if (m_tool)
		m_tool -> leave(e);
}

void KisView::canvasGotMouseWheelEvent(QWheelEvent *event)
{
	QApplication::sendEvent(m_vScroll, event);
}

void KisView::canvasGotKeyPressEvent(QKeyEvent *event)
{
	if (m_tool)
		m_tool -> keyPress(event);
}

void KisView::canvasGotKeyReleaseEvent(QKeyEvent *event)
{
	if (m_tool)
		m_tool -> keyRelease(event);
}

void KisView::canvasRefresh()
{
	m_canvas -> repaint();
}

void KisView::layerToggleVisible()
{
	KisImageSP img = currentImg();

	if (img) {
		KisLayerSP layer = img -> activeLayer();

		if (layer) {
			layer -> visible(!layer -> visible());
			img -> invalidate();
			m_doc -> setModified(true);
			resizeEvent(0);
			layersUpdated();
			canvasRefresh();
		}
	}
}

void KisView::layerSelected(int n)
{
	KisImageSP img = currentImg();

	layerUpdateGUI(img -> activateLayer(n));
}

void KisView::docImageListUpdate()
{
	disconnectCurrentImg();
	m_current = 0;
	zoomUpdateGUI(0, 0, 1.0);
	resizeEvent(0);
	updateCanvas();

	if (!currentImg())
		layersUpdated();

	imgUpdateGUI();
}

void KisView::layerToggleLinked()
{
	KisImageSP img = currentImg();

	if (img) {
		KisLayerSP layer = img -> activeLayer();

		if (layer) {
			layer -> linked(!layer -> linked());
			m_doc -> setModified(true);
			layersUpdated();
		}
	}
}

void KisView::layerProperties()
{
	KisImageSP img = currentImg();

	if (img) {
		KisLayerSP layer = img -> activeLayer();

		if (layer) {
			KisPaintPropertyDlg dlg(layer -> name(), QPoint(layer -> x(), layer -> y()), layer -> opacity());

			if (dlg.exec() == QDialog::Accepted) { 
				QPoint pt = dlg.getPosition();
				bool changed = layer -> name() != dlg.getName();

				changed = changed || layer -> opacity() != dlg.getOpacity() || pt.x() != layer -> x() || pt.y() != layer -> y();

				if (changed)
					m_doc -> beginMacro(i18n("Property changes"));

				if (layer -> name() != dlg.getName() || layer -> opacity() != dlg.getOpacity())
					m_doc -> layerProperties(img, layer, dlg.getOpacity(), dlg.getName());

				if (pt.x() != layer -> x() || pt.y() != layer -> y())
					KisStrategyMove(this, m_doc).simpleMove(QPoint(layer -> x(), layer -> y()), pt);

				if (changed)
					m_doc -> endMacro();
			}
		}
	}
}

void KisView::layerAdd()
{
	KisImageSP img = currentImg();

	if (img) {
		KisConfig cfg;
		NewLayerDialog dlg(cfg.maxLayerWidth(), cfg.defLayerWidth(), cfg.maxLayerHeight() , cfg.defLayerHeight(), this);

		dlg.exec();

		if (dlg.result() == QDialog::Accepted) {
			KisLayerSP layer = m_doc -> layerAdd(img, dlg.layerWidth(), dlg.layerHeight(), img -> nextLayerName(), OPACITY_OPAQUE);

			if (layer) {
				m_layerBox -> setCurrentItem(img -> index(layer));
				resizeEvent(0);
				updateCanvas(layer -> x(), layer -> y(), layer -> width(), layer -> height());
			} else {
				KMessageBox::error(this, i18n("Could not add layer to image."), i18n("Layer Error"));
			}
		}
	}
}

void KisView::layerRemove()
{
	KisImageSP img = currentImg();

	if (img) {
		KisLayerSP layer = img -> activeLayer();

		if (layer) {
			Q_INT32 n = img -> index(layer);

			m_doc -> layerRemove(img, layer);
			m_layerBox -> setCurrentItem(n - 1);
			resizeEvent(0);
			updateCanvas();
			layerUpdateGUI(img -> activeLayer() != 0);
		}
	}
}

void KisView::layerDuplicate()
{
}

void KisView::layerAddMask(int /*n*/)
{
}

void KisView::layerRmMask(int /*n*/)
{
}

void KisView::layerRaise()
{
	KisImageSP img = currentImg();
	KisLayerSP layer;

	if (!img)
		return;

	layer = img -> activeLayer();

	if (layer) {
		m_doc -> layerRaise(img, layer);
		layersUpdated();
		resizeEvent(0);
		updateCanvas();
	}
}

void KisView::layerLower()
{
	KisImageSP img = currentImg();
	KisLayerSP layer;

	if (!img)
		return;

	layer = img -> activeLayer();

	if (layer) {
		m_doc -> layerLower(img, layer);
		layersUpdated();
		resizeEvent(0);
		updateCanvas();
	}
}

void KisView::layerFront()
{
	KisImageSP img = currentImg();
	KisLayerSP layer;

	if (img && (layer = img -> activeLayer())) {
		img -> top(layer);
		layersUpdated();
		resizeEvent(0);
		updateCanvas();
	}
}

void KisView::layerBack()
{
	KisImageSP img = currentImg();
	KisLayerSP layer;

	if (img && (layer = img -> activeLayer())) {
		img -> bottom(layer);
		layersUpdated();
		resizeEvent(0);
		updateCanvas();
	}
}

void KisView::layerLevel(int /*n*/)
{
}

void KisView::layersUpdated()
{
	KisImageSP img = currentImg();

	layerUpdateGUI(img && img -> activeLayer());
	m_layerBox -> setUpdatesEnabled(false);
	m_layerBox -> clear();

	if (img) {
		vKisLayerSP l = img -> layers();

		for (vKisLayerSP_it it = l.begin(); it != l.end(); it++)
			m_layerBox -> insertItem((*it) -> name(), (*it) -> visible(), (*it) -> linked());

		m_layerBox -> setCurrentItem(img -> index(img -> activeLayer()));
		m_doc -> setModified(true);
	}

	m_layerBox -> setUpdatesEnabled(true);
}

void KisView::layersUpdated(KisImageSP img)
{
	if (img == currentImg())
		layersUpdated();
}

void KisView::selectImage(const QString& name)
{
	disconnectCurrentImg();
	m_current = m_doc -> findImage(name);
	connectCurrentImg();
	layersUpdated();
	resizeEvent(0);
	updateCanvas();
}

void KisView::selectImage(KisImageSP img)
{
	disconnectCurrentImg();
	m_current = img;
	connectCurrentImg();
	layersUpdated();
	resizeEvent(0);
	updateCanvas();
	m_tabBar -> slotImageListUpdated();
}

void KisView::setPaintOffset()
{
	KisDlgPaintOffset dlg(m_xoff, m_yoff, this, "dlg_paint_offset");

	dlg.exec();

	if (dlg.result() == QDialog::Accepted && (dlg.xoff() != m_xoff || dlg.yoff() != m_yoff)) {
		m_xoff = dlg.xoff();
		m_yoff = dlg.yoff();
		resizeEvent(0);
	}
}

void KisView::scrollH(int value)
{
	m_hRuler -> setOffset(value);
	m_canvas -> repaint();
}

void KisView::scrollV(int value)
{
	m_vRuler -> setOffset(value);
	m_canvas -> repaint();
}

QWidget *KisView::canvas()
{
	return m_canvas;
}

int KisView::canvasXOffset() const
{
	return static_cast<Q_INT32>(static_cast<double>(m_xoff) * zoom());
}

int KisView::canvasYOffset() const
{
	return static_cast<Q_INT32>(static_cast<double>(m_yoff) * zoom());
}

void KisView::setupCanvas()
{
	m_canvas = new KisCanvas(this, "kis_canvas");

	QObject::connect(m_canvas, SIGNAL(mousePressed(QMouseEvent*)), this, SLOT(canvasGotMousePressEvent(QMouseEvent*)));
	QObject::connect(m_canvas, SIGNAL(mouseMoved(QMouseEvent*)), this, SLOT(canvasGotMouseMoveEvent(QMouseEvent*)));
	QObject::connect(m_canvas, SIGNAL(mouseReleased(QMouseEvent*)), this, SLOT(canvasGotMouseReleaseEvent(QMouseEvent*)));
	QObject::connect(m_canvas, SIGNAL(gotPaintEvent(QPaintEvent*)), this, SLOT(canvasGotPaintEvent(QPaintEvent*)));
	QObject::connect(m_canvas, SIGNAL(gotEnterEvent(QEvent*)), this, SLOT(canvasGotEnterEvent(QEvent*)));
	QObject::connect(m_canvas, SIGNAL(gotLeaveEvent(QEvent*)), this, SLOT(canvasGotLeaveEvent(QEvent*)));
	QObject::connect(m_canvas, SIGNAL(mouseWheelEvent(QWheelEvent*)), this, SLOT(canvasGotMouseWheelEvent(QWheelEvent*)));
	QObject::connect(m_canvas, SIGNAL(gotKeyPressEvent(QKeyEvent*)), this, SLOT(canvasGotKeyPressEvent(QKeyEvent*)));
	QObject::connect(m_canvas, SIGNAL(gotKeyReleaseEvent(QKeyEvent*)), this, SLOT(canvasGotKeyReleaseEvent(QKeyEvent*)));
}

void KisView::projectionUpdated(KisImageSP img)
{
	if (img == currentImg())
		canvasRefresh();
}

bool KisView::selectColor(KoColor& result)
{
	QColor color;
	bool rc;

	if ((rc = (KColorDialog::getColor(color) == KColorDialog::Accepted)))
		result.setRGB(color.red(), color.green(), color.blue());

	return rc;
}

void KisView::selectFGColor()
{
	KoColor c;

	if (selectColor(c))
		m_sideBar -> slotSetFGColor(c);
}

void KisView::selectBGColor()
{
	KoColor c;

	if (selectColor(c))
		m_sideBar -> slotSetBGColor(c);
}

void KisView::reverseFGAndBGColors()
{
	KoColor oldFg = m_fg;
	KoColor oldBg = m_bg;

	m_sideBar -> slotSetFGColor(oldBg);
	m_sideBar -> slotSetBGColor(oldFg);
}

void KisView::imgSelectionChanged(KisImageSP img)
{
	if (img == currentImg())
		selectionUpdateGUI(img -> selection() != 0);
}

void KisView::connectCurrentImg() const
{
	if (m_current) {
		connect(m_current, SIGNAL(selectionChanged(KisImageSP)), SLOT(imgSelectionChanged(KisImageSP)));
		connect(m_current, SIGNAL(update(KisImageSP, const QRect&)), SLOT(imgUpdated(KisImageSP, const QRect&)));
	}
}

void KisView::disconnectCurrentImg() const
{
	if (m_current)
		m_current -> disconnect(this);
}

void KisView::duplicateCurrentImg()
{
	KisImageSP img = currentImg();

	Q_ASSERT(img);

	if (img) {
		KisImageSP dubbed = new KisImage(*img);

		dubbed -> setName(m_doc -> nextImageName());
		m_doc -> addImage(dubbed);
	}
}

KoColor KisView::bgColor()
{
	return m_bg;
}

KoColor KisView::fgColor()
{
	return m_fg;
}

void KisView::setupClipboard()
{
	QClipboard *cb = QApplication::clipboard();

	connect(cb, SIGNAL(dataChanged()), SLOT(clipboardDataChanged()));
}

void KisView::clipboardDataChanged()
{
	m_clipboardHasImage = !QApplication::clipboard() -> image().isNull();
}

void KisView::imgUpdated(KisImageSP img, const QRect& rc)
{
	if (img == currentImg()) {
		QRect ur = rc;

		resizeEvent(0);
		ur.setX(ur.x() - static_cast<Q_INT32>(horzValue() * zoom()));
		ur.setY(ur.y() - static_cast<Q_INT32>(vertValue() * zoom()));
		updateCanvas(ur);
	}
}

void KisView::layerResize()
{
	KisImageSP img = currentImg();

	if (img) {
		KisLayerSP layer = img -> activeLayer();

		if (layer) {
			KisConfig cfg;
			KisDlgDimension dlg(cfg.maxLayerWidth(), layer -> width(), cfg.maxLayerHeight(), layer -> height(), this);

			if (dlg.exec() == QDialog::Accepted) {
				QSize size = dlg.getSize();

				layer -> resize(size.width(), size.height());
				img -> invalidate();
				layersUpdated();
				resizeEvent(0);
				canvasRefresh();
			}
		}
	}
}

void KisView::layerResizeToImage()
{
	KisImageSP img = currentImg();

	if (img) {
		KisLayerSP layer = img -> activeLayer();

		if (layer) {
			if (layer -> width() == img -> width() || layer -> height() == img -> height())
				return;

			layer -> resize(img -> width(), img -> height());
			img -> invalidate();
			layersUpdated();
			resizeEvent(0);
			canvasRefresh();
		}
	}
}

void KisView::layerScale()
{
}

QPoint KisView::viewToWindow(const QPoint& pt)
{
	QPoint r = pt;

	if (zoom() < 1.0 || zoom() > 1.0) {
		r.setX(r.x() + horzValue());
		r.setY(r.y() + vertValue());
		r /= zoom();
	}

	return r;
}

QRect KisView::viewToWindow(const QRect& rc)
{
	QRect r = rc;

	if (zoom() < 1.0 || zoom() > 1.0) {
		r.setX(static_cast<Q_INT32>((r.x() + horzValue()) * zoom()));
		r.setY(static_cast<Q_INT32>((r.y() + vertValue()) * zoom()));
		r.setWidth(static_cast<Q_INT32>(r.width() * zoom()));
		r.setHeight(static_cast<Q_INT32>(r.height() * zoom()));
	}

	return r;
}

void KisView::viewToWindow(Q_INT32 *x, Q_INT32 *y)
{
	if (zoom() < 1.0 || zoom() > 1.0) {
		*x += horzValue();
		*y += vertValue();
		*x /= static_cast<Q_INT32>(zoom());
		*y /= static_cast<Q_INT32>(zoom());
	}
}

QPoint KisView::windowToView(const QPoint& pt)
{
	QPoint r = pt;

	if (zoom() < 1.0 || zoom() > 1.0) {
		r *= zoom();
		r.setX(r.x() - horzValue());
		r.setY(r.y() - vertValue());
	}

	return r;
}

QRect KisView::windowToView(const QRect& rc)
{
	QRect r;

	if (zoom() > 1.0 || zoom() < 1.0) {
		r.setX(static_cast<Q_INT32>(rc.x() * zoom()) - horzValue());
		r.setY(static_cast<Q_INT32>(rc.y() * zoom()) - vertValue());
		r.setWidth(static_cast<Q_INT32>(rc.width() * zoom()));
		r.setHeight(static_cast<Q_INT32>(rc.height() * zoom()));
	}

	return r;
}

void KisView::windowToView(Q_INT32 *x, Q_INT32 *y)
{
	if (x && y) {
		*x *= static_cast<Q_INT32>(zoom());
		*y *= static_cast<Q_INT32>(zoom());
		*x -= horzValue();
		*y -= vertValue();
	}
}

void KisView::guiActivateEvent(KParts::GUIActivateEvent *event)
{
	KStatusBar *sb = statusBar();

	if (sb)
		sb -> show();

	super::guiActivateEvent(event);
}

void KisView::nBuilders(Q_INT32 size)
{
	m_imgImport -> setEnabled(size == 0);
	m_imgExport -> setEnabled(size == 0);

	if (m_imgScan)
		m_imgScan -> setEnabled(size == 0);
}

#include "kis_view.moc"

