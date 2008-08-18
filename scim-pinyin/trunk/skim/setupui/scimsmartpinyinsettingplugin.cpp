/***************************************************************************
 *   Copyright (C) 2003-2005 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "scimsmartpinyinsettingplugin.h"

#include "smartpinyin.h"
#include "smartpinyinui.h"

#include <qcheckbox.h>

#include <kgenericfactory.h>
#include <klocale.h>

typedef KGenericFactory<ScimSmartPinyinSettingPlugin> ScimSmartPinyinSettingLoaderFactory;

K_EXPORT_COMPONENT_FACTORY( kcm_skimplugin_scim_smartpinyin, 
    ScimSmartPinyinSettingLoaderFactory( "kcm_skimplugin_scim_smartpinyin" ) )

class ScimSmartPinyinSettingPlugin::ScimSmartPinyinSettingPluginPrivate {
public:
    SmartPinyinSettingUI * ui;
};

ScimSmartPinyinSettingPlugin::ScimSmartPinyinSettingPlugin(QWidget *parent, 
  const char */*name*/, const QStringList &args)
 : KAutoCModule( ScimSmartPinyinSettingLoaderFactory::instance(), 
     parent, args, SmartPinyinConfig::self() ),
   d(new ScimSmartPinyinSettingPluginPrivate)
{
    KGlobal::locale()->insertCatalogue("skim-scim-pinyin");
    d->ui = new SmartPinyinSettingUI(this);
    setMainWidget(d->ui);
    
    //FIXME: emit toggle signals to update corresponding connected widgets
    d->ui->advancedCBox->setChecked(true);
    d->ui->advancedCBox->toggle();
    d->ui->kcfg__IMEngine_Pinyin_Ambiguities->toggle();
    d->ui->kcfg__IMEngine_Pinyin_Ambiguities->toggle();
}

ScimSmartPinyinSettingPlugin::~ScimSmartPinyinSettingPlugin() 
{
    KGlobal::locale()->removeCatalogue("skim-scim-pinyin");
}


#include "scimsmartpinyinsettingplugin.moc"
