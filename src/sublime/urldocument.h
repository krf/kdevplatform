/***************************************************************************
 *   Copyright (C) 2006-2007 by Alexander Dymo  <adymo@kdevelop.org>       *
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
#ifndef SUBLIMEPARTDOCUMENT_H
#define SUBLIMEPARTDOCUMENT_H

#include <kurl.h>

#include <sublimeexport.h>

#include <document.h>

namespace Sublime {

/**
@short Basic document that has an URL.
*/
class SUBLIME_EXPORT UrlDocument: public Document {
public:
    UrlDocument(Controller *controller, const KUrl &url);
    ~UrlDocument();

protected:
    virtual QWidget *createViewWidget(QWidget *parent = 0);
    KUrl url() const;

private:
    struct UrlDocumentPrivate * const d;

};

}

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
