/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/
#ifndef SETUPWINDOW_H
#define SETUPWINDOW_H

//#include <kconfigdialog.h> 
#include <kcmultidialog.h> 

class ScimSetupWindowPrivate;
class SkimPluginManager;
class scim::SocketServerThread;

class ScimSetupWindow : public KCMultiDialog
{
Q_OBJECT
public:
    ScimSetupWindow(scim::SocketServerThread *parent, const char *name = 0, KConfigSkeleton *config = 0);
    ~ScimSetupWindow();
    virtual class KActionCollection* actionCollection();
    virtual void hide();
signals:
    void readyForUnload();
protected slots:
    virtual void slotApply();
    virtual void slotOk();
    void load();
    void slotConfigurationChangedFor( const QCString & component);

protected:
    SkimPluginManager* m_mc;
private:
    ScimSetupWindowPrivate* d;
};

#include "src/skimplugin.h"

class SkimConfigPlugin : public SkimPlugin
{
Q_OBJECT
public:
    SkimConfigPlugin( QObject *parent, const char *name, const QStringList &args );

    virtual ~SkimConfigPlugin();
public slots:
    void SlotConfigure();
private:
    ScimSetupWindow* m_configdialog;
};

#endif
