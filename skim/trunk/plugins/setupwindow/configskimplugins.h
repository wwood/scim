/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/
#ifndef CONFIGSKIMPLUGINS_H
#define CONFIGSKIMPLUGINS_H

#include <kcmodule.h>
#include <klistview.h>

class ScimKDEPluginListItem;
class ScimKDEPluginListView : public KListView
{
Q_OBJECT
friend class ScimKDEPluginListItem;

public:
    ScimKDEPluginListView (QWidget *parent = 0, const char *name = 0);

signals:
    void stateChange(ScimKDEPluginListItem *, bool);
private:
    void stateChanged(ScimKDEPluginListItem *, bool);
};

class ConfigSkimPlugins : public KCModule
{
Q_OBJECT
public:
    ConfigSkimPlugins(QWidget *parent, const char */*name*/, const QStringList &args);

    virtual void defaults();
    virtual void load();
    virtual void save();
protected slots:
    void stateChange(ScimKDEPluginListItem *, bool);
private:
    class ConfigSkimPluginsPrivate;
    ConfigSkimPluginsPrivate *d;
};

#endif
