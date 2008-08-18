/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/
#ifndef SCIMLISTBOX_H
#define SCIMLISTBOX_H

#ifndef NO_CONFIG_H
#include "config.h"
#endif

#define Uses_SCIM_ATTRIBUTE
#include <scim.h>

#include <qlabel.h>
#include <qlayout.h>

#include "scimstringlistitem.h"
#include "scimmovehandle.h"
/**
@author liuspider
*/
#define MAX_CANDIDATES 16
class ScimListBox : public ScimDragableFrame
{
Q_OBJECT
public:
    ScimListBox(QWidget *parent = 0, const char *name = 0, bool _isVertical = false);

    unsigned updateContent( const QStringList & labellist, const QStringList &lookupstringlist,
                            QValueList<scim::AttributeList> & attrlist, bool pagesizefixed );
    void setVertical(bool b);

    bool eventFilter ( QObject * o, QEvent * e );

signals:
    void itemSelected(int);
public slots:
    void updateHighlight(int);
    
protected:
//     void rearrangeStringItem();
    QBoxLayout*          layout;
    ScimStringListItem*  candidatesLbl[MAX_CANDIDATES];
    QMap<QObject*, int>  candidatesLblIndexHash;
    uint                 curSelection;
//     bool                 enableHighlight;
//     bool                 enableMouse;
};

#endif
