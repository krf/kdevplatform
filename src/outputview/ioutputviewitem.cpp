/***************************************************************************
 *   Copyright (C) 2007 by Dukju Ahn <dukjuahn@gmail.com>                  *
 *   Copyright (C) 2007 by Andreas Pakulat  <apaku@gmx.de>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "ioutputviewitem.h"
#include <QtCore/QList>
#include <QtGui/QAction>

IOutputViewItem::IOutputViewItem( const QString& text )
    : QStandardItem( text ), d(0)
{}

IOutputViewItem::~IOutputViewItem()
{
}

void IOutputViewItem::activate()
{
}

QList<QAction*> IOutputViewItem::contextMenuActions()
{
    return QList<QAction*>();
}

IOutputViewItem::StopAtItemMode IOutputViewItem::stopHere()
{
    return DontStop;
}

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
