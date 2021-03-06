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

#include "Scrobbler.h"
#include "NowPlaying.h"
#include "ScrobbleCache.h"
#include "ScrobblerHandshake.h"
#include "ScrobblerSubmission.h"
#include "lib/lastfm/ws/WsKeys.h"

using lastfm::Track;


Scrobbler::Scrobbler( const QString& clientId )
        : m_clientId( clientId ),
          m_handshake( 0 ), 
          m_np( 0 ), 
          m_submitter( 0 ),
          m_hard_failures( 0 )
{
    m_cache = new ScrobbleCache( Ws::Username );

    handshake();
    submit(); // will submit what's there once the handshake completes
}


Scrobbler::~Scrobbler()
{
    delete m_cache;
    delete m_handshake;
    delete m_np;
    delete m_submitter;
}


void
Scrobbler::handshake() //private
{
    m_hard_failures = 0;

    // if we are here due to hard failure then we need to save what we were 
    // doing and load it back into the new requests
    QByteArray np_data;
    QList<Track> tracks;
    if (m_np) np_data = m_np->postData();
    if (m_submitter) tracks = m_submitter->unsubmittedTracks();

    // we delete to be sure of the state of the QHttp objects, as they are 
    // rather black box
    delete m_handshake;
    delete m_np;
    delete m_submitter;
    
    m_handshake = new ScrobblerHandshake( m_clientId );
    connect( m_handshake, SIGNAL(done( QByteArray )), SLOT(onHandshakeReturn( QByteArray )), Qt::QueuedConnection );
    connect( m_handshake, SIGNAL(responseHeaderReceived( QHttpResponseHeader )), SLOT(onHandshakeHeaderReceived( QHttpResponseHeader )) );
    m_np = new NowPlaying( np_data );
    connect( m_np, SIGNAL(done( QByteArray )), SLOT(onNowPlayingReturn( QByteArray )), Qt::QueuedConnection );
    m_submitter = new ScrobblerSubmission;
    m_submitter->setTracks( tracks );
    connect( m_submitter, SIGNAL(done( QByteArray )), SLOT(onSubmissionReturn( QByteArray )), Qt::QueuedConnection );
    connect( m_submitter, SIGNAL(requestStarted( int )), SLOT(onSubmissionStarted( int )) );
}


void
Scrobbler::rehandshake() //public
{
    if (!m_submitter->hasSession())
    {
        m_handshake->request();
    }
    else
        // if we still have a valid session, np may have been failing, so just
        // send it as it doesn't hurt
        m_np->request();
}


void
Scrobbler::nowPlaying( const Track& track )
{
    m_np->submit( track );
}


void
Scrobbler::cache( const Track& track )
{
    m_cache->add( track );
}


void
Scrobbler::cache( const QList<Track>& tracks )
{
    m_cache->add( tracks );
}


void
Scrobbler::submit()
{
    m_submitter->setTracks( m_cache->tracks() );
    m_submitter->submitNextBatch();
}


void
Scrobbler::onError( Scrobbler::Error code )
{
    qDebug() << code; //TODO error text

    switch (code)
    {
        case Scrobbler::ErrorBannedClientVersion:
        case Scrobbler::ErrorInvalidSessionKey:
        case Scrobbler::ErrorBadTime:
            // np and submitter are in invalid state and won't send any requests
            // the app has to tell the user and let them decide what to do
            break;

        default:
            Q_ASSERT( false ); // you (yes you!) have missed an enum value out

        case Scrobbler::ErrorThreeHardFailures:
        case Scrobbler::ErrorBadSession:
            handshake();
            break;
    }

    emit status( code );
}


#define SPLIT( x ) QList<QByteArray> const results = x.split( '\n' ); QByteArray const code =  results.value( 0 ); qDebug() << x.trimmed();


void
Scrobbler::onHandshakeReturn( const QByteArray& result ) //TODO trim before passing here
{
    SPLIT( result )

    if (code == "OK" && results.count() >= 4)
    {
        m_np->setSession( results[1] );
        m_np->setUrl( QString::fromUtf8( results[2] ) );
        m_submitter->setSession( results[1] );
        m_submitter->setUrl( QString::fromUtf8( results[3] ) );

        emit status( Scrobbler::Handshaken );

        // submit any queued work
        m_np->request();
        m_submitter->request();
    }
    else if (code == "BANNED")
    {
        onError( Scrobbler::ErrorBannedClientVersion );
    }
    else if (code == "BADAUTH")
    {
        onError( Scrobbler::ErrorInvalidSessionKey );
    }
    else if (code == "BADTIME")
    {
        onError( Scrobbler::ErrorBadTime );
    }
    else
        m_handshake->retry(); //TODO increasing time up to 2 hours
}


void
Scrobbler::onNowPlayingReturn( const QByteArray& result )
{
    SPLIT( result )

    if (code == "OK")
    {
        m_np->reset();
    }
    else if (code == "BADSESSION")
    {
        if (!m_submitter->hasPendingRequests())
        {
            // if scrobbling is happening then there is no way I'm causing
            // duplicate scrobbles! We'll fail next time we try to contact 
            // Last.fm instead
            onError( Scrobbler::ErrorBadSession );
        }
    }
    // yep, no else. The protocol says hard fail, I say, don't:
    //  1) if only np is down, then hard failing will just mean a lot of work for the handshake php script with no good reason
    //  2) if both are down, subs will hard fail too, so just rely on that
    //  3) if np is up and subs is down, successful np requests will reset the failure count and possibly prevent timely scrobbles

    // TODO retry if server replies with busy, at least
    // TODO you need a lot more error handling for the scrobblerHttp returns "" case
}


void
Scrobbler::onSubmissionReturn( const QByteArray& result )
{
    SPLIT( result )

    if (code == "OK")
    {
        m_hard_failures = 0;
        m_cache->remove( m_submitter->batch() );
        m_submitter->submitNextBatch();

        if (m_submitter->batch().isEmpty())
        {
            emit status( Scrobbler::TracksScrobbled );
        }
    }
    else if (code == "BADSESSION")
    {
        onError( Scrobbler::ErrorBadSession );
    }
	else if (code.startsWith( "FAILED Plugin bug" ))
	{
		qWarning() << "YOU SUCK! Attempting reasonable error handling...";
		m_cache->remove( m_submitter->batch() );
	}
	else if (++m_hard_failures >= 3)
    {
        onError( Scrobbler::ErrorThreeHardFailures );
    }
    else
        m_submitter->retry();
}


void
Scrobbler::onHandshakeHeaderReceived( const QHttpResponseHeader& header )
{
    if (header.statusCode() != 200)
    {
        m_handshake->abort(); //TEST
        m_handshake->retry();
    }
}


void
Scrobbler::onSubmissionStarted( int id )
{
    if (id == m_submitter->requestId())
        emit status( Scrobbling );
}
