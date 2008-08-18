/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "kautocmodule.h"
#include "scimkdesettings.h"

#include <qlayout.h>
#include <kconfigdialogmanager.h>

class KAutoCModule::KAutoCModulePrivate
{
    public:
    KConfigDialogManager *kautoconfig;
};

#include <iostream>
KAutoCModule::KAutoCModule( KInstance * instance, QWidget * parent, 
    const QStringList & args, KConfigSkeleton * conf )
    : KCModule( instance, parent, args )
    , d( new KAutoCModulePrivate )
{
    if(!conf)
      conf = ScimKdeSettings::self();

    d->kautoconfig = new KConfigDialogManager( this, conf );
    connect(d->kautoconfig, SIGNAL(widgetModified()), SLOT(slotWidgetModified()));
    connect(d->kautoconfig, SIGNAL(settingsChanged()), SLOT(slotWidgetModified()));
}

KAutoCModule::~KAutoCModule()
{
    delete d;
}


void KAutoCModule::load()
{
    d->kautoconfig->updateWidgets();
}

void KAutoCModule::save()
{
    d->kautoconfig->updateSettings();
}

void KAutoCModule::defaults()
{
    d->kautoconfig->updateWidgetsDefault();
    if(d->kautoconfig->hasChanged())
      emit changed(true);
}

//Remove this if we do not want KDE < 3.4.0 support
void KAutoCModule::slotWidgetModified()
{
    emit changed(d->kautoconfig->hasChanged());
}

KConfigDialogManager *KAutoCModule::configDialgoMgr()
{
    return d->kautoconfig;
}

void KAutoCModule::setMainWidget(QWidget *widget/*, const QString& group*/ )
{
    QBoxLayout * l = new QVBoxLayout( this );
    l->addWidget( widget );
  
    d->kautoconfig->addWidget(widget/*,group*/);
    d->kautoconfig->updateWidgets();
}


#include "kautocmodule.moc"
