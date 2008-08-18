/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "systemtrayplugin.h"
#include "kdetraystatus.h"
#include <kgenericfactory.h>

K_EXPORT_COMPONENT_FACTORY( skimplugin_systemtray,
                              KGenericFactory<SystemTrayPlugin>( "skimplugin_systemtray" ) )

SystemTrayPlugin::SystemTrayPlugin( QObject *parent, const char *name, const QStringList & /*args*/ )
 : SkimPlugin(KGenericFactory<SystemTrayPlugin>::instance(), parent, name)
{
     m_tray = new KDETrayStatus(static_cast<SkimPluginManager *>(parent), 0, "KdeTrayStatus");
     connect( this, SIGNAL(settingsChanged()), m_tray, SLOT(initContextMenu()));
}

SystemTrayPlugin::~SystemTrayPlugin()
{
    delete m_tray;
}

#include "systemtrayplugin.moc"
