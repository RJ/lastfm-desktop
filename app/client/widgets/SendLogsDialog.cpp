/***************************************************************************
 *   Copyright 2005-2008 Last.fm Ltd                                       *
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

#include "SendLogsDialog.h"
#include "Settings.h"
#include "lib/moose/MooseCommon.h"
#include "lib/unicorn/UnicornCommon.h"
#include "lib/unicorn/LastMessageBox.h"
#include "lib/unicorn/ws/SendLogsRequest.h"
#include <QDir>
#include <QFileInfo>
#include <QStringList>
#include <QMovie>

#ifdef WIN32
#include <windows.h>
#include <stdio.h>
#endif

#include "version.h"


SendLogsDialog::SendLogsDialog( QWidget *parent )
              : QDialog( parent )
{
    ui.setupUi( this );
    
    ui.spinner->setMovie( new QMovie( ":/spinner.mng" ) );
    ui.spinner->movie()->setParent( this );
    ui.spinner->hide();
    
    ui.buttonBox->addButton( "Send", QDialogButtonBox::AcceptRole );

    connect( ui.buttonBox, SIGNAL( accepted() ), SLOT( onSendClicked() ) );
}


void
SendLogsDialog::onSendClicked()
{
    SendLogsRequest* request = new SendLogsRequest( PRODUCT_NAME, VERSION, ui.moreInfoTextEdit->toPlainText() );
    connect( request, SIGNAL( success() ), SLOT( onSuccess() ) );
    connect( request, SIGNAL( error() ), SLOT( onError() ) );
    
    QDir logDir( Unicorn::logPath( ) );
    QStringList logExt("*.log");
        
    // find logs
    logDir.setFilter( QDir::Files | QDir::Readable );
    logDir.setNameFilters( logExt );
    QList<QFileInfo> logFiles = logDir.entryInfoList();
    
    foreach( QFileInfo log, logFiles )
        request->addLog( log.completeBaseName(), log.absoluteFilePath() );

    request->addLogData( "clientinfo", Moose::sessionInformation() );
    request->addLogData( "sysinfo", Unicorn::systemInformation() );
    
    request->send();
    
    ui.buttonBox->setEnabled( false );
    ui.moreInfoTextEdit->setEnabled( false );
    ui.spinner->show();
    ui.spinner->movie()->start();
}


void
SendLogsDialog::onSuccess()
{
    ui.spinner->movie()->stop();
    ui.spinner->hide();

    LastMessageBox::information( tr( "Logs sent" ), 
                                 tr( "Your logs have been sent to the Last.fm support team.\n"
                                     "We will get back to you as soon as possible." ) );
    ui.moreInfoTextEdit->clear();
    ui.buttonBox->setEnabled( true );
    ui.moreInfoTextEdit->setEnabled( true );
    QDialog::accept();
}


void
SendLogsDialog::onError()
{
    LastMessageBox::warning( tr( "Couldn't send logs" ), 
                             tr( "Failed to send the logs. Please try again later." ) );
    ui.spinner->movie()->stop();
    ui.spinner->hide();
    ui.buttonBox->setEnabled( true );
    ui.moreInfoTextEdit->setEnabled( true );
}
