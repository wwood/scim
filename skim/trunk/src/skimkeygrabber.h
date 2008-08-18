/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/
#ifndef SKIMKEYGRABBER_H
#define SKIMKEYGRABBER_H

#include <kdialogbase.h>

class KLineEdit;

#include <qtoolbutton.h>

class SkimShortcutEditor : public QWidget
{
Q_OBJECT
public:
    SkimShortcutEditor(QWidget *parent = 0, const char *name = 0);
    KLineEdit * m_lineEditor;
private slots:
    void invokeGrabber();
private:
    QToolButton * m_button;
};

class KEditListBox;
class SkimShortcutListEditor : public KDialogBase
{
Q_OBJECT
public:
    SkimShortcutListEditor(QWidget *parent = 0, const char *name = 0);
    void setStringList(QStringList &);
    QString getCombinedString();
private slots:
    void verifyShortcut(const QString &);
private:
    SkimShortcutEditor* m_editor;
    KEditListBox* m_listBox;
};

class SkimEditShortcutButton : public QToolButton
{
Q_OBJECT
public:
    SkimEditShortcutButton(QWidget *parent = 0, const char *name = 0);
public slots:
    void setShortcuts(const QString &);
//     void setEditor(KLineEdit *k);
signals:
    void setEditorText(const QString &);
private slots:
    void invokeShortcutListEditor();
private:
    KLineEdit * m_lineEditor;
    QString m_shortcuts;
};

//uncomment the following line to test none-X11 platform
// #undef Q_WS_X11

class QCheckBox;

class SkimKeyGrabber : public KDialogBase
{
Q_OBJECT
public:
    SkimKeyGrabber(QWidget *parent = 0, const char *name = 0);
    QString getShortcut() {return m_finalKeys;};
    ~SkimKeyGrabber();

public slots:
    int exec();
protected:
#ifdef Q_WS_X11
    bool x11Event ( XEvent * );
    void addCheckModifier(QCheckBox *, UINT16);
#else
    void keyPressEvent ( QKeyEvent * e );
    void keyReleaseEvent ( QKeyEvent * e );
    QString m_keys;
#endif
private:
    class SkimKeyGrabberPrivate;
    SkimKeyGrabberPrivate* d;
    void checkKeys(int key, QValueList<int> & oldkeys, QValueList<int> & newkeys, QCheckBox * cb);
    QString m_finalKeys;
    QCheckBox * m_ctrl;
    QCheckBox * m_alt;
    QCheckBox * m_shift;
    QCheckBox * m_release;
#ifdef Q_WS_X11
    QCheckBox * m_capslock;
    QCheckBox * m_meta;
    QCheckBox * m_hyper;
    QCheckBox * m_super;
#endif
//     QCheckBox * leftRight;
};

#endif
