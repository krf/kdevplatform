/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "annotateview.h"

#include <QTextBrowser>
#include <QRegExp>
#include <QDir>
#include <QHeaderView>
#include <KDebug>

#include <vcsannotationmodel.h>

#include "cvsplugin.h"
#include "cvsjob.h"


AnnotateView::AnnotateView(CvsPlugin* plugin, CvsJob* job, QWidget *parent)
    : QWidget(parent), Ui::AnnotateViewBase(), m_plugin(plugin)
{
    Ui::AnnotateViewBase::setupUi(this);

    m_model = new KDevelop::VcsAnnotationModel( KUrl() );
    annotations->setModel( m_model );

    annotations->verticalHeader()->setVisible(false);

    QHeaderView* header = annotations->horizontalHeader();
    header->setResizeMode(0, QHeaderView::ResizeToContents);
    header->setResizeMode(1, QHeaderView::ResizeToContents);
    header->setResizeMode(2, QHeaderView::Stretch);

    if (job) {
        connect(job, SIGNAL( result(KJob*) ),
                this, SLOT( slotJobFinished(KJob*) ));
    }
}

AnnotateView::~AnnotateView()
{
}

void AnnotateView::slotJobFinished(KJob* job)
{
    if ( job->error() )
    {
        return;
    }

    CvsJob * cvsjob = dynamic_cast<CvsJob*>(job);
    if (!cvsjob) {
        return;
    }


    // Convert job's output into KDevelop::VcsAnnotation
    KDevelop::VcsAnnotation annotateInfo;
    parseOutput(cvsjob->output(), cvsjob->getDirectory(), annotateInfo);

    QList<KDevelop::VcsAnnotationLine> lines;
    for(int i=0; i < annotateInfo.lineCount(); i++) {
        KDevelop::VcsAnnotationLine line = annotateInfo.line(i);

        lines << line;
    }

    // Fill model with data
    m_model->addLines( lines );
}

void AnnotateView::parseOutput(const QString& jobOutput, const QString& workingDirectory, KDevelop::VcsAnnotation& annotateInfo)
{
    // each annotation line looks like this:
    // 1.1 (kdedev 10-Nov-07): #include <QApplication>
    static QRegExp re("([^\\s]+)\\s+\\(([^\\s]+)\\s+([^\\s]+)\\):\\s(.*)");

    // the file is pomoted like this:
    // Annotations for main.cpp
    static QRegExp reFile("Annotations for\\s(.*)");

    QStringList lines = jobOutput.split("\n");

    QString filename;
    for (int i=0, linenumber=1; i<lines.count(); ++i) {
        QString s = lines[i];

        if (re.exactMatch(s)) {
            KDevelop::VcsAnnotationLine item;

            item.setLineNumber( linenumber );
            item.setText( re.cap(4) );
            item.setAuthor( re.cap(2)  );

            KDevelop::VcsRevision rev;
            rev.setRevisionValue( re.cap(1), KDevelop::VcsRevision::FileNumber );
            item.setRevision( rev );

            ///@todo find correct time format code
            //item.setDate( QDateTime::fromString( re.cap(3)/*, Qt::ISODate*/ ) );

            annotateInfo.insertLine( linenumber, item );
            linenumber++;
        } else if (reFile.exactMatch(s)) {
            KUrl url(workingDirectory + QDir::separator() + reFile.cap(1));
            annotateInfo.setLocation( url );
        } else {
            kDebug(9500) << "Unmatched:"<<s<<endl;
        }
    }
}

#include "annotateview.moc"
