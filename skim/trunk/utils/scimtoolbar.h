/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/
#ifndef SCIMTOOLBAR_H
#define SCIMTOOLBAR_H

#include <ktoolbar.h>
#include <ktoolbarbutton.h>
#include <qlayout.h>

class ScimToolBar : public KToolBar
{
Q_OBJECT
public:
    ScimToolBar(QWidget *parent = 0, const char *name = 0);

    ~ScimToolBar();

    virtual void adjustSize();
    virtual QSize sizeHint() const;

protected:
    QBoxLayout * m_boxlayout;
};

#endif
