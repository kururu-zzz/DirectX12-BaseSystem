#ifndef __MATERIAL__
#define __MATERIAL__

#include <fbxsdk.h>
#include <string>
#include <memory>
#include <DirectXMath.h>

namespace fbx
{
	namespace element
	{
		struct Material
		{
			std::string materialName;

			DirectX::XMFLOAT3 ambient;
			DirectX::XMFLOAT3 diffuse;
			DirectX::XMFLOAT3 emissive;
			double transparency;

			DirectX::XMFLOAT3 specular;
			double shininess;

			std::string diffuseTextureName;
			std::string normalTextureName;
			std::string specularTextureName;
			std::string falloffTextureName;
			std::string reflectionMapTextureName;
		};

		std::shared_ptr<Material> LoadMaterial(FbxSurfaceMaterial* material);
	}
}

#endif