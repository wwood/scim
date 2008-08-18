/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/
#ifndef SKIMPLUGIN_H
#define SKIMPLUGIN_H

#include "skimpluginmanager.h"

#include <qobject.h>
#include <kxmlguiclient.h>

class SkimPlugin : public QObject, public KXMLGUIClient
{
Q_OBJECT
public:
    typedef QValueList<class KAction *> CustomActionsList;
    SkimPlugin(KInstance *instance, QObject *parent, const char *name);

    virtual ~SkimPlugin();

    CustomActionsList customActions();
    QString pluginId() const;

    QString pluginIcon() const;

    QString displayName() const;

    virtual void aboutToUnload();

signals:
    /**
    * Notify that the settings of a plugin were changed.
    * These changes are passed on from the new KCDialog code in kdelibs/kutils.
    */
    void settingsChanged();

    /**
    * Indicate when we're ready for unload. See aboutToUnload()
    */
    void readyForUnload();

protected:
    class scim::SocketServerThread * inputServer;
    class SkimGlobalActions * globalActionCollection;
};

#endif
