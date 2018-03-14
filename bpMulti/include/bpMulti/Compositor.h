#ifndef BP_COMPOSITOR_H
#define BP_COMPOSITOR_H

#include <bp/Renderer.h>
#include "RenderStep.h"
#include "DeviceToHostStep.h"
#include "HostCopyStep.h"
#include "HostToDeviceStep.h"

namespace bpMulti
{

class Compositor : public bp::Renderer
{
};

}

#endif