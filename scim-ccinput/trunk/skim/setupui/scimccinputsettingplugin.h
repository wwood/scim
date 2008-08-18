/***************************************************************************
 *   Copyright (C) 2004-2005 by cooleyes                                   *
 *   cooleyes_lf@users.sourceforge.net                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/
#ifndef SCIMCCINPUTSETTINGPLUGIN_H
#define SCIMCCINPUTSETTINGPLUGIN_H

#include "utils/kautocmodule.h"

class ScimCCInputSettingPlugin : public KAutoCModule
{
Q_OBJECT
public:
    ScimCCInputSettingPlugin(QWidget *parent, 
  const char */*name*/, const QStringList &args);

    ~ScimCCInputSettingPlugin();
private slots:
    void showShuangPinDetail();
private:
    class ScimCCInputSettingPluginPrivate;
    ScimCCInputSettingPluginPrivate * d;
};

#endif
