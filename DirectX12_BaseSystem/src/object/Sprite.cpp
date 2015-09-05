#include "Sprite.h"
#include "../core/d3d.h"
#include "TextureContainer.h"

using namespace DirectX;

namespace DX12
{
	inline XMFLOAT3 operator+ (const XMFLOAT3& f1, const XMFLOAT3& f2)
	{
		return XMFLOAT3(f1.x + f2.x, f1.y + f2.y, f1.z + f2.z);
	}

	inline XMFLOAT3 operator- (const XMFLOAT3& f1, const XMFLOAT3& f2)
	{
		return XMFLOAT3(f1.x - f2.x, f1.y - f2.y, f1.z - f2.z);
	}
	static D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	//static std::shared_ptr<ID3D12Resource> vertexResource;

	void SetVertexPos(std::array<DefaultVertex,4>& v,XMFLOAT3 pos,XMFLOAT2 base,XMFLOAT2 size,float degree)
	{
		v[0].pos = XMFLOAT3(pos.x, pos.y , pos.z);
		v[1].pos = XMFLOAT3(pos.x + size.x, pos.y, pos.z);
		v[2].pos = XMFLOAT3(pos.x, pos.y + size.y, pos.z);
		v[3].pos = XMFLOAT3(pos.x + size.x, pos.y + size.y, pos.z);
		std::for_each(v.begin(), v.end(), [base](DefaultVertex& p)
		{
			p.pos = p.pos - XMFLOAT3(base.x, base.y, 0.f);
		});
		XMFLOAT3X3 matRotate;
		XMStoreFloat3x3(&matRotate, XMMatrixIdentity());
		float rad = XMConvertToRadians(degree);
		matRotate._11 = matRotate._22 = cosf(rad);
		matRotate._12 = sinf(rad);
		matRotate._21 = -sinf(rad);
		std::for_each(v.begin(), v.end(), [base, matRotate](DefaultVertex& p)
		{
			XMStoreFloat3(&p.pos, XMVector3Transform(XMLoadFloat3(&p.pos), XMLoadFloat3x3(&matRotate)));
			p.pos = XMFLOAT3(p.pos.x + base.x, p.pos.y + base.y, p.pos.z);
		});
	}

	void UpdateVertex(DefaultVertex* vertex,ID3D12Resource* resource)
	{
		vertexBufferView = d3d::CreateVetexBufferView(resource, vertex, sizeof(DefaultVertex), 4);
	}

	Sprite::Sprite()
	{
		if (vertexResource == nullptr)
		{
			std::array<DefaultVertex, 4> vertex;
			vertexResource = d3d::CreateResource(sizeof(vertex));
		}
	}

	void Sprite::Init(const XMFLOAT3& pos, const XMFLOAT2& size, const XMFLOAT4& uv, const std::string& fileName)
	{
		this->ChangeImage(fileName);

		this->pos = pos;
		this->size = size;
		this->degree = 0.f;

		if (this->pos.z >= 1.0f)
			this->pos.z = 0.99999f;

		pV[0].color = pV[1].color = pV[2].color = pV[3].color = XMFLOAT4(1.f,1.f,1.f,1.f);

		this->base = XMFLOAT2(pos.x + size.x / 2.f,pos.y + size.y / 2.f);

		XMFLOAT3 faceEdgeA = pV[1].pos - pV[0].pos;
		XMFLOAT3 faceEdgeB = pV[2].pos - pV[0].pos;

		XMFLOAT3 faceNormal;
		XMStoreFloat3(&faceNormal, XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&faceEdgeA), XMLoadFloat3(&faceEdgeB))));

		pV[0].normal = pV[1].normal = pV[2].normal = pV[3].normal = faceNormal;

		pV[0].uv = XMFLOAT2(uv.x, uv.y);
		pV[1].uv = XMFLOAT2(uv.z, uv.y);
		pV[2].uv = XMFLOAT2(uv.x, uv.w);
		pV[3].uv = XMFLOAT2(uv.z, uv.w);
	}

	void Sprite::ChangeImage(const std::string& fileName)
	{
		this->fileName = fileName;
		DX12::CreateTexture(fileName);
	}

	void Sprite::SetUV(const XMFLOAT4& uv){
		pV[0].uv = XMFLOAT2(uv.x, uv.y);
		pV[1].uv = XMFLOAT2(uv.z, uv.y);
		pV[2].uv = XMFLOAT2(uv.x, uv.w);
		pV[3].uv = XMFLOAT2(uv.z, uv.w);
	}

	void Sprite::MovePos(const XMFLOAT3& spd)
	{
		pos = pos + spd;
	}

	void Sprite::SetBasePoint(const XMFLOAT2& base)
	{
		this->base = base;
	}

	void Sprite::SetRotate(const float degree)
	{
		this->degree = degree;
	}

	void Sprite::SetRotateFromCenter(const float degree)
	{
		this->degree = degree;
		this->base = XMFLOAT2(pos.x + size.x / 2, pos.y + size.y/2);
	}

	void Sprite::SetPos(const XMFLOAT3& pos){
		this->pos = pos;
		if (this->pos.z >= 1.0f)
			this->pos.z = 0.99999f;
	}

	//画像の中心からのサイズ変更
	void Sprite::SetSizeFromCenter(const XMFLOAT2& size){
		XMFLOAT3 difference = XMFLOAT3((size.x - this->size.x) / 2.f, (size.y - this->size.y) / 2.f, this->pos.z);
		this->pos = XMFLOAT3(this->pos.x - difference.x, this->pos.y - difference.y, this->pos.z);
		this->size = size;
	}

	void Sprite::SetAlpha(float alpha){
		std::for_each(pV.begin(), pV.end(), [alpha](DefaultVertex& v)
		{
			if (alpha <= 0.f)
			{
				v.color.w = 0.f;
			}
			else if (alpha >= 1.f)
			{
				v.color.w = 1.f;
			}
			else
			{
				v.color.w = alpha;
			}
		});
	}

	float Sprite::GetAlpha(){
		return pV[0].color.w;
	}

	void Sprite::SetSize(const XMFLOAT2& size){
		this->size = size;
	}

	void Sprite::Draw(ID3D12GraphicsCommandList* bundle){
		SetVertexPos(pV, pos, base, size, degree);
		UpdateVertex(pV.data(),vertexResource.get());
		DX12::SetTexture(bundle,fileName);
		
		UINT SizeTbl[] = { sizeof(DefaultVertex) };
		UINT OffsetTbl[] = { 0 };
		bundle->IASetVertexBuffers(0, 1, &vertexBufferView);
		bundle->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		bundle->DrawInstanced(4, 1, 0, 0);
	}
}
