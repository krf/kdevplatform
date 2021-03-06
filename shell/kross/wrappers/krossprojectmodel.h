#ifndef KROSSPROJECTMODEL_H
#define KROSSPROJECTMODEL_H

#include<QtCore/QVariant>

//This is file has been generated by xmltokross, you should not edit this file but the files used to generate it.

namespace KDevelop { class ProjectVisitor; }
namespace KDevelop { class ProjectBaseItem; }
namespace KDevelop { class ProjectFolderItem; }
namespace KDevelop { class ProjectBuildFolderItem; }
namespace KDevelop { class ProjectTargetItem; }
namespace KDevelop { class ProjectExecutableTargetItem; }
namespace KDevelop { class ProjectLibraryTargetItem; }
namespace KDevelop { class ProjectFileItem; }
namespace KDevelop { class ProjectModel; }
namespace Handlers
{
	QVariant kDevelopProjectModelHandler(KDevelop::ProjectModel* type);
	QVariant kDevelopProjectModelHandler(const KDevelop::ProjectModel* type);

	QVariant kDevelopProjectFileItemHandler(KDevelop::ProjectFileItem* type);
	QVariant kDevelopProjectFileItemHandler(const KDevelop::ProjectFileItem* type);

	QVariant kDevelopProjectLibraryTargetItemHandler(KDevelop::ProjectLibraryTargetItem* type);
	QVariant kDevelopProjectLibraryTargetItemHandler(const KDevelop::ProjectLibraryTargetItem* type);

	QVariant kDevelopProjectExecutableTargetItemHandler(KDevelop::ProjectExecutableTargetItem* type);
	QVariant kDevelopProjectExecutableTargetItemHandler(const KDevelop::ProjectExecutableTargetItem* type);

	QVariant kDevelopProjectTargetItemHandler(KDevelop::ProjectTargetItem* type);
	QVariant kDevelopProjectTargetItemHandler(const KDevelop::ProjectTargetItem* type);

	QVariant kDevelopProjectBuildFolderItemHandler(KDevelop::ProjectBuildFolderItem* type);
	QVariant kDevelopProjectBuildFolderItemHandler(const KDevelop::ProjectBuildFolderItem* type);

	QVariant kDevelopProjectFolderItemHandler(KDevelop::ProjectFolderItem* type);
	QVariant kDevelopProjectFolderItemHandler(const KDevelop::ProjectFolderItem* type);

	QVariant kDevelopProjectBaseItemHandler(KDevelop::ProjectBaseItem* type);
	QVariant kDevelopProjectBaseItemHandler(const KDevelop::ProjectBaseItem* type);

	QVariant kDevelopProjectVisitorHandler(KDevelop::ProjectVisitor* type);
	QVariant kDevelopProjectVisitorHandler(const KDevelop::ProjectVisitor* type);

}

#endif
