/***************************************************************************
 *   Copyright (C) 2003-2005 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/
#ifndef SCIMSMARTPINYINSETTINGPLUGIN_H
#define SCIMSMARTPINYINSETTINGPLUGIN_H

#include "utils/kautocmodule.h"

class ScimSmartPinyinSettingPlugin : public KAutoCModule
{
Q_OBJECT
public:
    ScimSmartPinyinSettingPlugin(QWidget *parent, 
  const char */*name*/, const QStringList &args);

    ~ScimSmartPinyinSettingPlugin();
private:
    class ScimSmartPinyinSettingPluginPrivate;
    ScimSmartPinyinSettingPluginPrivate * d;
};

#endif
