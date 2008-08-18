/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "scimlookuptable.h"
#include "scimmovehandle.h"
#include "scimactions.h"
#include "scimkdesettings.h"  //needed by the next header file
#include "src/extra_utils.h"

#include <kiconloader.h>

ScimLookupTable::ScimLookupTable(QWidget *parent, QVBoxLayout* _dockLayout, const char *name, bool _isVertical)
    : ScimListBox(parent, name, _isVertical), attachedParent(parent), dockWindowAreaLayout(_dockLayout)
{
    PrePageBtn = new SkimToolButton( this, "PrePageBtn" );
    PrePageBtn->setAutoRaise(true);
    NextPageBtn = new SkimToolButton( this, "NextPageBtn" );
    NextPageBtn->setAutoRaise(true);

    ModeSwitchBtn = new ScimMoveHandle( attachedParent, this );
    ModeSwitchBtn->setKeepVisible( false );

    bottomLineLayout = new QHBoxLayout(layout);
    BottomSpacer = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
    bottomLineLayout->addItem( BottomSpacer );
    bottomLineLayout->addWidget( PrePageBtn, 0, Qt::AlignRight);
    bottomLineLayout->addWidget( NextPageBtn, 0, Qt::AlignRight);
    bottomLineLayout->addWidget( ModeSwitchBtn, 0, Qt::AlignRight);

    connect( PrePageBtn, SIGNAL( clicked() ), this, SIGNAL( previousPageRequested() ) );
    connect( NextPageBtn, SIGNAL( clicked() ), this, SIGNAL( nextPageRequested() ) );
    connect( ModeSwitchBtn, SIGNAL( doubleClicked() ), this, SLOT( switchMode() ) );
    connect( this, SIGNAL( doubleClicked() ), this, SLOT( switchMode() ) );
}

ScimLookupTable::~ScimLookupTable(){}


void ScimLookupTable::setVertical( bool b )
{
    //the icons should be reset if we changed direction
    ScimListBox::setVertical(b);
    QIconSet iconSet;
    if( layout->direction() == QBoxLayout::TopToBottom )
    {
        iconSet = SmallIconSet( "up" );
        PrePageBtn->setIconSet( iconSet );
        iconSet = SmallIconSet( "down" );
        NextPageBtn->setIconSet( iconSet );
        ModeSwitchBtn->hide();
    }
    else
    {
        iconSet = SmallIconSet( "back", fontMetrics().height() );
        PrePageBtn->setIconSet( iconSet );
        iconSet = SmallIconSet( "forward", fontMetrics().height() );
        NextPageBtn->setIconSet( iconSet );
        ModeSwitchBtn->show();
    }
}
bool ScimLookupTable::isAttached()
{
    return (attachedParent == parentWidget());
}

void ScimLookupTable::switchMode()
{
    if(isAttached())
    {
        reparent(0,
                 Qt::WStyle_Customize|Qt::WStyle_NoBorder|Qt::WStyle_StaysOnTop|Qt::WX11BypassWM|Qt::WStyle_Tool,
                 mapToGlobal(geometry().topLeft()), true);
        reTopParent(this);
        setKeepVisible( true );
        ModeSwitchBtn->reTopParent(this);
        dockWindowAreaLayout->remove(this);
        dockWindowAreaLayout->invalidate();
        attachedParent->adjustSize();
        adjustSize();
        UPDATE_WINDOW_OPACITY(this);
    }
    else
    {
        reparent(attachedParent, QPoint(0,0), true);
        reTopParent(attachedParent);
        setKeepVisible( false );
        ModeSwitchBtn->reTopParent(attachedParent);
        dockWindowAreaLayout->addWidget(this);
        attachedParent->adjustSize();
        if(!attachedParent->isVisible())
            attachedParent->show();
    }
}

void ScimLookupTable::enablePreviousPage( bool b )
{
    if (b)
        PrePageBtn->show();
    else
        PrePageBtn->hide();
}

void ScimLookupTable::enableNextPage( bool b )
{
    if (b)
        NextPageBtn->show();
    else
        NextPageBtn->hide();
}

#include "scimlookuptable.moc"
