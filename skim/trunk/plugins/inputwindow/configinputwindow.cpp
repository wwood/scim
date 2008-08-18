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
#include "configinputwindow.h"

#include <kdebug.h>
#include <klocale.h>
#include <kfontrequester.h>
#include <kapplication.h>
#include <kconfigdialogmanager.h>
#include <qcheckbox.h>

#define MODULE_UUID "a1d34730-6d08-4f59-a418-e63334a46538"
class ConfigInputWindowPrivate
{
  public:
//     KConfigDialogManager * mgr;
    bool changed;
    QFont oldCustomFont;
};

ConfigInputWindow::ConfigInputWindow(QWidget *parent, const char */*name*/)
 : ConfigInputWindowBase(parent), d(new ConfigInputWindowPrivate())
{
    load();
    d->changed = false;

    connect(defaultFontRequester, SIGNAL(fontSelected (const QFont &)), SLOT(fontChanged (const QFont &)));
}

ConfigInputWindow::~ConfigInputWindow () {}

void ConfigInputWindow::load()
{
//     kdDebug(18010) << k_lineinfo << " IWKCMLoader::load()\n";
    if(!ScimKdeSettings::iW_Font().isEmpty())
    {
      d->oldCustomFont.fromString(ScimKdeSettings::iW_Font());
      defaultFontRequester->setFont(d->oldCustomFont);
    }
    else
    {
      d->oldCustomFont = font();
      defaultFontRequester->setFont(font());
    }
}

bool ConfigInputWindow::hasChanged()
{
    return d->changed;
}

void ConfigInputWindow::save()
{
    if(!hasChanged())
      return;

    d->oldCustomFont = defaultFontRequester->font();
    if(kapp->font() != d->oldCustomFont)
      ScimKdeSettings::setIW_Font(d->oldCustomFont.toString());
    else
      ScimKdeSettings::setIW_Font("");

    d->changed = false;
}

void ConfigInputWindow::fontChanged (const QFont &f)
{
    if(f == d->oldCustomFont)
      d->changed = false;
    else
      d->changed = true;

    emit changed(d->changed);
}

void ConfigInputWindow::defaults()
{
    if(kapp->font() == defaultFontRequester->font())
      return;
      
    defaultFontRequester->setFont(kapp->font());
    fontChanged(kapp->font()); 
}

#include <kgenericfactory.h>

typedef KGenericFactory<IWKCMLoader> KCMLoaderFactory;

K_EXPORT_COMPONENT_FACTORY( kcm_skimplugin_inputwindow, KCMLoaderFactory( "kcm_skimplugin_inputwindow" ) )

IWKCMLoader::IWKCMLoader( QWidget *parent, const char * /* name */, const QStringList &args )
 : KAutoCModule( KCMLoaderFactory::instance(), parent, args )
{
//   kdDebug(18010) << k_lineinfo << " " << KCMLoaderFactory::instance()->instanceName() << "\n";
  d = new ConfigInputWindow(this);
  setMainWidget(d);
  d->kcfg_LookupTable_MinimumWidth->setEnabled(d->kcfg_LookupTable_IsVertical->isChecked());
  connect(d, SIGNAL(changed(bool)), SLOT(slotWidgetModified()));
}

void IWKCMLoader::save(){  d->save(); KAutoCModule::save();}

void IWKCMLoader::load() { d->load(); KAutoCModule::load();}

void IWKCMLoader::defaults() {d->defaults(); KAutoCModule::defaults(); }

void IWKCMLoader::slotWidgetModified()
{
    emit changed(d->hasChanged() || configDialgoMgr()->hasChanged());
}

IWKCMLoader::~IWKCMLoader(){}

#include "configinputwindow.moc"
