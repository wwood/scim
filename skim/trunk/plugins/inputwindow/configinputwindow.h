/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/
#ifndef CONFIGINPUTWINDOW_H
#define CONFIGINPUTWINDOW_H

#include "inputwindow_settings.h"

class ConfigInputWindowPrivate;

class ConfigInputWindow : public ConfigInputWindowBase
{
Q_OBJECT
public:
    ConfigInputWindow(QWidget *parent = 0, const char *name = 0);

    virtual ~ConfigInputWindow();
    virtual void save();
    virtual bool hasChanged();
    virtual void load();
    virtual void defaults();
signals:
    void changed(bool);
protected slots:
    void fontChanged (const QFont &font);
private:
    ConfigInputWindowPrivate * d;
};

#include "kautocmodule.h"

class IWKCMLoader : public KAutoCModule
{
Q_OBJECT
public:
  IWKCMLoader( QWidget *parent, const char * /* name */, const QStringList &args );

  virtual void save();

  virtual void load();

  virtual void defaults();

  virtual ~IWKCMLoader();
protected slots:
  virtual void slotWidgetModified();
private:
  ConfigInputWindow * d;
};
#endif
