#include "Model.h"
#include "Mesh.h"
#include "Material.h"
#include "fbxLoader.h"
#include <d3d12.h>
#include <array>
#include <algorithm>


namespace DX12
{
	struct MaterialParam
	{
		DirectX::XMFLOAT4 ambient;
		DirectX::XMFLOAT4 diffuse;
		DirectX::XMFLOAT4 emissive;
		DirectX::XMFLOAT4 specular;
	};
	struct BoneMatrix
	{
		std::array<DirectX::XMFLOAT4X4,64> boneMat;
		BoneMatrix()
		{
			std::for_each(boneMat.begin(), boneMat.end(), [](DirectX::XMFLOAT4X4& mat)
			{
				DirectX::XMStoreFloat4x4(&mat, DirectX::XMMatrixIdentity());
			});
		}
	};
	MaterialParam GetMaterialParam(const fbx::element::Material* material)
	{
		MaterialParam param;
		param.ambient  = DirectX::XMFLOAT4(material->ambient.x, material->ambient.y, material->ambient.z, 1.f);
		param.diffuse  = DirectX::XMFLOAT4(material->diffuse.x, material->diffuse.y, material->diffuse.z, 1.f);
		param.emissive = DirectX::XMFLOAT4(material->emissive.x, material->emissive.y, material->emissive.z, 1.f);
		param.specular = DirectX::XMFLOAT4(material->specular.x, material->specular.y, material->specular.z, 1.f);
		return param;
	}
	void LoadMaterial(
		FbxScene* scene,
		std::vector<std::shared_ptr<fbx::element::Material>>* _materialContainer,
		std::unordered_map<std::string,int>* materialIndexTable
		)
	{
		auto materialCount = scene->GetMaterialCount();
		_materialContainer->reserve(materialCount);
		for (int i = 0; i < materialCount; ++i) {
			auto _material = fbx::element::LoadMaterial(scene->GetMaterial(i));
			_materialContainer->emplace_back(_material);
			materialIndexTable->emplace(_material->materialName, i);
		}
	}

	void LoadMesh(
		FbxScene* scene,
		std::vector<std::shared_ptr<fbx::element::Mesh>> *meshContainer,
		std::map<std::string,int>* meshIndexTable
		)
	{
		auto meshCount = scene->GetMemberCount<FbxMesh>();
		meshContainer->reserve(meshCount);
		for (int i = 0; i < meshCount; ++i) {
			meshContainer->emplace_back(
				std::make_shared<fbx::element::Mesh>(scene->GetMember<FbxMesh>(i))
				);
			auto meshName = scene->GetMember<FbxMesh>(i)->GetNode()->GetName();
			meshIndexTable->emplace(meshName,i);
		}
	}
	static DirectX::XMMATRIX convertFbx2XM(const FbxAMatrix & fbxAMtx) 
	{
		DirectX::XMFLOAT4X4 xmMtx;
		for (int i = 0; i < 4; ++i) for (int j = 0; j < 4; ++j) xmMtx.m[i][j] = static_cast<float>(fbxAMtx[i][j]);
		return DirectX::XMLoadFloat4x4(&xmMtx);
	}
	void CreateBaseposeMatrix(FbxScene* fbxScene, std::unordered_map<int, DirectX::XMFLOAT4X4>* invBaseposeMatrixList, std::unordered_map<std::string, int>* boneNodeNameList)
	{
		auto nodeCount = fbxScene->GetMemberCount<FbxNode>();
		for (int i = 0; i < nodeCount; ++i)
		{
			auto node = fbxScene->GetNode(i);
			if (node->GetNodeAttribute() != nullptr && node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
			{
				boneNodeNameList->emplace(node->GetName(), i);
				DirectX::XMFLOAT4X4 matrix;

				auto baseposeMatrix = node->EvaluateGlobalTransform().Inverse();
				DirectX::XMStoreFloat4x4(&matrix, convertFbx2XM(baseposeMatrix));
				invBaseposeMatrixList->emplace(i, matrix);
			}
		}
	}
	/*void LoadAnimation(
		std::shared_ptr<FbxScene>* scene,
		std::vector<std::shared_ptr<fbx::Animation>>* animContainer,
		std::unordered_map<std::string,int>* animIndexTable,
		std::unordered_map<int, DirectX::XMFLOAT4X4>* invBaseposeMatrixList, 
		std::unordered_map<std::string, int>* boneNodeNameList,
		const std::string& modelName
		)
	{
		if (animContainers.find(modelName) == animContainers.end())
		{
			auto animCount = (*scene)->GetMemberCount<FbxAnimStack>();
			auto animLayerCount = (*scene)->GetMemberCount<FbxAnimLayer>();
			if (animCount == 0)
				return;
			else
			{
				CreateBaseposeMatrix(scene->get(), invBaseposeMatrixList, boneNodeNameList);
			}
			std::vector<std::shared_ptr<fbx::Animation>> anims;
			std::unordered_map<std::string,int> animIndexes;
			anims.reserve(animCount);
			for (int i = 0; i < animCount; ++i)
			{
				auto animationName = (*scene)->GetMember<FbxAnimStack>(i)->GetName();
				animIndexes.emplace(std::pair<std::string, int>(animationName, i));
				anims.emplace_back(std::make_shared<fbx::Animation>(scene, invBaseposeMatrixList, boneNodeNameList));
				auto& it = anims.back();
				it->Init(animationName, i);
			}
			animContainers.emplace(modelName, anims);
			animIndexTables.emplace(modelName, animIndexes);
		}
		*animIndexTable = animIndexTables.at(modelName);
		*animContainer = animContainers.at(modelName);
	}
	*/
	Model::Model(const std::string& modelName){
		auto scene = fbx::resource::LoadModel(modelName);

		LoadMaterial(scene.get(), &materialContainer,&materialIndexTable);///< シーンに含まれるマテリアルの解析

		LoadMesh(scene.get(), &meshContainer,&meshIndexTable);///< シーンに含まれるメッシュの解析

		//LoadAnimation(&_scene, &_animContainer, &_animIndexTable,&_invBaseposeMatrixList,&_boneNodeNameList,modelName);

		//if (animContainer.size() != 0)
		//currentAnimationName = animIndexTable.begin()->first;
	}

	void Model::SetTranslate(const DirectX::XMFLOAT3& translate){
		for (auto& mesh : meshContainer){
			mesh->SetTranslate(translate);
		}
	}

	void Model::SetScale(const DirectX::XMFLOAT3& scale)
	{
		for (auto& mesh : meshContainer){
			mesh->SetScale(scale);
		}
	}

	void Model::SetRotate(const DirectX::XMFLOAT3& rotate){
		for (auto& mesh : meshContainer)
			mesh->SetRotate(rotate);
	}

	void Model::SetRotate(const DirectX::XMFLOAT4X4& rotate) {
		for (auto& mesh : meshContainer)
			mesh->SetRotate(rotate);
	}

	/*std::shared_ptr<fbx::Animation> Model::AccessAnimation(const std::string& animationName)
	{
		auto animIndex = _animIndexTable.at(animationName);
		return AccessAnimation(animIndex);
	}

	std::shared_ptr<fbx::Animation> Model::AccessAnimation(int animIndex)
	{
		if (animIndex < _animContainer.size())
			return _animContainer.at(animIndex);
		return nullptr;
	}

	void Model::ApplyAnimation(const std::string& animationName)
	{
		_currentAnimationName = animationName;
	}*/

	void Model::Draw(ID3D12GraphicsCommandList* commandList)
	{
		/*std::shared_ptr<fbx::Animation> currentAnim;
		if (_animContainer.size() != 0)
			currentAnim = _animContainer.at(_animIndexTable.at(_currentAnimationName));
			*/
		for (auto& meshInfo : meshIndexTable)
		{
			auto meshId = meshInfo.second;
			auto& mesh = meshContainer.at(meshId);
			/*if (currentAnim != nullptr)
			{
				auto boneMat = currentAnim->GetBoneMatrix(meshId);
				static BoneMatrix out;
				std::copy(boneMat.begin(), boneMat.end(), out.boneMat.begin());
				DX11::ConstantBuffer::SetBuffer(deviceContext, out, ConstantBuffer::cBufferSlot::Bone);
			}*/
			auto& material = materialContainer.at(materialIndexTable.at(mesh->GetMaterialName()));
			//DX11::ConstantBuffer::SetBuffer(deviceContext, GetMaterialParam(material.get()), DX11::ConstantBuffer::cBufferSlot::Material);
			//SetTexture(deviceContext,material->diffuseTextureName);
			mesh->Draw(commandList);
		}
	}

	void Model::Draw(ID3D12GraphicsCommandList* commandList,int instanceNum)
	{
		for (auto mesh : meshContainer)
		{
			auto material = materialContainer.at(materialIndexTable.at(mesh->GetMaterialName()));
			//std::vector<XMFLOAT4X4> boneMat;
			//currentAnim->GetBoneMatrix(,,&boneMat);
			//DX11::ConstantBuffer::SetBuffer(deviceContext, GetMaterialParam(material.get()), DX11::ConstantBuffer::cBufferSlot::Material);
			//SetTexture(deviceContext,material->diffuseTextureName);
			mesh->Draw(commandList,instanceNum);
		}
	}
}