#include "Mesh.h"
#include "Vertex.h"
#include "WorldMatrix.h"
#include "../../core/d3d.h"

namespace fbx
{
	namespace element
	{

		Mesh::Mesh(const FbxMesh* fbxmesh)
		{
			assert(fbxmesh != nullptr);

			indexNum = fbxmesh->GetPolygonCount() * 3;

			auto node = fbxmesh->GetNode();

			meshName = std::string(node->GetName());
			materialName = std::string(node->GetMaterial(0)->GetName());

			this->vertexBufferView = std::make_shared<D3D12_VERTEX_BUFFER_VIEW>();
			this->indexBufferView = std::make_shared<D3D12_INDEX_BUFFER_VIEW>();
			
			CreateVertex(fbxmesh,vertexBufferView.get(),indexBufferView.get(),&vertexResource,&indexResource);

			world = std::make_shared<DX12::WorldMatrix>();
		}

		void Mesh::SetTranslate(const DirectX::XMFLOAT3& translate)
		{
			world->SetTranslate(translate);
		}

		void Mesh::SetScale(const DirectX::XMFLOAT3& scale)
		{
			world->SetScale(scale);
		}

		void Mesh::SetRotate(const DirectX::XMFLOAT3& rotate)
		{
			world->SetRotate(rotate);
		}

		void Mesh::SetRotate(const DirectX::XMFLOAT4X4& rotate)
		{
			world->SetRotate(rotate);
		}

		void Mesh::Draw(ID3D12GraphicsCommandList* commandList,int instanceNum)
		{
			world->SetConstantBuffer(commandList);

			commandList->IASetVertexBuffers(0, 1, vertexBufferView.get());
			commandList->IASetIndexBuffer(indexBufferView.get());

			commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			commandList->DrawIndexedInstanced(indexNum, instanceNum, 0, 0, 0);
		}
	}
}