/*
    This plugin is part of KDevelop.

    Copyright (C) 2010 Milian Wolff <mail@milianw.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/

#ifndef EXTERNALSCRIPTPLUGIN_H
#define EXTERNALSCRIPTPLUGIN_H

#include <interfaces/iplugin.h>
#include <QVariantList>
#include <KConfigGroup>
#include <QModelIndex>

class ExternalScriptItem;

class QStandardItem;
class QStandardItemModel;

class ExternalScriptPlugin : public KDevelop::IPlugin
{
  Q_OBJECT

public:
  ExternalScriptPlugin( QObject *parent, const QVariantList &args = QVariantList() );

  virtual ~ExternalScriptPlugin();
  virtual void unload();

  static ExternalScriptPlugin* self();

  /**
   * @return The model storing all external scripts managed by this plugin.
   * @NOTE: always append() items, never insert in the middle!
   */
  QStandardItemModel* model() const;

  /**
   * Executes @p script.
   */
  void execute(ExternalScriptItem* item) const;

  /**
   * Returns config group to store all settings for this plugin in.
   */
  KConfigGroup getConfig() const;

  void saveItem(const ExternalScriptItem* item);

public slots:
  void executeScriptFromActionData() const;

private slots:
  void rowsRemoved( const QModelIndex& parent, int start, int end );
  void rowsInserted( const QModelIndex& parent, int start, int end );

private:
  /// @param item row in the model for the item to save
  void saveItemForRow( int row );

  QStandardItemModel* m_model;
  static ExternalScriptPlugin* m_self;

  class ExternalScriptViewFactory *m_factory;
};

#endif // EXTERNALSCRIPTPLUGIN_H

// kate: indent-mode cstyle; space-indent on; indent-width 2; replace-tabs on; 