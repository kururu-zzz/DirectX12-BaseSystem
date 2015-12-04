#ifndef WORLDMATRIX___
#define WORLDMATRIX___

#include <vector>
#include <memory>
#include <DirectXMath.h>

struct ID3D12GraphicsCommandList;
struct ID3D12Resource;

namespace DX12
{
	class WorldMatrix{
	private:
		struct WorldBuffer;
		std::shared_ptr<WorldBuffer> world;
		std::shared_ptr<ID3D12Resource> resource;
		DirectX::XMFLOAT3 scale, translate;
		DirectX::XMFLOAT4X4 rotate;
	public:
		WorldMatrix();
		void SetConstantBuffer(ID3D12GraphicsCommandList* commandList);
		void SetTranslate(const DirectX::XMFLOAT3& translate);
		void SetScale(const DirectX::XMFLOAT3& scale);
		void SetRotate(const DirectX::XMFLOAT3& rotate);
		void SetRotate(const DirectX::XMFLOAT4X4& rotate);
	};
}
#endif