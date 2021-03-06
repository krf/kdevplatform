/*
    This file is part of KDevelop
    Copyright (C) 2012  Morten Danielsen Volden mvolden2@gmail.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "outputfilteringstrategies.h"
#include "outputformats.h"
#include "filtereditem.h"

#include <KDebug>
#include <KLocalizedString>

#include <QFileInfo>

#include <algorithm>


namespace KDevelop
{

/// --- No filter strategy ---

NoFilterStrategy::NoFilterStrategy()
{
}

FilteredItem NoFilterStrategy::actionInLine(const QString& line)
{
    return FilteredItem( line );
}

FilteredItem NoFilterStrategy::errorInLine(const QString& line)
{
    return FilteredItem( line );
}


/// --- Compiler error filter strategy ---

/// Impl. of CompilerFilterStrategy.
struct CompilerFilterStrategyPrivate
{
    CompilerFilterStrategyPrivate(const KUrl& buildDir);
    KUrl urlForFile( const QString& ) const;
    bool isMultiLineCase(ErrorFormat curErrFilter) const;
    void putDirAtEnd(const QString& );

    QVector<QString> m_currentDirs;
    KUrl m_buildDir;

    typedef QMap<QString, int> PositionMap;
    PositionMap m_positionInCurrentDirs;
};

// All the possible string that indicate an error if we via Regex have been able to
// extract file and linenumber from a given outputline
typedef QPair<QString, FilteredItem::FilteredOutputItemType> Indicator;
static const QVector<Indicator> INDICATORS = QVector<Indicator>()
    // ld
    << Indicator("undefined reference", FilteredItem::ErrorItem)
    << Indicator("undefined symbol", FilteredItem::ErrorItem)
    << Indicator("ld: cannot find", FilteredItem::ErrorItem)
    << Indicator("no such file", FilteredItem::ErrorItem)
    // gcc
    << Indicator("error", FilteredItem::ErrorItem)
    // generic
    << Indicator("warning", FilteredItem::WarningItem)
    << Indicator("info", FilteredItem::InformationItem)
    << Indicator("note", FilteredItem::InformationItem);

// A list of filters for possible compiler, linker, and make errors
const QList<ErrorFormat> ERROR_FILTERS = QList<ErrorFormat>()
    // GCC - another case, eg. for #include "pixmap.xpm" which does not exists
    << ErrorFormat( "^([^:\t]+):([0-9]+):([0-9]+):([^0-9]+)", 1, 2, 4, 3 )
    // GCC
    << ErrorFormat( "^([^:\t]+):([0-9]+):([^0-9]+)", 1, 2, 3 )
    // GCC
    << ErrorFormat( "^(In file included from |[ ]+from )([^: \\t]+):([0-9]+)(:|,)(|[0-9]+)", 2, 3, 5 )
    // ICC
    << ErrorFormat( "^([^: \\t]+)\\(([0-9]+)\\):([^0-9]+)", 1, 2, 3, "intel" )
    //libtool link
    << ErrorFormat( "^(libtool):( link):( warning): ", 0, 0, 0 )
    // make
    << ErrorFormat( "No rule to make target", 0, 0, 0 )
    // cmake
    << ErrorFormat( "^([^: \\t]+):([0-9]+):", 1, 2, 0, "cmake" )
    // cmake
    << ErrorFormat( "CMake (Error|Warning) (|\\([a-zA-Z]+\\) )(in|at) ([^:]+):($|[0-9]+)", 4, 5, 1, "cmake" )
    // Fortran
    << ErrorFormat( "\"(.*)\", line ([0-9]+):(.*)", 1, 2, 3 )
    // GFortran
    << ErrorFormat( "^(.*):([0-9]+)\\.([0-9]+):(.*)", 1, 2, 4, "gfortran", 3 )
    // Jade
    << ErrorFormat( "^[a-zA-Z]+:([^: \t]+):([0-9]+):[0-9]+:[a-zA-Z]:(.*)", 1, 2, 3 )
    // ifort
    << ErrorFormat( "^fortcom: (.*): (.*), line ([0-9]+):(.*)", 2, 3, 1, "intel" )
    // PGI
    << ErrorFormat( "PGF9(.*)-(.*)-(.*)-(.*) \\((.*): ([0-9]+)\\)", 5, 6, 4, "pgi" )
    // PGI (2)
    << ErrorFormat( "PGF9(.*)-(.*)-(.*)-Symbol, (.*) \\((.*)\\)", 5, 5, 4, "pgi" );

// A list of filters for possible compiler, linker, and make actions
QList<ActionFormat> ACTION_FILTERS = QList<ActionFormat>()
    << ActionFormat( i18n("compiling"), 1, 2, "(?:^|[^=])\\b(gcc|CC|cc|distcc|c\\+\\+|"
                     "g\\+\\+|icc|icpc)\\s+.*-c.*[/ '\\\\]+(\\w+\\.(?:cpp|CPP|c|C|cxx|CXX|cs|"
                     "java|hpf|f|F|f90|F90|f95|F95))")
    //moc and uic
    << ActionFormat( i18n("generating"), 1, 2, "/(moc|uic)\\b.*\\s-o\\s([^\\s;]+)")
    //libtool linking
    << ActionFormat( i18nc("Linking object files into a library or executable", "linking"),
                     "libtool", "/bin/sh\\s.*libtool.*--mode=link\\s.*\\s-o\\s([^\\s;]+)", 1 )
    //unsermake
    << ActionFormat( i18n("compiling"), 1, 1, "^compiling (.*)" )
    << ActionFormat( i18n("generating"), 1, 2, "^generating (.*)" )
    << ActionFormat( i18nc("Linking object files into a library or executable",
                     "linking"), 1, 2, "(gcc|cc|c\\+\\+|g\\+\\+|icc|icpc)\\S* (?:\\S* )*-o ([^\\s;]+)")
    << ActionFormat( i18nc("Linking object files into a library or executable",
                     "linking"), 1, 2, "^linking (.*)" )
    //cmake
    << ActionFormat( i18n("built"), -1, 1, "\\[.+%\\] Built target (.*)" )
    << ActionFormat( i18n("compiling"), "cmake", "\\[.+%\\] Building .* object (.*)CMakeFiles/", 1 )
    << ActionFormat( i18n("generating"), -1, 1, "\\[.+%\\] Generating (.*)" )
    << ActionFormat( i18nc("Linking object files into a library or executable",
                     "linking"), -1, 1, "^Linking (.*)" )
    << ActionFormat( i18n("configuring"), "cmake", "(-- Configuring (done|incomplete)|-- Found|-- Adding|-- Enabling)", -1 )
    << ActionFormat( i18n("installing"), -1, 1, "-- Installing (.*)" )
    //libtool install
    << ActionFormat( i18n("creating"), "", "/(?:bin/sh\\s.*mkinstalldirs).*\\s([^\\s;]+)", 1 )
    << ActionFormat( i18n("installing"), "", "/(?:usr/bin/install|bin/sh\\s.*mkinstalldirs"
                     "|bin/sh\\s.*libtool.*--mode=install).*\\s([^\\s;]+)", 1 )
    //dcop
    << ActionFormat( i18n("generating"), "dcopidl", "dcopidl .* > ([^\\s;]+)", 1 )
    << ActionFormat( i18n("compiling"), "dcopidl2cpp", "dcopidl2cpp (?:\\S* )*([^\\s;]+)", 1 )
    // match against Entering directory to update current build dir
    << ActionFormat( "cd", "", "make\\[\\d+\\]: Entering directory (\\`|\\')(.+)'", 2);


CompilerFilterStrategyPrivate::CompilerFilterStrategyPrivate(const KUrl& buildDir)
: m_buildDir(buildDir)
{
}

KUrl CompilerFilterStrategyPrivate::urlForFile(const QString& filename) const
{
    QFileInfo fi( filename );
    KUrl currentUrl;
    if( fi.isRelative() ) {
        if( m_currentDirs.isEmpty() ) {
            currentUrl = m_buildDir;
            currentUrl.addPath( filename );
            return currentUrl;
        }

        QVector<QString>::const_iterator it = m_currentDirs.constEnd() - 1;
        do {
            currentUrl = KUrl( *it );
            currentUrl.addPath( filename );
        } while( (it-- !=  m_currentDirs.constBegin()) && !QFileInfo(currentUrl.toLocalFile()).exists() );

        return currentUrl;
    } else {
        currentUrl = KUrl( filename );
    }
    return currentUrl;
}

bool CompilerFilterStrategyPrivate::isMultiLineCase(KDevelop::ErrorFormat curErrFilter) const
{
    if(curErrFilter.compiler == "gfortran" || curErrFilter.compiler == "cmake") {
        return true;
    }
    return false;
}

void CompilerFilterStrategyPrivate::putDirAtEnd(const QString& dirNameToInsert)
{
    CompilerFilterStrategyPrivate::PositionMap::iterator it = m_positionInCurrentDirs.find( dirNameToInsert );
    // Encountered new build directory?
    if (it == m_positionInCurrentDirs.end()) {
        m_currentDirs.push_back( dirNameToInsert );
        m_positionInCurrentDirs.insert( dirNameToInsert, m_currentDirs.size() - 1 );
    } else {
        // Build dir already in currentDirs, but move it to back of currentDirs list
        // (this gives us most-recently-used semantics in urlForFile)
        std::rotate(m_currentDirs.begin() + it.value(), m_currentDirs.begin() + it.value() + 1, m_currentDirs.end() );
        it.value() = m_currentDirs.size() - 1;
    }
}


CompilerFilterStrategy::CompilerFilterStrategy(const KUrl& buildDir)
: d(new CompilerFilterStrategyPrivate( buildDir ))
{
}

CompilerFilterStrategy::~CompilerFilterStrategy()
{
    delete d;
}

QVector< QString > CompilerFilterStrategy::getCurrentDirs()
{
    return d->m_currentDirs;
}

FilteredItem CompilerFilterStrategy::actionInLine(const QString& line)
{
    FilteredItem item(line);
    foreach( const ActionFormat& curActFilter, ACTION_FILTERS ) {
        QRegExp regEx = curActFilter.expression;
        if( regEx.indexIn( line ) != -1 )
        {
            item.type = FilteredItem::ActionItem;
            if( curActFilter.fileGroup != -1 && curActFilter.toolGroup != -1 )
            {
                item.shortenedText = QString( "%1 %2 (%3)").arg( curActFilter.action ).arg( regEx.cap( curActFilter.fileGroup ) ).arg( regEx.cap( curActFilter.toolGroup ) );
            }
            if( curActFilter.action == "cd" )
            {
                d->m_currentDirs.push_back( regEx.cap( curActFilter.fileGroup ) );
                d->m_positionInCurrentDirs.insert( regEx.cap( curActFilter.fileGroup ) , d->m_currentDirs.size() - 1 );
            }

            // Special case for cmake: we parse the "Compiling <objectfile>" expression
            // and use it to find out about the build paths encountered during a build.
            // They are later searched by urlForFile to find source files corresponding to
            // compiler errors.
            if ( curActFilter.action == i18n("compiling") && curActFilter.tool == "cmake") {
                KUrl url = d->m_buildDir;
                url.addPath(regEx.cap( curActFilter.fileGroup ));
                d->putDirAtEnd(url.toLocalFile());
            }
            break;
        }
    }
    return item;
}

FilteredItem CompilerFilterStrategy::errorInLine(const QString& line)
{
    FilteredItem item(line);
    foreach( const ErrorFormat& curErrFilter, ERROR_FILTERS ) {
        QRegExp regEx = curErrFilter.expression;
        if( regEx.indexIn( line ) != -1 && !( line.contains( "Each undeclared identifier is reported only once" ) || line.contains( "for each function it appears in." ) ) ) {
            if(curErrFilter.fileGroup > 0) {
                if( curErrFilter.compiler == "cmake" ) { // Unfortunately we cannot know if an error or an action comes first in cmake, and therefore we need to do this
                    if( d->m_currentDirs.empty() ) {
                        d->putDirAtEnd( d->m_buildDir.upUrl().toLocalFile() );
                    }
                }
                item.url = d->urlForFile( regEx.cap( curErrFilter.fileGroup ) );
            }
            item.lineNo = regEx.cap( curErrFilter.lineGroup ).toInt() - 1;
            if(curErrFilter.columnGroup >= 0) {
                item.columnNo = regEx.cap( curErrFilter.columnGroup ).toInt() - 1;
            } else {
                item.columnNo = 0;
            }

            QString txt = regEx.cap(curErrFilter.textGroup);

            // Find the indicator which happens most early.
            int earliestIndicatorIdx = txt.length();
            foreach( const Indicator& curIndicator, INDICATORS ) {
                int curIndicatorIdx = txt.indexOf(curIndicator.first, 0, Qt::CaseInsensitive);
                if((curIndicatorIdx >= 0) && (earliestIndicatorIdx > curIndicatorIdx)) {
                    earliestIndicatorIdx = curIndicatorIdx;
                    item.type = curIndicator.second;
                }
            }

            // Make the item clickable if it comes with the necessary file & line number information
            if (curErrFilter.fileGroup > 0 && curErrFilter.lineGroup > 0) {
                item.isActivatable = true;
                if(item.type == FilteredItem::InvalidItem) {
                    // If there are no error indicators in the line
                    // maybe this is a multiline case
                    if(d->isMultiLineCase(curErrFilter)) {
                        item.type = FilteredItem::ErrorItem;
                    } else {
                        // Okay so we couldn't find anything to indicate an error, but we have file and lineGroup
                        // Lets keep this item clickable and indicate this to the user.
                        item.type = FilteredItem::InformationItem;
                    }
                }
            }
            break;
        }
    }
    return item;
}


/// --- Script error filter strategy ---

// A list of filters for possible Python and PHP errors
const QList<ErrorFormat> SCRIPT_ERROR_FILTERS = QList<ErrorFormat>()
    //QRegExp python("^  File \"(.*)\", line (\\d*), in(.*)$");
    << ErrorFormat( "^  File \"(.*)\", line ([0-9]+)(.*$|, in(.*)$)", 1, 2, -1 )
    //QRegExp phpstacktrace("^.*(/.*):(\\d*).*$");
    << ErrorFormat( "^.*(/.*):([0-9]+).*$", 1, 2, -1 )
    //QRegExp phperror("^.* in (/.*) on line (\\d*).*$");
    << ErrorFormat( "^.* in (/.*) on line ([0-9]+).*$", 1, 2, -1 );


ScriptErrorFilterStrategy::ScriptErrorFilterStrategy()
{
}

FilteredItem ScriptErrorFilterStrategy::actionInLine(const QString& line)
{
    return FilteredItem(line);
}

FilteredItem ScriptErrorFilterStrategy::errorInLine(const QString& line)
{
    FilteredItem item(line);
    foreach( const ErrorFormat& curErrFilter, SCRIPT_ERROR_FILTERS ) {
        QRegExp regEx = curErrFilter.expression;
        if( regEx.indexIn( line ) != -1 )
        {
            item.url = regEx.cap( curErrFilter.fileGroup );
            item.lineNo = regEx.cap( curErrFilter.lineGroup ).toInt() - 1;
            if(curErrFilter.columnGroup >= 0) {
                item.columnNo = regEx.cap( curErrFilter.columnGroup ).toInt() - 1;
            } else {
                item.columnNo = 0;
            }

            QString txt = regEx.cap(curErrFilter.textGroup);

            item.type = FilteredItem::ErrorItem;

            // Make the item clickable if it comes with the necessary file & line number information
            if (curErrFilter.fileGroup > 0 && curErrFilter.lineGroup > 0)
                item.isActivatable = true;

            break;
        }
    }
    return item;
}

/// --- Static Analysis filter strategy ---

// A list of filters for static analysis tools (krazy2, cppcheck)
const QList<ErrorFormat> STATIC_ANALYSIS_FILTERS = QList<ErrorFormat>()
    // CppCheck
    << ErrorFormat( "^\\[(.*):([0-9]+)\\]:(.*)", 1, 2, 3 )
    // krazy2
    << ErrorFormat( "^\\t([^:]+).*line#([0-9]+).*", 1, 2, -1 )
    // krazy2 without line info
    << ErrorFormat( "^\\t(.*): missing license", 1, -1, -1 );

StaticAnalysisFilterStrategy::StaticAnalysisFilterStrategy()
{
}

FilteredItem StaticAnalysisFilterStrategy::actionInLine(const QString& line)
{
    return FilteredItem(line);
}

FilteredItem StaticAnalysisFilterStrategy::errorInLine(const QString& line)
{
    FilteredItem item(line);
    foreach( const ErrorFormat& curErrFilter, STATIC_ANALYSIS_FILTERS ) {
        QRegExp regEx = curErrFilter.expression;
        if( regEx.indexIn( line ) != -1 )
        {
            item.url = regEx.cap( curErrFilter.fileGroup );
            item.lineNo = regEx.cap( curErrFilter.lineGroup ).toInt() - 1;
            if(curErrFilter.columnGroup >= 0) {
                item.columnNo = regEx.cap( curErrFilter.columnGroup ).toInt() - 1;
            } else {
                item.columnNo = 0;
            }

            QString txt = regEx.cap(curErrFilter.textGroup);

            item.type = FilteredItem::ErrorItem;

            // Make the item clickable if it comes with the necessary file & line number information
            if (curErrFilter.fileGroup > 0 && curErrFilter.lineGroup > 0) {
                item.isActivatable = true;
            }
            break;
        }
    }
    return item;
}


} // namespace KDevelop

