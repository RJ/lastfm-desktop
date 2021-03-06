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
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02111-1307, USA.          *
 ***************************************************************************/

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "CheckDirTree.h"
#include "ui_mainwindow.h"
#include "lib/unicorn/UnicornMainWindow.h"
#include <QDragEnterEvent>


/** @author <petgru@last.fm> 
  * @author <adam@last.fm>
  * @motherfuckingrewrite <max@last.fm>
  */
class MainWindow : public unicorn::MainWindow
{
    Q_OBJECT
    
signals:
    void startFingerprinting( QStringList dirs );
    void wantsToClose( QCloseEvent* event );
    void logout();
    
public:
    MainWindow();
    
    void dragEnterEvent( QDragEnterEvent* );
    void dropEvent( QDropEvent* );
    
public slots:
    void start();
    
    void showFAQ();
    void aboutDialog();
    
protected:
    void closeEvent( QCloseEvent* );

	Ui::MainWindow ui;

private slots:
    void fingerprintButtonClicked();
};

#endif
