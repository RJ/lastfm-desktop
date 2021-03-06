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

#ifndef CLIENT_APP_H
#define CLIENT_APP_H

#include "State.h"
#include "lib/lastfm/types/Track.h"
#include "lib/unicorn/UnicornApplication.h"
#include "lib/lastfm/ws/WsError.h"
#include "the/definitions.h"


class App : public unicorn::Application
{
    Q_OBJECT

    friend Radio& The::radio();
    friend MainWindow& The::mainWindow();
    
public:
    class UnsupportedPlatformException
    {};

    App( int&, char** ) throw( StubbornUserException );
    ~App();

    void setMainWindow( class MainWindow* ); //access via The::mainWindow()

	/** if it isn't a lastfm url, we'll try to start it anyway, but it won't
	  * work, and the user will get an error message */
    void open( const QUrl& );
    void open( const class RadioStation& );
    void openXspf( const QUrl& );
    void openLocalContent( const RadioStation& );
    void open( class AbstractTrackSource* );
    class LocalRql* localRql();
    
signals:
    /** documented in StateMachine */
    void playerChanged( const QString& name );
    void trackSpooled( const Track&, class StopWatch* = 0 );
    void trackUnspooled( const Track& );
    void stopped();
    void stateChanged( State newstate, const Track& = Track() ); //convenience
    void scrobblePointReached( const Track& );
    
    /** something should show it. Currently MainWindow does */
    void error( const QString& message );
    void status( const QString& message, const QString& id );
    
public slots:
	void love( bool loveOrUnlove );
	void ban();
    void parseArguments( const QStringList& );
    
    void setScrobblingEnabled( bool );
    
private slots:
	void onScrobblerStatusChanged( int );
    void onTrackSpooled( const Track& );
    void onTrackUnspooled( const Track& );
    void onUserGotInfo( WsReply* );
    
    /** all webservices connect to this and emit in the case of bad errors that
     * need to be handled at a higher level */
    void onWsError( Ws::Error );
    
    void onRadioError( int, const class QVariant& );
    
private:
    class Scrobbler* m_scrobbler;
    class MainWindow* m_mainWindow;
    class Radio* m_radio;
    class Resolver *m_resolver;
    class LocalRql *m_localRql;
    class StateMachine* m_stateMachine;
#ifndef __APPLE__
    class PlayerListener* m_listener;
#endif

    class QSystemTrayIcon* m_trayIcon;
    
    void submitTwiddleCache( const QString& path );
};

#endif
