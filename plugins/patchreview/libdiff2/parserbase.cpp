/**************************************************************************
**                             parserbase.cpp
**                              -------------------
**      begin                   : Sun Aug  4 15:05:35 2002
**      copyright               : (C) 2002-2004 Otto Bruggeman <otto.bruggeman@home.nl>
**
***************************************************************************/
/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   ( at your option ) any later version.
**
***************************************************************************/

#include "parserbase.h"
#include <qobject.h>

#include <kdebug.h>

#include "diffmodel.h"
#include "diffhunk.h"
#include "difference.h"
#include "komparemodellist.h"


using namespace Diff2;

ParserBase::ParserBase( const KompareModelList* list, const QStringList& diff ) :
    m_diffLines( diff ),
    m_currentModel( 0 ),
    m_models( 0 ),
    m_diffIterator( m_diffLines.begin() ),
    m_singleFileDiff( false ),
    m_list( list )
{
//	kDebug(8101) << diff;
//	kDebug(8101) << m_diffLines;
	m_models = new DiffModelList();

	// used in contexthunkheader
	m_contextHunkHeader1.setPattern( "\\*{15} ?(.*)\\n" ); // capture is for function name
	m_contextHunkHeader2.setPattern( "\\*\\*\\* ([0-9]+),([0-9]+) \\*\\*\\*\\*\\n" );
	// used in contexthunkbody
	m_contextHunkHeader3.setPattern( "--- ([0-9]+),([0-9]+) ----\\n" );

	m_contextHunkBodyRemoved.setPattern( "- (.*)" );
	m_contextHunkBodyAdded.setPattern  ( "\\+ (.*)" );
	m_contextHunkBodyChanged.setPattern( "! (.*)" );
	m_contextHunkBodyContext.setPattern( "  (.*)" );
	m_contextHunkBodyLine.setPattern   ( "[-\\+! ] (.*)" );

	// This regexp sucks... i'll see what happens
	m_normalDiffHeader.setPattern( "diff (?:(?:-|--)[a-zA-Z0-9=\\\"]+ )*(?:|-- +)(.*) +(.*)\\n" );

	m_normalHunkHeaderAdded.setPattern  ( "([0-9]+)a([0-9]+)(|,[0-9]+)(.*)\\n" );
	m_normalHunkHeaderRemoved.setPattern( "([0-9]+)(|,[0-9]+)d([0-9]+)(.*)\\n" );
	m_normalHunkHeaderChanged.setPattern( "([0-9]+)(|,[0-9]+)c([0-9]+)(|,[0-9]+)(.*)\\n" );

	m_normalHunkBodyRemoved.setPattern  ( "< (.*)" );
	m_normalHunkBodyAdded.setPattern    ( "> (.*)" );
	m_normalHunkBodyDivider.setPattern  ( "---" );

	m_unifiedDiffHeader1.setPattern    ( "--- ([^\\t]+)\\t([^\\t]+)(?:\\t?)(.*)\\n" );
	m_unifiedDiffHeader2.setPattern    ( "\\+\\+\\+ ([^\\t]+)\\t([^\\t]+)(?:\\t?)(.*)\\n" );
	m_unifiedHunkHeader.setPattern     ( "@@ -([0-9]+)(|,([0-9]+)) \\+([0-9]+)(|,([0-9]+)) @@(?: ?)(.*)\\n" );
	m_unifiedHunkBodyAdded.setPattern  ( "\\+(.*)" );
	m_unifiedHunkBodyRemoved.setPattern( "-(.*)" );
	m_unifiedHunkBodyContext.setPattern( " (.*)" );
	m_unifiedHunkBodyLine.setPattern   ( "([-+ ])(.*)" );
}

ParserBase::~ParserBase()
{
	if ( m_models )
		m_models = 0; // do not delete this, i pass it around...
}

enum Kompare::Format ParserBase::determineFormat()
{
	// Write your own format detection routine damn it :)
	return Kompare::UnknownFormat;
}

DiffModelList* ParserBase::parse()
{
	switch( determineFormat() )
	{
		case Kompare::Context :
			return parseContext();
		case Kompare::Ed :
			return parseEd();
		case Kompare::Normal :
			return parseNormal();
		case Kompare::RCS :
			return parseRCS();
		case Kompare::Unified :
			return parseUnified();
		default: // Unknown and SideBySide for now
			return 0L;
	}
}

bool ParserBase::parseContextDiffHeader()
{
//	kDebug(8101) << "ParserBase::parseContextDiffHeader()";
	bool result = false;

	while ( m_diffIterator != m_diffLines.end() )
	{
		if ( !m_contextDiffHeader1.exactMatch( *(m_diffIterator)++ ) )
		{
			continue;
		}
//		kDebug(8101) << "Matched length Header1 =" << m_contextDiffHeader1.matchedLength();
//		kDebug(8101) << "Matched string Header1 =" << m_contextDiffHeader1.cap( 0 );
		if ( m_diffIterator != m_diffLines.end() && m_contextDiffHeader2.exactMatch( *m_diffIterator ) )
		{
//			kDebug(8101) << "Matched length Header2 =" << m_contextDiffHeader2.matchedLength();
//			kDebug(8101) << "Matched string Header2 =" << m_contextDiffHeader2.cap( 0 );

			m_currentModel = new DiffModel( m_contextDiffHeader1.cap( 1 ), m_contextDiffHeader2.cap( 1 ) );
			QObject::connect( m_currentModel, SIGNAL( setModified( bool ) ), m_list, SLOT( slotSetModified( bool ) ) );
			m_currentModel->setSourceTimestamp     ( m_contextDiffHeader1.cap( 2 ) );
			m_currentModel->setSourceRevision      ( m_contextDiffHeader1.cap( 4 ) );
			m_currentModel->setDestinationTimestamp( m_contextDiffHeader2.cap( 2 ) );
			m_currentModel->setDestinationRevision ( m_contextDiffHeader2.cap( 4 ) );

			++m_diffIterator;
			result = true;

			break;
		}
		else
		{
			// We're screwed, second line does not match or is not there...
			break;
		}
		// Do not inc the Iterator because the second line might be the first line of
		// the context header and the first hit was a fluke (impossible imo)
		// maybe we should return false here because the diff is broken ?
	}

	return result;
}

bool ParserBase::parseEdDiffHeader()
{
	return false;
}

bool ParserBase::parseNormalDiffHeader()
{
//	kDebug(8101) << "ParserBase::parseNormalDiffHeader()";
	bool result = false;

	while ( m_diffIterator != m_diffLines.end() )
	{
		if ( m_normalDiffHeader.exactMatch( *m_diffIterator ) )
		{
//			kDebug(8101) << "Matched length Header =" << m_normalDiffHeader.matchedLength();
//			kDebug(8101) << "Matched string Header =" << m_normalDiffHeader.cap( 0 );

			m_currentModel = new DiffModel();
			QObject::connect( m_currentModel, SIGNAL( setModified( bool ) ), m_list, SLOT( slotSetModified( bool ) ) );
			m_currentModel->setSourceFile          ( m_normalDiffHeader.cap( 1 ) );
			m_currentModel->setDestinationFile     ( m_normalDiffHeader.cap( 2 ) );

			result = true;

			++m_diffIterator;
			break;
		}
		else
		{
			kDebug(8101) << "No match for:" << ( *m_diffIterator );
		}
		++m_diffIterator;
	}

	if ( result == false )
	{
		// Set this to the first line again and hope it is a single file diff
		m_diffIterator = m_diffLines.begin();
		m_currentModel = new DiffModel();
		QObject::connect( m_currentModel, SIGNAL( setModified( bool ) ), m_list, SLOT( slotSetModified( bool ) ) );
		m_singleFileDiff = true;
	}

	return result;
}

bool ParserBase::parseRCSDiffHeader()
{
	return false;
}

bool ParserBase::parseUnifiedDiffHeader()
{
//	kDebug(8101) << "ParserBase::parseUnifiedDiffHeader()";
	bool result = false;

	while ( m_diffIterator != m_diffLines.end() ) // do not assume we start with the diffheader1 line
	{
		if ( !m_unifiedDiffHeader1.exactMatch( *m_diffIterator ) )
		{
			++m_diffIterator;
			continue;
		}
//		kDebug(8101) << "Matched length Header1 =" << m_unifiedDiffHeader1.matchedLength();
//		kDebug(8101) << "Matched string Header1 =" << m_unifiedDiffHeader1.cap( 0 );
		++m_diffIterator;
		if ( m_diffIterator != m_diffLines.end() && m_unifiedDiffHeader2.exactMatch( *m_diffIterator ) )
		{
			m_currentModel = new DiffModel( m_unifiedDiffHeader1.cap( 1 ), m_unifiedDiffHeader2.cap( 1 ) );
			QObject::connect( m_currentModel, SIGNAL( setModified( bool ) ), m_list, SLOT( slotSetModified( bool ) ) );
			m_currentModel->setSourceTimestamp( m_unifiedDiffHeader1.cap( 2 ) );
			m_currentModel->setSourceRevision( m_unifiedDiffHeader1.cap( 4 ) );
			m_currentModel->setDestinationTimestamp( m_unifiedDiffHeader2.cap( 2 ) );
			m_currentModel->setDestinationRevision( m_unifiedDiffHeader2.cap( 4 ) );

			++m_diffIterator;
			result = true;

			break;
		}
		else
		{
			// We're screwed, second line does not match or is not there...
			break;
		}
	}

	return result;
}

bool ParserBase::parseContextHunkHeader()
{
//	kDebug(8101) << "ParserBase::parseContextHunkHeader()";

	if ( m_diffIterator == m_diffLines.end() )
		return false;

	if ( !m_contextHunkHeader1.exactMatch( *(m_diffIterator) ) )
		return false; // big fat trouble, aborting...

	++m_diffIterator;

	if ( m_diffIterator == m_diffLines.end() )
		return false;

	if ( !m_contextHunkHeader2.exactMatch( *(m_diffIterator) ) )
		return false; // big fat trouble, aborting...

	++m_diffIterator;

	return true;
}

bool ParserBase::parseEdHunkHeader()
{
	return false;
}

bool ParserBase::parseNormalHunkHeader()
{
//	kDebug(8101) << "ParserBase::parseNormalHunkHeader()";
	if ( m_diffIterator != m_diffLines.end() )
	{
//		kDebug(8101) << "Header =" << *m_diffIterator;
		if ( m_normalHunkHeaderAdded.exactMatch( *m_diffIterator ) )
		{
			m_normalDiffType = Difference::Insert;
		}
		else if ( m_normalHunkHeaderRemoved.exactMatch( *m_diffIterator ) )
		{
			m_normalDiffType = Difference::Delete;
		}
		else if ( m_normalHunkHeaderChanged.exactMatch( *m_diffIterator ) )
		{
			m_normalDiffType = Difference::Change;
		}
		else
			return false;

		++m_diffIterator;
		return true;
	}

	return false;
}

bool ParserBase::parseRCSHunkHeader()
{
	return false;
}

bool ParserBase::parseUnifiedHunkHeader()
{
//	kDebug(8101) << "ParserBase::parseUnifiedHunkHeader()";
	if( m_diffIterator == m_diffLines.end() ) return false;

	if ( m_unifiedHunkHeader.exactMatch( *m_diffIterator ) )
	{
		++m_diffIterator;
		return true;
	}
	else
	{
//		kDebug(8101) << "This is not a unified hunk header :" << (*m_diffIterator);
		return false;
	}

}

bool ParserBase::parseContextHunkBody()
{
//	kDebug(8101) << "ParserBase::parseContextHunkBody()";

	// Storing the src part of the hunk for later use
	QStringList oldLines;
	for( ; m_diffIterator != m_diffLines.end() && m_contextHunkBodyLine.exactMatch( *m_diffIterator ); ++m_diffIterator ) {
//		kDebug(8101) << "Added old line:" << *m_diffIterator;
		oldLines.append( *m_diffIterator );
	}

	if( !m_contextHunkHeader3.exactMatch( *m_diffIterator ) )
		return false;

	++m_diffIterator;

	// Storing the dest part of the hunk for later use
	QStringList newLines;
	for( ; m_diffIterator != m_diffLines.end() && m_contextHunkBodyLine.exactMatch( *m_diffIterator ); ++m_diffIterator ) {
//		kDebug(8101) << "Added new line:" << *m_diffIterator;
		newLines.append( *m_diffIterator );
	}

	QString function = m_contextHunkHeader1.cap( 1 );
//	kDebug(8101) << "Captured function:" << function;
	int linenoA      = m_contextHunkHeader2.cap( 1 ).toInt();
//	kDebug(8101) << "Source line number:" << linenoA;
	int linenoB      = m_contextHunkHeader3.cap( 1 ).toInt();
//	kDebug(8101) << "Dest   line number:" << linenoB;

	DiffHunk* hunk = new DiffHunk( linenoA, linenoB, function );

	m_currentModel->addHunk( hunk );

	QStringList::Iterator oldIt = oldLines.begin();
	QStringList::Iterator newIt = newLines.begin();

	Difference* diff;
	while( oldIt != oldLines.end() || newIt != newLines.end() )
	{
		if( oldIt != oldLines.end() && m_contextHunkBodyRemoved.exactMatch( *oldIt ) )
		{
//			kDebug(8101) << "Delete:";
			diff = new Difference( linenoA, linenoB );
			diff->setType( Difference::Delete );
			m_currentModel->addDiff( diff );
//			kDebug(8101) << "Difference added";
			hunk->add( diff );
			for( ; oldIt != oldLines.end() && m_contextHunkBodyRemoved.exactMatch( *oldIt ); ++oldIt )
			{
//				kDebug(8101) << "" << m_contextHunkBodyRemoved.cap( 1 );
				diff->addSourceLine( m_contextHunkBodyRemoved.cap( 1 ) );
				linenoA++;
			}
		}
		else if( newIt != newLines.end() && m_contextHunkBodyAdded.exactMatch( *newIt ) )
		{
//			kDebug(8101) << "Insert:";
			diff = new Difference( linenoA, linenoB );
			diff->setType( Difference::Insert );
			m_currentModel->addDiff( diff );
//			kDebug(8101) << "Difference added";
			hunk->add( diff );
			for( ; newIt != newLines.end() && m_contextHunkBodyAdded.exactMatch( *newIt ); ++newIt )
			{
//				kDebug(8101) << "" << m_contextHunkBodyAdded.cap( 1 );
				diff->addDestinationLine( m_contextHunkBodyAdded.cap( 1 ) );
				linenoB++;
			}
		}
		else if(  ( oldIt == oldLines.end() || m_contextHunkBodyContext.exactMatch( *oldIt ) ) &&
		          ( newIt == newLines.end() || m_contextHunkBodyContext.exactMatch( *newIt ) ) )
		{
//			kDebug(8101) << "Unchanged:";
			diff = new Difference( linenoA, linenoB );
			// Do not add this diff with addDiff to the model... no unchanged differences allowed in there...
			diff->setType( Difference::Unchanged );
			hunk->add( diff );
			while( ( oldIt == oldLines.end() || m_contextHunkBodyContext.exactMatch( *oldIt ) ) &&
			       ( newIt == newLines.end() || m_contextHunkBodyContext.exactMatch( *newIt ) ) &&
			       ( oldIt != oldLines.end() || newIt != newLines.end() ) )
			{
				QString l;
				if( oldIt != oldLines.end() )
				{
					l = m_contextHunkBodyContext.cap( 1 );
//					kDebug(8101) << "old:" << l;
					++oldIt;
				}
				if( newIt != newLines.end() )
				{
					l = m_contextHunkBodyContext.cap( 1 );
//					kDebug(8101) << "new:" << l;
					++newIt;
				}
				diff->addSourceLine( l );
				diff->addDestinationLine( l );
				linenoA++;
				linenoB++;
			}
		}
		else if( ( oldIt != oldLines.end() && m_contextHunkBodyChanged.exactMatch( *oldIt ) ) ||
		         ( newIt != newLines.end() && m_contextHunkBodyChanged.exactMatch( *newIt ) ) )
		{
//			kDebug(8101) << "Changed:";
			diff = new Difference( linenoA, linenoB );
			diff->setType( Difference::Change );
			m_currentModel->addDiff( diff );
//			kDebug(8101) << "Difference added";
			hunk->add( diff );
			while( oldIt != oldLines.end() && m_contextHunkBodyChanged.exactMatch( *oldIt ) )
			{
//				kDebug(8101) << "" << m_contextHunkBodyChanged.cap( 1 );
				diff->addSourceLine( m_contextHunkBodyChanged.cap( 1 ) );
				linenoA++;
				++oldIt;
			}
			while( newIt != newLines.end() && m_contextHunkBodyChanged.exactMatch( *newIt ) )
			{
//				kDebug(8101) << "" << m_contextHunkBodyChanged.cap( 1 );
				diff->addDestinationLine( m_contextHunkBodyChanged.cap( 1 ) );
				linenoB++;
				++newIt;
			}
		}
		else
			return false;
		diff->determineInlineDifferences();
	}

	return true;
}

bool ParserBase::parseEdHunkBody()
{
	return false;
}

bool ParserBase::parseNormalHunkBody()
{
//	kDebug(8101) << "ParserBase::parseNormalHunkBody";

	QString type;

	int linenoA = 0, linenoB = 0;

	if ( m_normalDiffType == Difference::Insert )
	{
		linenoA = m_normalHunkHeaderAdded.cap( 1 ).toInt();
		linenoB = m_normalHunkHeaderAdded.cap( 2 ).toInt();
	}
	else if ( m_normalDiffType == Difference::Delete )
	{
		linenoA = m_normalHunkHeaderRemoved.cap( 1 ).toInt();
		linenoB = m_normalHunkHeaderRemoved.cap( 3 ).toInt();
	}
	else if ( m_normalDiffType == Difference::Change )
	{
		linenoA = m_normalHunkHeaderChanged.cap( 1 ).toInt();
		linenoB = m_normalHunkHeaderChanged.cap( 3 ).toInt();
	}

	DiffHunk* hunk = new DiffHunk( linenoA, linenoB );
	m_currentModel->addHunk( hunk );
	Difference* diff = new Difference( linenoA, linenoB );
	hunk->add( diff );
	m_currentModel->addDiff( diff );

	diff->setType( m_normalDiffType );

	if ( m_normalDiffType == Difference::Change || m_normalDiffType == Difference::Delete )
		for( ; m_diffIterator != m_diffLines.end() && m_normalHunkBodyRemoved.exactMatch( *m_diffIterator ); ++m_diffIterator )
		{
//			kDebug(8101) << "Line =" << *m_diffIterator;
			diff->addSourceLine( m_normalHunkBodyRemoved.cap( 1 ) );
		}
	if ( m_normalDiffType == Difference::Change )
		if( m_diffIterator != m_diffLines.end() && m_normalHunkBodyDivider.exactMatch( *m_diffIterator ) )
		{
//			kDebug(8101) << "Line =" << *m_diffIterator;
			++m_diffIterator;
		}
		else
			return false;
	if ( m_normalDiffType == Difference::Insert || m_normalDiffType == Difference::Change )
		for( ; m_diffIterator != m_diffLines.end() && m_normalHunkBodyAdded.exactMatch( *m_diffIterator ); ++m_diffIterator )
		{
//			kDebug(8101) << "Line =" << *m_diffIterator;
			diff->addDestinationLine( m_normalHunkBodyAdded.cap( 1 ) );
		}

	return true;
}

bool ParserBase::parseRCSHunkBody()
{
	return false;
}

bool ParserBase::matchesUnifiedHunkLine( QString line ) const
{
	static const QChar context( ' ' );
	static const QChar added  ( '+' );
	static const QChar removed( '-' );

	QChar first = line[0];

	return ( first == context || first == added || first == removed );
}

bool ParserBase::parseUnifiedHunkBody()
{
//	kDebug(8101) << "ParserBase::parseUnifiedHunkBody";

	int linenoA = 0, linenoB = 0;

	// Fetching the stuff we need from the hunkheader regexp that was parsed in parseUnifiedHunkHeader();
	linenoA = m_unifiedHunkHeader.cap( 1 ).toInt();
	linenoB = m_unifiedHunkHeader.cap( 4 ).toInt();
	QString function = m_unifiedHunkHeader.cap( 7 );
	for ( int i = 0; i < 9; i++ )
	{
//		kDebug(8101) << "Capture" << i << ":" << m_unifiedHunkHeader.cap( i );
	}

	DiffHunk* hunk = new DiffHunk( linenoA, linenoB, function );
	m_currentModel->addHunk( hunk );

	const QStringList::ConstIterator m_diffLinesEnd = m_diffLines.end();

	const QString context = QString( " " );
	const QString added   = QString( "+" );
	const QString removed = QString( "-" );

	while( m_diffIterator != m_diffLinesEnd && matchesUnifiedHunkLine( *m_diffIterator ) )
	{
		Difference* diff = new Difference( linenoA, linenoB );
		hunk->add( diff );

		if( (*m_diffIterator).startsWith( context ) )
		{	// context
			for( ; m_diffIterator != m_diffLinesEnd && (*m_diffIterator).startsWith( context ); ++m_diffIterator )
			{
				diff->addSourceLine( QString( *m_diffIterator ).remove( 0, 1 ) );
				diff->addDestinationLine( QString( *m_diffIterator ).remove( 0, 1 ) );
				linenoA++;
				linenoB++;
			}
		}
		else
		{	// This is a real difference, not context
			for( ; m_diffIterator != m_diffLinesEnd && (*m_diffIterator).startsWith( removed ); ++m_diffIterator )
			{
				diff->addSourceLine( QString( *m_diffIterator ).remove( 0, 1 ) );
				linenoA++;
			}
			for( ; m_diffIterator != m_diffLinesEnd && (*m_diffIterator).startsWith( added ); ++m_diffIterator )
			{
				diff->addDestinationLine( QString( *m_diffIterator ).remove( 0, 1 ) );
				linenoB++;
			}
			if ( diff->sourceLineCount() == 0 )
			{
				diff->setType( Difference::Insert );
//				kDebug(8101) << "Insert difference";
			}
			else if ( diff->destinationLineCount() == 0 )
			{
				diff->setType( Difference::Delete );
//				kDebug(8101) << "Delete difference";
			}
			else
			{
				diff->setType( Difference::Change );
//				kDebug(8101) << "Change difference";
			}
			diff->determineInlineDifferences();
			m_currentModel->addDiff( diff );
		}
	}

	return true;
}

DiffModelList* ParserBase::parseContext()
{
	while ( parseContextDiffHeader() )
	{
		while ( parseContextHunkHeader() )
			parseContextHunkBody();
		if ( m_currentModel->differenceCount() > 0 )
			m_models->append( m_currentModel );
	}

	m_models->sort();

	if ( m_models->count() > 0 )
	{
		return m_models;
	}
	else
	{
		delete m_models;
		return 0L;
	}
}

DiffModelList* ParserBase::parseEd()
{
	while ( parseEdDiffHeader() )
	{
		while ( parseEdHunkHeader() )
			parseEdHunkBody();
		if ( m_currentModel->differenceCount() > 0 )
			m_models->append( m_currentModel );
	}

	m_models->sort();

	if ( m_models->count() > 0 )
	{
		return m_models;
	}
	else
	{
		delete m_models;
		return 0L;
	}
}

DiffModelList* ParserBase::parseNormal()
{
	while ( parseNormalDiffHeader() )
	{
		while ( parseNormalHunkHeader() )
			parseNormalHunkBody();
		if ( m_currentModel->differenceCount() > 0 )
			m_models->append( m_currentModel );
	}

	if ( m_singleFileDiff )
	{
		while ( parseNormalHunkHeader() )
			parseNormalHunkBody();
		if ( m_currentModel->differenceCount() > 0 )
			m_models->append( m_currentModel );
	}

	m_models->sort();

	if ( m_models->count() > 0 )
	{
		return m_models;
	}
	else
	{
		delete m_models;
		return 0L;
	}
}

DiffModelList* ParserBase::parseRCS()
{
	while ( parseRCSDiffHeader() )
	{
		while ( parseRCSHunkHeader() )
			parseRCSHunkBody();
		if ( m_currentModel->differenceCount() > 0 )
			m_models->append( m_currentModel );
	}

	m_models->sort();

	if ( m_models->count() > 0 )
	{
		return m_models;
	}
	else
	{
		delete m_models;
		return 0L;
	}
}

DiffModelList* ParserBase::parseUnified()
{
	while ( parseUnifiedDiffHeader() )
	{
		while ( parseUnifiedHunkHeader() )
			parseUnifiedHunkBody();
//		kDebug(8101) << "New model ready to be analyzed...";
//		kDebug(8101) << "differenceCount() ==" << m_currentModel->differenceCount();
		if ( m_currentModel->differenceCount() > 0 )
			m_models->append( m_currentModel );
	}

	m_models->sort();

	if ( m_models->count() > 0 )
	{
		return m_models;
	}
	else
	{
		delete m_models;
		return 0L;
	}
}
