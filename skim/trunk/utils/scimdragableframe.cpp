/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "scimdragableframe.h"
#include <qtimer.h>

//this variable should be shared by all instances of this class
static bool mousePressed = false;

ScimDragableFrame::ScimDragableFrame( QWidget *body, QWidget *parent, const char *name, WFlags f )
    : QFrame( parent, name, f), m_keepVisible( FALSE ), m_bodyWidget( body ), m_timer( 0 )
{
//     ctrlDown = FALSE;
    m_screen = QApplication::desktop()->screenGeometry();
}

bool ScimDragableFrame::isMouseButtonPressed()
{
    return mousePressed;
}

void ScimDragableFrame::reTopParent(QWidget *body)
{
    m_bodyWidget = body;
}

void ScimDragableFrame::mousePressEvent( QMouseEvent *e )
{
//     ctrlDown = ( e->state() & ControlButton ) == ControlButton;
    e->ignore();
    if ( e->button() == LeftButton )
    {
        e->accept();
        m_hadDblClick = FALSE;
        mousePressed = TRUE;
        if(m_bodyWidget)
            m_offset = mapTo(m_bodyWidget, e->pos());
    }
    QFrame::mousePressEvent(e);
}
void ScimDragableFrame::mouseMoveEvent( QMouseEvent *e )
{
    if ( mousePressed && e->pos() != m_offset )
    {
    //     ctrlDown = ( e->state() & ControlButton ) == ControlButton;
        if(m_bodyWidget)
            m_bodyWidget->move(e->globalPos()-m_offset);
    }
    QFrame::mouseMoveEvent(e);
}
void ScimDragableFrame::mouseReleaseEvent( QMouseEvent *e )
{
//     ctrlDown = FALSE;
    if ( mousePressed )
    {
        mousePressed = FALSE;
        adjustSize();
    }
    QFrame::mouseReleaseEvent(e);
}

void ScimDragableFrame::mouseDoubleClickEvent( QMouseEvent *e )
{
    e->ignore();
    if ( e->button() != LeftButton )
        return;
    e->accept();
    emit doubleClicked();
    m_hadDblClick = TRUE;
}

bool ScimDragableFrame::scheduleAdjustSize()
{
    if(!m_timer)
    {
        m_timer = new QTimer(this);
        connect( m_timer, SIGNAL(timeout()), SLOT(adjustSize()) );
    }
    if(!m_timer->isActive()) {
        m_timer->start( 0, true );
        return true;    //another is waiting
    } else
        return false;
}

void ScimDragableFrame::adjustSize()
{
    if(isKeepVisible())
    {
      QFrame::adjustSize();
      QRect visiblerect = frameGeometry();

      if( !screenContainsRect(visiblerect) )
      {
          move( visiblerect.topLeft() );
      }
    }
    else if(m_bodyWidget) //otherwise if this widget is not keepvisible in screen,
        m_bodyWidget->adjustSize();  //then adjustSize() of m_bodyWidget should be called
    else
        QFrame::adjustSize();
}

bool ScimDragableFrame::screenContainsRect(QRect & visiblerect)
{
    if( !m_screen.contains( visiblerect ) )
    {
        QRect intersect = m_screen & visiblerect;
        if ( intersect.isEmpty() )
        {
            visiblerect.moveCenter( m_screen.center() );
        }
        else
        {
            if( intersect.contains( visiblerect.topLeft() ) )
            {
                visiblerect.moveBottomRight( intersect.bottomRight() );
            }
            else
            {
                visiblerect = intersect;
            }
        }
        return false;
    }

    return true;
}
#include "scimdragableframe.moc"
