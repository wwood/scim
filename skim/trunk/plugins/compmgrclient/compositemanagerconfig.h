/***************************************************************************
 *   Copyright (C) 2003 by liuspider                                          *
 *   sharecash@163.net                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef COMPOSITEMANAGERCONFIG_H
#define COMPOSITEMANAGERCONFIG_H

#include "utils/kautocmodule.h"

class QListViewItem;

class CompositeManagerConfig : public KAutoCModule
{
Q_OBJECT
public:
    CompositeManagerConfig(QWidget *parent, const char *name, const QStringList &args);
    ~CompositeManagerConfig();
    virtual void defaults();
    virtual void load();
    virtual void save();

protected slots:
    void selectedWindowChanged( QListViewItem * );
    void slotWidgetModified();
    void modifyCurrentWindowSetting();

private:
    class CompositeManagerWidgetBase * m_ui;
    class CompMgrClient * m_compmgrclient;
};

#endif
