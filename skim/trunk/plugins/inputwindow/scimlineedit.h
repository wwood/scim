/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/
#ifndef SCIMLINEEDIT_H
#define SCIMLINEEDIT_H

#include "scimstringrender.h"

#include "scimdragableframe.h"

class ScimLineEdit : public ScimDragableFrame, public ScimStringRender
{
Q_OBJECT
public:
    ScimLineEdit(QWidget *parent = 0, const char *name = 0);

    ~ScimLineEdit();
//     QString text() const {return _text;};
    void setText(const QString &, const scim::AttributeList &);
    QSize minimumSizeHint() const;
    QSize minimumSize() const { return minimumSizeHint(); }
    QSize sizeHint() const { return minimumSize(); }
//     void setTextFormat(const scim::AttributeList &);
//     int cursorPosition();
public slots:
    void setCursorPosition ( int );
protected:
    void drawContents( QPainter *p );
};

#endif
