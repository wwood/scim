/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/
#ifndef SCIMLAUNCHER_H
#define SCIMLAUNCHER_H

#define Uses_SCIM_FRONTEND_MODULE
#define Uses_SCIM_IMENGINE_MODULE
#define Uses_SCIM_BACKEND
#define Uses_SCIM_CONFIG_PATH
#define Uses_C_LOCALE
#include <scim.h>

#include "src/skimplugin.h"

class ScimThread : public QThread
{
public:
    ScimThread(QString _args = "");
    ~ScimThread();
private:
    bool check_socket_frontend ();
    int   m_new_argc;
    char *m_new_argv [80];
    
protected:
    virtual void run();
    QString m_args;
};

class ScimLauncher : public SkimPlugin
{
Q_OBJECT
public:
    ScimLauncher(QObject *parent, const char *name, const QStringList & /*args*/);
    virtual ~ScimLauncher();
    virtual void aboutToUnload();

protected:
    ScimThread * m_scimThread;
};

#endif
