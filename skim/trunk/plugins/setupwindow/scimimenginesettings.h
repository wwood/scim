/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/
#ifndef SCIMIMENGINESETTINGS_H
#define SCIMIMENGINESETTINGS_H

#define Uses_SCIM_IMENGINE

#define USE_SCIM_KDE_SETTINGS
#define Uses_SCIM_IMENGINE_MODULE
#define Uses_SCIM_CONFIG_MODULE

#include "src/skimpluginmanager.h"

#include "utils/kautocmodule.h"

class QListViewItem;
class QCheckListItem;

class ScimIMEngineSettings : public KAutoCModule
{
Q_OBJECT
public:
    ScimIMEngineSettings(QWidget *parent, 
  const char */*name*/, const QStringList &args);
    virtual ~ScimIMEngineSettings();
    virtual void save();
    virtual void defaults();
    virtual void load();
protected slots:
    void checkBoxModified(QListViewItem * item, const QPoint &, int);
    void enableAllIMEs();
    void disableAllIMEs();
    void updateEditHotkeysBtnStates(QListViewItem *);
    void editHotkeys();
    void editIMFilters();
private:
    void clear();
    void toggleAllIMEs(bool enabled);
    void get_factory_list (QStringList &uuids,
                          QStringList &names,
                          QStringList &langs,
                          QStringList &icons);
    void setIMFilters(QCheckListItem * item, std::vector <scim::String> & filter_list);
    void checkModification();

    class SCIMIMEConfigUI * ui;
    scim::ConfigPointer m_config;

    class ScimIMEngineSettingsPrivate;
    ScimIMEngineSettingsPrivate * const d;

};

#endif
