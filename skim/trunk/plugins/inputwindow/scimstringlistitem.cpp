/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "scimstringlistitem.h"
#include <qpainter.h> 

class ScimStringListItemPrivate
{
public:
    QString text;
    scim::AttributeList attrs;
    QSize storedSizeHint;
};
ScimStringListItem::ScimStringListItem(QWidget *parent, const char *name)
 : QFrame(parent, name), ScimStringRender(this), d(new ScimStringListItemPrivate())
{
}

void ScimStringListItem::drawContents ( QPainter * p )
{
    drawString(p, contentsRect());
}

void ScimStringListItem::setText(const QString & text, const scim::AttributeList & attrlist ) 
{
    ScimStringRender::setText(text, attrlist);
    updateGeometry();

    //notify Qt to repaint this widget
    update();
}
QSize ScimStringListItem::sizeHint() const
{
    return minimumSizeHint ();
}
QSize ScimStringListItem::minimumSizeHint () const
{
    int w = 2*frameWidth();
    return ScimStringRender::minimumSizeHint() + QSize(w, w);
}

ScimStringListItem::~ScimStringListItem()
{
    delete d;
}


#include "scimstringlistitem.moc"
