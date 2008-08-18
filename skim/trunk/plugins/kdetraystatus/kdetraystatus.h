/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/
#ifndef KDETRAYSTATUS_H
#define KDETRAYSTATUS_H

#include "src/skimpluginmanager.h"
#include <ksystemtray.h>


class KDETrayStatus : public KSystemTray
{
Q_OBJECT
public:
    KDETrayStatus(SkimPluginManager*, QWidget *parent = 0, const char *name = 0);

    ~KDETrayStatus();

private:
    SkimPluginManager* m_allModules;
    class ScimComboAction * m_serverAction;

public slots:
    void updateIcon(const scim::PanelFactoryInfo &);
    void initContextMenu();

protected:
    virtual void mousePressEvent( QMouseEvent *e );
    virtual void showEvent ( QShowEvent * );
};

#endif
