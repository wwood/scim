/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#define USE_SCIM_KDE_SETTINGS
#include "inputwindow.h"

#include "scimlookuptable.h"
#include "scimlineedit.h"
#include "scimdragableframe.h"
#include "src/skimglobalactions.h"
#include "src/extra_utils.h"

#include <qlineedit.h>
#include "scimstringlistitem.h"
#include <kdebug.h>
#include <kiconloader.h>

#define TOP_LEVEL_WFLAGS \
   Qt::WStyle_Customize|Qt::WStyle_NoBorder|Qt::WStyle_StaysOnTop|Qt::WX11BypassWM|Qt::WStyle_Tool

inputWindow::inputWindow( SkimPluginManager* is, QWidget* parent, const char* name, WFlags fl )
    : ScimDragableFrame( parent, parent, name, fl ), m_inTransaction( false )
{
    setKeepVisible( TRUE );
    m_allModules = is;
    m_allModules->registerSpecialProperyObject(this);

    m_inputServer = m_allModules->getInputServer();

    setFrameStyle( QFrame::NoFrame );

//     ScreenRect = QApplication::desktop()->screenGeometry();
    setCaption(i18n("Input Window"));
    setName( "inputWindow" );
    setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum ) );
    m_inputWindowLayout = new QVBoxLayout( this, 0, 1, "m_inputWindowLayout");
    m_inputWindowLayout->setResizeMode( QLayout::FreeResize );

    m_preEditText = new ScimLineEdit( this, "m_preEditText" );
    m_preEditText->hide();
    m_preEditText->setBackgroundOrigin( QLineEdit::AncestorOrigin );
    m_preEditText->setMouseTracking( FALSE );
    m_preEditText->setAcceptDrops( FALSE );
    m_inputWindowLayout->addWidget( m_preEditText );

    m_auxText = new ScimStringListItem(this);
    m_auxText->setFrameStyle( QFrame::LineEditPanel | QFrame::Raised );

    m_inputWindowLayout->addWidget( m_auxText );
    m_auxText->hide();

    m_lookupLbl = new ScimLookupTable( this, m_inputWindowLayout, "m_lookupLbl",
      ScimKdeSettings::lookupTable_IsVertical());

    m_allModules->registerSpecialProperyObject(m_lookupLbl);

    m_lookupLbl->setCaption(i18n("Candidates Window (Lookup Table)"));

    m_lookupLbl->setFrameStyle( QFrame::StyledPanel | QFrame::Raised );

    m_inputWindowLayout->addWidget( m_lookupLbl );

    languageChange();
    clearWState( WState_Polished );

    // signals and slots connections
    connect( m_inputServer, SIGNAL( updatePreeditStringReq(const QString &, const scim::AttributeList &) ),
        this, SLOT( updatePreeditString(const QString &, const scim::AttributeList & ) ) );
    connect( m_inputServer,
             SIGNAL(updateLookupTableReq(const scim::LookupTable &, size_t &)),
             SLOT( updateLookupTable(const scim::LookupTable &, size_t & ) ) );
    connect( m_inputServer, SIGNAL( updateAuxStringReq(const QString &, const scim::AttributeList & ) ),
        this, SLOT(updateAuxString(const QString &, const scim::AttributeList & ) ) );

    connect( m_inputServer, SIGNAL( showAuxStringReq() ), this, SLOT( showAuxString() ) );
    connect( m_inputServer, SIGNAL( hideAuxStringReq() ), this, SLOT( hideAuxString() ) );
    connect( m_inputServer, SIGNAL( showLookupTableReq() ), this, SLOT( showLookupTable() ) );
    connect( m_inputServer, SIGNAL( showPreeditStringReq() ), this, SLOT( showPreeditString() ) );
    connect( m_inputServer, SIGNAL( hidePreeditStringReq() ), this, SLOT( hidePreeditString() ) );
    connect( m_inputServer, SIGNAL( hideLookupTableReq() ),
        this, SLOT( hideLookupTable() ) );
    connect( m_inputServer, SIGNAL( updateSpotLocationReq( int, int ) ),
        this, SLOT( updateSpotLocation( int, int ) ) );
    connect( m_inputServer, SIGNAL(updatePreeditCaretReq(int ) ),
        m_preEditText, SLOT(setCursorPosition ( int )));

    connect( m_inputServer, SIGNAL( turnOffPanelReq() ), this, SLOT( hideInputWindow() ) );

    connect( m_inputServer, SIGNAL( transaction_start() ), this, SLOT( disableUpdates() ) );
    connect( m_inputServer, SIGNAL( transaction_end() ), this, SLOT( enableUpdates() ) );

    connect( m_lookupLbl, SIGNAL( previousPageRequested() ), m_inputServer, SLOT( lookupTablePageUp() ) );
    connect( m_lookupLbl, SIGNAL( nextPageRequested() ), m_inputServer, SLOT( lookupTablePageDown() ) );
    connect( m_lookupLbl, SIGNAL(itemSelected(int )), m_inputServer, SLOT(selectLookupTableItem(int )));

    m_stickAction =
        static_cast<KToggleAction*>(m_allModules->getActionCollection()->action( "stick_inputwindow" ));

    changeSetting();

    m_lookupLbl->hide();
}

void inputWindow::disableUpdates()
{
    m_inTransaction = true;
    setUpdatesEnabled( false );
//     m_lookupLbl->setUpdatesEnabled( false );
}

void inputWindow::enableUpdates()
{
    setUpdatesEnabled( true );
//     m_lookupLbl->setUpdatesEnabled( true );
    if(!m_lookupLbl->isAttached() && !isVisible() && !m_sticky)
    {
        m_lookupLbl->move(m_insertPointX, m_insertPointY);
        m_lookupLbl->adjustSize();
    }

    m_inTransaction = false;

    try_hide();

    adjustSize();
}

inputWindow::~inputWindow(){
    ScimKdeSettings::setIs_Sticky(m_sticky);
    ScimKdeSettings::setEmbedded_Lookup_Table(m_lookupLbl->isAttached());
}

void inputWindow::hideInputWindow ()
{
    hide();

    if(!m_lookupLbl->isAttached())
        m_lookupLbl->hide();
}

void inputWindow::toggleStick()
{
    m_stickAction->setIcon( m_stickAction->isChecked() ? "pin_down": "pin_up" );
    m_sticky = m_stickAction->isChecked();
}

void inputWindow::changeSetting()
{
//     kdDebug(18010) << k_lineinfo << " inputWindow::changeSetting()\n";
    if(!ScimKdeSettings::iW_Font().isEmpty())
    {
      QFont customFont;
      customFont.fromString(ScimKdeSettings::iW_Font());
      if(customFont != font())
        setFont(customFont);
    } else
      unsetFont();

    m_lookupLbl->setVertical(ScimKdeSettings::lookupTable_IsVertical());

    if(ScimKdeSettings::embedded_Lookup_Table() != m_lookupLbl->isAttached())
        m_lookupLbl->switchMode();

    if(ScimKdeSettings::lookupTable_IsVertical() &&  !ScimKdeSettings::lookupTable_MinimumWidth())
        m_lookupLbl->setMinimumWidth(100);
    else
        m_lookupLbl->setMinimumWidth(0);

    m_sticky = ScimKdeSettings::is_Sticky();
    m_stickAction->setChecked(m_sticky);
    toggleStick();

    UPDATE_WINDOW_OPACITY(this);
}

void inputWindow::languageChange()
{
}

void inputWindow::updateAuxString( const QString & aux, const scim::AttributeList & attrs)
{
    m_auxText->setText( aux, attrs );
}

void inputWindow::showAuxString()
{
    m_auxText->show();
    if( !isVisible() )
        show();
    adjustSize();
}

void inputWindow::hideAuxString()
{
    m_auxText->hide();
    try_hide();
}

void inputWindow::updateSpotLocation( int x, int y )
{
    m_insertPointX = x; m_insertPointY = y;

    if(m_sticky)
        return;

    move(m_insertPointX, m_insertPointY);
//     adjustSize();
}
void inputWindow::updatePreeditString( const QString & newprestr, const scim::AttributeList & atl)
{
    m_preEditText->setText( newprestr, atl );
//     adjustSize();
}

void inputWindow::hidePreeditString()
{
    m_preEditText->hide();
    try_hide();
}

void inputWindow::showPreeditString()
{
    m_preEditText->show();
    if( !isVisible() )
      show();
    adjustSize();
}

void inputWindow::updateLookupTable(const scim::LookupTable &table, size_t & exact_item_num)
{
    size_t i;
    size_t item_num = table.get_current_page_size ();

    scim::String         mbs;
    scim::WideString     wcs, label;
    scim::AttributeList  attrs;
    QValueList<scim::AttributeList> attrslist;
    QStringList    items, labels;
    for (i = 0; i < SCIM_LOOKUP_TABLE_MAX_PAGESIZE; ++ i) {
        if (i < item_num) {
            mbs = scim::String ();

            wcs = table.get_candidate_in_current_page (i);

            label =  table.get_candidate_label (i);

            labels <<  QString::fromUtf8( scim::utf8_wcstombs (label).c_str() );

            mbs = scim::utf8_wcstombs (wcs);

            // Update attributes;
            attrs = table.get_attributes_in_current_page (i);
            attrslist.push_back(attrs);
            items << QString::fromUtf8( mbs.c_str() );
        }
    }

    //FIXME: the line below should be called after updateContent, but now it's a ugly hack
    //to push_front a highlight attribute to the attrslist
    m_lookupLbl->updateHighlight( table.is_cursor_visible () ? table.get_cursor_pos_in_current_page () : -1 );

    exact_item_num = m_lookupLbl->updateContent(labels, items, attrslist, table.is_page_size_fixed ());
    m_lookupLbl->enablePreviousPage(table.get_current_page_start ());
    m_lookupLbl->enableNextPage(table.get_current_page_start () + exact_item_num < table.number_of_candidates ());

    m_lookupLbl->adjustSize();
//     adjustSize();
}

bool inputWindow::can_hide (void)
{
    if (m_preEditText->isVisible() ||
        m_auxText->isVisible() ||
        (m_lookupLbl->isAttached() && m_lookupLbl->isVisible()))
        return false;
    return true;
}

void inputWindow::try_hide()
{
    if(!m_inTransaction)
        if(can_hide())
            hide();
}

void inputWindow::adjustSize()
{
    if(m_inTransaction)
        return;

    ScimDragableFrame::adjustSize();

    if(isVisible() && !m_lookupLbl->isAttached() && !m_sticky) {
        std::vector<QRect> places;
        QRect candidate;
        {
            //find all possible locations where lookuptable can be placed
            candidate = m_lookupLbl->frameGeometry();
            candidate.moveTopLeft(QPoint(m_insertPointX, frameGeometry().bottomLeft().y() + 5)); //below
            places.push_back(candidate);
            candidate = m_lookupLbl->frameGeometry();
            candidate.moveTopLeft(QPoint(frameGeometry().topRight().x() + 5, m_insertPointY)); //right
            places.push_back(candidate);
            candidate = m_lookupLbl->frameGeometry();
            candidate.moveTopRight(QPoint(frameGeometry().topLeft().x() - 5, m_insertPointY)); //left
            places.push_back(candidate);
        }

        for(uint i=0; i<places.size(); i++)
        {
            //this will verify that candidate is in the screen, if not, try next condidate
            screenContainsRect(places[i]);
            QRect intersect = places[i] & frameGeometry();
            if ( intersect.isEmpty() )
            {
                m_lookupLbl->move(places[i].topLeft());
                break;
            }
        }
    }

    return;
}

void inputWindow::hideLookupTable()
{
    m_lookupLbl->hide();
    try_hide();
}

void inputWindow::showLookupTable()
{
    m_lookupLbl->show();
    if( m_lookupLbl->isAttached() )
    {
        show();
        adjustSize();
    } else if(!isVisible() && !m_sticky) {
        if(!m_inTransaction)
            m_lookupLbl->move(m_insertPointX, m_insertPointY);
    }
}

#include <kgenericfactory.h>
typedef KGenericFactory<InputWindowPlugin> InputWindowPluginFactory;
K_EXPORT_COMPONENT_FACTORY( skimplugin_inputwindow,
                              InputWindowPluginFactory( "skimplugin_inputwindow" ) )

InputWindowPlugin::InputWindowPlugin( QObject *parent, const char *name, const QStringList & /*args*/ )
 : SkimPlugin(InputWindowPluginFactory::instance(), parent, name)
{
    m_inputw = new inputWindow(static_cast<SkimPluginManager *>(parent), 0,0, TOP_LEVEL_WFLAGS);
    connect( this, SIGNAL(settingsChanged()), m_inputw, SLOT(changeSetting()));
}

void InputWindowPlugin::toggleStick() {
    m_inputw->toggleStick();
}

InputWindowPlugin::~InputWindowPlugin()
{
    delete m_inputw;
}

#include "inputwindow.moc"
