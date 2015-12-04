#ifndef CONSTANTBUFFERMANAGER___
#define CONSTANTBUFFERMANAGER___

struct ID3D12GraphicsCommandList;
struct ID3D12Resource;
struct ID3D12DescriptorHeap;

namespace DX12
{
	namespace ConstantBuffer
	{
		enum class BufferSlot
		{
			World,
			Camera,
			Material,
			Light,
			ConstantBufferNum,
		};
		template<typename T_n>
		void SetBuffer(ID3D12GraphicsCommandList* commandList, T_n& data, BufferSlot slotIndex);
		ID3D12DescriptorHeap* GetDescriptorHeap();
	}
}

#endif