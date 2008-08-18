/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef SKIMPLUGININFO_H
#define SKIMPLUGININFO_H

#include <kplugininfo.h>

class KAction;

struct SkimPluginActionInfo {
    QString name;
    QString internalName;
    QString icon;
    QString slot;
    QString type;
    int id;
};

class SkimPluginInfo : public KPluginInfo
{
public:
    typedef QValueList< SkimPluginInfo * >   List;
    
    SkimPluginInfo (const KService::Ptr service);
    SkimPluginInfo (const QString &filename, const char *resource=0);

    ~SkimPluginInfo();

    bool isNoDisplay() const;
    bool isHasActions() const;
    bool isOnDemand() const;
    QStringList & overloadedModules() const;
    QValueList<SkimPluginActionInfo> & actions() const;
    int weight() const;

    static SkimPluginInfo::List fromServices(const KService::List &services, KConfig *config=0, const QString &group=QString::null);
private:
    void init();
    void readActions();
    class SkimPluginInfoPrivate;
    SkimPluginInfoPrivate * const d;

};

#endif
