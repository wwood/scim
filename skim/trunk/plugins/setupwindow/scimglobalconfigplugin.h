/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/
#ifndef SCIMGLOBALCONFIGPLUGIN_H
#define SCIMGLOBALCONFIGPLUGIN_H

#include "utils/kautocmodule.h"

class ScimGlobalConfigPlugin : public KAutoCModule
{
Q_OBJECT
public:
    ScimGlobalConfigPlugin(QWidget *parent, 
  const char */*name*/, const QStringList &args);

    ~ScimGlobalConfigPlugin();

    virtual void load();
    virtual void save();
protected slots:
    void widgetChanged();
    void editLocale();
private:
    void checkSettings();
    bool checkLocales();

    class ScimGlobalConfigPluginPrivate;
    ScimGlobalConfigPluginPrivate *d;
};

#endif
