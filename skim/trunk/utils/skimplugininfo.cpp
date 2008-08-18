/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "skimplugininfo.h"
#include "scimactions.h"

#include <kdesktopfile.h>

static int _pluginActionRepository_id_counter = 0;

class SkimPluginInfo::SkimPluginInfoPrivate
{
public:
    SkimPluginInfoPrivate() : noDisplay(false), hasActions(false), onDemand(false) {}
    bool noDisplay;
    bool hasActions;
    bool onDemand;
    int weight;
    QStringList overloadedModules;

    QValueList<SkimPluginActionInfo> actions;
};

SkimPluginInfo::SkimPluginInfo (const KService::Ptr service)
    : KPluginInfo(service), d(new SkimPluginInfoPrivate)
{
    init();
}

SkimPluginInfo::SkimPluginInfo (const QString &filename, const char *resource)
    : KPluginInfo(filename, resource), d(new SkimPluginInfoPrivate)
{
    init();
}

void SkimPluginInfo::init()
{
    QVariant v;

    v = property("NoDisplay");
    if(v.isValid())
        d->noDisplay = v.toBool();

    v = property("X-KDE-SKIM-Overload-SCIM-Modules");
    if(v.isValid ())
        d->overloadedModules = v.toStringList();

    v = property("X-KDE-PluginInfo-HasActions");
    if(v.isValid ())
        d->hasActions = v.toBool();

    v = property("X-KDE-PluginInfo-OnDemand");
    if(v.isValid ())
        d->onDemand = v.toBool();

    v = property("X-KDE-SKIM-Weight");
    if(v.isValid ())
        d->weight = v.toInt();

    if(d->hasActions)
        readActions();

}

bool SkimPluginInfo::isNoDisplay() const
{
    return d->noDisplay;
}

bool SkimPluginInfo::isHasActions() const
{
    return d->hasActions;
}

bool SkimPluginInfo::isOnDemand() const
{
    return d->onDemand;
}

int SkimPluginInfo::weight() const
{
    return d->weight;
}

QStringList & SkimPluginInfo::overloadedModules() const
{
    return d->overloadedModules;
}

QValueList<SkimPluginActionInfo> & SkimPluginInfo::actions() const
{
    return d->actions;
}

void SkimPluginInfo::readActions()
{
    d->actions.clear();
    KDesktopFile kdesk(service()->desktopEntryPath(), false, "services");
    QStringList actions = kdesk.readActions();
    int id;
    for(uint ita = 0; ita < actions.size(); ita++) {
        kdesk.setActionGroup(actions[ita]);
        id = _pluginActionRepository_id_counter++;
        //TODO: QT4 rewrite: qt4 changed the format of slot
        SkimPluginActionInfo info = {kdesk.readName(), kdesk.readEntry("InternalName"),
            kdesk.readIcon(), "1" + kdesk.readEntry("Slot") + "()", kdesk.readEntry("Type"), id};

        d->actions.push_back(info);
    }
}

SkimPluginInfo::List SkimPluginInfo::fromServices(const KService::List &services, KConfig *config, const QString &group)
{
    QValueList<SkimPluginInfo*> infolist;
    SkimPluginInfo * info;
    for( KService::List::ConstIterator it = services.begin();
         it != services.end(); ++it )
    {
        info = new SkimPluginInfo( *it );
        info->setConfig( config, group );
        infolist += info;
    }
    return infolist;
}

SkimPluginInfo::~SkimPluginInfo()
{
    delete d;
}


