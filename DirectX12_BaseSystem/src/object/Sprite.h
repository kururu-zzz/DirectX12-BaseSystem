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

		DirectX::XMFLOAT3X3 mtxRotate;
		float degree;

		std::array<DefaultVertex, 4> pV;
		std::string fileName;
	public:
		Sprite();
		void Init(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT2& size, const DirectX::XMFLOAT4& uv, const std::string& fileName);
		void Draw(ID3D12GraphicsCommandList* commandList);
		/*---------------*/

		void SetUV(const DirectX::XMFLOAT4& uv);
		void SetPos(const DirectX::XMFLOAT3& pos);
		void MovePos(const DirectX::XMFLOAT3& spd);
		void SetSize(const DirectX::XMFLOAT2& size);
		void SetSizeFromCenter(const DirectX::XMFLOAT2& size);
		void SetBasePoint(const DirectX::XMFLOAT2& base);
		void SetRotate(const float degree);
		void SetRotateFromCenter(const float degree);
		void SetAlpha(float alpha);

		void ChangeImage(const std::string& fileName);


		const DirectX::XMFLOAT3& GetPos(){ return pos; }
		const DirectX::XMFLOAT2& GetSize() { return size; }
		const DirectX::XMFLOAT2& GetBasePoint() { return base; }
		float GetAlpha();
	};
}
#endif