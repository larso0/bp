#ifndef BPMULTITEST_MAINWINDOW_H
#define BPMULTITEST_MAINWINDOW_H

#include <bpQt/Window.h>
#include <bp/Framebuffer.h>
#include <bp/Texture.h>
#include <bpMulti/SortLastCompositor.h>
#include "MainRenderer.h"
#include "CameraController.h"

class MainWindow : public bpQt::Window
{
public:
	MainWindow(QVulkanInstance& instance) :
		bpQt::Window{instance},
		camera{&cameraNode},
		mouseButton{false}
	{
		setContinuousRendering(true);
		setVSync(true);
		swapchain.setClearValue({0.1f, 0.1f, 0.2f, 1.f});
	}

private:
	bp::Device secondaryDevice;
	bpMulti::SortLastCompositor compositor;
	MainRenderer primaryRenderer, secondaryRenderer;
	bp::Texture depthAttachment;
	bp::Framebuffer framebuffer;

	bpScene::Node node1, node2, cameraNode;
	bpScene::Camera camera;
	CameraController cameraController;
	QPoint previousMousePos;
	bool mouseButton;

	void initRenderResources(uint32_t width, uint32_t height) override;
	void resizeRenderResources(uint32_t width, uint32_t height) override;
	void specifyDeviceRequirements(bp::DeviceRequirements& requirements) override;
	void render(VkCommandBuffer cmdBuffer) override;
	void update(double frameDeltaTime) override;

	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

};

#endif
