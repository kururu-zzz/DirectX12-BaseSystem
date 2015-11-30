#ifndef CONSTANTBUFFERMANAGER___
#define CONSTANTBUFFERMANAGER___

#include "d3d.h"

namespace DX12
{
	namespace ConstantBuffer
	{
		enum class BufferSlot
		{
			World,
			Camera,
			Material,
			Light
		};
		template<typename T_n>
		std::shared_ptr<ID3D12Resource> CreateResource();

		template<typename T_n>
		void SetBuffer(ID3D12GraphicsCommandList* commandList, T_n& data, BufferSlot slotIndex);
		ID3D12DescriptorHeap* GetDescriptorHeap();
	}
}

#endif