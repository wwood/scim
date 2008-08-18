/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/
#ifndef SCIMACTIONS_H
#define SCIMACTIONS_H

#include <qtoolbutton.h>

class SkimToolButton : public QToolButton
{
Q_OBJECT
public:
    SkimToolButton ( QWidget * parent, const char * name = 0 );
    virtual ~SkimToolButton ();
    virtual QSize sizeHint () const;
    virtual QSize minimumSizeHint () const;
public slots:
    virtual void setTextLabel(const QString &, bool);
protected:
    virtual void drawButtonLabel( QPainter * );
private:
    class SkimToolButtonPrivate;
    SkimToolButtonPrivate * d;
};

#include <kaction.h>
namespace scim_kde{
  enum ScimActionPlugOption{
    ToolBarPreferIcon = 1,/*when no icon, just text label, otherwise the same as ToolBarIconOnly*/
    ToolBarIconOnly = 1 << 1,
    ToolBarIconUseText = 1 << 2,
    ToolBarTextOnly = 1 << 3,
  };
}

class ScimToolBar;

class ScimAction : public KAction
{
Q_OBJECT
public:
    ScimAction(const QString &text, const QIconSet &pix, const KShortcut &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const char *name);
    ScimAction (const QString &text, const KShortcut &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const char *name);
    ScimAction (const QString &text, const QIconSet &pix, 
      KActionCollection *parent, const QString & _uuid, const char *name);
    ScimAction (const QString &text, const QIconSet &pix, 
      KActionCollection *parent, const int & _id, const char *name);
    ScimAction (const QString &text, KActionCollection *parent, const int & _id, const char *name);
    ScimAction (const KGuiItem &item, const KShortcut &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const char *name);
    virtual ~ScimAction();

    void unplugAll();
    void setVisible(bool b) {m_visible = b;};
    void setCurrentShown(bool b) {m_currentShown = b;};
    bool visible() {return m_visible;};
    bool currentShown() {return m_currentShown;};
    virtual int plug (QWidget *widget, int index=-1);
    virtual void setOption(scim_kde::ScimActionPlugOption op){m_option = op;};
    virtual void setDisplayedText(const QString &);
    virtual void setCurrentIconSet(const QIconSet &);

public slots:
    virtual void setIcon(const QString &icon);
    virtual void setIconSet (const QIconSet &iconSet);

signals:
    void activated(QString & uuid);
    void activated(int id);

protected slots:
    void init();

    //These functions are used to honor QToolButton's individual configurable TextIcon Mode.
    virtual void updateDText (int i);
    virtual void updateCIconSet( int id );
    virtual void slotActivated ();

protected:
    bool updateButtonIcon(QToolButton *, ScimToolBar *);
    bool iconOnlyButton();
    virtual void updateShortcut (QPopupMenu* menu, int id);
    
    scim_kde::ScimActionPlugOption m_option;
    QString m_displayText;
    QIconSet m_curIconset;
    QString m_uuid;
    bool m_visible, m_currentShown;
    int m_id;
};
// plug into toolbar QComboBox, while pluging into popupmenu QMenu
#include <qpopupmenu.h>

struct SubMenuInfo {
    QString path;
    QMenuItem* mitem;
    int parent;  //parent item id
};

typedef QMap<int, SubMenuInfo> SubMenuInfoRepository;
typedef QMap<QString, int> Path2ID;
typedef QMap<int, QPopupMenu*> PopupRepository;

class ScimComboAction : public ScimAction
{
Q_OBJECT
public:
    ScimComboAction (const QString &text, const QIconSet &pix, const KShortcut &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const char *name);
    ScimComboAction (const QString &text, const KShortcut &cut, const QObject *receiver, const char *slot, KActionCollection *parent, const char *name);

    virtual ~ScimComboAction();
    virtual QPopupMenu * popup () {  return &m_data;  };
    virtual int plug (QWidget *widget, int index=-1);
    int current(){return m_currentItemIndex;};
    int count(){ return m_data.count(); };
    void setItemVisible(int id, bool);
    void setItemEnabled(int id, bool);
signals:
    void itemActivated(int);
    void itemActivated(const QString & path);
public slots:
    void menuItemActivated(int id);
    int insertItem ( const QString & path, const QPixmap & existpix,
      const QString & text, bool isMenu = false, int id = -1);
    void insertItem ( const QIconSet & icon, const QString & text, int id = -1);
    void insertItem ( const QIconSet & icon, const QString & text, 
      const QObject * receiver, const char * member, int id = -1);
    void insertItem ( const QIconSet & icon, const QString & text, QPopupMenu * popup, int id = -1);

    void changeItem ( const QPixmap & im, const QString & t, int index );
    QString text ( int index ) const;
    const QIconSet* iconSet(int) const;
    void clear();
    void slotPopup();

// protected slots: this should be a protected slot
    virtual void slotActivated ();
protected:
    void init();
private:
    QPopupMenu m_data;
    int m_currentItemIndex;
    SubMenuInfoRepository m_subInfoRep;
    Path2ID m_path2ID;
    PopupRepository m_popups;
};
#endif
