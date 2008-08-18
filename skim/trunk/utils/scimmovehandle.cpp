/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "scimmovehandle.h"

ScimMoveHandle::ScimMoveHandle( QWidget *body, QWidget *dw )
    : ScimDragableFrame( body, dw, "scim_movehandle_internal" ), m_direction(QBoxLayout::LeftToRight)
{
}

void ScimMoveHandle::paintEvent( QPaintEvent *e )
{
    QPainter p( this );
    QStyle::SFlags flags = QStyle::Style_Default | QStyle::Style_Horizontal;
    if ( isEnabled() )
        flags |= QStyle::Style_Enabled;

    style().drawPrimitive( QStyle::PE_DockWindowHandle, &p,
                        QStyle::visualRect(QRect( 0, 0, width(), height() ), this),
                        colorGroup(), flags );
    QWidget::paintEvent( e );
}

QSize ScimMoveHandle::minimumSizeHint() const
{
    if(m_direction == QBoxLayout::LeftToRight || m_direction == QBoxLayout::RightToLeft)
        return QSize( style().pixelMetric( QStyle::PM_DockWindowHandleExtent, this ), 0 );
    else
        return QSize( 0, style().pixelMetric( QStyle::PM_DockWindowHandleExtent, this ) );
}

QSizePolicy ScimMoveHandle::sizePolicy() const
{
    if(m_direction == QBoxLayout::LeftToRight || m_direction == QBoxLayout::RightToLeft)
        return QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Preferred );
    else
        return QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
}

QBoxLayout::Direction ScimMoveHandle::direction()
{
    return m_direction;
}

void ScimMoveHandle::setDirection(QBoxLayout::Direction d)
{
    m_direction = d;
}

#include "scimmovehandle.moc"
