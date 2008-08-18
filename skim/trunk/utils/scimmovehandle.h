/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/
#ifndef SCIMMOVEHANDLE_H
#define SCIMMOVEHANDLE_H

#include "scimdragableframe.h"

#include <qtimer.h>
#include <qstyle.h>
#include <qpainter.h>
#include <qapplication.h>
#include <qlayout.h>

class ScimMoveHandle : public ScimDragableFrame
{
    Q_OBJECT
public:
    ScimMoveHandle( QWidget *body, QWidget *dw );

    QSize minimumSizeHint() const;
    QSize minimumSize() const { return minimumSizeHint(); }
    QSize sizeHint() const { return minimumSize(); }
    QSizePolicy sizePolicy() const;

    QBoxLayout::Direction direction();
    void setDirection(QBoxLayout::Direction);

protected:
    virtual void paintEvent( QPaintEvent *e );
    QBoxLayout::Direction m_direction;
// private slots:
//     void minimize();
};


#endif
