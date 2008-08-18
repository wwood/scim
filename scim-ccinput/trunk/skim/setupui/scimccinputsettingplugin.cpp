/***************************************************************************
 *   Copyright (C) 2004-2005 by cooleyes                                   *
 *   cooleyes_lf@users.sourceforge.net                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "scimccinputsettingplugin.h"

#include "ccinput.h"
#include "ccinputui.h"

#include <kgenericfactory.h>
#include <klocale.h>
#include <kiconloader.h>

#include <qlabel.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>

typedef KGenericFactory<ScimCCInputSettingPlugin> ScimCCInputSettingLoaderFactory;

K_EXPORT_COMPONENT_FACTORY( kcm_skimplugin_scim_ccinput, 
    ScimCCInputSettingLoaderFactory( "kcm_skimplugin_scim_ccinput" ) )

class ScimCCInputSettingPlugin::ScimCCInputSettingPluginPrivate {
public:
    CCInputSettingUI * ui;
};

ScimCCInputSettingPlugin::ScimCCInputSettingPlugin(QWidget *parent, 
  const char */*name*/, const QStringList &args)
 : KAutoCModule( ScimCCInputSettingLoaderFactory::instance(), 
     parent, args, CCInputConfig::self() ),
   d(new ScimCCInputSettingPluginPrivate)
{
    KGlobal::locale()->insertCatalogue("skim-scim-ccinput");
    d->ui = new CCInputSettingUI(this);
    
    d->ui->detailLabel->clear();
    d->ui->detailLabel->hide();

    connect(d->ui->_kcfg__IMEngine_Chinese_CCIN_QuanPinKey, SIGNAL(toggled(bool)), SLOT(showShuangPinDetail()));
    connect(d->ui->detailButton, SIGNAL(clicked()), SLOT(showShuangPinDetail()));
    connect(d->ui->kcfg__IMEngine_Chinese_CCIN_ShuangPinKindKey, SIGNAL(activated(int)) , SLOT(showShuangPinDetail()));
    setMainWidget(d->ui);

    showShuangPinDetail();
}

ScimCCInputSettingPlugin::~ScimCCInputSettingPlugin() 
{
    KGlobal::locale()->removeCatalogue("skim-scim-ccinput");
}

void ScimCCInputSettingPlugin::showShuangPinDetail()
{
    if(d->ui->detailButton->isEnabled() && d->ui->detailButton->isOn())
    {
        QString filename;
        int index = d->ui->kcfg__IMEngine_Chinese_CCIN_ShuangPinKindKey->currentItem();
        switch(index)
        {
            case 1: //Ziran (nature)
                filename = "sp_zr.png";
                break;
            case 2: //microsoft
                filename = "sp_ms.png";
                break;
            case 3: //ziguang
                filename = "sp_zg.png";
                break;
            case 4: //abc
                filename = "sp_zn.png";
                break;
            case 5: //microsoft
                filename = "sp_ls.png";
                break;
            default: //chinesestart
                filename = "sp_st.png";
        }
        d->ui->detailLabel->setPixmap(KGlobal::iconLoader()->loadIcon(filename, KIcon::User));
	d->ui->detailLabel->show();
    } else
        d->ui->detailLabel->hide();
}

#include "scimccinputsettingplugin.moc"
