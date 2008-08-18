/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/
#ifndef SCIMLOOKUPTABLE_H
#define SCIMLOOKUPTABLE_H

#include <scimlistbox.h>
#include <qtoolbutton.h>

class ScimMoveHandle;
/**
@author spider
*/
class ScimLookupTable : public ScimListBox
{
Q_OBJECT
public:
    ScimLookupTable(QWidget *parent, QVBoxLayout* _dockLayout, const char *name = 0, bool _isVertical = false);

    ~ScimLookupTable();
    void setVertical(bool b);
signals:
  void previousPageRequested();
  void nextPageRequested();
public slots:
  void enablePreviousPage( bool );
  void enableNextPage( bool );
  void switchMode();
  bool isAttached();
protected:
    QToolButton* PrePageBtn;
    QToolButton* NextPageBtn;
    ScimMoveHandle* ModeSwitchBtn;
private:
    QHBoxLayout* bottomLineLayout;
    QSpacerItem* BottomSpacer;

    QWidget* attachedParent;
    QVBoxLayout* dockWindowAreaLayout;
};

#endif
