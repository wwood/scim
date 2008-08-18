/***************************************************************************
 *   Copyright (C) 2003-2006 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "scimstringrender.h"

#include <qwidget.h>
#include <kapplication.h>

class ScimStringRenderPrivate
{
public:
    QString text;
    scim::AttributeList attrs;
    int valid_widthhint;
    QSize storedSizeHint;
    QWidget * parent;
    int cursorPos;
    QPixmap contentBuffer;
    bool validBuffer;
    bool displayCursor;
};

ScimStringRender::ScimStringRender(QWidget * w) 
    : d(new ScimStringRenderPrivate)//, fm(w->fontMetrics())
{
    d->parent = w;
    d->cursorPos = -1;
    d->validBuffer = false;
    d->displayCursor = false;
}

void ScimStringRender::setDisplayCursor(bool b)
{
    d->displayCursor = b;
}

void ScimStringRender::drawString ( QPainter * globalp, const QRect & cr) const
{
    if(!cr.isValid())
        return;

    const QColorGroup& cg = KApplication::palette().active();

    QBrush bg = QBrush( d->parent->paletteBackgroundColor() );
    if ( d->parent->paletteBackgroundPixmap() )
        bg = QBrush( cg.background(), *(d->parent->paletteBackgroundPixmap()) );

    globalp->fillRect( cr, bg );

    if(!d->validBuffer)
    {
        d->validBuffer = true;
        if(d->storedSizeHint.isEmpty())
            return;

        d->contentBuffer.resize(d->storedSizeHint);

        QPainter painter(&d->contentBuffer);
        QPainter * p = &painter;

        p->fillRect( d->contentBuffer.rect(), bg );

        QFontMetrics fm( d->parent->fontMetrics() );

        p->setFont(d->parent->font());

        p->setPen( cg.text() );

        if(d->attrs.size())
        {
        uint start_index, length, wlen = d->text.length();
        QRect curRect;
        std::vector<QRect> drawForegroundRect;
        std::vector<QString> drawForegroundString;
        std::vector<scim::Attribute *> drawForegroundAttribute;
    
        //first draw all background and save the qrect for those which
        //need to draw customized foreground
        for(uint i=0; i<d->attrs.size (); ++i)
        {
            curRect = cr;
            start_index = d->attrs[i].get_start();
            length = d->attrs[i].get_length();
            const QString leftPart = d->text.mid(0, start_index);
            const QString curString = d->text.mid(start_index, length);
            int newLineCount = leftPart.contains('\n');

            if(d->text.contains('\n') > 0) {
            int newLineBegin = d->text.findRev('\n', start_index) + 1;
            curRect.setLeft(cr.x() +
                fm.width(d->text.mid(newLineBegin , start_index - newLineBegin)));
            curRect.setTop(cr.y() + fm.height() * newLineCount + fm.leading() * newLineCount);
            curRect.setBottom(curRect.y() + fm.height());
            } else {
            curRect.setLeft(cr.x() + fm.width(leftPart));
            curRect.setRight(curRect.x() + fm.width(curString));
            }

            uint dtype = d->attrs[i].get_type(), dvalue = d->attrs[i].get_value();
            bool needDrawForeground = false;
            if (start_index + length <= wlen)
            {
                if(dtype == scim::SCIM_ATTR_DECORATE)
                {
                    if( dvalue == scim::SCIM_ATTR_DECORATE_UNDERLINE) {
                        needDrawForeground = true;
                    } else if( dvalue == scim::SCIM_ATTR_DECORATE_REVERSE){
                        p->fillRect(curRect, cg.text());
                        needDrawForeground = true;
                    } else if( dvalue == scim::SCIM_ATTR_DECORATE_HIGHLIGHT){
                        p->fillRect(curRect, cg.highlight());
                        needDrawForeground = true;
                    }
                } else if(dtype == scim::SCIM_ATTR_FOREGROUND){
                    needDrawForeground = true;
                } else if(dtype == scim::SCIM_ATTR_BACKGROUND){
                    p->fillRect(curRect, QColor(SCIM_RGB_COLOR_RED(dvalue),
                        SCIM_RGB_COLOR_GREEN(dvalue), SCIM_RGB_COLOR_BLUE(dvalue)));
                }
                if(needDrawForeground)
                {
                    drawForegroundRect.push_back(curRect);
                    drawForegroundString.push_back(curString);
                    drawForegroundAttribute.push_back(&d->attrs[i]);
                }
            }
        }
        
        //now draw all the string in default painter
        p->drawText(cr, Qt::AlignVCenter, d->text);
    
        //draw customized foreground text
        for(uint rectIndex = 0; rectIndex < drawForegroundRect.size(); ++rectIndex)
        {
    //       std::cout << d->attrs[i].get_start() << "ok \n";
            uint dtype = drawForegroundAttribute[rectIndex]->get_type(),
            dvalue = drawForegroundAttribute[rectIndex]->get_value();

            p->save();
            if(dtype == scim::SCIM_ATTR_DECORATE)
            {
                if( dvalue == scim::SCIM_ATTR_DECORATE_UNDERLINE) {
                QFont f = d->parent->font();
                f.setUnderline(true);
                p->setFont(f);
                } else if( dvalue == scim::SCIM_ATTR_DECORATE_REVERSE){
                p->setPen( cg.background() );
                } else if( dvalue == scim::SCIM_ATTR_DECORATE_HIGHLIGHT){
                p->setPen( cg.highlightedText() );
                }
            } else if(dtype == scim::SCIM_ATTR_FOREGROUND){
                p->setPen( QColor(SCIM_RGB_COLOR_RED(dvalue),
                    SCIM_RGB_COLOR_GREEN(dvalue), SCIM_RGB_COLOR_BLUE(dvalue)) );
            }
            p->drawText(drawForegroundRect[rectIndex],
                Qt::AlignVCenter, drawForegroundString[rectIndex]);
            p->restore();
        }
        } else
        p->drawText(cr, Qt::AlignVCenter, d->text);

        //draw cursor if it is visible
        //this will not modify the painter, so we do not need to save/restore its state
        if(d->displayCursor && d->cursorPos >= 0 && (uint)d->cursorPos <= d->text.length())
        {
            int cix = fm.width(d->text, cursorPosition());
            QPoint from( cr.x() + cix, cr.top() + (cr.height() - fm.height()) / 2);
            QPoint to = from + QPoint( 0, fm.height() );
            p->drawLine( from, to );
        }
        p->end();
    }

    if(!d->contentBuffer.isNull() && !d->storedSizeHint.isEmpty())
        bitBlt(d->parent, cr.x(), cr.y(),
            &d->contentBuffer, 0, 0, d->storedSizeHint.width(), d->storedSizeHint.height());
}

void ScimStringRender::setCursorPosition ( int p )
{
    d->cursorPos = p;
    d->validBuffer = false;
}

int ScimStringRender::cursorPosition() const  { return d->cursorPos; }

void ScimStringRender::setText(const QString & text, const scim::AttributeList & attrlist ) 
{
    d->text = text;
    d->attrs = attrlist;
    d->valid_widthhint = -1;
    d->validBuffer = false;
}

QSize ScimStringRender::minimumSizeHint () const
{
    if(d->valid_widthhint == 1)
        return d->storedSizeHint;

    d->valid_widthhint = 1;
    if(d->text.length())
    {
        QFontMetrics fm( d->parent->fontMetrics() );
        QRect br = fm.boundingRect( 0, 0, 2000 ,2000, 
            Qt::AlignAuto | Qt::AlignVCenter | Qt::ExpandTabs, d->text );
        if(d->displayCursor)
            d->storedSizeHint.setWidth(br.width() + 2); //more space to display the cursor
        else
            d->storedSizeHint.setWidth(br.width());

        d->storedSizeHint.setHeight(fm.lineSpacing() + 4);
    }
    else
        d->storedSizeHint = QSize(0, 0);

    return d->storedSizeHint;
}

ScimStringRender::~ScimStringRender()
{
    delete d;
}


