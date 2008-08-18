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
#include "src/skimpluginmanager.h"
#include "mainwindow.h"

#include <qlayout.h>
#include <qevent.h>
#include <qtoolbutton.h>
#include <qlabel.h>

#include <kpopupmenu.h>
#include <ktoolbar.h>
#include <kiconloader.h>
#include <kxmlguifactory.h>
#include <kapplication.h>
#include <kdebug.h>

#include "utils/scimtoolbar.h"
#include "utils/scimmovehandle.h"
#include "src/skimglobalactions.h"
#include "utils/scimxmlguibuilder.cpp"
#include "utils/scimactions.h"
#include "src/extra_utils.h"

#include <dcopref.h>
#include <dcopclient.h>

#include <kdeversion.h>
#define KDEGT340 (KDE::version() >= KDE_MAKE_VERSION(3,4,0))

#define ACTIONLIST_FRONTEND "Frontend Properties"
#define ACTIONLIST_GUI "GUI Properties"
#include <qxembed.h>

MainWindow::MainWindow( QWidget* parent, const char* name, WFlags fl )
    : ScimDragableFrame( parent, parent, name, fl ),
    ScimXMLGUIClient(SkimPluginManager::self()->getActionCollection()), DCOPObject("Skim_MainWindow"),
    m_autoHideTimeoutTimer(0), m_showHandleTimer(0), m_showExtHandleTimer(0),m_mode(PanelEmbedded),
    m_embedInitialized(false), m_contentIsVisible( false ), m_panelTurnedOn( false ),
    m_updatePropertiesNeeded(false), m_applet_direction(Qt::Horizontal), 
    m_shouldChangeDirection(false), m_logo(0) {

    QXEmbed::initialize ();

//     setWFlags(Qt::WStyle_Customize | Qt::WX11BypassWM | Qt::WStyle_StaysOnTop);
    setKeepVisible( true );
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_allModules = SkimPluginManager::self();

    m_allModules->registerSpecialProperyObject(this);

    m_inputServer = m_allModules->getInputServer();
    ScimXMLGUIBuilder* guiBuilder = new ScimXMLGUIBuilder(this);
    m_guiFactory = new KXMLGUIFactory(guiBuilder, this);

    setCaption(i18n("Main Toolbar"));

    m_serverAction =
      dynamic_cast<ScimAction *>(m_allModules->getActionCollection()->action( "change_server" ));

    m_mainWindowLayout = new QBoxLayout( this, QBoxLayout::LeftToRight, -1, 0, "m_mainWindowLayout" );
    m_mainWindowLayout->setAlignment(Qt::AlignCenter);
    m_mainWindowLayout->setResizeMode(QLayout::FreeResize);
    m_moveHandle = new ScimMoveHandle( this, this );
    m_moveHandle->setKeepVisible( false );
    m_mainWindowLayout->addWidget( m_moveHandle );

    //FIXME:qpopupmenu will steal focus from current editing widget, however 
    //kpopupmenu won't. this should be a bug in Qt
    m_contextMenu = new KPopupMenu(this, "MainToolBar_RightClickMenu");

    createToolbar();

    m_defaultActionCollection = m_allModules->getActionCollection();

    m_toggleDockingAction =
            static_cast<KToggleAction*>(m_defaultActionCollection->action( "toggledocking_mainwindow" ));
    connect( m_moveHandle, SIGNAL( doubleClicked() ), m_toggleDockingAction, SLOT( activate() ) );

    clearWState( WState_Polished );

    if(m_mode == StandAlone)
        move( ScimKdeSettings::mainWindow_Position());

    changeSetting();

    #if 0 && ENABLE_DEBUG
    KAction * reloadaction;
    //we should remove the old one first if exist
    if(reloadaction = m_defaultActionCollection->action( "reload_wm" ))
        delete reloadaction;

    reloadaction = new KAction( KGuiItem( i18n( "Reload" ), "reload" ),
                "", this, SLOT(requestReloadSelf()), m_defaultActionCollection, "reload_wm");

    reloadaction->plug(m_toolbar);
    #endif

    connect(m_defaultActionCollection, SIGNAL(propertiesRegistered(bool)), SLOT(updateProperties (bool)));
    connect(m_defaultActionCollection, SIGNAL(propertiesRemoved(bool)), SLOT(updateProperties (bool)));
    connect(m_defaultActionCollection, SIGNAL(propertyChanged(bool)), SLOT(updateProperties (bool)));
    connect(m_inputServer, SIGNAL( turnOnPanelReq() ), SLOT( slotTurnOn() ));
    connect(m_inputServer, SIGNAL( turnOffPanelReq() ), SLOT( slotTurnOff() ));

    #if ENABLE_DEBUG
    show();
    #endif

    connect(m_allModules->getActionCollection(), SIGNAL(standaloneHelperActionsChanged()),
        this, SLOT(initContextMenu()));

    //fix bug: https://bugzilla.novell.com/show_bug.cgi?id=149541
    //KXMLGUIClient keeps a list of pluged actions, if actions are deleted, this list
    //won't be changed, so we have to unplug all actions before remove any actions 
    connect( m_defaultActionCollection, SIGNAL( preparePropertiesRemove(bool) ), SLOT( emptyToolbar(bool) ) );

    initContextMenu();
}

MainWindow::~MainWindow() {
    if(m_mode == StandAlone)
    {
        ScimKdeSettings::setMainWindow_Position(pos());
        ScimKdeSettings::setMainWindow_Direction( (int)m_mainWindowLayout->direction() );
    }

    m_contextMenu->clear();
}

void MainWindow::emptyToolbar(bool isFrontEndProperties)
{
    if(isFrontEndProperties)
        unplugActionList(ACTIONLIST_FRONTEND);
    else
        unplugActionList(ACTIONLIST_GUI);
}

void MainWindow::initContextMenu() {
    m_contextMenu->clear();
    //load helper actions (copied from kdetraystatus.cpp, with slight modification)
    //TODO: add a new function in m_defaultActionCollection to handle this?
    KActionCollection * helperActioncol = m_defaultActionCollection->helperActionCollection();
    if ( helperActioncol->count() ) {
        for(size_t i=0; i<helperActioncol->count();i++) {
            helperActioncol->action(i)->plug(m_contextMenu);
        }
        m_contextMenu->insertSeparator();
    }

    m_toggleDockingAction->plug(m_contextMenu);

    KAction* configureaction = m_defaultActionCollection->action( "configure" );
    if(configureaction)
        configureaction->plug(m_contextMenu);

    KAction* helpAction = m_defaultActionCollection->action( "help" );
    if(helpAction)
        helpAction->plug(m_contextMenu);
}

void MainWindow::createToolbar () {
      m_toolbar = new ScimToolBar( this, "MainToolBar" );
      m_toolbar->setIconSize(fontMetrics().height() + 2);
      m_toolbar->setEnableContextMenu(true);
      m_toolbar->setFrameStyle( QFrame::NoFrame );
      m_toolbar->setBackgroundOrigin(QWidget::AncestorOrigin);
      m_toolbar->setBackgroundMode(QWidget::X11ParentRelative);
      m_toolbar->boxLayout()->setAlignment(Qt::AlignCenter);
      m_mainWindowLayout->addWidget( m_toolbar );
}

void MainWindow::updateProperties (bool isFrontEndProperties) {
    KActionPtrList exta = isFrontEndProperties ?
            m_defaultActionCollection->frontendPropertyActions() : m_defaultActionCollection->guiPropertyActions();
    m_actionListName = isFrontEndProperties ? ACTIONLIST_FRONTEND : ACTIONLIST_GUI;

    m_insertedActions.clear();
    for(uint i=0; i<exta.size(); i++) {
        bool visible = true;
        if(ScimAction * action = dynamic_cast<ScimAction *>(exta[i])){
            if(!action->visible() || !action->currentShown())
                visible = false;
        }
        if(visible)
            m_insertedActions.append(exta[i]);
    }

    if(m_insertedActions.count())
        m_updatePropertiesNeeded = true;

    unplugActionList(m_actionListName);

    if(m_updatePropertiesNeeded && m_contentIsVisible)
        show();

    //FIXME: see the FIXME in MainWindow::show()
    if(isVisible())
        QTimer::singleShot( 100, this, SLOT( adjustSize() ) );
}

void MainWindow::changeDirection(QBoxLayout::Direction d)
{
    m_shouldChangeDirection = false;

    m_mainWindowLayout->setDirection( d );
    m_moveHandle->setDirection( d );
    m_toolbar->boxLayout()->setDirection( d );
}

void MainWindow::requestReloadSelf () {
    setUpdatesEnabled( FALSE );
    m_guiFactory->removeClient(this);
    if(m_serverAction)
    m_serverAction->setOption(
        ScimKdeSettings::show_Factory_Text()?
        scim_kde::ToolBarIconUseText : scim_kde::ToolBarPreferIcon);
    setXMLFile("mainwindowui.rc");
    setXMLGUIBuildDocument( QDomDocument() );
    m_guiFactory->addClient(this);
    m_insertedActions.clear();
    updateProperties (true);
    updateProperties (false);
    setUpdatesEnabled( TRUE );
    adjustSize();
}

void MainWindow::adjustSize() {
    //only do the real adjustSize when we are in StandAlone mode
    if( m_mode == StandAlone && isUpdatesEnabled())
    {
        if(!isMouseButtonPressed() && m_shouldChangeDirection)
            changeDirection(m_changeToDirection);
        ScimDragableFrame::adjustSize();
        if(m_autoSnap && !isMouseButtonPressed()) {
        //     std::cout << "Successfully exited.\n" << screen.x() << " " <<screen.height()  << " " << x() << "\n";
            if(m_screen.width() - x() - width() < x())
                move(m_screen.width() - width(), y());
            else
                move(0, y());
        }
    }
//     else
//         QFrame::adjustSize();
}

void MainWindow::contextMenuEvent ( QContextMenuEvent * e) {
    m_contextMenu->popup(e->globalPos());
}

void MainWindow::slotTurnOn()
{
    show();
    m_panelTurnedOn = true;
}

void MainWindow::show() {
    if (m_updatePropertiesNeeded) {
        plugActionList(m_actionListName.latin1(), m_insertedActions);
        m_updatePropertiesNeeded=false;
        //FIXME: ugly hack: at this point, the m_toolbar has not been changed,
        // so if call adjustSize here will take no effect
        // This is a work around although may be not feasible in some old/slow
        // or heavily loaded machines. IMPROVEMENT for this work around: implement
        // a check in adjustSize, if 100 ms is not long enough, just start
        // another singleShot
        if(isUpdatesEnabled())
            QTimer::singleShot( 100, this, SLOT( adjustSize() ) );
    }

//     if(isUpdatesEnabled())
    {
        if(m_mode == StandAlone && m_autoHideTimeoutTimer)
        {
            m_autoHideTimeoutTimer->stop();
            m_toolbar->show();
            m_moveHandle->show();
            m_logo->hide();
        }

        QFrame::show();

        //adjustSize immediately is not useful here, so we schedule
        //an adjustSize after all the events in the queue are processed
        QTimer::singleShot( 0, this, SLOT( adjustSize() ) );
    }
    m_contentIsVisible = true;
}

void MainWindow::standaloneModeHide()
{
    if(m_mode == StandAlone)
        if(m_alwaysShow)
            hideToolbar();
        else
        {
            hide();
            m_contentIsVisible = false;
        }
}

void MainWindow::slotTurnOff() {
    m_panelTurnedOn = false;
    standaloneModeHide();
}

void MainWindow::hideToolbar()
{
    if( m_contentIsVisible )
        if( m_autoHideTimeoutTimer && !m_autoHideTimeoutTimer->isActive())
            m_autoHideTimeoutTimer->start( m_autoHideTimeout, true );

    if( m_autoHideTimeoutTimer && !m_autoHideTimeoutTimer->isActive())
    {
        m_toolbar->hide();
        m_moveHandle->hide();
        m_logo->show();
        adjustSize();
    }

    m_contentIsVisible = false;
}

void MainWindow::changeSetting()
{
    m_toggleDockingAction->setChecked(ScimKdeSettings::dockingToPanelApplet());
    toggleDocking(true); //retrieve the m_mode first

    if(m_mode == StandAlone)
    {
        //re-initialize embed if next time toggle to that docking mode
        m_embedInitialized = false;

        setName("mainWindow");
        reparent(0, Qt::WStyle_Customize | Qt::WX11BypassWM | Qt::WStyle_StaysOnTop,
                 ScimKdeSettings::mainWindow_Position(),false);

        m_moveHandle->show();
        setFrameStyle( QFrame::PopupPanel | QFrame::Raised );
        //if margin is 0, the frame can not be drawn correctly, frameWidth() is
        // sometimes too big (>1 and the main m_toolbar is not so decent), so
        // just fix it to 1
        m_mainWindowLayout->setMargin(1);
        setBackgroundOrigin(QWidget::WindowOrigin);
    }
    else
    {
        //change name so that it won't be listed in the composite management page
        setName("mainWindow_embedded");
        setWFlags(Qt::WStyle_Customize);

        m_moveHandle->hide();
        setFrameStyle( QFrame::NoFrame );
        m_mainWindowLayout->setMargin(0);
        setBackgroundOrigin(QWidget::AncestorOrigin);
        setBackgroundMode(QWidget::X11ParentRelative);
    }

    m_autoHideTimeout = ScimKdeSettings::hide_Timeout() * 1000;
    m_alwaysShow = ScimKdeSettings::always_Show();
    m_autoSnap = ScimKdeSettings::auto_Snap();
    requestReloadSelf();

    if( m_alwaysShow && m_autoHideTimeout > 0 && m_mode == StandAlone) {
        if(!m_autoHideTimeoutTimer)
        {
            m_autoHideTimeoutTimer = new QTimer(this);
            connect( m_autoHideTimeoutTimer, SIGNAL( timeout() ), this, SLOT( hideToolbar() ) );
        }
    } else {
        if(m_autoHideTimeoutTimer)
            m_autoHideTimeoutTimer->deleteLater();
        m_autoHideTimeoutTimer = 0;
        m_toolbar->show();
        if(m_mode == StandAlone)
            m_moveHandle->show();
    }

    m_alwaysShowHandle = ScimKdeSettings::alwaysShowHandle();

    m_alwaysShowExtensionHandle = ScimKdeSettings::alwaysShowExtensionHandle();

    if(m_mode == PanelEmbedded)
    {
        //init m_alwaysShowHandle option support timer
        if(m_alwaysShowHandle)
        {
            if(m_showHandleTimer)
                m_showHandleTimer->deleteLater();
            m_showHandleTimer = 0;
        }
        else
        {
            if(!m_showHandleTimer)
            {
                m_showHandleTimer = new QTimer(this);
                connect(m_showHandleTimer, SIGNAL(timeout()), this, SLOT(showHandleRequest()));
            }
        }

        //init m_alwaysShowExtensionHandle option support timer
        if(m_alwaysShowExtensionHandle)
        {
            if(m_showExtHandleTimer)
                m_showExtHandleTimer->deleteLater();
            m_showExtHandleTimer = 0;
            if(m_embeddedAreaSize.isValid())  //only reset m_toolbar size if m_embeddedAreaSize is set before
                resetToolbarSize(m_embeddedAreaSize);
        }
        else
        {
            if(!m_showExtHandleTimer)
            {
                m_showExtHandleTimer = new QTimer(this);
                connect(m_showExtHandleTimer, SIGNAL(timeout()), this, SLOT(showExtHandleRequest()));
            }
        }

        initEmbedPanel();
    }

    if(m_mode == StandAlone)
    {
        if(m_alwaysShow)
        {
            if(m_autoHideTimeoutTimer && !m_logo)
            {
                m_logo = new QLabel(this);
                m_logo->setPixmap(KGlobal::iconLoader()->loadIcon("skim", KIcon::Toolbar));
                m_mainWindowLayout->addWidget(m_logo);
                m_logo->hide();
            }
            QTimer::singleShot(0, this, SLOT(show()));//FIXME
        } else if(m_panelTurnedOn)
            QTimer::singleShot(0, this, SLOT(show()));//FIXME

        if((!m_alwaysShow || !m_autoHideTimeoutTimer) && m_logo)
        {
            m_logo->deleteLater();
            m_logo = 0;
        }

        UPDATE_WINDOW_OPACITY(this);
    }

    if(m_mode == StandAlone || (!ScimKdeSettings::force_LeftToRight_In_Kicker() && m_mode == PanelEmbedded))
        changeDirection((QBoxLayout::Direction)(ScimKdeSettings::mainWindow_Direction()));
    else
        changeDirection(QBoxLayout::LeftToRight);

    if(m_mode == StandAlone || (!m_alwaysShowExtensionHandle && m_mode == PanelEmbedded))
    {
        resetToolbarSize();
    }
}

void MainWindow::reInit()
{
    changeSetting();
}

void MainWindow::initEmbedPanel()
{
    static int kicker_start_timeout_counter = 0, load_skimapplet_timeout_counter = 0;

    if(kapp->dcopClient()->isApplicationRegistered("kicker"))
    {
        kicker_start_timeout_counter = 0;
        if(kapp->dcopClient()->remoteObjects("kicker").contains("SkimApplet"))
        {
            load_skimapplet_timeout_counter = 0;
            DCOPRef m_kickerSkimApplet("kicker","SkimApplet");

            if(!m_embedInitialized)
            {
                m_embedInitialized = true;

                disconnectDCOPSignal("kicker", "SkimApplet",
                                     "preferedSizeChanged(QSize, int)", "changePreferedSize(QSize, int)");
                disconnectDCOPSignal("kicker", "SkimApplet",
                                     "appletDestroyed(bool)", "appletDestroyed(bool)");
                disconnectDCOPSignal("kicker", "SkimApplet",
                                     "doubleCliked()", "toggleDocking()");

                connectDCOPSignal("kicker", "SkimApplet",
                                  "preferedSizeChanged(QSize, int)", "changePreferedSize(QSize, int)", false);
                connectDCOPSignal("kicker", "SkimApplet",
                                  "appletDestroyed(bool)", "appletDestroyed(bool)", false);
                connectDCOPSignal("kicker", "SkimApplet",
                                  "doubleCliked()", "toggleDocking()", false);

                m_kickerSkimApplet.call("embedWindow(Q_UINT32)", (Q_UINT32)winId());
            }

            m_kickerSkimApplet.call("setAutoHideHandle(bool)", !m_alwaysShowHandle);
            show();
        }
        else
        {
            if(load_skimapplet_timeout_counter < 1)
            {
                DCOPRef m_kickerPanel("kicker","Panel");
                m_kickerPanel.call("addApplet(QString)", QString("skimapplet.desktop"));

                // if this is KDE 3.3.x, then we have to restart kicker after we add
                // the new applet, otherwise, the new added applet will be a separate
                // dcop object, rather than an object under kicker.
                if(!KDEGT340)
                    m_kickerPanel.call("restart()");
            }

            if(load_skimapplet_timeout_counter<20)
            {
                QTimer::singleShot(500, this, SLOT(initEmbedPanel()));
                load_skimapplet_timeout_counter++;
            }
            else
            {
                //TODO: tell user: timeout when waiting for kicker to load SkimApplet
                kdDebug() << k_lineinfo << " timeout when waiting for kicker to load SkimApplet\n";
                toggleDocking();
            }
        }
    }
    else    //waiting for kicker to start
    {
        if(kicker_start_timeout_counter < 1)
        {
            kapp->dcopClient()->setNotifications(true);
            connect(kapp->dcopClient(), SIGNAL(applicationRegistered (const QCString &)),
                    SLOT(slotApplicationRegistered (const QCString &)));

            //this timer will make sure if kicker is not started within 50s, the initEmbedPanel
            // will be called. ATTENTION here: if no timeout occur, this will init another call
            // to initEmbedPanel(). However as we have m_embedInitialized variable, there should
            // be no problems.
            QTimer::singleShot(50000, this, SLOT(initEmbedPanel()));
            kicker_start_timeout_counter++;
        }
        else
        {
            //TODO: tell user: timeout when waiting for kicker to start
            kdDebug() << k_lineinfo << " timeout when waiting for kicker to start\n";
            slotApplicationRegistered("kicker");    //ATTENTION: call this to disconnect the applicationRegistered signal
            toggleDocking();
        }
    }
}

void MainWindow::slotApplicationRegistered (const QCString & appid)
{
    if(appid == "kicker")
    {
        kapp->dcopClient()->setNotifications(false);
        disconnect(kapp->dcopClient(), SIGNAL(applicationRegistered (const QCString &)),
                   this, SLOT(slotApplicationRegistered (const QCString &)));
        if(kapp->dcopClient()->isApplicationRegistered("kicker"))
            QTimer::singleShot(1000, this, SLOT(initEmbedPanel())); //add one second to wait for kicker to load skimapplet
    }
}

void MainWindow::appletDestroyed(bool appClosing)
{
    if(m_mode == PanelEmbedded)
    {
        m_embedInitialized = false;
        reparent(0, ScimKdeSettings::mainWindow_Position(), false);
        hide();
        if(appClosing)  
        {
            //TODO: kicker is shutting down, may be caused by logout of KDE, crash of kicker or killed by users
            toggleDocking();
            //FIXME: ugly hack: why the m_toolbar is so small after the previous call? And why call adjustSize here
            //won't do any good either: so we have to use a timer. This may be a potential bug
            QTimer::singleShot(1000, this, SLOT(adjustSize()));
        }
        else
        {   //TODO:applet is closed by user, ask what he want to do
            QTimer::singleShot(1000, this, SLOT(initEmbedPanel())); //reload
        }
    }
//     kdDebug() << k_lineinfo << " with para appClosing = " << appClosing << "\n";
}

void MainWindow::changePreferedSize(QSize newsize, int d)
{
    Q_UNUSED( d );

    if( m_mode == PanelEmbedded )
    {
        m_embeddedAreaSize = newsize;

        //only Horizontal m_toolbar can have the extension handle correctly
        //displayed, so currently just fix applet direction to Horizontal
        if(ScimKdeSettings::force_LeftToRight_In_Kicker())
        {
            m_applet_direction = Qt::Horizontal; //(Qt::Orientation)d;
        }

        settleToolbar();
    }
}

void MainWindow::settleToolbar()
{
    if(!isVisible())
    {
        QTimer::singleShot(200, this, SLOT(settleToolbar()));
        return;
    }

    QSize as = m_embeddedAreaSize;
    if(!m_alwaysShowExtensionHandle)
        if(m_applet_direction == Qt::Horizontal)
            as.setWidth(2000);

    resetToolbarSize(as);
}

#define HandleTimeout 200
#define ExtHandleTimeout 300

void MainWindow::startDockingTimer()
{
    if(!m_alwaysShowHandle && m_showHandleTimer)
    {
        //FIXME: do we need to make this time customizable
        m_showHandleTimer->start(HandleTimeout, true);
    }
    else if(!m_alwaysShowExtensionHandle && m_showExtHandleTimer)
        m_showExtHandleTimer->start(ExtHandleTimeout, true);
}

void MainWindow::enterEvent ( QEvent * e )
{
    if(!isMouseButtonPressed())
    {
        if(m_mode == PanelEmbedded)
        {
            setMouseTracking(true); //so we can receive mouseMoveEvent
            startDockingTimer();
        }
        else  //m_mode == StandAlone
        {
            if(m_alwaysShow && m_logo)
                show();
        }
    }
    ScimDragableFrame::enterEvent ( e );
}

void MainWindow::mouseMoveEvent ( QMouseEvent * e )
{
    if(m_mode == PanelEmbedded)
        if(!isMouseButtonPressed() && hasMouseTracking())
        {
            startDockingTimer();
        }

    ScimDragableFrame::mouseMoveEvent(e);

    if(m_mode == StandAlone)
    {
        if(ScimKdeSettings::auto_Adjust_Direction())
        {
            QRect visiblerect = frameGeometry();
            if( !m_shouldChangeDirection && !screenContainsRect(visiblerect) )
            {
                //some part of this mainwindow is invisible: we check if less than half
                // of the mainwindow is visible, if so try to change the direction
                QRect intersect = m_screen & frameGeometry();
                QRect frect = frameGeometry();
                double visible_radio = (double)(intersect.width() * intersect.height()) / (frect.width() * frect.height());
                if(!intersect.isEmpty() && visible_radio < 0.48)
                {
                    m_shouldChangeDirection = true;
                    switch(m_mainWindowLayout->direction())
                    {
                        case QBoxLayout::LeftToRight:
                            m_changeToDirection = QBoxLayout::TopToBottom;
                            break;
                        case QBoxLayout::TopToBottom:
                            m_changeToDirection = QBoxLayout::RightToLeft;
                            break;
                        case QBoxLayout::RightToLeft:
                            m_changeToDirection = QBoxLayout::BottomToTop;
                            break;
                        case QBoxLayout::BottomToTop:
                            m_changeToDirection = QBoxLayout::LeftToRight;
                            break;
                    }
                }
            }
        }
    }
}

void MainWindow::showHandleRequest()
{
    if( m_mode == PanelEmbedded )
    {
        if(!m_alwaysShowHandle)
        {
            DCOPRef m_kickerSkimApplet("kicker","SkimApplet");
            m_kickerSkimApplet.call("slotEnterEvent()");

            setMouseTracking(false);
        }

        //only after the handle is shown, we can start the timer of m_showExtHandleTimer
        //otherwise, the ext handle can not be shown reliably. Here ExtHandleTimeout
        //must be long enough for the m_kickerSkimApplet.call to take effective
        if(!m_alwaysShowExtensionHandle && m_showExtHandleTimer)
            m_showExtHandleTimer->start(ExtHandleTimeout, true);
    }
}

void MainWindow::showExtHandleRequest()
{
    if(!m_alwaysShowExtensionHandle)
        resetToolbarSize(m_embeddedAreaSize);
}

void MainWindow::resetToolbarSize (QSize newsize)
{
    bool isHorizontal = false;
    if( (m_applet_direction == Qt::Horizontal && m_mode == PanelEmbedded) ||
         m_mode == StandAlone && (m_mainWindowLayout->direction() == QBoxLayout::LeftToRight ||
         m_mainWindowLayout->direction() == QBoxLayout::RightToLeft))
        isHorizontal = true;

    if(!newsize.isValid())
        if(isHorizontal)
            newsize = QSize(2000, 10);
        else
            newsize = QSize(10, 2000);

    if(isHorizontal)
    {
        m_toolbar->setMaximumWidth(newsize.width() - 2);
        m_toolbar->setMinimumHeight(newsize.height() - 2);
    }
    else
    {
        m_toolbar->setMinimumWidth(newsize.width() - 2);
        m_toolbar->setMaximumHeight(newsize.height() - 2);
    }
}

void MainWindow::leaveEvent ( QEvent * e )
{
    setMouseTracking(false);    //we must disable the mousetracking as soon as possible here
    if(!isMouseButtonPressed())
    {
        if(m_mode == PanelEmbedded && (!m_alwaysShowHandle || !m_alwaysShowExtensionHandle))
        {
            //we have to use a timer, otherwise when the user move the mouse
            //to a popupmenu of mainwindow, if the leaveEvent is processed
            //immediately, the popupmenu will be hiden before the user have
            //a chance to go into it
            QTimer::singleShot(100, this, SLOT(slotLeaveEvent()));
        }
        else if(m_mode == StandAlone && m_alwaysShow && m_autoHideTimeoutTimer && !m_panelTurnedOn)
        {
            hideToolbar();
        }
    }

    ScimDragableFrame::enterEvent ( e );
}

#include <qobjectlist.h>
void MainWindow::slotLeaveEvent()
{
    //FIXME: ugly hack to make sure the user can move to a popup menu
    //of this widget, when in docking mode and auto hiding of the handle 
    //and ext handle
    QPopupMenu * mouseContainer = 0;
    QObjectList *l = queryList( "QPopupMenu" );
    QObjectListIt it( *l ); // iterate over the buttons
    QObject *obj;

    while ( (obj = it.current()) != 0 ) {
        ++it;
        if(((QPopupMenu*)obj)->hasMouse())
        {
            mouseContainer = (QPopupMenu*)obj;
            break;
        }
    }
    delete l;
    
    if(!mouseContainer)
    {
        hideHandleRequest();
    } else
    {
        mouseContainer->disconnect(this);
        connect(mouseContainer, SIGNAL(aboutToHide()), this, SLOT(hideHandleRequest()));
    }
}

void MainWindow::hideHandleRequest()
{
    if(m_showHandleTimer)
    {
        m_showHandleTimer->stop();
        if(!m_alwaysShowHandle)
        {
            DCOPRef m_kickerSkimApplet("kicker","SkimApplet");
            m_kickerSkimApplet.call("slotLeaveEvent()");
        }
    }
    if(m_showExtHandleTimer)
    {
        m_showExtHandleTimer->stop();
        if(!m_alwaysShowExtensionHandle)
        {
            resetToolbarSize();
        }
    }
}

void MainWindow::toggleDocking()
{
    m_toggleDockingAction->activate();
}

void MainWindow::toggleDocking(bool internal)
{
    ToolBarModeType oldMode = m_mode;
    m_mode = m_toggleDockingAction->isChecked() ? PanelEmbedded : StandAlone;

    m_toggleDockingAction->setIcon( m_toggleDockingAction->isChecked() ? "skim_restore": "skim_minimize" );

    if(!internal)
    {
        //as the detach/attach requires some time to finish
        //it is better to hide self first. changeSetting()
        //will re-show this if required
        if( m_mode != oldMode )
            hide();

        //if original mode is StandAlone, then save current direction before change to embeded mode
        if( oldMode == StandAlone )
            ScimKdeSettings::setMainWindow_Direction((int)m_mainWindowLayout->direction());

        ScimKdeSettings::setDockingToPanelApplet(m_toggleDockingAction->isChecked());
        changeSetting();
        updateProperties (true);
        updateProperties (false);
    }
}

#include <kgenericfactory.h>
typedef KGenericFactory<MainWindowPlugin> MainWindowPluginFactory;

K_EXPORT_COMPONENT_FACTORY( skimplugin_mainwindow,
  MainWindowPluginFactory( "skimplugin_mainwindow" ) )

MainWindowPlugin::MainWindowPlugin( QObject *parent, const char *name, const QStringList & /*args*/ )
    : SkimPlugin(MainWindowPluginFactory::instance(), parent, name)
{
    Qt::WidgetFlags flags = Qt::WStyle_Customize;
    if(!ScimKdeSettings::dockingToPanelApplet())
        flags = (Qt::WidgetFlags)(Qt::WStyle_Customize | Qt::WX11BypassWM | Qt::WStyle_StaysOnTop);

    mainwindow = new MainWindow(0, "mainWindow", flags);
    connect( this, SIGNAL(settingsChanged()), mainwindow, SLOT(changeSetting()));
}

void MainWindowPlugin::toggleDocking()
{
    //do not call mainwindow->toggleDocking(void), because it will
    //toggle the action again. While this function is called
    //after the action has been toggled already
    mainwindow->toggleDocking(false);
}

MainWindowPlugin::~MainWindowPlugin()
{
    delete mainwindow;
}

#include "mainwindow.moc"
