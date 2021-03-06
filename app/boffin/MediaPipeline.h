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
 
#include <lastfm/global.h>
#include <lastfm/Track>
#include <lastfm/WsError>
#include <QPointer>
#include <QObject>
#include <phonon/phononnamespace.h>


namespace Phonon
{
	class MediaObject;
 	class AudioOutput;
    class MediaSource;
}



class MediaPipeline : public QObject
{
    Q_OBJECT

public: 
    MediaPipeline( Phonon::AudioOutput*, QObject* parent );
    ~MediaPipeline();

    Phonon::State state() const;

    void play( class TrackSource* );

public slots:
    void setPaused( bool );
    void stop();
    void skip();

signals:
    void started( const Track& );
    void paused();
    void resumed();
    void stopped();
    void error( const QString& );

private slots:
    void onPhononSourceChanged( const Phonon::MediaSource& );
    void onPhononStateChanged( Phonon::State, Phonon::State );
    void onSourceError( Ws::Error );
    void enqueue();

private:
	Phonon::MediaObject* mo;    
    Phonon::AudioOutput* ao;
    TrackSource* m_source;

    QMap<QUrl, Track> m_tracks;

    bool m_errorRecover;
    bool m_phonon_sucks;
};
