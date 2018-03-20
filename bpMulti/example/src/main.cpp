#include "MainWindow.h"
#include <QGuiApplication>
#include <QLoggingCategory>

int main(int argc, char** argv)
{
	QGuiApplication app(argc, argv);

	QVulkanInstance instance;

#ifndef NDEBUG
	QLoggingCategory::setFilterRules(QStringLiteral("qt.vulkan=true"));
	instance.setLayers(QByteArrayList() << "VK_LAYER_LUNARG_standard_validation");
#endif
	if (!instance.create())
		qFatal("Failed to create instance.");

	MainWindow window(instance);
	window.resize(1024, 768);
	window.show();

	return app.exec();
}