/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "socketserverthread.h"
#include "utils/scimxmlguibuilder.h"
#include "utils/scimdragableframe.h"
#include <dcopobject.h>
#include <qlayout.h>

class ScimToolBar;
class ScimMoveHandle;
class SkimPluginManager;
class SkimGlobalActions;
class ScimAction;

class QPopupMenu;
class QLabel;

class MainWindow : public ScimDragableFrame, public ScimXMLGUIClient, public DCOPObject
{
    Q_OBJECT
    K_DCOP
public:
    enum ToolBarModeType {StandAlone=0, PanelEmbedded};
    MainWindow( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~MainWindow();

    void toggleDocking(bool internel);
    ScimMoveHandle* m_moveHandle;

k_dcop:
    void changePreferedSize(QSize, int d);
    void appletDestroyed(bool);
    void toggleDocking();
    void reInit();

public slots:
    void show();
//     void hide();
    void adjustSize();
    void requestReloadSelf ();
    void changeSetting();
    void updateProperties (bool);

private:
    void createToolbar ();
    void resetToolbarSize (QSize newsize = QSize());
    SkimPluginManager* m_allModules;
    scim::SocketServerThread* m_inputServer;

    KXMLGUIFactory* m_guiFactory;

    QTimer* m_autoHideTimeoutTimer;
    QTimer* m_showHandleTimer;
    QTimer* m_showExtHandleTimer;

    QSize m_embeddedAreaSize;
    ToolBarModeType m_mode;
    bool m_embedInitialized;
    int m_autoHideTimeout;
    bool m_contentIsVisible;
    bool m_panelTurnedOn;
    bool m_updatePropertiesNeeded;
    bool m_alwaysShow;
    bool m_autoSnap;
    bool m_alwaysShowHandle;
    bool m_alwaysShowExtensionHandle;
    bool m_updateGUIProperties, m_updateFrontendProperties;
    Qt::Orientation m_applet_direction;
    QString m_actionListName;

    QBoxLayout::Direction m_changeToDirection;
    bool m_shouldChangeDirection;

protected:
    void enterEvent ( QEvent * );
    void leaveEvent ( QEvent * );
    void mouseMoveEvent ( QMouseEvent * e );

    void startDockingTimer();
    void standaloneModeHide();
//     void resizeEvent ( QResizeEvent * );
    QBoxLayout* m_mainWindowLayout;
//     QPoint m_clickPos;
    ScimToolBar* m_toolbar;
    SkimGlobalActions* m_defaultActionCollection;
    ScimAction* m_serverAction;
    QPopupMenu* m_contextMenu;
    QLabel* m_logo;
    class KToggleAction* m_toggleDockingAction;
    QPtrList<class KAction> m_insertedActions;
protected slots:
    void slotTurnOn();
    void slotTurnOff();
    void initContextMenu();
    void initEmbedPanel();
    void showHandleRequest();
    void showExtHandleRequest();
    void hideHandleRequest();
    void slotLeaveEvent();
    void hideToolbar();
    void settleToolbar();
    void contextMenuEvent ( QContextMenuEvent * );
    void changeDirection(QBoxLayout::Direction d);
    void slotApplicationRegistered (const QCString &);
    void emptyToolbar(bool);
};

#include "src/skimplugin.h"

class MainWindowPlugin : public SkimPlugin
{
Q_OBJECT
public:
    MainWindowPlugin( QObject *parent, const char *name, const QStringList &args );
    virtual ~MainWindowPlugin();

public slots:
    void toggleDocking();

private:
    MainWindow* mainwindow;
    class KPanelApplet * applet;
};

#endif // MAINWINDOW_H
