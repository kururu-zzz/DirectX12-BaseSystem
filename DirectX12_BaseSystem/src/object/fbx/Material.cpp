#include <d3d12.h>
#include "Material.h"
#include "../TextureContainer.h"

namespace fbx
{
	namespace element
	{
		namespace Load
		{
			std::string getTexture(const FbxProperty & property) {
				int fileTextureCount = property.GetSrcObjectCount<FbxFileTexture>();
				if (0 >= fileTextureCount) return "";

				for (int j = 0; fileTextureCount > j; j++) {
					auto texture = property.GetSrcObject<FbxFileTexture>(j);
					if (!texture) continue;

					return texture->GetRelativeFileName();
				}
				return "";
			}

			DirectX::XMFLOAT3 getProperty(
				const FbxSurfaceMaterial * material,
				const char * pPropertyName,
				const char * pFactorPropertyName,
				DirectX::XMFLOAT3 default = DirectX::XMFLOAT3(0, 0, 0)
				) {
				auto property = material->FindProperty(pPropertyName);
				auto factor = material->FindProperty(pFactorPropertyName);
				if (property.IsValid() && factor.IsValid()) {
					auto f = factor.Get<FbxDouble>();
					auto _ = property.Get<FbxDouble3>();
					return DirectX::XMFLOAT3(static_cast<float>(_[0] * f), static_cast<float>(_[1] * f), static_cast<float>(_[2] * f));
				}
				return default;
			}
		}

		std::shared_ptr<Material> LoadMaterial(FbxSurfaceMaterial* material)
		{
			Material _material;
			_material.materialName = material->GetName();

			_material.emissive = Load::getProperty(material, FbxSurfaceMaterial::sEmissive, FbxSurfaceMaterial::sEmissiveFactor);
			_material.ambient = Load::getProperty(material, FbxSurfaceMaterial::sAmbient, FbxSurfaceMaterial::sAmbientFactor);
			_material.diffuse = Load::getProperty(material, FbxSurfaceMaterial::sDiffuse, FbxSurfaceMaterial::sDiffuseFactor, DirectX::XMFLOAT3(1, 1, 1));
			_material.specular = Load::getProperty(material, FbxSurfaceMaterial::sSpecular, FbxSurfaceMaterial::sSpecularFactor);

			auto transparency = material->FindProperty(FbxSurfaceMaterial::sTransparencyFactor);
			_material.transparency = (transparency.IsValid()) ? transparency.Get<FbxDouble>() : 1;
			auto shininess = material->FindProperty(FbxSurfaceMaterial::sShininess);
			_material.shininess = (shininess.IsValid()) ? shininess.Get<FbxDouble>() : 1;

			// FbxSurfaceMaterial::sNormalMapが法線マップだけどメタセコイヤはsBumpのバンプマップしかないっぽい
			_material.diffuseTextureName = Load::getTexture(material->FindProperty(FbxSurfaceMaterial::sDiffuse));
			_material.normalTextureName = Load::getTexture(material->FindProperty(FbxSurfaceMaterial::sBump));
			_material.reflectionMapTextureName = Load::getTexture(material->FindProperty(FbxSurfaceMaterial::sReflection));
			_material.specularTextureName = Load::getTexture(material->FindProperty(FbxSurfaceMaterial::sSpecular));
			//_material.falloffTextureName = detail::getTexture(material->FindProperty(FbxSurfaceMaterial::));

			/*-----修正-----*/
			DX12::CreateTexture(_material.diffuseTextureName);
			//CreateTexture(_material.normalTextureName);
			//CreateTexture(_material.reflectionMapTextureName);
			//CreateTexture(_material.specularTextureName);

			// CGFXのみ対応
			auto implementation = GetImplementation(material, FBXSDK_IMPLEMENTATION_HLSL);
			if (implementation != nullptr) {
				auto rootTable = implementation->GetRootTable();
				auto entryCount = rootTable->GetEntryCount();

				for (unsigned int i = 0; i < entryCount; ++i) {
					auto entry = rootTable->GetEntry(i);

					auto fbxProperty = material->FindPropertyHierarchical(entry.GetSource());
					if (!fbxProperty.IsValid()) {
						fbxProperty = material->RootProperty.FindHierarchical(entry.GetSource());
					}

					auto textureCount = fbxProperty.GetSrcObjectCount<FbxTexture>();
					if (textureCount > 0) {
						std::string src = entry.GetSource();

						for (int j = 0; j < fbxProperty.GetSrcObjectCount<FbxFileTexture>(); ++j) {
							auto tex = fbxProperty.GetSrcObject<FbxFileTexture>(j);
							std::string texName = tex->GetFileName();
							texName = texName.substr(texName.find_last_of('/') + 1);

							if (src == "Maya|DiffuseTexture") {
								_material.diffuseTextureName = texName;
							}
							else if (src == "Maya|NormalTexture") {
								_material.normalTextureName = texName;
							}
							else if (src == "Maya|SpecularTexture") {
								_material.specularTextureName = texName;
							}
							else if (src == "Maya|FalloffTexture") {
								_material.falloffTextureName = texName;
							}
							else if (src == "Maya|ReflectionMapTexture") {
								_material.reflectionMapTextureName = texName;
							}
						}
					}
				}
				return std::make_shared<Material>(_material);
			}
			return std::make_shared<Material>(_material);
		}
	}
}