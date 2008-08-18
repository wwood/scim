/***************************************************************************
 *   Copyright (C) 2003 by liuspider                                          *
 *   sharecash@163.net                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "skimapplet.h"

#include <klocale.h>
#include <kglobal.h>
#include <kapplication.h>

#include <kdeversion.h>
#include <dcopclient.h>  //only needed when KDE < 3.4.0
#define KDEGT340 (KDE::version() >= KDE_MAKE_VERSION(3,4,0))

#include <kwin.h>
#include <kdebug.h>

#include <qxembed.h>
#include <qlayout.h>
#include <qcursor.h>
#include <qtimer.h>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

extern "C"
{
    KDE_EXPORT KPanelApplet* init(QWidget *parent, const QString& configFile)
    {
        return new SkimApplet(configFile, KPanelApplet::Stretch,
                                    KPanelApplet::Preferences, parent, "skimapplet");
    }
}

SkimApplet::SkimApplet(const QString& configFile, Type type, int actions,
                       QWidget *parent, const char *name)
    : KPanelApplet(configFile, type, actions, parent, name), DCOPObject("SkimApplet"),
      m_emittedDestroyedSignal(true), m_autoHideHandle(false), m_scheduledWindow(0)
{
//     kapp->dcopClient()->detach();
//     kapp->dcopClient()->registerAs("kicker_skimapplet", false);
//     kapp->dcopClient()->attach();
//     KGlobal::locale()->insertCatalogue("skimapplet");
    setBackgroundOrigin(QWidget::AncestorOrigin);
//     setAlignment(KPanelApplet::Center);
//     setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QHBoxLayout * hbox = new QHBoxLayout(this, 0, 0);
    hbox->setAutoAdd(true);
    hbox->setAlignment(Qt::AlignCenter);
    m_embedWin = new QXEmbed(this);
    connect(m_embedWin, SIGNAL(embeddedWindowDestroyed()), this, SLOT(hideAll()));
    m_embedWin->setBackgroundOrigin(QWidget::AncestorOrigin);
    m_embedWin->setBackgroundMode(QWidget::X11ParentRelative);

    //immediately after the contruction of this applet, the pluginmanager will call
    //show in the parent->parentWidget(), so we have to use a timer to schedule a hide call
    QTimer::singleShot(0, this, SLOT(hideAll()));

    connect(kapp, SIGNAL(shutDown()), this, SLOT(shuttingDown()));

    if(appletHandleDragWidget())
        appletHandleDragWidget()->installEventFilter(this);
}

SkimApplet::~SkimApplet()
{
    //KApplication::closingDown() always return false
    notifyEmbedWindow(KApplication::closingDown());
//     KGlobal::locale()->removeCatalogue("skimapplet");
}

void SkimApplet::shuttingDown()
{
    notifyEmbedWindow(true);
}

void SkimApplet::notifyEmbedWindow(bool exiting)
{
    if(!m_emittedDestroyedSignal)
    {
        m_emittedDestroyedSignal = true;
        if(kapp->dcopClient()->isAttached())
        {
    //         QErrorMessage * warningBox = QErrorMessage::qtHandler ();
    //         warningBox->message(QString("%d closing applet").arg(KApplication::closingDown()));
            QByteArray data;
            QDataStream arg(data, IO_WriteOnly);
            arg << exiting;
            emitDCOPSignal("appletDestroyed(bool)", data);
        }
    }
}

void SkimApplet::hideAll()
{
    if(QWidget * w = appletHandleWidget())
        w->hide();
}

void SkimApplet::resizeEvent ( QResizeEvent * e )
{
    if(e->size().width() >= 0 && e->size().height() >= 0)
    {
        QSize contentSize = e->size();
        if(QWidget * w = appletHandleWidget())
            if(orientation() == Qt::Horizontal)
                contentSize.setWidth(contentSize.width() - w->width());
            else
                contentSize.setHeight(contentSize.height() - w->height());

        emit preferedSizeChanged(contentSize, (int)orientation());
        {
            QByteArray data;
            QDataStream arg(data, IO_WriteOnly);
            arg << contentSize << (int)orientation();
            emitDCOPSignal("preferedSizeChanged(QSize, int)", data);
        }
    }
    KPanelApplet::resizeEvent(e);
}

void SkimApplet::positionChange( Position p )
{
    //hide the handle if required
    slotLeaveEvent ();

    //this function will generate resizeEvent
    KPanelApplet::positionChange(p);
}

void SkimApplet::embedWindow(Q_UINT32 intw)
{
    m_emittedDestroyedSignal = false;
    WId wid = (WId)(intw);
    KWin::WindowInfo wininfo = KWin::windowInfo(wid);

    if(wininfo.valid(true))
    {
        m_scheduledWindow = wid;
        realEmbedWindow();
    }
    else
        kdError() << "Something is terribly WRONG: embeded window WId is not correct!!";
}

void SkimApplet::realEmbedWindow()
{
    if(m_scheduledWindow)
    {
        KWin::WindowInfo wininfo = KWin::windowInfo(m_scheduledWindow);

        if(wininfo.valid(true))
        {
            m_embedWin->embed(m_scheduledWindow);
#ifdef Q_WS_X11
            //as the embeding window may not have be shown, so 
            //we map it. We can not get the corresponding qwidget 
            //object, so X call is used.
            XMapWindow(qt_xdisplay(), m_scheduledWindow);
#endif
            m_scheduledWindow = 0;
            parentWidget()->parentWidget()->show();
            show();

            //these two calls will make sure handler is shown correctly
            slotEnterEvent ();
            slotLeaveEvent ();
        }
    }
}

void SkimApplet::setAutoHideHandle(bool b)
{
    m_autoHideHandle = b;
    if(QWidget * w = appletHandleWidget())
        if(m_autoHideHandle)
        {
            w->installEventFilter(this);
            w->hide();
        }
        else
        {
            w->removeEventFilter(this);
            w->show();
        }
}

#include <qwidgetlist.h>
QWidget * SkimApplet::containerWidget()
{
    if(KDEGT340)   //this is KDE 3.4
        if( parentWidget() && parentWidget()->parentWidget())
            return parentWidget()->parentWidget();
        else
            return 0;
    else
    {   //this is KDE 3.3
        //FIXME: shall we cache the result?
        QWidgetList  *list = QApplication::topLevelWidgets();
        QWidgetListIt it( *list );  // iterate over the widgets
        QWidget * w, *ret = 0;
        while ( (w=it.current()) != 0 ) {   // for each top level widget...
            if(!strcmp(w->name(), "Panel"))
            {
                ret = dynamic_cast<QWidget *>(w->child("skim_panelappletcontainer"));
                break;
            }
        ++it;
        }
        delete list;
        return ret;
    }
}

QWidget * SkimApplet::appletHandleWidget()
{
    if(QWidget * w = containerWidget())
        return dynamic_cast<QWidget *>(w->child(0, "AppletHandle"));
    else
        return 0;
}

QWidget * SkimApplet::appletHandleDragWidget()
{
    if(appletHandleWidget())
        return dynamic_cast<QWidget *>(appletHandleWidget()->child(0, "AppletHandleDrag"));
    else
        return 0;
}

bool SkimApplet::eventFilter(QObject * object, QEvent * e)
{
    bool ret = false;

    if(object == appletHandleWidget())
    {
        switch ( e->type() ) {
            case QEvent::Leave:
                if(m_autoHideHandle)
                    slotLeaveEvent ();
                break;
            default:
                break;
        }
    }
    else if(object == appletHandleDragWidget())
    {
        static QMouseEvent * lastMouseButtonPressEvent = 0;
        static bool omitNextButtonPress = false;
        switch ( e->type() ) {
            //HACK START: this is an ungly hack to achieve emitting doubleClick signal when
            // the user double click the AppletHandleDrag:
            // this hack highly depends on the underlying implementation details of kicker,
            // especially: in kicker/core/applethandle.cpp, AppletHandle installs an
            // eventfilter to AppletHandleDrag, which filter all MouseButtonPress events of
            // widget AppletHandleDrag. Thus, without this hack, no MouseButtonDblClick
            // events can be generated. So here we filter all MouseButtonPress events, to
            // make them go through Qt built-in handlers, which will generate DblClick
            // events. However, we need to feed MouseButtonPress to AppletHandle::eventfilter
            // to activate the movement of applet: if users move the mouse, then we send
            // the last mouse button press event to appletHandleDragWidget, so AppletHandle
            // can emit a signal to notify its parent to move the applet.

            // As in void ContainerArea::startContainerMove(BaseContainer *a), ContainerArea
            // would grab the mouse, so no further MouseMove event will be received by this
            // filter after sending out lastMouseButtonPressEvent, until the movement is
            // completed. 
            case QEvent::MouseMove:
                if(lastMouseButtonPressEvent)
                {
                    omitNextButtonPress = true;
                    QApplication::sendEvent(object, lastMouseButtonPressEvent);
                    ret = true;
                }
                break;
            case QEvent::MouseButtonPress:
                if(!omitNextButtonPress)
                {
                    QMouseEvent * me = static_cast<QMouseEvent *>(e);
                    if(me->button() == Qt::LeftButton)  //only interested in LeftButton
                    {
                        if(lastMouseButtonPressEvent)
                            delete lastMouseButtonPressEvent;
                        lastMouseButtonPressEvent = new QMouseEvent(*me);
                        ret = true;
                    }
                    else
                        omitNextButtonPress = false;
                } else
                    omitNextButtonPress = false;
                break;
            //HACK END

            case QEvent::MouseButtonDblClick:
            {
                QByteArray data;
                emitDCOPSignal("doubleCliked()", data);
                ret = true;
            }
                break;
            default:
                break;
        }
    }

    return ret;
}

void SkimApplet::slotEnterEvent ()
{
    if(QWidget * w = appletHandleWidget())
        w->show();
}

void SkimApplet::slotLeaveEvent ()
{
    QWidget * area = containerWidget();
    if(m_autoHideHandle && area)
    {
        //if mouse is still in the area of the applet (including handle), do not hide the handle
        if(!area->rect().contains(area->mapFromGlobal(QCursor::pos())))
            hideAll();
    }
}

int SkimApplet::heightForWidth(int width) const
{
    if(m_embedWin->embeddedWinId())
        return KPanelApplet::heightForWidth(width);
    else
        return 0;
}

int SkimApplet::widthForHeight(int height) const
{
    if(m_embedWin->embeddedWinId())
        return KPanelApplet::widthForHeight(height);
    else
        return 0;
}

void SkimApplet::action(Action a)
{
    KPanelApplet::action(a);
}

#include "skimapplet.moc"
