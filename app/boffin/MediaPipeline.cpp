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

#include <cmath>
#include <QEventLoop>
#include <QStringList>
#include <QThread>
#include <phonon/mediaobject.h>
#include <phonon/audiooutput.h>
#include "MediaPipeline.h"
#include "TrackSource.h"


MediaPipeline::MediaPipeline( Phonon::AudioOutput* ao, QObject* parent )
             : QObject( parent )
             , mo( 0 )
             , ao( ao )
             , m_source( 0 )
             , m_errorRecover( false )
             , m_phonon_sucks( false )
{
    mo = new Phonon::MediaObject;
    connect( mo, SIGNAL(stateChanged( Phonon::State, Phonon::State )), SLOT(onPhononStateChanged( Phonon::State, Phonon::State )) );
    connect( mo, SIGNAL(aboutToFinish()), SLOT(enqueue()) ); // fires just before track finishes
    connect( mo, SIGNAL(currentSourceChanged( Phonon::MediaSource )), SLOT(onPhononSourceChanged( Phonon::MediaSource )) ); 
    Phonon::createPath( mo, ao );
}


MediaPipeline::~MediaPipeline()
{    
    // I'm not confident about the sleep code on Windows --mxcl
#ifndef WIN32
	if (mo->state() != Phonon::PlayingState)
        return;

    qreal starting_volume = ao->volume();
    //sigmoid curve
    for (int x = 18; x >= -60; --x)
    {
        qreal y = x;
        y /= 10;
        y = qreal(1) / (qreal(1) + std::exp( -y ));
        y *= starting_volume;
        ao->setVolume( y );

		struct Thread : QThread { using QThread::msleep; };
		Thread::msleep( 7 );
    }
#endif

    //delete mo; //don't as crashes often
}


static inline QWidget* findTopLevelWidget( QObject* o )
{
    for (int x = 0;;) {
        o = o->parent();
        if (!o) return 0;
        if (o->isWidgetType()) return (QWidget*)o;
        if (x++ >= 10) return 0; // code that cannot be proved to exit is evil
    }
}


void
MediaPipeline::play( TrackSource* trackSource )
{
//	delete m_source;
    m_source = trackSource;
    enqueue();
}


void
MediaPipeline::setPaused( bool b )
{
    if (b)
        mo->pause();
    else if (mo->state() == Phonon::PausedState)
        mo->play();
}


void
MediaPipeline::skip()
{    
    using namespace Phonon;
        
    enqueue();
        
    QList<MediaSource> q = mo->queue();
    if (q.isEmpty()) { stop(); return; } // oh well :( enqueue() fails
    
    m_phonon_sucks = true; // phonon is broken hack
    mo->setCurrentSource( q.takeFirst() );
    Q_ASSERT( q.isEmpty() );
    mo->setQueue( q ); //now empty
    mo->play();
}


void
MediaPipeline::stop()
{
    using namespace Phonon;
    
    qDebug() << mo->state();
    
    m_tracks.clear();
    
    if (mo->state() != Phonon::StoppedState)
    {
        // lol @ Phonon's shit API. We're 99% sure we're using it right
        mo->stop();
        mo->setCurrentSource( MediaSource() );
        mo->setQueue( QList<MediaSource>() );
        
        if (mo->state() == Phonon::LoadingState)
            emit stopped(); //phonon is broken and shit
    }
    else if (m_source)
        // otherwise we have a source and it is doing something, but it could be
        // slow. Slow enough that the user wants to push the stop() button. So
        // the user did push the stop button. So tell the GUI that we stopped.
        emit stopped();

    delete m_source;
    m_source = 0;
}


void
MediaPipeline::onPhononStateChanged( Phonon::State newstate, Phonon::State oldstate )
{
    using namespace Phonon;
    
    // this is a HACK because Phonon docs lie that after you setCurrentSource
    // and call play() it will go to PlayingState. Actually it ALWAYS goes to
    // StoppedState. Phonon is absolute shit. With a shit API. And a bunch of
    // shit like the inability to play all mp3s and support all sound cards and
    // not hang the soding GUI whenever we call a function on MediaObject.
    // We're fucking unimpressed.
    if (m_phonon_sucks)
        switch (newstate)
        {
            case PlayingState:
            case ErrorState:
            case PausedState:
                m_phonon_sucks = false;
                break;
            default:
                return;
        }
    
    qDebug() << newstate << "was" << oldstate;
        
    switch (newstate)
    {
        case StoppedState:
            if (m_errorRecover) {
                m_errorRecover = false;
                skip();
            } else {
                m_tracks.clear();
                emit stopped();
            }
            break;
            
        case ErrorState:
            qWarning() << mo->errorString();
            // need to request a stop to clear the error state before we trying to play the next track
            m_errorRecover = true;
            mo->stop();
            emit error( "There was an error during playback." );
            break;
            
        case PausedState:
            emit paused();
            break;
            
        case PlayingState:
            if (oldstate == PausedState)
                emit resumed();
            else
                enqueue();
            break;

        default:
            break;
    }
}


void
MediaPipeline::onPhononSourceChanged( const Phonon::MediaSource& source )
{
    emit started( m_tracks.value( source.url() ) );
}


void
MediaPipeline::enqueue()
{    
    if (mo->queue().size() || !m_source) return;

    // keep only one track in the phononQueue
    // Loop until we get a null url or a valid url.
    for (;;)
    {
        // consume next track from the track source. a null track 
        // response means wait until the trackAvailable signal
        Track t = m_source->takeNextTrack();
        if (t.isNull()) { qDebug() << t; break; }

        // Invalid urls won't trigger the correct phonon
        // state changes, so we must prefilter them.
        if (!t.url().isValid()) continue;
        
        

        qDebug() << t.url().toString();

        m_tracks[t.url()] = t;
        qDebug() << "Will play:" << t;

        // if we are playing a track now, enqueue, otherwise start now!
        if (mo->currentSource().url().isValid())
            mo->enqueue( Phonon::MediaSource( t.url() ) );
        else {
            m_phonon_sucks = true; //Phonon is shit and broken
            mo->setCurrentSource( Phonon::MediaSource( t.url() ) );
            mo->play();
        }
        break;
    }
}


void
MediaPipeline::onSourceError( Ws::Error e )
{
    qCritical() << e;
    emit error( "There was an error generating the playlist." );
}


Phonon::State
MediaPipeline::state() const
{
    return mo->state(); 
}
