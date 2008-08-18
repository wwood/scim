/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "scimtoolbar.h"
#include "scimdragableframe.h"
#include "qapplication.h"
#include "qstyle.h"

ScimToolBar::ScimToolBar(QWidget *parent, const char *name)
    : KToolBar(parent, name)
{
    //we have to save it here, because boxLayout->sizeHint() can
    //not be called in ScimToolBar::sizeHint(): discard the qualifier
    m_boxlayout = boxLayout();
}

ScimToolBar::~ScimToolBar() {}

QSize ScimToolBar::sizeHint() const
{
    //do not call minimumSizeHint(), otherwise it will lead to infinite loop
    return m_boxlayout->sizeHint().expandedTo( QSize(minimumWidth(), minimumHeight()) );
}

void ScimToolBar::adjustSize()
{
    if(ScimDragableFrame * dragframe = dynamic_cast<ScimDragableFrame *>(parentWidget())) {
        dragframe->scheduleAdjustSize();
    }
    else
        parentWidget()->adjustSize();
}

#include "scimtoolbar.moc"
