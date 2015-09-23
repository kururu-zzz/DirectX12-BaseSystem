#ifndef MODEL___
#define MODEL___

#include <unordered_map>
#include <map>
#include <memory>
#include <string>
#include <DirectXMath.h>

struct ID3D12GraphicsCommandList;

namespace fbx
{
	namespace element
	{
		class Mesh;
		struct Material;
	}
}

namespace DX12
{
	class Model{
	private:
		std::vector<std::shared_ptr<fbx::element::Mesh>> meshContainer;
		std::vector<std::shared_ptr<fbx::element::Material>> materialContainer;
		std::map<std::string, int> meshIndexTable;
		std::unordered_map<std::string, int> materialIndexTable;
		//std::vector<std::shared_ptr<fbx::Animation>> animContainer;
		//std::unordered_map<std::string, int> _animIndexTable;
		//std::string currentAnimationName;
		std::unordered_map<int, DirectX::XMFLOAT4X4> invBaseposeMatrixList;
		std::unordered_map<std::string, int> boneNodeNameList;
	public:
		Model(const std::string& modelname);
		void SetTranslate(const DirectX::XMFLOAT3& pos);
		void SetScale(const DirectX::XMFLOAT3& scale);
		void SetRotate(const DirectX::XMFLOAT3& angle);
		void SetRotate(const DirectX::XMFLOAT4X4& angle);

		//std::shared_ptr<fbx::Animation> AccessAnimation(const std::string& animationName);
		//void ApplyAnimation(const std::string& animationName);

		void Draw(ID3D12GraphicsCommandList* deviceContext);
		void Draw(ID3D12GraphicsCommandList* deviceContext,int instanceNum);
	};
}

#endif