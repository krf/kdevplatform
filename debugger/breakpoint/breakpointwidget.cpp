/*
 * This file is part of KDevelop
 *
 * Copyright 2008 Vladimir Prus <ghost@cs.msu.su>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "breakpointwidget.h"

#include <QHBoxLayout>
#include <QSplitter>
#include <QTableView>
#include <QHeaderView>
#include <QMenu>
#include <QContextMenuEvent>

#include <KIcon>
#include <KLocalizedString>
#include <KPassivePopup>
#include <KDebug>

#include "breakpointdetails.h"
#include "../breakpoint/breakpoint.h"
#include "../breakpoint/breakpointmodel.h"
#include <interfaces/idebugcontroller.h>

#define IF_DEBUG(x)
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>

using namespace KDevelop;

BreakpointWidget::BreakpointWidget(IDebugController *controller, QWidget *parent)
: QWidget(parent), firstShow_(true), m_debugController(controller),
  breakpointDisableAll_(0), breakpointEnableAll_(0), breakpointRemoveAll_(0)
{
    setWindowTitle(i18nc("@title:window", "Debugger Breakpoints"));
    setWhatsThis(i18nc("@info:whatsthis", "<b>Breakpoint list</b><p>"
                        "Displays a list of breakpoints with "
                        "their current status. Clicking on a "
                        "breakpoint item allows you to change "
                        "the breakpoint and will take you "
                        "to the source in the editor window.</p>"));
    setWindowIcon( KIcon("process-stop") );

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    QSplitter *s = new QSplitter(this);
    layout->addWidget(s);

    table_ = new QTableView(s);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->horizontalHeader()->setHighlightSections(false);
    table_->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    details_ = new BreakpointDetails(s);

    s->setStretchFactor(0, 2);

    table_->verticalHeader()->hide();

    table_->setModel(m_debugController->breakpointModel());

    connect(table_, SIGNAL(clicked(QModelIndex)), this, SLOT(slotRowClicked(QModelIndex)));
    connect(table_->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(slotUpdateBreakpointDetail()));
    connect(m_debugController->breakpointModel(), SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(slotUpdateBreakpointDetail()));
    connect(m_debugController->breakpointModel(), SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(slotUpdateBreakpointDetail()));
    connect(m_debugController->breakpointModel(), SIGNAL(modelReset()), SLOT(slotUpdateBreakpointDetail()));
    connect(m_debugController->breakpointModel(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(slotUpdateBreakpointDetail()));


    connect(m_debugController->breakpointModel(),
            SIGNAL(hit(KDevelop::Breakpoint*)),
            SLOT(breakpointHit(KDevelop::Breakpoint*)));

    connect(m_debugController->breakpointModel(),
            SIGNAL(error(KDevelop::Breakpoint*,QString,int)),
            SLOT(breakpointError(KDevelop::Breakpoint*,QString,int)));

    setupPopupMenu();
}

void BreakpointWidget::setupPopupMenu()
{
    popup_ = new QMenu(this);

    QMenu* newBreakpoint = popup_->addMenu( i18nc("New breakpoint", "&New") );

    QAction* action = newBreakpoint->addAction(
        i18nc("Code breakpoint", "&Code"),
        this,
        SLOT(slotAddBlankBreakpoint()) );
    // Use this action also to provide a local shortcut
    action->setShortcut(QKeySequence(Qt::Key_B + Qt::CTRL,
                                        Qt::Key_C));
    addAction(action);

    newBreakpoint->addAction(
        i18nc("Data breakpoint", "Data &write"),
        this, SLOT(slotAddBlankWatchpoint()));
    newBreakpoint->addAction(
        i18nc("Data read breakpoint", "Data &read"),
        this, SLOT(slotAddBlankReadWatchpoint()));
    newBreakpoint->addAction(
        i18nc("Data access breakpoint", "Data &access"),
        this, SLOT(slotAddBlankAccessWatchpoint()));

    #if 0
    m_breakpointShow = m_ctxMenu->addAction( i18n( "Show text" ) );


    m_breakpointEdit = m_ctxMenu->addAction( i18n( "Edit" ) );
    m_breakpointEdit->setShortcut(Qt::Key_Enter);

    m_breakpointDisable = m_ctxMenu->addAction( i18n( "Disable" ) );
    #endif

    QAction* breakpointDelete = popup_->addAction(
        KIcon("edit-delete"),
        i18n( "&Delete" ),
        this,
        SLOT(slotRemoveBreakpoint()));
    breakpointDelete->setShortcut(Qt::Key_Delete);
    breakpointDelete->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(breakpointDelete);


    popup_->addSeparator();
    breakpointDisableAll_ = popup_->addAction(i18n("Disable &all"), this, SLOT(slotDisableAllBreakpoints()));
    breakpointEnableAll_ = popup_->addAction(i18n("&Enable all"), this, SLOT(slotEnableAllBreakpoints()));
    breakpointRemoveAll_ = popup_->addAction(i18n("&Remove all"), this, SLOT(slotRemoveAllBreakpoints()));

    connect(popup_,SIGNAL(aboutToShow()), this, SLOT(slotPopupMenuAboutToShow()));

#if 0
    connect( m_ctxMenu,     SIGNAL(triggered(QAction*)),
        this,          SLOT(slotContextMenuSelect(QAction*)) );
#endif
}


void BreakpointWidget::contextMenuEvent(QContextMenuEvent* event)
{
#if 0
    Breakpoint *bp = breakpoints()->breakpointForIndex(indexAt(event->pos()));

    if (!bp)
    {
        bp = breakpoints()->breakpointForIndex(currentIndex());
    }

    if (bp)
    {
        m_breakpointShow->setEnabled(bp->hasFileAndLine());

        if (bp->isEnabled( ))
        {
            m_breakpointDisable->setText( i18n("Disable") );
        }
        else
        {
            m_breakpointDisable->setText( i18n("Enable") );
        }
    }
    else
    {
        m_breakpointShow->setEnabled(false);
    }

    m_breakpointDisable->setEnabled(bp);
    m_breakpointDelete->setEnabled(bp);
    m_breakpointEdit->setEnabled(bp);

    bool has_bps = !breakpoints()->breakpoints().isEmpty();
    m_breakpointDisableAll->setEnabled(has_bps);
    m_breakpointEnableAll->setEnabled(has_bps);
    m_breakpointDelete->setEnabled(has_bps);

    m_ctxMenuBreakpoint = bp;
    m_ctxMenu->popup( event->globalPos() );
#endif
    popup_->popup(event->globalPos());
}

void BreakpointWidget::slotPopupMenuAboutToShow()
{
    if (m_debugController->breakpointModel()->rowCount() < 2) {
        breakpointDisableAll_->setDisabled(true);
        breakpointEnableAll_->setDisabled(true);
        breakpointRemoveAll_->setDisabled(true);
    } else {
        breakpointRemoveAll_->setEnabled(true);
        bool allDisabled = true;
        bool allEnabled = true;
        for (int i = 0; i < m_debugController->breakpointModel()->rowCount() - 1 ; i++) {
            Breakpoint *bp = m_debugController->breakpointModel()->breakpoint(i);
            if (bp->enabled())
                allDisabled = false;
            else
                allEnabled = false;
        }
        breakpointDisableAll_->setDisabled(allDisabled);
        breakpointEnableAll_->setDisabled(allEnabled);
    }
       
}


#if 0
void slotContextMenuSelect( QAction* action )
{
    #if 0
    int                  col;
    Breakpoint          *bp = m_ctxMenuBreakpoint;

    if ( action == m_breakpointShow ) {
        if (FilePosBreakpoint* fbp = dynamic_cast<FilePosBreakpoint*>(bp))
            emit gotoSourcePosition(fbp->fileName(), fbp->lineNum()-1);

    } else if ( action == m_breakpointEdit ) {
        col = currentIndex().column();
        if (col == BreakpointController::Location || col ==  BreakpointController::Condition || col == BreakpointController::IgnoreCount)
            openPersistentEditor(model()->index(currentIndex().row(), col, QModelIndex()));

    } else if ( action == m_breakpointDisable ) {
        bp->setEnabled( !bp->isEnabled( ) );
        bp->sendToGdb();

    } else if ( action == m_breakpointDisableAll || action == m_breakpointEnableAll ) {
        foreach (Breakpoint* breakpoint, breakpoints()->breakpoints())
        {
            breakpoint->setEnabled(action == m_breakpointEnableAll);
            breakpoint->sendToGdb();
        }
    }
    #endif
}
#endif


void BreakpointWidget::showEvent(QShowEvent * event)
{
    Q_UNUSED(event);
    if (firstShow_)
    {
        /* FIXME: iterate over all possible names. */
        int id_width = QFontMetrics(font()).width("MMWrite");
        QHeaderView* header = table_->horizontalHeader();
        int width = header->width();

        header->resizeSection(0, 32);
        width -= 32;
        header->resizeSection(1, 32);
        width -= 32;
        header->resizeSection(2, id_width);
        width -= id_width;
        header->resizeSection(3, width/2);
        header->resizeSection(4, width/2);
        firstShow_ = false;
    }
}

void BreakpointWidget::edit(KDevelop::Breakpoint *n)
{
    QModelIndex index = m_debugController->breakpointModel()->breakpointIndex(n, Breakpoint::LocationColumn);
    table_->setCurrentIndex(index);
    table_->edit(index);
}


void BreakpointWidget::slotAddBlankBreakpoint()
{
    edit(m_debugController->breakpointModel()->addCodeBreakpoint());
}

void BreakpointWidget::slotAddBlankWatchpoint()
{
    edit(m_debugController->breakpointModel()->addWatchpoint());
}

void BreakpointWidget::slotAddBlankReadWatchpoint()
{
    edit(m_debugController->breakpointModel()->addReadWatchpoint());
}


void KDevelop::BreakpointWidget::slotAddBlankAccessWatchpoint()
{
    edit(m_debugController->breakpointModel()->addAccessWatchpoint());
}


void BreakpointWidget::slotRemoveBreakpoint()
{
    QItemSelectionModel* sel = table_->selectionModel();
    QModelIndexList selected = sel->selectedIndexes();
    IF_DEBUG( kDebug() << selected; )
    if (!selected.isEmpty()) {
        m_debugController->breakpointModel()->removeRow(selected.first().row());
    }
}

void BreakpointWidget::slotRemoveAllBreakpoints()
{
    m_debugController->breakpointModel()->removeRows(0, m_debugController->breakpointModel()->rowCount());
}


void BreakpointWidget::slotUpdateBreakpointDetail()
{
    QModelIndexList selected = table_->selectionModel()->selectedIndexes();
    IF_DEBUG( kDebug() << selected; )
    if (selected.isEmpty()) {
        details_->setItem(0);
    } else {
        details_->setItem(m_debugController->breakpointModel()->breakpoint(selected.first().row()));
    }
}

void BreakpointWidget::breakpointHit(KDevelop::Breakpoint* b)
{
    IF_DEBUG( kDebug() << b; )
    QModelIndex index = m_debugController->breakpointModel()->breakpointIndex(b, 0);
    table_->selectionModel()->select(
        index,
        QItemSelectionModel::Rows
        | QItemSelectionModel::ClearAndSelect);
}

void BreakpointWidget::breakpointError(KDevelop::Breakpoint* b, const QString& msg, int column)
{
    IF_DEBUG( kDebug() << b << msg << column; )

    // FIXME: we probably should prevent this error notification during
    // initial setting of breakpoint, to avoid a cloud of popups.
    if (!table_->isVisible())
        return;

    QModelIndex index = m_debugController->breakpointModel()->breakpointIndex(b, column);
    QPoint p = table_->visualRect(index).topLeft();
    p = table_->mapToGlobal(p);

    KPassivePopup *pop = new KPassivePopup(table_);
    pop->setPopupStyle(KPassivePopup::Boxed);
    pop->setAutoDelete(true);
    // FIXME: the icon, too.
    pop->setView("", msg);
    pop->setTimeout(-1);
    pop->show(p);
}

void BreakpointWidget::slotRowClicked(const QModelIndex& index)
{
    if (index.column() != Breakpoint::LocationColumn)
        return;
    Breakpoint *bp = m_debugController->breakpointModel()->breakpoint(index.row());
    if (!bp)
        return;
   ICore::self()->documentController()->openDocument(bp->url().pathOrUrl(KUrl::RemoveTrailingSlash), KTextEditor::Cursor(bp->line(), 0));
   
}

void BreakpointWidget::slotDisableAllBreakpoints()
{
    for (int i = 0; i < m_debugController->breakpointModel()->rowCount() - 1 ; i++) {
        Breakpoint *bp = m_debugController->breakpointModel()->breakpoint(i);
        bp->setData(Breakpoint::EnableColumn, Qt::Unchecked);
    }
}

void BreakpointWidget::slotEnableAllBreakpoints()
{
    for (int i = 0; i < m_debugController->breakpointModel()->rowCount() - 1 ; i++) {
        Breakpoint *bp = m_debugController->breakpointModel()->breakpoint(i);
        bp->setData(Breakpoint::EnableColumn, Qt::Checked);
    }
}


#include "breakpointwidget.moc"
