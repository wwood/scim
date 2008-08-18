/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/
#ifndef SYSTEMTRAYPLUGIN_H
#define SYSTEMTRAYPLUGIN_H

#include "src/skimplugin.h"

class SystemTrayPlugin : public SkimPlugin
{
Q_OBJECT
public:
    SystemTrayPlugin( QObject *parent, const char *name, const QStringList &args );

    virtual ~SystemTrayPlugin();
private:
    class KDETrayStatus* m_tray;
//     virtual void aboutToUnload();
// signals:
//     void readyForUnload();
};

#endif
