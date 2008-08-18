/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#define Uses_SCIM_EVENT
#define USE_SCIM_KDE_SETTINGS
#include <skimpluginmanager.h>

#include <qlabel.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qtooltip.h> 
#include <klocale.h>
#include <kkeynative.h> 

#include <klineedit.h>
#include <qmessagebox.h>
#include <keditlistbox.h>

#include "skimkeygrabber.h"

#if defined(Q_WS_X11)
#include <X11/Xlib.h>
#include <x11/scim_x11_utils.h>
const int XKeyPress = KeyPress;
const int XKeyRelease = KeyRelease;
#undef KeyPress
#undef KeyRelease

inline void checkBoxEnabled(bool is_mask, QCheckBox * checkbox)
{
    if(is_mask)
        checkbox->setChecked(true);
}

inline void hideCheckBox(bool is_enabled, QCheckBox * checkbox)
{
    if(!is_enabled)
        checkbox->hide();
}

#endif

class SkimKeyGrabber::SkimKeyGrabberPrivate
{
    public:
        scim::KeyboardLayout   m_keyboard_layout;
        int                    m_valid_key_mask;
#ifdef Q_WS_X11
        scim::KeyEvent         m_scimkey;
        Display               *m_display;
#else
        QValueList<int>        m_keys;
#endif
};

SkimKeyGrabber::SkimKeyGrabber(QWidget *parent, const char *name)
 : KDialogBase(KDialogBase::Plain, 0, parent, name, true, i18n("Grab Key"), KDialogBase::Cancel)
{
    setInputMethodEnabled(false);
    enableButtonOK(false);
    enableButtonCancel(true);
    QVBoxLayout * vb = new QVBoxLayout(plainPage());
    QLabel * lb = new QLabel(i18n("<center>Please press shortcut...</center><p>When you release a key this dialog will be closed automatically.</p>"), plainPage());
    lb->setAlignment(Qt::WordBreak);
//     lb->setAlignment(Qt::AlignCenter);
    vb->addWidget(lb);
    vb->addItem(new QSpacerItem(0, 10));
    QHBoxLayout * hb = new QHBoxLayout(vb, 3);
    m_ctrl = new QCheckBox(i18n("Control"), plainPage());
    hb->addWidget(m_ctrl);
    m_alt = new QCheckBox(i18n("Alt"), plainPage());
    hb->addWidget(m_alt);
    m_shift = new QCheckBox(i18n("Shift"), plainPage());
    hb->addWidget(m_shift);
    m_release = new QCheckBox(i18n("Release"), plainPage());
    hb->addWidget(m_release);

    d = new SkimKeyGrabberPrivate();
    d->m_keyboard_layout = scim::scim_get_default_keyboard_layout ();
    scim::KeyEvent key;
    scim::scim_string_to_key (key, scim::String(ScimKdeSettings::_Hotkeys_FrontEnd_ValidKeyMask().latin1()));

    d->m_valid_key_mask = (key.mask > 0)?(key.mask):0xFFFF;
    d->m_valid_key_mask |= scim::SCIM_KEY_ReleaseMask;

    hideCheckBox(key.is_alt_down(), m_alt);
    hideCheckBox(key.is_control_down(), m_ctrl);
    hideCheckBox(key.is_shift_down(), m_shift);
#ifdef Q_WS_X11
    QHBoxLayout * hb2 = new QHBoxLayout(vb, 3);
    m_capslock = new QCheckBox(i18n("CapsLock"), plainPage());
    hb2->addWidget(m_capslock);
    m_meta = new QCheckBox(i18n("Meta"), plainPage());
    hb2->addWidget(m_meta);
    m_hyper = new QCheckBox(i18n("Hyper"), plainPage());
    hb2->addWidget(m_hyper);
    m_super = new QCheckBox(i18n("Super"), plainPage());
    hb2->addWidget(m_super);

    hideCheckBox(key.is_caps_lock_down(), m_capslock);
    hideCheckBox(key.is_meta_down(), m_meta);
    hideCheckBox(key.is_hyper_down(), m_hyper);
    hideCheckBox(key.is_super_down(), m_super);

    d->m_display = qt_xdisplay();
#endif
}

SkimKeyGrabber::~SkimKeyGrabber()
{
    delete d;
}

#ifdef Q_WS_X11
void SkimKeyGrabber::addCheckModifier(QCheckBox * checkbox, UINT16 modifier)
{
    if(checkbox->isChecked())
        d->m_scimkey.mask |= modifier;
}

#endif

int SkimKeyGrabber::exec() {
    grabKeyboard();
#ifdef Q_WS_X11
    d->m_scimkey.mask = scim::SCIM_KEY_NullMask;
#endif
    int result = KDialogBase::exec();

    if( result == QDialog::Accepted)
    {
#ifdef Q_WS_X11
        addCheckModifier(m_release, scim::SCIM_KEY_ReleaseMask);
        addCheckModifier(m_ctrl, scim::SCIM_KEY_ControlMask);
        addCheckModifier(m_alt, scim::SCIM_KEY_AltMask);
        addCheckModifier(m_shift, scim::SCIM_KEY_ShiftMask);
        addCheckModifier(m_capslock, scim::SCIM_KEY_CapsLockMask);
        addCheckModifier(m_meta, scim::SCIM_KEY_MetaMask);
        addCheckModifier(m_hyper, scim::SCIM_KEY_HyperMask);
        addCheckModifier(m_super, scim::SCIM_KEY_SuperMask);

        d->m_scimkey.mask &= d->m_valid_key_mask;
        d->m_scimkey.layout = d->m_keyboard_layout;

        scim::String s;
        scim_key_to_string(s, d->m_scimkey);
        m_finalKeys = QString::fromLatin1(s.c_str());
#else
        QValueList<int> oldkeys = d->m_keys;
        d->m_keys.clear();
        checkKeys(Qt::Key_Control, oldkeys, d->m_keys, m_ctrl);
        checkKeys(Qt::Key_Shift, oldkeys, d->m_keys, m_shift);
        checkKeys(Qt::Key_Alt, oldkeys, d->m_keys, m_alt);
        d->m_keys += oldkeys;
        releaseKeyboard();

        for(uint i = 0; i<d->m_keys.size(); i++) {
            int qtKeyCode = d->m_keys[i];
            int mod;
            switch (qtKeyCode) {
                case Qt::Key_Control:
                    mod = scim::SCIM_KEY_ControlMask;
                break;
                case Qt::Key_Shift:
                    mod = scim::SCIM_KEY_ShiftMask;
                break;
                case Qt::Key_Alt:
                    mod = scim::SCIM_KEY_AltMask;
                break;
                case Qt::Key_Meta:
                    mod = scim::SCIM_KEY_MetaMask;
                break;
                default:
                    mod = scim::SCIM_KEY_NullMask;
            }
        KKeyNative b = KKey(qtKeyCode);
    //       std::cout << "code mod sym " << b.code() << " " << b.mod() << " " << b.sym() << "\n";

        scim::KeyEvent scimkey(b.sym(), mod);
        scimkey.layout = d->keyboard_layout;
    //       std::cout << " scimkey ascii: " << scimkey.get_ascii_code()<< "\n";
        scim::String s;
        scim_key_to_string(s, scimkey);

        if( i == d->keys.size() - 1 && (s == "Control" || s == "Shift" || s == "Alt") ) {
            if(m_release->isChecked())
                s += "+" + s;
        }
        m_finalKeys += QString::fromLatin1(s.c_str()) + "+";
        }
        if(m_release->isChecked())
            m_finalKeys += "KeyRelease";
        else
            m_finalKeys = finalKeys.left(m_finalKeys.length() - 1);
#endif
    }

    return result;
}

void SkimKeyGrabber::checkKeys(int key, QValueList<int> & oldkeys, QValueList<int> & newkeys, QCheckBox * cb) {
    if(!oldkeys.contains(key) && cb->isChecked()) {
      newkeys.push_back(key);
      oldkeys.remove(key);
    }
}

#ifdef Q_WS_X11
bool SkimKeyGrabber::x11Event ( XEvent * event )
{
    bool ret = false;
    if(event->type == XKeyPress)
    {
        XKeyEvent & xkey = event->xkey;
        d->m_scimkey = scim_x11_keyevent_x11_to_scim(d->m_display, xkey);

        checkBoxEnabled(d->m_scimkey.is_shift_down(), m_shift);
        checkBoxEnabled(d->m_scimkey.is_control_down(), m_ctrl);
        checkBoxEnabled(d->m_scimkey.is_alt_down(), m_alt);
        checkBoxEnabled(d->m_scimkey.is_caps_lock_down(), m_capslock);
        checkBoxEnabled(d->m_scimkey.is_meta_down(), m_meta);
        checkBoxEnabled(d->m_scimkey.is_super_down(), m_super);
        checkBoxEnabled(d->m_scimkey.is_hyper_down(), m_hyper);

        d->m_scimkey.mask &= d->m_valid_key_mask;
        ret = true;
    }
    else if(event->type == XKeyRelease)
    {
        KDialogBase::accept();
        ret = true;
    }

    return ret;
}

#else
void SkimKeyGrabber::keyPressEvent ( QKeyEvent * e ) {
    if(!e->isAutoRepeat() && !keys.contains(e->key())) {
        d->m_keys.push_back(e->key());
        switch (e->key()) {
            case Qt::Key_Control:
                m_ctrl->setChecked(true);
            break;
            case Qt::Key_Shift:
                m_shift->setChecked(true);
            break;
            case Qt::Key_Alt:
                m_alt->setChecked(true);
            break;
        }
    }

    e->accept();
}

void SkimKeyGrabber::keyReleaseEvent ( QKeyEvent * e ) {
    e->accept();
    KDialogBase::accept();
}
#endif

SkimShortcutEditor::SkimShortcutEditor(QWidget *parent, const char *name)
  : QWidget(parent, name)
{
    setInputMethodEnabled(false);
    (new QHBoxLayout(this))->setAutoAdd(true);
    m_lineEditor = new KLineEdit(this);
    m_lineEditor->setInputMethodEnabled(false);
    m_button = new QToolButton(this);
    m_button->setUsesTextLabel(true);
    m_button->setTextLabel("...");
    connect(m_button, SIGNAL(clicked()), SLOT(invokeGrabber()));
}

void SkimShortcutEditor::invokeGrabber() 
{
    SkimKeyGrabber g;
    if(g.exec() == QDialog::Accepted)
        m_lineEditor->setText(g.getShortcut());
}

SkimShortcutListEditor::SkimShortcutListEditor(QWidget *parent, const char *name)
 : KDialogBase(KDialogBase::Plain, 0, parent, name, true, 
   i18n("Edit Shortcut List"), KDialogBase::Ok | KDialogBase::Cancel)
{
    m_editor = new SkimShortcutEditor(plainPage());
    KEditListBox::CustomEditor * customeditor = 
            new KEditListBox::CustomEditor(m_editor, m_editor->m_lineEditor);
    m_listBox = new KEditListBox(i18n("Edit Shortcuts"), *customeditor, plainPage());
    (new QVBoxLayout(plainPage()))->addWidget(m_listBox);
    connect(m_listBox, SIGNAL(added(const QString &)), SLOT(verifyShortcut(const QString &)));
}

void SkimShortcutListEditor::setStringList(QStringList & s)
{
    m_listBox->insertStringList(s);
}

QString SkimShortcutListEditor::getCombinedString()
{
    return m_listBox->items().join(",");
}

void SkimShortcutListEditor::verifyShortcut(const QString &k)
{
    scim::KeyEventList keylist;
    scim::String str = k.latin1();
    if(!scim::scim_string_to_key_list(keylist, str))
    {
        QListBox * l = m_listBox->listBox();
        if( QListBoxItem * i = l->findItem(k))
        {
        l->removeItem(l->index(i));
        }
        QMessageBox::warning(this, i18n("Warning"), i18n("\"%1\" is not a valid shortcut!").arg(k),
        QMessageBox::Ok, QMessageBox::NoButton );
    }
}


SkimEditShortcutButton::SkimEditShortcutButton(QWidget *parent, const char *name)
    : QToolButton(parent, name), m_lineEditor(0)
{
    setUsesTextLabel(true);
    setTextLabel("...");
    connect(this, SIGNAL(clicked()), SLOT(invokeShortcutListEditor()));
}


void SkimEditShortcutButton::invokeShortcutListEditor()
{
    SkimShortcutListEditor ed;
    QStringList keys = QStringList::split(",", m_shortcuts);
    ed.setStringList(keys);
    if(ed.exec() == QDialog::Accepted) {
        if(m_shortcuts != ed.getCombinedString()) {
            emit setEditorText(ed.getCombinedString());
        }
    }
}
void SkimEditShortcutButton::setShortcuts(const QString & s)
{
    m_shortcuts = s.simplifyWhiteSpace().replace(' ', "");
}

#include "skimkeygrabber.moc"
