/*
 * KDevelop Class Browser
 *
 * Copyright 2009 Lior Mualem <lior.m.kde@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
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

#ifndef ALLCLASSESFOLDER_H
#define ALLCLASSESFOLDER_H

#include "classmodelnode.h"

namespace KDevelop
{
  class IProject;
}

namespace ClassModelNodes
{

class StaticNamespaceFolderNode;

/// This folder displays all the classes that relate to a list of documents.
class DocumentClassesFolder : public QObject, public DynamicFolderNode
{
  Q_OBJECT
public:
  DocumentClassesFolder(const QString& a_displayName, NodesModelInterface* a_model);

protected: // Documents list handling.
  /// Parse a single document for classes and add them to the list.
  void parseDocument(const KDevelop::IndexedString& a_file);

  /// Re-parse the given document - remove old declarations and add new declarations.
  bool updateDocument(const KDevelop::IndexedString& a_file);

  /// Close and remove all the nodes related to the specified document.
  void closeDocument(const KDevelop::IndexedString& a_file);

  /// Returns a list of documents we have monitored.
  QList< KDevelop::IndexedString > getAllOpenDocuments();

protected: // Overridables
  /// Override this to filter the found classes.
  virtual bool isClassFiltered(const KDevelop::QualifiedIdentifier&) { return false; }
  
public: // Node overrides
  virtual void nodeCleared();
  virtual void populateNode();
  virtual bool hasChildren() const { return true; }

private Q_SLOTS:
  // Files update.
  void branchRemoved(KDevelop::DUContextPointer context);
  void branchModified(KDevelop::DUContextPointer context);
  void updateChangedFiles();

private: // File updates related.
  /// List of updated files we check this list when update timer expires.
  QSet<KDevelop::IndexedString> m_updatedFiles;

  /// Timer for batch updates.
  QTimer* m_updateTimer;

private:
  typedef QMap< KDevelop::IndexedQualifiedIdentifier, StaticNamespaceFolderNode* > NamespacesMap;
  /// Holds a map between an identifier and a namespace folder we hold.
  NamespacesMap m_namespaces;

  typedef QMap< KDevelop::IndexedQualifiedIdentifier, ClassNode* > ClassNodeIDsMap;
  typedef QMap< KDevelop::IndexedString, ClassNodeIDsMap > OpenFilesMap;
  /// Maps all displayed classes and their referenced files.
  OpenFilesMap m_openFiles;

  /// Recursively create a namespace folder for the specified identifier if it doesn't
  /// exist, cache it and return it (or just return it from the cache).
  StaticNamespaceFolderNode* getNamespaceFolder(const KDevelop::QualifiedIdentifier& a_identifier);

  /// Removes the given namespace identifier recursively if it's empty.
  void removeEmptyNamespace(const KDevelop::QualifiedIdentifier& a_identifier);

  /// Remove a single class node from the lists.
  void removeClassNode(ClassNode* a_node);
};

/// Special folder.
/// It displays all the classes in the projects by using the IProject
class AllClassesFolder : public DocumentClassesFolder
{
  Q_OBJECT
public:
  AllClassesFolder(NodesModelInterface* a_model);

public: // Node overrides
  virtual void nodeCleared();
  virtual void populateNode();

private Q_SLOTS:
  // Project watching
  void projectOpened(KDevelop::IProject* project);
  void projectClosing(KDevelop::IProject* project);
};

/// Contains a filter for the all classes folder.
class FilteredAllClassesFolder : public AllClassesFolder
{
  Q_OBJECT
public:
  FilteredAllClassesFolder(NodesModelInterface* a_model);

public: // Operations.
  /// Call this to update the classes filter string.
  void updateFilterString(QString a_newFilterString);
  
private: // DocumentClassesFolder overrides
  virtual bool isClassFiltered(const KDevelop::QualifiedIdentifier& a_id);

private:
  /// We'll use this string to display only classes that match this string.
  QString m_filterString;
};

} // namespace ClassModelNodes

#endif