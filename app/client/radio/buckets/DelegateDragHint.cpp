/***************************************************************************
 *   Copyright 2005-2009 Last.fm Ltd.                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.          *
 ***************************************************************************/

#include "DelegateDragHint.h"
#include <QCoreApplication>
#include <QAbstractItemView>

void
DelegateDragHint::onFinishedAnimation()
{
 
    if( m_mimeData )
    {
        if( QAbstractScrollArea* abstractScrollArea = qobject_cast<QAbstractScrollArea*>(m_target))
            m_target = abstractScrollArea->viewport();
        
        QDragEnterEvent* eevent = new QDragEnterEvent( m_target->pos(), Qt::CopyAction | Qt::MoveAction, m_mimeData, Qt::LeftButton, Qt::NoModifier );
        QCoreApplication::postEvent( m_target, eevent );
        QDropEvent* event = new QDropEvent( m_target->pos(), Qt::CopyAction | Qt::MoveAction, m_mimeData, Qt::LeftButton, Qt::NoModifier );
        QCoreApplication::postEvent( m_target, event );
    }
    hide();
    deleteLater();
}


void 
DelegateDragHint::onDragFrameChanged( int frame )
{
    const QPoint destPoint = m_target->mapToGlobal( m_target->rect().center() - QPoint( width() / 2.0f, height() / 2.0f ));
    QPoint distanceIncrement = destPoint - m_startPoint;
    distanceIncrement /= ( frame );
    move( destPoint - distanceIncrement );
}


void
DelegateDragHint::dragTo( QWidget* target )
{
    m_target = target;

    m_startPoint = pos();
    
    QTimeLine* timeLine = new QTimeLine( 500, this );
    timeLine->setFrameRange( 0, 100 );
    timeLine->setCurveShape( QTimeLine::EaseInCurve );
    
    connect( timeLine, SIGNAL( frameChanged(int)), SLOT( onDragFrameChanged(int)));
    connect( timeLine, SIGNAL( finished()), SIGNAL( finishedAnimation()));
    connect( timeLine, SIGNAL( finished()), SLOT( onFinishedAnimation()));
    timeLine->start();
    show();
}


void 
DelegateDragHint::paintEvent( QPaintEvent* e )
{
    QPainter p( this );
    p.setClipRect( e->rect() );
    m_options.palette = palette();
    m_options.state = QStyle::State_Selected | QStyle::State_Active | QStyle::State_Enabled | QStyle::State_HasFocus;
    m_options.showDecorationSelected = true;
    m_options.rect = rect();
    m_d->paint( &p, m_options, m_i );
}