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

			D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
			D3D12_INDEX_BUFFER_VIEW indexBufferView;
			CreateVertex(fbxmesh,&vertexBufferView,&indexBufferView);

			this->vertexBufferView = std::shared_ptr<D3D12_VERTEX_BUFFER_VIEW>(&vertexBufferView);
			this->indexBufferView = std::shared_ptr<D3D12_INDEX_BUFFER_VIEW>(&indexBufferView);
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

		void Mesh::Draw(ID3D12GraphicsCommandList* commandList,int instanceNum){
			// VB‚ÌƒZƒbƒg
			UINT sizeTbl[] = { sizeof(Vertex) };
			UINT offsetTbl[] = { 0 };
			commandList->IASetVertexBuffers(0, 1, vertexBufferView.get());
			commandList->IASetIndexBuffer(indexBufferView.get());

			commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);



			commandList->DrawIndexedInstanced(indexNum, instanceNum, 0, 0, 0);
		}
	}
}