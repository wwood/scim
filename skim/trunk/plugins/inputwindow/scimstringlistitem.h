/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/
#ifndef SCIMSTRINGLISTITEM_H
#define SCIMSTRINGLISTITEM_H

#ifndef NO_CONFIG_H
#include "config.h"
#endif
#define Uses_SCIM_ATTRIBUTE
#include <scim.h>

#include "scimstringrender.h"
#include <qframe.h>

class ScimStringListItem : public QFrame, public ScimStringRender
{
Q_OBJECT
public:
    ScimStringListItem(QWidget *parent = 0, const char *name = 0);

    ~ScimStringListItem();
    virtual QSize minimumSizeHint () const;
    virtual QSize sizeHint() const;
public slots:
    void setText(const QString &, const scim::AttributeList &);
protected:
    void drawContents( QPainter * );
private:
    class ScimStringListItemPrivate * d;
};

#endif
