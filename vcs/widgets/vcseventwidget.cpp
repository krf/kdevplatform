/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
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

#include "vcseventwidget.h"

#include <QHeaderView>
#include <QAction>

#include <kdebug.h>
#include <kmenu.h>


#include <interfaces/iplugin.h>
#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>

#include "ui_vcseventwidget.h"
#include "vcsdiffwidget.h"

#include "../vcsjob.h"
#include "../interfaces/ibasicversioncontrol.h"
#include "../vcsrevision.h"
#include "../vcsevent.h"
#include "../vcslocation.h"

#include "../models/vcsitemeventmodel.h"
#include "../models/vcseventmodel.h"


namespace KDevelop
{

class VcsEventWidgetPrivate
{
public:
    VcsEventWidgetPrivate( VcsEventWidget* w )
        : q( w )
    {}

    Ui::VcsEventWidget* m_ui;
    VcsItemEventModel* m_detailModel;
    VcsEventModel *m_logModel;
    KDevelop::VcsJob* m_job;
    KUrl m_url;
    QModelIndex m_contextIndex;
    VcsEventWidget* q;
    KDevelop::IBasicVersionControl* m_iface;
    void eventViewCustomContextMenuRequested( const QPoint &point );
    void eventViewClicked( const QModelIndex &index );
    void jobReceivedResults( KDevelop::VcsJob* job );
    void diffToPrevious();
    void diffRevisions();
    void currentRowChanged(const QModelIndex& start, const QModelIndex& end);
};

void VcsEventWidgetPrivate::eventViewCustomContextMenuRequested( const QPoint &point )
{
    m_contextIndex = m_ui->eventView->indexAt( point );
    if( !m_contextIndex.isValid() ){
        kDebug() << "contextMenu is not in TreeView";
        return;
    }

    KMenu menu( m_ui->eventView );

    QAction* action = menu.addAction(i18n("Diff to previous revision"));
    QObject::connect( action, SIGNAL(triggered(bool)), q, SLOT(diffToPrevious()) );

    action = menu.addAction(i18n("Diff between revisions"));
    QObject::connect( action, SIGNAL(triggered(bool)), q, SLOT(diffRevisions()) );
    action->setEnabled(m_ui->eventView->selectionModel()->selectedRows().size()>=2);

    menu.exec( m_ui->eventView->viewport()->mapToGlobal(point) );
}

void VcsEventWidgetPrivate::currentRowChanged(const QModelIndex& start, const QModelIndex& end)
{
    Q_UNUSED(end);
    if(start.isValid())
        eventViewClicked(start);
}

void VcsEventWidgetPrivate::eventViewClicked( const QModelIndex &index )
{
    KDevelop::VcsEvent ev = m_logModel->eventForIndex( index );
    m_detailModel->removeRows(0, m_detailModel->rowCount()-1);
    
    if( ev.revision().revisionType() != KDevelop::VcsRevision::Invalid )
    {
        m_ui->message->setPlainText( ev.message() );
        m_detailModel->addItemEvents( ev.items() );
    }else
    {
        m_ui->message->clear();
    }
}

void VcsEventWidgetPrivate::jobReceivedResults( KDevelop::VcsJob* job )
{
    if( job == m_job )
    {
        QList<QVariant> l = job->fetchResults().toList();
        QList<KDevelop::VcsEvent> newevents;
        foreach( const QVariant &v, l )
        {
            if( qVariantCanConvert<KDevelop::VcsEvent>( v ) )
            {
                newevents << qVariantValue<KDevelop::VcsEvent>( v );
            }
        }
        m_logModel->addEvents( newevents );
    }
}


void VcsEventWidgetPrivate::diffToPrevious()
{
    KDevelop::VcsEvent ev = m_logModel->eventForIndex( m_contextIndex );
    KDevelop::VcsRevision prev = KDevelop::VcsRevision::createSpecialRevision(KDevelop::VcsRevision::Previous);
    KDevelop::VcsJob* job = m_iface->diff( m_url, prev, ev.revision() );

    VcsDiffWidget* widget = new VcsDiffWidget( job );
    widget->setRevisions( prev, ev.revision() );
    KDialog* dlg = new KDialog( q );
    
    widget->connect(widget, SIGNAL(destroyed(QObject*)), dlg, SLOT(deleteLater()));
    
    dlg->setCaption( i18n("Difference To Previous") );
    dlg->setButtons( KDialog::Ok );
    dlg->setMainWidget( widget );
    dlg->show();
}

void VcsEventWidgetPrivate::diffRevisions()
{
    QModelIndexList l = m_ui->eventView->selectionModel()->selectedRows();
    KDevelop::VcsEvent ev1 = m_logModel->eventForIndex( l.first() );
    KDevelop::VcsEvent ev2 = m_logModel->eventForIndex( l.last() );
    KDevelop::VcsJob* job = m_iface->diff( m_url, ev1.revision(), ev2.revision() );

    VcsDiffWidget* widget = new VcsDiffWidget( job );
    widget->setRevisions( ev1.revision(), ev2.revision() );
    KDialog* dlg = new KDialog( q );
    dlg->setCaption( i18n("Difference between Revisions") );
    dlg->setButtons( KDialog::Ok );
    dlg->setMainWidget( widget );
    dlg->show();
}

VcsEventWidget::VcsEventWidget( const KUrl& url, KDevelop::VcsJob *job, QWidget *parent )
    : QWidget(parent), d(new VcsEventWidgetPrivate(this) )
{

    d->m_job = job;
    //Don't autodelete this job, its metadata will be used later on
    d->m_job->setAutoDelete( false );
    
    d->m_iface = job->vcsPlugin()->extension<KDevelop::IBasicVersionControl>();
    Q_ASSERT(d->m_iface);

    d->m_url = url;
    d->m_ui = new Ui::VcsEventWidget();
    d->m_ui->setupUi(this);

    d->m_logModel= new VcsEventModel(this);
    d->m_ui->eventView->setModel( d->m_logModel );
    d->m_ui->eventView->sortByColumn(0, Qt::DescendingOrder);
    d->m_ui->eventView->setContextMenuPolicy( Qt::CustomContextMenu );
    QHeaderView* header = d->m_ui->eventView->horizontalHeader();
    header->setResizeMode( 0, QHeaderView::ResizeToContents );
    header->setResizeMode( 1, QHeaderView::ResizeToContents );
    header->setResizeMode( 2, QHeaderView::ResizeToContents );
    header->setResizeMode( 3, QHeaderView::Stretch );

    d->m_detailModel = new VcsItemEventModel(this);
    d->m_ui->itemEventView->setModel( d->m_detailModel );
    header = d->m_ui->itemEventView->horizontalHeader();
    header->setResizeMode( 0, QHeaderView::ResizeToContents );
    header->setResizeMode( 1, QHeaderView::ResizeToContents );
    header->setResizeMode( 2, QHeaderView::ResizeToContents );
    header->setResizeMode( 3, QHeaderView::Stretch );

    connect( d->m_ui->eventView, SIGNAL(clicked(QModelIndex)),
             this, SLOT(eventViewClicked(QModelIndex)) );
    connect( d->m_ui->eventView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
             this, SLOT(currentRowChanged(QModelIndex,QModelIndex)));
    connect( d->m_ui->eventView, SIGNAL(customContextMenuRequested(QPoint)),
             this, SLOT(eventViewCustomContextMenuRequested(QPoint)) );

    connect( d->m_job, SIGNAL(resultsReady(KDevelop::VcsJob*)),
             this, SLOT(jobReceivedResults(KDevelop::VcsJob*)) );
    ICore::self()->runController()->registerJob( d->m_job );
}

VcsEventWidget::~VcsEventWidget()
{
    delete d->m_ui;
    delete d;
}

}


#include "vcseventwidget.moc"
