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
 
#ifndef UNICORN_VOLUME_SLIDER_H
#define UNICORN_VOLUME_SLIDER_H

#include "phonon/volumeslider.h"
#include <QToolButton>

namespace Phonon{ class AudioOutput; }
class UnicornVolumeSlider : public Phonon::VolumeSlider
{
    Q_OBJECT
public:
    UnicornVolumeSlider( QWidget* parent = 0 ): Phonon::VolumeSlider( parent )
    {
        installFilters();
    }
    
    UnicornVolumeSlider( Phonon::AudioOutput* output, QWidget* parent = 0 ): Phonon::VolumeSlider( output, parent )
    {
        installFilters();
    }
    
    void installFilters();
    
    virtual bool eventFilter( QObject* watched, QEvent* event );
    
    bool sliderEventFilter( class QSlider*, QEvent* ) const;
    bool toolButtonEventFilter( class QToolButton*, QEvent* ) const;
    
    void paintEvent( QPaintEvent* );
    void resizeEvent( QResizeEvent* );
    
protected slots:
    void onVolumeValueChanged( int )
    {
        static_cast<QWidget*>(sender()->parent())->update();
    }
};

#endif //UNICORN_VOLUME_SLIDER_H