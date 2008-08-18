/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/
#ifndef CONFIGMAINWINDOW_H
#define CONFIGMAINWINDOW_H

#include "mainwindow_settings.h"
class KEditToolbarWidget;
class ConfigMainWindow : public MainWindow_Settings
{
Q_OBJECT
public:
    ConfigMainWindow(QWidget *parent = 0, const char *name = 0);

    ~ConfigMainWindow();
    virtual bool hasChanged();
    void defaults();
    virtual void saveSettings();
signals:
    void changed(bool);
protected slots:
    void checkChanged(bool e);
    void changeHiddenProperty(int);
private:
    bool m_changed;
    KEditToolbarWidget* m_editToolbar;

};

#include "utils/kautocmodule.h"

class KCMLoader_MW : public KAutoCModule
{
Q_OBJECT
public:
    KCMLoader_MW(QWidget *parent, const char * /* name */, const QStringList &args );

    virtual ~KCMLoader_MW();

    virtual void save();
  
//   virtual void load();
  
    virtual void defaults();
protected slots:
    virtual void slotWidgetModified();
    
private:
    ConfigMainWindow * d;
};

#endif
