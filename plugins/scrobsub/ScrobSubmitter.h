/* Copyright 205-2009, Last.fm Ltd. <client@last.fm>                       
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY <copyright holder> ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*************************************************************************/ /**
    \mainpage ScrobSub
 
    \section intro_sec Introduction
 
    Win32 library for writing media player plugins that submit listening data
    to the Last.fm client.
     
    \section install_sec Installation
    
    The library consists of seven source files, so the easiest way of
    using them would be to simply include them in your project and compile
    them alongside your other code. The only class with a public interface
    is the ScrobSubmitter.
    
    Files needed:
    \li ScrobSubmitter.h
    \li ScrobSubmitter.cpp
    \li EncodingUtils.h
    \li EncodingUtils.cpp
    \li RegistryUtils.h
    \li RegistryUtils.cpp
    \li scrobSubPipeName.cpp

    \section dep_sec Dependencies

******************************************************************************/

#ifndef SCROBSUBMITTER_H
#define SCROBSUBMITTER_H

#include <string>
#include <deque>
#include <windows.h>

/*************************************************************************/ /**
    Class for submitting tracks to the Last.fm client. It communicates with
    the client via a named pipe.
    
    To use it, call Init on player startup passing the required parameters.
    This will launch the Last.fm client if it isn't already runnning and
    initialise the socket communication and worker thread.
    
    Then simply call functions Start, Stop, Pause and Resume in response to
    player events to communicate these events to the Last.fm client. They all
    operate asynchronously, i.e. they will return straight away so as to
    prevent hanging the calling thread if an operation takes some time. The 
    functions will return a request ID uniquely identifying the command just 
    sent. When the request is finished, the status callback will be called 
    with the ID as a parameter and a message of either "OK" or a description 
    of the error if one occurred.
    
    It's important to call Term at shutdown to clean up all resources properly.
        
    @author <erik@last.fm>
******************************************************************************/
class ScrobSubmitter 
{
public:

    /*********************************************************************/ /**
        Callback function type for status reporting.

        @param[in] reqId    The request ID the message is in regards to.
        @param[in] error    True if an error occurred.
        @param[in] msg      Status message.
        @param[in] userData The user data passed to Init.
    **************************************************************************/
    typedef void (*StatusCallback)(
        int  reqId,
        bool error,
        std::string msg,
        void* userData);

    /*********************************************************************/ /**
        Text encoding used for track submissions.
    **************************************************************************/
    enum Encoding
    {
        ISO_8859_1, // standard Windows western codepage
        UTF_8
    };

    /*********************************************************************/ /**
        Constructor
    **************************************************************************/
	ScrobSubmitter();

    /*********************************************************************/ /**
        Destructor
    **************************************************************************/
	virtual
	~ScrobSubmitter();

    /*********************************************************************/ /**
        Call this to initialise the class. Must be called before any tracks
        are submitted or things won't work. This function will also launch
        Last.fm if it isn't running.
        
        @param[in] pluginId  Three-letter plugin code as assigned by Last.fm.
                             Contact russ at last.fm to get one for your plugin.
        @param[in] callback  Function pointer for reporting result status of
                             asynchronous operations.
        @param[in] userData  Pointer that will be passed back through the
                             callback. Use for "this" pointer.
    **************************************************************************/
    void
    Init(
        const std::string& pluginId,
        StatusCallback callback,
        void* userData);

    /*********************************************************************/ /**
        Call this to terminate. Must be called in order for the  thread to be
        shut down properly.
    **************************************************************************/
    void
    Term();

    /*********************************************************************/ /**
        Sends a START command to client. All parameter strings are treated as
        standard char strings.
        
        @param[in] artist   Name of artist.
        @param[in] track    Name of track.
        @param[in] album    Name of album.
        @param[in] mbId     Musicbrainz ID if present.
        @param[in] length   Length of track in seconds.
        @param[in] filename Full path to the file on the disk.
        @param[in] encoding Text encoding specifying how to interpret the
                            characters in the strings passed in.

        @return A unique request ID.
    **************************************************************************/
    int
    Start(
        std::string artist,
        std::string track,
        std::string album,
        std::string mbId,
        int         length,
        std::string filename,
        Encoding    encoding = UTF_8);

    /*********************************************************************/ /**
        Sends a STOP command to client. Call this both for stop and when a song
        plays to the end.

        @return A unique request ID.
    **************************************************************************/
    int
    Stop();

    /*********************************************************************/ /**
        Sends a PAUSE command to client. Use this only for pause.

        @return A unique request ID.
    **************************************************************************/
    int
    Pause();

    /*********************************************************************/ /**
        Sends a RESUME command to client. Should only be used following a
        preceding START-PAUSE sequence to resume a paused track.

        @return A unique request ID.
    **************************************************************************/
    int
    Resume();

    /*********************************************************************/ /**
        Get ScrobSub version.
        
        @return Version string.
    **************************************************************************/
    std::string
    GetVersion();

    /*********************************************************************/ /**
        Returns a suitable directory for putting log files in. Normally
        "Documents and Settings/[user]/Local Settings/Application Data/
        Last.fm/Client".
        
        @return Path to log folder.
    **************************************************************************/
    static std::string
    GetLogPath();

    /*********************************************************************/ /**
        Returns a suitable unicode directory for putting log files in. Normally
        "Documents and Settings/[user]/Local Settings/Application Data/
        Last.fm/Client".

        @return unicode Path to log folder.
    **************************************************************************/
    static std::wstring
        GetLogPathW();

    int
    BootstrapComplete( const WCHAR* username );

private:

    /**************************************************************************
        Copy ctor private because a CRITICAL_SECTION cannot be copied.
    **************************************************************************/
	ScrobSubmitter(
	    const ScrobSubmitter&);

    /**************************************************************************
        Opens the Last.fm client.
    **************************************************************************/
    bool
	LaunchClient();

    /**************************************************************************
        Sends the command cmd.
    **************************************************************************/
    int
	SendToAS(
	    const std::string& cmd);

    /**************************************************************************
        Thread starter.
    **************************************************************************/
    static unsigned __stdcall
    SendToASThreadMain(
        LPVOID p)
	{ reinterpret_cast<ScrobSubmitter*>(p)->SendToASThread(); return 0; }
	 
    /**************************************************************************
        Worker thread function.
    **************************************************************************/
    void
	SendToASThread();

    /**************************************************************************
        Connects to AS. Returns true if successful.
    **************************************************************************/
    bool
    ConnectToAS(
        int reqId);

    /**************************************************************************
        Reports status back to caller through callback.
    **************************************************************************/
    void
    ReportStatus(
        int reqId,
        bool error,
        const std::string& msg);

    /**************************************************************************
        Converts the passed in string to UTF-8 from the encoding specified
        as the second parameter.
    **************************************************************************/
    void
	ConvertToUTF8(
	    std::string& text,
	    Encoding encoding);

    /*********************************************************************/ /**
        Escapes &s with &&s. Return a reference to the same string that was
        passed in but escaped.
    **************************************************************************/
    std::string&
	Escape(
	    std::string& text);


    std::string      mPluginId;
    
    int              mNextId;
    std::deque<std::pair<int, std::string> > mRequestQueue;

    HANDLE           mSocketThread;
    HANDLE           mRequestAvailable;
    HANDLE           mExit;
    CRITICAL_SECTION mMutex;
    bool             mStopThread;
    std::string      mPipeName;

    StatusCallback   mpReportStatus;
    void*            mpUserData;

    DWORD            mLaunchTime;
    
    friend class BootStrap; // so that we can use the ReportStatus function
};

#endif // SCROBSUBMITTER_H
