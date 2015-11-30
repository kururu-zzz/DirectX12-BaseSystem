#ifndef MESH___
#define MESH___

#include <vector>
#include <memory>
#include <DirectXMath.h>
#include <fbxsdk.h>

struct ID3D12GraphicsCommandList;
struct D3D12_VERTEX_BUFFER_VIEW;
struct D3D12_INDEX_BUFFER_VIEW;
struct ID3D12Resource;
namespace DX12
{
	class WorldMatrix;
}

namespace fbx
{
	namespace element
	{
		class Mesh{
		public:
			Mesh(const FbxMesh* fbxMesh);
		private:
			std::string meshName;
			std::string materialName;
			int materialIndex;
			int indexNum;
			std::shared_ptr<D3D12_VERTEX_BUFFER_VIEW> vertexBufferView;
			std::shared_ptr<D3D12_INDEX_BUFFER_VIEW> indexBufferView;
			std::shared_ptr<ID3D12Resource> vertexResource;
			std::shared_ptr<ID3D12Resource> indexResource;
			std::shared_ptr<DX12::WorldMatrix> world;
		public:
			void SetTranslate(const DirectX::XMFLOAT3& translate);
			void SetScale(const DirectX::XMFLOAT3& scale);
			void SetRotate(const DirectX::XMFLOAT3& rotate);
			void SetRotate(const DirectX::XMFLOAT4X4& rotate);

			const std::string& GetMaterialName() const{ return materialName; }

			void Draw(ID3D12GraphicsCommandList* commandList, int instanceNum = 1);
		};
	}
}
#endif