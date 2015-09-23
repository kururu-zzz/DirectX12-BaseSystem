#ifndef _SPRITE
#define _SPRITE

#include <array>
#include <string>
#include <memory>
#include <DirectXMath.h>

struct ID3D12GraphicsCommandList;
struct ID3D12Resource;

namespace DX12
{
	struct DefaultVertex 
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 uv;
	};
	class Sprite{
		std::shared_ptr<ID3D12Resource> vertexResource;
	private:
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 size;
		DirectX::XMFLOAT2 base;
		float angle;

		std::array<DefaultVertex, 4> vertex;
		std::string fileName;
	public:
		Sprite();
		void Init(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT2& size, const DirectX::XMFLOAT4& uv, const std::string& fileName);
		void Draw(ID3D12GraphicsCommandList* commandList);

		void SetUV(const DirectX::XMFLOAT4& uv);
		void SetPos(const DirectX::XMFLOAT3& pos);
		void SetSize(const DirectX::XMFLOAT2& size);
		void SetSizeFromCenter(const DirectX::XMFLOAT2& size);
		void SetBasePoint(const DirectX::XMFLOAT2& base);
		void SetRotate(const float angle);
		void SetRotateFromCenter(const float angle);
		void SetAlpha(float alpha);

		void ChangeImage(const std::string& fileName);
	};
}
#endif