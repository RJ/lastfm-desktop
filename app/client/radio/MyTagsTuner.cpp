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

#include "MyTagsTuner.h"
#include "StationDelegate.h"
#include <lastfm/User>
#include <lastfm/RadioStation>
#include <lastfm/WsReply>


MyTagsTuner::MyTagsTuner()
{
	setItemDelegate( new StationDelegate );
	WsReply* reply = AuthenticatedUser().getTopTags();
	connect( reply, SIGNAL( finished( WsReply*)), SLOT(onFetchedTags( WsReply*)) );
	connect( this, SIGNAL( itemClicked( QListWidgetItem* )), SLOT( onTagClicked( QListWidgetItem*)) );
}


void
MyTagsTuner::onFetchedTags( WsReply* r )
{
	QMap<int, QString> tags = Tag::list( r );
	
	if( tags.isEmpty() )
		return;
	
	static_cast<StationDelegate*>( itemDelegate() )->setMaxCount( tags.keys().last() );

    QMapIterator<int, QString> i( tags );
    while (i.hasNext())
	{
		QListWidgetItem* item = new QListWidgetItem( i.next().value() );
		item->setData( StationDelegate::CountRole, i.key() );
		addItem( item );
	}
}


void
MyTagsTuner::onTagClicked( QListWidgetItem* i )
{
	QString tag = i->data( Qt::DisplayRole ).toString();
	emit tune( RadioStation::globalTag( tag ) );
	i->setSelected( false );
}
