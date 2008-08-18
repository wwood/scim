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
#include "kdetraystatus.h"

#include <qpixmap.h>
#include <qimage.h>
#include <qiconset.h>
#include <kpopupmenu.h>
#include <qtooltip.h>

#include "src/skimglobalactions.h"
#include "utils/scimactions.h"

KDETrayStatus::KDETrayStatus(SkimPluginManager* ms, QWidget *parent, const char *name)
 : KSystemTray(parent, name)
{
    QToolTip::add(this, i18n("Skim - SCIM KDE Frontend"));
    m_allModules = ms;

    setPixmap( loadIcon("skim") );

    connect( m_allModules->getInputServer(),
      SIGNAL(updateFactoryInfoReq(const scim::PanelFactoryInfo &)),
      this, SLOT(updateIcon(const scim::PanelFactoryInfo &)));

    m_serverAction =
      dynamic_cast<ScimComboAction *>(m_allModules->getActionCollection()->action( "change_server" ));

    connect( this, SIGNAL( quitSelected() ), m_allModules, SLOT( shutdown() ));

    connect(m_allModules->getActionCollection(), SIGNAL(standaloneHelperActionsChanged()),
        this, SLOT(initContextMenu()));

    actionCollection()->action(0)->setShortcut(""); // clear shortcut of action Quit, it is of no use

    initContextMenu();

    show();
}

KDETrayStatus::~KDETrayStatus()
{
    contextMenu()->clear();
}

void KDETrayStatus::showEvent ( QShowEvent * )
{
    //this virtual funtion is used to overwrite the default one which will
    //always insert quit actions into the context menu
}

void KDETrayStatus::initContextMenu()
{
    KAction *action;

    contextMenu()->clear();

    #if ENABLE_DEBUG
    action = m_allModules->getActionCollection()->action( "reload" );
    if(action)
    {
      action->plug(contextMenu());
      contextMenu()->insertSeparator();
    }
    #endif

    //load helper actions
    KActionCollection * helperActioncol = m_allModules->getActionCollection()->helperActionCollection();
    if ( helperActioncol->count() ) {
        KActionPtrList helperActions = helperActioncol->actions();
        for(size_t i=0; i<helperActions.size();i++) {
            helperActions[i]->plug(contextMenu());
        }
        contextMenu()->insertSeparator();
    }

    action = m_allModules->getActionCollection()->action( "configure" );
    if(action)
      action->plug(contextMenu());

    action = m_allModules->getActionCollection()->action( "aboutapp" );
    if(action)
      action->plug(contextMenu());

    action = m_allModules->getActionCollection()->action( "aboutkde" );
    if(action)
      action->plug(contextMenu());

    if( ScimKdeSettings::show_Quit() ) {
        contextMenu()->insertSeparator();
        actionCollection()->action(0)->plug(contextMenu());
    }
}

void KDETrayStatus::mousePressEvent( QMouseEvent *e )
{
    if ( !rect().contains( e->pos() ) )
        return;

    switch ( e->button() ) {
    case LeftButton:
        if(m_serverAction)
          m_serverAction->slotActivated();
        break;
    default:
        KSystemTray::mousePressEvent(e);
        break;
    }
}

void KDETrayStatus::updateIcon(const scim::PanelFactoryInfo & info)
{
    setPixmap( loadIcon(QString::fromLocal8Bit(info.icon.c_str())) );
}

#include "kdetraystatus.moc"
