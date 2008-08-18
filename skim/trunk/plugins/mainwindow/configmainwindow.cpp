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
#include "src/skimglobalactions.h"
#include "configmainwindow.h"

// #include "plugins/setupwindow/scimsetupwindow.h"
// #include <klocale.h>
#include <kedittoolbar.h>
#include <kconfigdialogmanager.h>
#include <qlayout.h>
#include <qgroupbox.h>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qwidgetstack.h>
#include <qspinbox.h>

ConfigMainWindow::ConfigMainWindow(QWidget *parent, const char */*name*/)
    : MainWindow_Settings(parent), m_changed(false)
{
    m_editToolbar = new KEditToolbarWidget("MainToolBar",
     SkimPluginManager::self()->getActionCollection(),"mainwindowui.rc",false,this);
    layout()->add(m_editToolbar);
    connect(m_editToolbar,SIGNAL(enableOk(bool)),SLOT(checkChanged(bool)));
    connect(modeComboBox, SIGNAL(activated ( int )), modeWidgetStack, SLOT(raiseWidget ( int )));

    //as the defaults value are managed by KAutoCModule, so in this class
    //the defaults only read in the current setting. However, as the default
    //value only can be read after KCMLoader_MW::setMainWidget(), so we have
    //to set the value manually here
    kcfg_DockingToPanelApplet->setChecked(ScimKdeSettings::dockingToPanelApplet());
    defaults();

    kcfg_DockingToPanelApplet->hide();

    connect(modeComboBox, SIGNAL(activated(int )), this, SLOT(changeHiddenProperty(int)));
}

void ConfigMainWindow::changeHiddenProperty(int index)
{
    kcfg_DockingToPanelApplet->setChecked(index);
}

void ConfigMainWindow::saveSettings()
{
    if(!m_changed)
      return;
    m_changed = false;
    if(!m_editToolbar->save())
      std::cerr <<"Can no save settings for toolbar " << m_editToolbar->name() << "\n";
}
void ConfigMainWindow::checkChanged(bool e)
{
    m_changed = e;
    emit changed(e);
}
bool ConfigMainWindow::hasChanged()
{
    return m_changed;
}

void ConfigMainWindow::defaults()
{
    //From Qt doc: Note that the activated() and highlighted() signals are only emitted
    //when the user changes the current item, not when it is changed programmatically.
    //so we have to set all the necessary dependencies ourself.
    modeComboBox->setCurrentItem(kcfg_DockingToPanelApplet->isChecked() ? 1: 0);
    modeWidgetStack->raiseWidget(kcfg_DockingToPanelApplet->isChecked() ? 1: 0);

    kcfg_Hide_Timeout->setEnabled(kcfg_Always_Show->isChecked());
}

ConfigMainWindow::~ConfigMainWindow()
{
//     _configmainwindow_widget = 0;
}

#include <kgenericfactory.h>

typedef KGenericFactory<KCMLoader_MW> KCMLoaderMWFactory;

K_EXPORT_COMPONENT_FACTORY( kcm_skimplugin_mainwindow, KCMLoaderMWFactory( "kcm_skimplugin_mainwindow" ) )

KCMLoader_MW::KCMLoader_MW( QWidget *parent, const char * /* name */, const QStringList &args )
 : KAutoCModule( KCMLoaderMWFactory::instance(), parent, args )
{
  d = new ConfigMainWindow(this);
  connect(d, SIGNAL(changed(bool )), SLOT(slotWidgetModified()));
  setMainWidget(d);
}

void KCMLoader_MW::save(){  d->saveSettings(); KAutoCModule::save();}

void KCMLoader_MW::defaults() {  KAutoCModule::defaults(); d->defaults();}

void KCMLoader_MW::slotWidgetModified()
{
    emit changed(d->hasChanged() || configDialgoMgr()->hasChanged());
}

// void KCMLoader_MW::load() { d->load(); KAutoCModule::load();}

KCMLoader_MW::~KCMLoader_MW()
{
}

#include "configmainwindow.moc"
