/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "scimactions.h"
#include "scimtoolbar.h"

#include <qpainter.h>
#include <qstyle.h>

#include <kiconloader.h>
class SkimToolButton::SkimToolButtonPrivate
{
public:
    QSize sizeHintCache;
    bool validSizeHintCache;
};

SkimToolButton::SkimToolButton( QWidget * parent, const char * name )
    : QToolButton(parent, name)
{
    d = new SkimToolButtonPrivate;
    d->validSizeHintCache = false;
}

SkimToolButton::~SkimToolButton ()
{
    delete d;
}

void SkimToolButton::setTextLabel(const QString & t, bool tipToo)
{
    if(t != textLabel())
    {
        d->validSizeHintCache = false;
        QToolButton::setTextLabel(t, tipToo);
    }
}

QSize SkimToolButton::sizeHint() const {
    return SkimToolButton::minimumSizeHint();
}

QSize SkimToolButton::minimumSizeHint() const {
    if(usesTextLabel())
    {
        //if only text is used, here we try to use minimum width
        if((!pixmap() || pixmap()->isNull()) && iconSet().isNull ())
        {
            if(!d->validSizeHintCache)
            {
                int fontwidth = fontMetrics().width(textLabel());
                if(fontwidth < QToolButton::sizeHint().height())
                    fontwidth = QToolButton::sizeHint().height();
                d->sizeHintCache = QSize(fontwidth, QToolButton::sizeHint().height());
            }
            return d->sizeHintCache;
        }
        else    //qt will add 4 pixels to the width of a button using text label.
            return QSize(QToolButton::sizeHint().width() - 4,
                        QToolButton::sizeHint().height());
    }
    else    //if there is only icon, when want a square box
        return QSize(QToolButton::sizeHint().height(),
                     QToolButton::sizeHint().height());
}

void SkimToolButton::drawButtonLabel( QPainter * p )
{
    //we have to implement our own drawButtonLabel if only text is available, otherwise the default
    // implementation add extra pixels on the left
    if(usesTextLabel() && ((!pixmap() || pixmap()->isNull()) && iconSet().isNull ()))
    {
        QColorGroup cg = colorGroup();
        style().drawItem(p, rect(), Qt::AlignCenter, cg, isEnabled(), 0, textLabel(), -1, &(cg.buttonText()));
    }
    else
        QToolButton::drawButtonLabel(p);
}

ScimAction::ScimAction(const QString &text, const QIconSet &pix, const KShortcut &cut,
                       const QObject *receiver, const char *slot, KActionCollection *parent, const char *name)
:KAction (text, pix, cut, receiver, slot, parent, name),m_option(scim_kde::ToolBarIconUseText) {
    init();
}
ScimAction::ScimAction (const QString &text, const KShortcut &cut, const QObject *receiver,
                        const char *slot, KActionCollection *parent, const char *name)
:KAction (text, cut, receiver, slot, parent, name) {
    init();
}
ScimAction::ScimAction (const QString &text, const QIconSet &pix,
                        KActionCollection *parent, const QString & _uuid, const char *name)
:KAction (text, pix, "", 0, 0, parent, name), m_uuid(_uuid) {
    init();
}
ScimAction::ScimAction (const QString &text, const QIconSet &pix, 
                        KActionCollection *parent, const int & _id, const char *name)
    :KAction (text, pix, "", 0, 0, parent, name), m_id(_id)
{
    init();
}
ScimAction::ScimAction (const QString &text, KActionCollection *parent, 
                        const int & _id, const char *name)
    :KAction (text, "", 0, 0, parent, name), m_id(_id) 
{
    init();
}
ScimAction::ScimAction (const KGuiItem &item, const KShortcut &cut, const QObject *receiver,
                        const char *slot, KActionCollection *parent, const char *name)
    :KAction (item, cut, receiver, slot, parent, name), m_option(scim_kde::ToolBarPreferIcon)
{
    init();
}

void ScimAction::init() {
    setDisplayedText(text());
    setCurrentIconSet(iconSet());
    m_visible = true;
    m_currentShown = true;
}

ScimAction::~ScimAction() {
}

void ScimAction::updateDText (int i) {
    QWidget *w = container( i );
    if ( w->inherits( "ScimToolBar" ) ) {
        QWidget *button = static_cast<ScimToolBar *>(w)->getWidget( itemId( i ) );
        if ( button->inherits( "QToolButton" ) ) {
            static_cast<QToolButton *>(button)->setTextLabel( m_displayText );
            w->adjustSize();
            return;
        }
    }
    KAction::updateText(i);
}

bool ScimAction::updateButtonIcon(QToolButton * button, ScimToolBar * bar)
{
    if(!m_curIconset.isNull())
        button->setIconSet(m_curIconset);
    else
        return false;

    return true;
}

void ScimAction::updateCIconSet( int id ) {
    QWidget *w = container( id );
    int itemId_ = itemId( id );
    if ( w->inherits( "ScimToolBar" ) ) {
        ScimToolBar * bar = static_cast<ScimToolBar *>(w);
        QWidget *widget = bar->getWidget( itemId_ );
        if ( widget->inherits( "QToolButton" ) ) {
            QToolButton * button = static_cast<QToolButton *>(widget);

            updateButtonIcon(button, bar);

            if(m_option & scim_kde::ToolBarPreferIcon)
                if(!iconOnlyButton())
                    button->setUsesTextLabel( true );
                else
                    button->setUsesTextLabel( false );

            w->adjustSize();
            return;
        }
    }
    KAction::updateIconSet(id);
}

void ScimAction::setDisplayedText(const QString &t) {
    m_displayText = t;
    int len = containerCount();
    for( int id = 0; id < len; ++id ) {
        updateDText(id);
    }
}

void ScimAction::setIconSet(const QIconSet & is) {
    setCurrentIconSet(is);
    KAction::setIconSet(is);
}

void ScimAction::setIcon(const QString &icon) {
    setCurrentIconSet(KGlobal::iconLoader()->loadIconSet(icon.local8Bit(), KIcon::Small ));
    KAction::setIcon(icon);
}

void ScimAction::unplugAll() {
    while ( containerCount() != 0 ) {
        QWidget * w = container( 0 );
        unplug (w);
    }
    KAction::unplugAll();
}

void ScimAction::setCurrentIconSet(const QIconSet &q) {
    m_curIconset = q;
    int len = containerCount();
    for( int id = 0; id < len; ++id )
        updateCIconSet(id);
}

bool ScimAction::iconOnlyButton()
{
    bool IconOnly = true;
    if(m_option & scim_kde::ToolBarPreferIcon) {
        if(m_curIconset.isNull())
            IconOnly = false;
    } else
        IconOnly = !( (m_option & scim_kde::ToolBarIconUseText) || (m_option & scim_kde::ToolBarTextOnly) )
                || (m_option & scim_kde::ToolBarIconOnly);

    return IconOnly;
}

int ScimAction::plug (QWidget *widget, int index) {
    if(widget->inherits("ScimToolBar")) {
        ScimToolBar *bar = static_cast<ScimToolBar *>( widget );
        int id_ = getToolButtonID();
        QToolButton* newScimTB =
            new SkimToolButton(bar,QCString("toolbutton_")+name());


        if(iconOnlyButton()) {
            newScimTB->setTextLabel(m_displayText, true);
        } else {
            newScimTB->setUsesTextLabel( true );
            newScimTB->setTextLabel(m_displayText, false);
            newScimTB->setTextPosition( QToolButton::BesideIcon );
        }

        bar->insertWidget(id_, 50, newScimTB,index);
        connect(newScimTB, SIGNAL( clicked() ), this, SLOT( slotActivated() ));

        addContainer( bar, id_ );
        connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

        if(!(m_option & scim_kde::ToolBarTextOnly))
            updateButtonIcon(newScimTB, bar);

        if ( parentCollection() )
            parentCollection()->connectHighlight( bar, this );
        bar->adjustSize();
        return containerCount() - 1;
    }
    return KAction::plug(widget, index);
}

void ScimAction::slotActivated () {
    if(m_uuid.length() > 0)
        emit activated(m_uuid);
    else
        emit activated(m_id);
    KAction::slotActivated ();
}
//FIXME: this is to overcome the warning:
// kdecore (KAction): WARNING: KAction::updateShortcut(): name = "change_server", cut = ; No KAccel, probably missing a parent collection.
void ScimAction::updateShortcut ( QPopupMenu* /*menu*/, int /*id*/ ) {}

#include <qpopupmenu.h>
#include <iostream>
#include <qcursor.h>

ScimComboAction::ScimComboAction (const QString &text, const QIconSet &pix, const KShortcut &cut,
                                  const QObject *receiver, const char *slot, 
                                  KActionCollection *parent, 
                                  const char *name)
: ScimAction(text,pix, cut, receiver, slot, parent, name),m_currentItemIndex(-1)
{
  init();
}
ScimComboAction::ScimComboAction (const QString &text, const KShortcut &cut,
                                  const QObject *receiver, const char *slot, 
                                  KActionCollection *parent, 
                                  const char *name)
: ScimAction(text,cut, receiver, slot, parent, name),m_currentItemIndex(-1)
{
  init();
}

ScimComboAction::~ScimComboAction() {  m_data.deleteLater(); }

void ScimComboAction::init()
{
    connect(popup(), SIGNAL(activated(int)), SLOT(menuItemActivated(int)));
}

int ScimComboAction::plug (QWidget *widget, int index) {
    bool handled = false;
    if(widget->inherits("ScimToolBar")) {
        ScimToolBar *bar = static_cast<ScimToolBar *>( widget );
        int id_ = getToolButtonID();
        QToolButton* newScimTB =
            new SkimToolButton(bar,QCString("toolbutton_")+name());
        addContainer( bar, id_ );
        if(!iconOnlyButton()) {
            newScimTB->setUsesTextLabel( true );
            newScimTB->setTextLabel(m_displayText, false);
            newScimTB->setTextPosition( QToolButton::BesideIcon );
        } else
            newScimTB->setTextLabel(m_displayText, true);
        connect(newScimTB, SIGNAL(clicked()), SLOT(slotActivated()));
        updateButtonIcon(newScimTB, bar);
        bar->insertWidget(id_, 50, newScimTB,index);
        connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );
        handled = true;
    } else if(widget->inherits("QPopupMenu")) {
        QPopupMenu *popupP = static_cast<QPopupMenu *>( widget );
        int id_ = popupP->insertItem(KAction::iconSet(), KAction::text(), popup());
        addContainer( popupP, id_ );
        connect( popupP, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );
        handled = true;
    }

    if(handled) {
        if ( parentCollection() )
            parentCollection()->connectHighlight( widget, this );

        return containerCount() - 1;
    } else
        return KAction::plug(widget, index);
}

void ScimComboAction::slotPopup () {
  popup()->popup(QCursor::pos(), 0);
}
void ScimComboAction::slotActivated () {
    KAction::slotActivated ();
    slotPopup ();
}

void ScimComboAction::clear() {
    popup()->clear();
    PopupRepository::Iterator pit;
    for ( pit = m_popups.begin(); pit != m_popups.end(); ++pit )
        pit.data()->deleteLater();

    m_popups.clear();
    m_path2ID.clear();
    m_subInfoRep.clear();
    m_data.clear();
}

int ScimComboAction::insertItem (const QString & path, const QPixmap & pix,
                                 const QString & text, bool isMenu, int requestedid)
{
    QString parentPath = path.section('/', 0, -2);
    SubMenuInfo item;
    bool parentIsSaved = path.contains('/') > 1 && m_path2ID.contains(parentPath);
    item.path = path;

    if(parentIsSaved)
        item.parent = m_path2ID[parentPath];
    else
        item.parent = 0xffffffff;

    bool usepix = !pix.isNull();

    int id;
    if(parentIsSaved) {
        QMenuItem* mitem = m_subInfoRep[item.parent].mitem;
        if(QPopupMenu *menu = mitem->popup()) {
            if(isMenu) {
              QPopupMenu* submenu = new QPopupMenu();
              if(usepix)
                id = menu->insertItem(pix, text, submenu, requestedid);
              else
                id = menu->insertItem(text, submenu, requestedid);
              connect(submenu, SIGNAL(activated(int)), SLOT(menuItemActivated(int)));
              m_popups[id] = submenu;
            } else {
              if(usepix)
                id = menu->insertItem(pix, text, requestedid);
              else
                id = menu->insertItem(text, requestedid);
              menu->setItemParameter(id, id);
            }
            item.mitem = menu->findItem(id);
        } else {
            std::cerr << "No popupmenu found in" << path << " " << id << " " << item.parent << "\n";
        }
    } else {
        if(isMenu) {
          QPopupMenu* submenu = new QPopupMenu();
          if(usepix) {
              id = m_data.insertItem(pix, text, submenu, requestedid);
          } else {
              id = m_data.insertItem(text, submenu, requestedid);
          }
          connect(submenu, SIGNAL(activated(int)), SLOT(menuItemActivated(int)));
          m_popups[id] = submenu;
        } else {
          if(usepix) {
              id = m_data.insertItem(pix, text, requestedid);
          } else {
              id = m_data.insertItem(text, requestedid);
          }
          m_data.setItemParameter(id, id);
        }
        item.mitem = m_data.findItem(id);
    }
    m_path2ID[path] = id;
    m_subInfoRep[id] = item;

    return id;
}

void ScimComboAction::menuItemActivated(int id) {
    if(m_subInfoRep.contains(id)) {
      emit itemActivated(m_subInfoRep[id].path);
      emit itemActivated(id);
    } else 
      std::cerr << "menuItemActivated id does not exist in m_subInfoRep " << id << "\n";
}

void ScimComboAction::insertItem ( const QIconSet & icon, const QString & text, int index ) {
    if(index < 0)
        index = count();
    m_data.insertItem(icon,text,index);
}
void ScimComboAction::insertItem ( const QIconSet & icon, const QString & text,
                                   const QObject * receiver, const char * member, int index ) {
    if(index < 0)
        index = count();
    m_data.insertItem(icon,text,receiver,member,0,index);
}

void ScimComboAction::insertItem ( const QIconSet & icon, const QString & text, QPopupMenu * popup, int index ) {
    if(index < 0)
        index = count();
    m_data.insertItem(icon,text,popup,index);
}

void ScimComboAction::setItemVisible(int id, bool b) {
    if(m_subInfoRep.contains(id)) {
      if(!m_subInfoRep.contains(m_subInfoRep[id].parent)) {  //top level entry
          m_data.setItemVisible(id, b);
          popup()->setItemVisible(id, b);
      } else { //sub entry
        QPopupMenu * submenu = m_subInfoRep[m_subInfoRep[id].parent].mitem->popup();
        if(submenu) {
          submenu->setItemVisible(id, b);
        }
      }
    }
}
void ScimComboAction::setItemEnabled(int id, bool b) {
    if(m_subInfoRep.contains(id)) {
      if(!m_subInfoRep.contains(m_subInfoRep[id].parent)) {  //top level entry
          m_data.setItemEnabled(id, b);
          popup()->setItemEnabled(id, b);
      } else { //sub entry
        QPopupMenu * submenu = m_subInfoRep[m_subInfoRep[id].parent].mitem->popup();
        if(submenu) {
          submenu->setItemEnabled(id, b);
        }
      }
    }
}
void ScimComboAction::changeItem ( const QPixmap & im, const QString & t, int id ) {
    if(m_subInfoRep.contains(id)) {
        if(!m_subInfoRep.contains(m_subInfoRep[id].parent)) {  //top level entry
            m_data.changeItem(id,im, t);
        } else { //sub entry
            QPopupMenu * submenu = m_subInfoRep[m_subInfoRep[id].parent].mitem->popup();
            if(submenu) {
            submenu->changeItem(id,im, t);
            }
        }
    }
}

QString ScimComboAction::text ( int index ) const {
    return m_data.text(index);
}

const QIconSet * ScimComboAction::iconSet ( int index ) const {
    return m_data.iconSet(index);
}

#include "scimactions.moc"
