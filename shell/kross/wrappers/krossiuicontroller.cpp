//This is file has been generated by xmltokross, you should not edit this file but the files used to generate it.

#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <kross/core/manager.h>
#include <kross/core/wrapperinterface.h>
#include <interfaces/iuicontroller.h>

class KrossKDevelopIToolViewFactory : public QObject, public Kross::WrapperInterface
{
	Q_OBJECT
	public:
		KrossKDevelopIToolViewFactory(KDevelop::IToolViewFactory* obj, QObject* parent=0) : QObject(parent), wrapped(obj)		{ setObjectName("KDevelop::IToolViewFactory"); }
		void* wrappedObject() const { return wrapped; }

		Q_SCRIPTABLE QWidget* create(QWidget* x0=0) { return wrapped->create(x0); }
		Q_SCRIPTABLE QString id() const { return wrapped->id(); }
		Q_SCRIPTABLE Qt::DockWidgetArea defaultPosition() { return wrapped->defaultPosition(); }
		Q_SCRIPTABLE QList< QAction* > toolBarActions(QWidget* x0) const { return wrapped->toolBarActions(x0); }
		Q_SCRIPTABLE void viewCreated(Sublime::View* x0) { wrapped->viewCreated(x0); }
	private:
		KDevelop::IToolViewFactory* wrapped;
};

class KrossKDevelopIUiController : public QObject, public Kross::WrapperInterface
{
	Q_OBJECT
	Q_ENUMS(SwitchMode)
	Q_FLAGS(SwitchMode ThisWindow NewWindow)

	public:
		enum KrossSwitchMode { ThisWindow=KDevelop::IUiController::ThisWindow, NewWindow=KDevelop::IUiController::NewWindow };
		KrossKDevelopIUiController(KDevelop::IUiController* obj, QObject* parent=0) : QObject(parent), wrapped(obj)		{ setObjectName("KDevelop::IUiController"); }
		void* wrappedObject() const { return wrapped; }

		Q_SCRIPTABLE void switchToArea(const QString& x0, KDevelop::IUiController::SwitchMode x1) { wrapped->switchToArea(x0, x1); }
		Q_SCRIPTABLE void addToolView(const QString& x0, KDevelop::IToolViewFactory* x1) { wrapped->addToolView(x0, x1); }
		Q_SCRIPTABLE void removeToolView(KDevelop::IToolViewFactory* x0) { wrapped->removeToolView(x0); }
		Q_SCRIPTABLE KParts::MainWindow* activeMainWindow() { return wrapped->activeMainWindow(); }
		Q_SCRIPTABLE void registerStatus(QObject* x0) { wrapped->registerStatus(x0); }
		Q_SCRIPTABLE Sublime::Controller* controller() { return wrapped->controller(); }
	private:
		KDevelop::IUiController* wrapped;
};

bool krossiuicontroller_registerHandler(const QByteArray& name, Kross::MetaTypeHandler::FunctionPtr* handler)
{ Kross::Manager::self().registerMetaTypeHandler(name, handler); return false; }

namespace Handlers
{
QVariant _kDevelopIUiControllerHandler(void* type)
{
	if(!type) return QVariant();
	KDevelop::IUiController* t=static_cast<KDevelop::IUiController*>(type);
	Q_ASSERT(dynamic_cast<KDevelop::IUiController*>(t));
	return qVariantFromValue((QObject*) new KrossKDevelopIUiController(t, 0));
}
bool b_KDevelopIUiController1=krossiuicontroller_registerHandler("IUiController*", _kDevelopIUiControllerHandler);
bool b_KDevelopIUiController=krossiuicontroller_registerHandler("KDevelop::IUiController*", _kDevelopIUiControllerHandler);
QVariant kDevelopIUiControllerHandler(KDevelop::IUiController* type){ return _kDevelopIUiControllerHandler(type); }
QVariant kDevelopIUiControllerHandler(const KDevelop::IUiController* type) { return _kDevelopIUiControllerHandler((void*) type); }

QVariant _kDevelopIToolViewFactoryHandler(void* type)
{
	if(!type) return QVariant();
	KDevelop::IToolViewFactory* t=static_cast<KDevelop::IToolViewFactory*>(type);
	Q_ASSERT(dynamic_cast<KDevelop::IToolViewFactory*>(t));
	return qVariantFromValue((QObject*) new KrossKDevelopIToolViewFactory(t, 0));
}
bool b_KDevelopIToolViewFactory1=krossiuicontroller_registerHandler("IToolViewFactory*", _kDevelopIToolViewFactoryHandler);
bool b_KDevelopIToolViewFactory=krossiuicontroller_registerHandler("KDevelop::IToolViewFactory*", _kDevelopIToolViewFactoryHandler);
QVariant kDevelopIToolViewFactoryHandler(KDevelop::IToolViewFactory* type){ return _kDevelopIToolViewFactoryHandler(type); }
QVariant kDevelopIToolViewFactoryHandler(const KDevelop::IToolViewFactory* type) { return _kDevelopIToolViewFactoryHandler((void*) type); }

}
#include "krossiuicontroller.moc"