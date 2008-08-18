/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/
#ifndef SCIMSTRINGRENDER_H
#define SCIMSTRINGRENDER_H

#ifndef NO_CONFIG_H
#include "config.h"
#endif
#define Uses_SCIM_ATTRIBUTE
#include <scim.h>

#include <qpainter.h>

class QWidget;

class ScimStringRender{
public:
    ScimStringRender(QWidget *);

    virtual ~ScimStringRender();
    void drawString( QPainter *, const QRect &) const;
    void setText(const QString &, const scim::AttributeList &);

    QSize minimumSizeHint () const;

    void setCursorPosition ( int );
    int cursorPosition() const;
    void setDisplayCursor(bool);

private:
    class ScimStringRenderPrivate * d;
};

#endif
