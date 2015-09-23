#ifndef VERTEX___
#define VERTEX___

#include <memory>
#include <string>
#include <array>
#include <DirectXMath.h>
#include <fbxsdk.h>

struct D3D12_VERTEX_BUFFER_VIEW;
struct D3D12_INDEX_BUFFER_VIEW;

namespace fbx
{
	namespace element
	{
		struct Vertex {
			DirectX::XMFLOAT3 pos;
			uint32_t color;
			DirectX::XMFLOAT3 normal;
			DirectX::XMFLOAT2 texel;
			std::array<uint8_t, 4> boneIndex;
			DirectX::XMFLOAT4 boneWeight;
			bool operator < (const Vertex& v) const {
				return std::memcmp(this, &v, sizeof(Vertex)) < 0;
			}
			Vertex()
			{
				pos = DirectX::XMFLOAT3(0.f, 0.f, 0.f);
				color = 0;
				normal = DirectX::XMFLOAT3(0.f, 0.f, 0.f);
				texel = DirectX::XMFLOAT2(0.f, 0.f);
				std::for_each(boneIndex.begin(), boneIndex.end(), [](uint8_t i)
				{
					i = 0;
				});
				boneWeight = DirectX::XMFLOAT4(0.f, 0.f, 0.f, 0.f);
			}
		};
		void CreateVertex(
			const FbxMesh* fbxmesh,
			D3D12_VERTEX_BUFFER_VIEW* vertexBufferView,
			D3D12_INDEX_BUFFER_VIEW* indexBufferView
			);
	}
}
#endif