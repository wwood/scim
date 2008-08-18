/***************************************************************************
 *   Copyright (C) 2003 by liuspider                                          *
 *   sharecash@163.net                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef SKIMAPPLET_H
#define SKIMAPPLET_H

#include <kpanelapplet.h>
#include <dcopobject.h>

/**
@author spider
*/
class SkimApplet : public KPanelApplet, public DCOPObject
{
Q_OBJECT
K_DCOP
public:
    SkimApplet(const QString& configFile, Type type, int actions = 0,
               QWidget *parent = 0, const char *name = 0);

    ~SkimApplet();

    bool eventFilter ( QObject * watched, QEvent * e );
    void action(Action a);

k_dcop:
    void embedWindow(Q_UINT32);
    void slotEnterEvent();
    void slotLeaveEvent();
    void setAutoHideHandle(bool);

public slots:
//     virtual void show();

signals:
    void preferedSizeChanged(QSize, int);
    void appletDestroyed(bool);
    void doubleCliked();

protected:
    int heightForWidth(int width) const;
    int widthForHeight(int height) const;
    void resizeEvent ( QResizeEvent * );
    void positionChange( Position );
    QWidget * containerWidget();
    QWidget * appletHandleWidget();
    QWidget * appletHandleDragWidget();
    void notifyEmbedWindow(bool);
protected slots:
    void realEmbedWindow();
    void hideAll();
    void shuttingDown();

private:
    bool m_emittedDestroyedSignal;
    bool m_autoHideHandle;
    class QXEmbed * m_embedWin;
    WId m_scheduledWindow;
};

#endif
