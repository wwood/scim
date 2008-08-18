/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "scimlistbox.h"
#include <qvariant.h>
#include <qlayout.h>
#include <qlabel.h>

#include <qwidget.h>
#include <qsize.h>

ScimListBox::ScimListBox(QWidget *parent, const char *name, bool _isVertical)
    : ScimDragableFrame(parent, parent, name), curSelection(0)
{
    setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum, 0, 0) );

    QBoxLayout::Direction defaultLayoutDirection = QBoxLayout::LeftToRight;

    if(_isVertical)
        defaultLayoutDirection = QBoxLayout::TopToBottom;

    layout = new QBoxLayout( this, defaultLayoutDirection, 3, 2, "defaultLayout");
    layout->setResizeMode( QLayout::FreeResize );

    for(int i=0; i < MAX_CANDIDATES; i++)
    {
        candidatesLbl[i] = new ScimStringListItem(this);
        candidatesLbl[i]->setFrameStyle( QFrame::NoFrame );
        candidatesLbl[i]->hide();
        candidatesLbl[i]->installEventFilter(this);
        candidatesLblIndexHash[candidatesLbl[i]] = i;
        layout->addWidget(candidatesLbl[i]);
    }
}

bool ScimListBox::eventFilter ( QObject * o, QEvent * e )
{
    if(candidatesLblIndexHash.contains(o))
        if(e->type() == QEvent::MouseButtonPress)
        {
            emit itemSelected(candidatesLblIndexHash[o]);
        }

    return false;
}

void ScimListBox::setVertical(bool b)
{
    QBoxLayout::Direction defaultLayoutDirection = QBoxLayout::LeftToRight;
    if(b)
        defaultLayoutDirection = QBoxLayout::TopToBottom;
    layout->setDirection(defaultLayoutDirection);
}

unsigned ScimListBox::updateContent( const QStringList & labellist,
                                     const QStringList & lookupstringlist,
                                     QValueList<scim::AttributeList> & attrslist,
                                     const bool pagesizefixed)
{
    static int max_width = m_screen.width() / 3;
    unsigned allitems = lookupstringlist.size();
    unsigned counter = 0;
    static QString labelSeparator, separator = " ";
    if(QBoxLayout::LeftToRight == layout->direction()) {
        labelSeparator = ".";
    } else {
        labelSeparator = ". ";
    }

    QString cache;
    scim::AttributeList attrs;
    scim::Attribute highlight;
    highlight.set_type(scim::SCIM_ATTR_DECORATE);
    highlight.set_value(scim::SCIM_ATTR_DECORATE_HIGHLIGHT);
    bool insertHighlight = false;

    static int cachedWidth = 0, curwidth;

    cachedWidth = 0;
    for(; counter < allitems && counter < MAX_CANDIDATES; counter++)
    {
        if(labellist[counter].isEmpty())
            cache = "";
        else    //only add the label if it is not empty
            cache = labellist[counter] + labelSeparator;

        attrs.clear();
        insertHighlight = (curSelection == counter);

        if(insertHighlight)
        {
            highlight.set_start(0);
            highlight.set_length(cache.length() + lookupstringlist[counter].length());
            attrs.push_back(highlight);
        }

        for(size_t j = 0; j < attrslist[counter].size(); j++)
        {
            attrslist[counter][j].set_start(attrslist[counter][j].get_start() + cache.length());
            attrs.push_back(attrslist[counter][j]);
        }
        candidatesLbl[counter]->setText(cache + lookupstringlist[counter] + separator, attrs);
        candidatesLbl[counter]->show();
        if(layout->direction() != QBoxLayout::TopToBottom && !pagesizefixed)
        {
            curwidth = candidatesLbl[counter]->minimumSizeHint().width();
            if(cachedWidth + curwidth > max_width)
            {
                counter++;
                break;
            }
            else
                cachedWidth += curwidth;
        }
    }

    for(int leftItems = counter; leftItems < MAX_CANDIDATES; leftItems++)
        candidatesLbl[leftItems]->hide();

    return counter;
}

void ScimListBox::updateHighlight(int i)
{
    curSelection = i;
}

#include "scimlistbox.moc"
