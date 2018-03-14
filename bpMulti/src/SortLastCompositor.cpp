#include <bpMulti/SortLastCompositor.h>
#include "SortLastCompositingSpv.inc"

namespace bpMulti
{

void SortLastCompositor::initShaders()
{
	vertexShader.init(getDevice(), VK_SHADER_STAGE_VERTEX_BIT,
			  static_cast<uint32_t>(fullscreenQuad_vert_spv_len),
			  reinterpret_cast<const uint32_t*>(fullscreenQuad_vert_spv));
	fragmentShader.init(getDevice(), VK_SHADER_STAGE_FRAGMENT_BIT,
			    static_cast<uint32_t>(directTextureDepthTest_frag_spv_len),
			    reinterpret_cast<const uint32_t*>(directTextureDepthTest_frag_spv));
}

}