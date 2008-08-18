/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "scimlineedit.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qfontmetrics.h"

ScimLineEdit::ScimLineEdit(QWidget *parent, const char *name)
 : ScimDragableFrame(parent, parent, name), ScimStringRender(this)
{
    setFrameStyle( QFrame::LineEditPanel  | QFrame::Raised );
    setDisplayCursor(true);
}

void ScimLineEdit::setText(const QString & text, const scim::AttributeList & attrlist) 
{
  ScimStringRender::setText(text, attrlist);
  updateGeometry();
  update();
}
QSize ScimLineEdit::minimumSizeHint () const
{
    return ScimStringRender::minimumSizeHint() + QSize(100, 2*frameWidth());
}

void ScimLineEdit::drawContents( QPainter *p )
{
    if(isVisible())
    {
        QRect cr = contentsRect();
        drawString(p, cr);
    }
}

void ScimLineEdit::setCursorPosition ( int p )
{
    ScimStringRender::setCursorPosition(p);
    update();
}

ScimLineEdit::~ScimLineEdit(){}


#include "scimlineedit.moc"
