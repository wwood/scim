/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "kcmloader.h"
#include "kdetraystatus_config.h"

#include <kgenericfactory.h>

typedef KGenericFactory<KCMLoader> KCMLoaderFactory;

K_EXPORT_COMPONENT_FACTORY( kcm_skimplugin_systemtray, KCMLoaderFactory( "kcm_skimplugin_systemtray" ) )

KCMLoader::KCMLoader( QWidget *parent, const char * /* name */, const QStringList &args )
 : KAutoCModule( KCMLoaderFactory::instance(), parent, args )
{
  setMainWidget(new KCM_SystemTray(this));
}

#include "kcmloader.moc"
