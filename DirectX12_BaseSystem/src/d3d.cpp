#include "d3d.h"
#include <PlatformHelpers.h>

namespace d3d
{
	namespace resource
	{
		// !! 
		void CreateDevice(std::shared_ptr<ID3D12Device> device)
		{
			ID3D12Device* _device;
			DirectX::ThrowIfFailed(
				D3D12CreateDevice(
					nullptr,
					D3D_FEATURE_LEVEL_11_0,
					IID_PPV_ARGS(&_device)
					));
		}
	}
}