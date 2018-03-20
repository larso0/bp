#include "MainWindow.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <stdexcept>

using namespace bp;
using namespace bpScene;
using namespace std;

void MainWindow::initRenderResources(uint32_t width, uint32_t height)
{
	DeviceRequirements requirements;
	requirements.queues = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT;
	requirements.features.samplerAnisotropy = VK_TRUE;

	auto physicalDevices = bpQt::queryDevices(*vulkanInstance(), requirements);
	VkPhysicalDevice selected = physicalDevices[0];
	if (selected == device && physicalDevices.size() > 1) selected = physicalDevices[1];

	secondaryDevice.init(selected, requirements);

	primaryRenderer.setCamera(camera);
	secondaryRenderer.setCamera(camera);

	compositor.init({{&device, &primaryRenderer}, {&secondaryDevice, &secondaryRenderer}},
			swapchain.getFormat(), width, height);
	depthAttachment.setClearValue({1.f, 0.f});
	depthAttachment.init(device, VK_FORMAT_D16_UNORM,
			     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, width, height);

	framebuffer.setAttachment(compositor.getColorAttachmentSlot(), swapchain);
	framebuffer.setAttachment(compositor.getDepthAttachmentSlot(), depthAttachment);
	framebuffer.init(compositor.getRenderPass(), width, height);

	cameraController.setCameraNode(cameraNode);

	Mesh mesh;
	mesh.loadObj("teapot.obj");
	unsigned primaryMeshId = primaryRenderer.addMesh(mesh);
	unsigned secondaryMeshId = secondaryRenderer.addMesh(mesh);

	camera.setPerspectiveProjection(glm::radians(60.f),
					static_cast<float>(width) / static_cast<float>(height),
					0.1f, 1000.f);
	cameraNode.translate(0.f, 0.f, 2.f);
	cameraNode.update();
	camera.update();
	node1.translate(-0.5f, -0.5f, 0.f);
	node2.translate(0.5f, -0.5f, 0.f);
	node1.update();
	node2.update();

	primaryRenderer.addEntity(primaryMeshId, node1);
	secondaryRenderer.addEntity(secondaryMeshId, node2);
}

void MainWindow::resizeRenderResources(uint32_t width, uint32_t height)
{
	compositor.resize(width, height);
	depthAttachment.resize(width, height);
	framebuffer.resize(width, height);
	camera.setPerspectiveProjection(glm::radians(60.f),
					static_cast<float>(width) / static_cast<float>(height),
					0.1f, 1000.f);
}

void MainWindow::specifyDeviceRequirements(DeviceRequirements& requirements)
{
	requirements.features.samplerAnisotropy = VK_TRUE;
}

void MainWindow::render(VkCommandBuffer cmdBuffer)
{
	compositor.render(framebuffer, cmdBuffer);
}

void MainWindow::update(double frameDeltaTime)
{
	cameraController.update(static_cast<float>(frameDeltaTime));
	camera.update();
	node1.rotate(static_cast<float>(frameDeltaTime), {0.f, 1.f, 0.f});
	node2.rotate(static_cast<float>(frameDeltaTime), {0.f, 1.f, 0.f});
	node1.update();
	node2.update();
}

void MainWindow::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		mouseButton = true;
		previousMousePos = event->globalPos();
	}
	QWindow::mousePressEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		mouseButton = false;
		QPoint delta = event->globalPos() - previousMousePos;
		cameraController.motion(static_cast<float>(delta.x()),
					static_cast<float>(delta.y()));
		previousMousePos = event->globalPos();
	}
	QWindow::mouseReleaseEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
	if (mouseButton)
	{
		QPoint delta = event->globalPos() - previousMousePos;
		cameraController.motion(static_cast<float>(delta.x()),
					static_cast<float>(delta.y()));
		previousMousePos = event->globalPos();
	}
	QWindow::mouseMoveEvent(event);
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
	switch (event->key())
	{
	case Qt::Key_Q: cameraController.setDownPressed(true); break;
	case Qt::Key_W: cameraController.setForwardPressed(true); break;
	case Qt::Key_E: cameraController.setUpPressed(true); break;
	case Qt::Key_A: cameraController.setLeftPressed(true); break;
	case Qt::Key_S: cameraController.setBackwardPressed(true); break;
	case Qt::Key_D: cameraController.setRightPressed(true); break;
	default: break;
	}
	QWindow::keyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent* event)
{
	switch (event->key())
	{
	case Qt::Key_Q: cameraController.setDownPressed(false); break;
	case Qt::Key_W: cameraController.setForwardPressed(false); break;
	case Qt::Key_E: cameraController.setUpPressed(false); break;
	case Qt::Key_A: cameraController.setLeftPressed(false); break;
	case Qt::Key_S: cameraController.setBackwardPressed(false); break;
	case Qt::Key_D: cameraController.setRightPressed(false); break;
	default: break;
	}
	QWindow::keyReleaseEvent(event);
}
