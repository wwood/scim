/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/
#ifndef SCIMXSETTINGS_H
#define SCIMXSETTINGS_H

#include "utils/kautocmodule.h"

/**
@author spider
*/
class ScimXSettings : public KAutoCModule
{
Q_OBJECT
public:
    ScimXSettings(QWidget *parent, const char *name, const QStringList &args);

private:
  class XWindowSettings * d;
};

#endif
