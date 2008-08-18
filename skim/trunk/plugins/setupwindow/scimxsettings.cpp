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

#include "scimxsettings.h"
#include "scim_x.h"

#include <kgenericfactory.h>

typedef KGenericFactory<ScimXSettings> KCMScimXSettingsLoaderFactory;

K_EXPORT_COMPONENT_FACTORY( kcm_skimplugin_scim_x, 
    KCMScimXSettingsLoaderFactory( "kcm_skimplugin_scim_x" ) )

ScimXSettings::ScimXSettings(QWidget *parent, const char */*name*/, const QStringList &args)
 : KAutoCModule( KCMScimXSettingsLoaderFactory::instance(), parent, args )
{
    d = new XWindowSettings(this);
    setMainWidget(d);
    load();
}

#include "scimxsettings.moc"
