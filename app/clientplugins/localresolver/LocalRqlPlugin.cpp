/***************************************************************************
 *   Copyright 2007-2009 Last.fm Ltd.                                      *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include <QDebug>
#include "LocalRqlPlugin.h"
#include "RqlQueryThread.h"



LocalRqlPlugin::LocalRqlPlugin()
:m_queryThread(0)
{
}

LocalRqlPlugin::~LocalRqlPlugin()
{
    delete m_queryThread;
}

void 
LocalRqlPlugin::init()
{
    Q_ASSERT(m_queryThread == 0);       // call init once only
    m_queryThread = RqlQueryThread::create();
}

void
LocalRqlPlugin::parse(const char *rql, ILocalRqlParseCallback *cb)
{
    Q_ASSERT(rql && cb);
    m_queryThread->enqueueParse(rql, cb);
}

void 
LocalRqlPlugin::finished()
{
    delete this;
}

