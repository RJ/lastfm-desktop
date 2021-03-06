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

#ifndef LASTFM_SCROBBLE_H
#define LASTFM_SCROBBLE_H

#include <lastfm/Track>


struct LASTFM_SCROBBLE_DLLEXPORT Scrobble : lastfm::Track
{
    Scrobble()
    {}
    
    Scrobble( const lastfm::Track& that ) : Track( that )
    {}
    
    QByteArray sourceString() const;
    
    QByteArray ratingCharacter() const
    {
        return d->extras["rating"].toAscii();
    }
    
    bool isLoved() const { return ratingCharacter() == QChar('L'); }
    bool isBanned() const { return ratingCharacter() == QChar('B'); }
    bool isSkipped() const { return ratingCharacter() == QChar('S'); }
    
#if 0
    bool isNotLovedSkippedOrBanned() const
    {
        return ratingCharacter() == "";
    }
#endif

    /** if isValid() returns false, Scrobbler will not scrobble the track */
    enum Invalidity
    {
        TooShort,
        ArtistNameMissing,
        TrackNameMissing,
        ArtistInvalid,
        
        // the user can setup rules that describe music that should never be
        // scrobbled. We obey these rules even in your implementation :P
        // it is also possible we may decide some paths are not sensible in the
        // future and add them as well
        ForbiddenPath,

        NoTimestamp,
        FromTheFuture,
        FromTheDistantPast
    };
    
    /** @returns true if the server is unlikely to reject this scrobble */
    bool isValid( Invalidity* = 0 ) const;
};

#endif
