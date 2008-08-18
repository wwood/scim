/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/
#ifndef SCIMDRAGABLEFRAME_H
#define SCIMDRAGABLEFRAME_H

#include <qframe.h>
#include <qapplication.h>
class ScimDragableFrame : public QFrame
{
    Q_OBJECT
public:
    ScimDragableFrame( QWidget *body, QWidget *parent, const char *name = 0, WFlags f = 0 );
    virtual void reTopParent(QWidget *body);
    virtual QWidget* getTopParent() {return m_bodyWidget;};
    virtual void setKeepVisible(bool v) {m_keepVisible = v;};
    virtual bool isKeepVisible() {return m_keepVisible;};
    virtual bool scheduleAdjustSize();
    virtual bool screenContainsRect(QRect & visiblerect);
    virtual bool isMouseButtonPressed();
public slots:
    virtual void adjustSize();
signals:
    void doubleClicked();
//     void clicked();

protected:
    void mousePressEvent( QMouseEvent *e );
    void mouseMoveEvent( QMouseEvent *e );
    void mouseReleaseEvent( QMouseEvent *e );
    void mouseDoubleClickEvent( QMouseEvent *e );

    QRect m_screen;
    QPoint m_offset;
    uint m_keepVisible : 1;
    QWidget *m_bodyWidget;

private:
    uint m_hadDblClick    : 1;
    class QTimer * m_timer;
//     uint ctrlDown : 1;
};

#endif
