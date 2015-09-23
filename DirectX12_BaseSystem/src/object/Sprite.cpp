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

	void SetVertexPos(std::array<DefaultVertex,4>& v,XMFLOAT3 pos,XMFLOAT2 base,XMFLOAT2 size,float angle)
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
		float rad = XMConvertToRadians(angle);
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
		vertexBufferView = d3d::CreateVertexBufferView(resource, vertex, sizeof(DefaultVertex), 4);
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
		this->angle = 0.f;

		if (this->pos.z >= 1.0f)
			this->pos.z = 0.99999f;

		vertex[0].color = vertex[1].color = vertex[2].color = vertex[3].color = XMFLOAT4(1.f,1.f,1.f,1.f);

		this->base = XMFLOAT2(pos.x + size.x / 2.f,pos.y + size.y / 2.f);

		XMFLOAT3 faceEdgeA = vertex[1].pos - vertex[0].pos;
		XMFLOAT3 faceEdgeB = vertex[2].pos - vertex[0].pos;

		XMFLOAT3 faceNormal;
		XMStoreFloat3(&faceNormal, XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&faceEdgeA), XMLoadFloat3(&faceEdgeB))));

		vertex[0].normal = vertex[1].normal = vertex[2].normal = vertex[3].normal = faceNormal;

		vertex[0].uv = XMFLOAT2(uv.x, uv.y);
		vertex[1].uv = XMFLOAT2(uv.z, uv.y);
		vertex[2].uv = XMFLOAT2(uv.x, uv.w);
		vertex[3].uv = XMFLOAT2(uv.z, uv.w);
	}

	void Sprite::ChangeImage(const std::string& fileName)
	{
		this->fileName = fileName;
		DX12::CreateTexture(fileName);
	}

	void Sprite::SetUV(const XMFLOAT4& uv){
		vertex[0].uv = XMFLOAT2(uv.x, uv.y);
		vertex[1].uv = XMFLOAT2(uv.z, uv.y);
		vertex[2].uv = XMFLOAT2(uv.x, uv.w);
		vertex[3].uv = XMFLOAT2(uv.z, uv.w);
	}

	void Sprite::SetBasePoint(const XMFLOAT2& base)
	{
		this->base = base;
	}

	void Sprite::SetRotate(const float angle)
	{
		this->angle = angle;
	}

	void Sprite::SetRotateFromCenter(const float angle)
	{
		this->angle = angle;
		this->base = XMFLOAT2(pos.x + size.x / 2, pos.y + size.y/2);
	}

	void Sprite::SetPos(const XMFLOAT3& pos){
		this->pos = pos;
		if (this->pos.z >= 1.0f)
			this->pos.z = 0.99999f;
	}

	void Sprite::SetSize(const XMFLOAT2& size) {
		this->size = size;
	}

	void Sprite::SetSizeFromCenter(const XMFLOAT2& size){
		XMFLOAT3 difference = XMFLOAT3((size.x - this->size.x) / 2.f, (size.y - this->size.y) / 2.f, this->pos.z);
		this->pos = XMFLOAT3(this->pos.x - difference.x, this->pos.y - difference.y, this->pos.z);
		this->size = size;
	}

	void Sprite::SetAlpha(float alpha){
		alpha = (alpha <= 0.f) ? alpha = 0.f :
				(alpha >= 1.f) ? alpha = 1.f :
				alpha;
		std::for_each(vertex.begin(), vertex.end(), [alpha](DefaultVertex& vertex)
		{
			vertex.color.w = alpha;
		});
	}

	void Sprite::Draw(ID3D12GraphicsCommandList* bundle){
		SetVertexPos(vertex, pos, base, size, angle);
		UpdateVertex(vertex.data(),vertexResource.get());
		DX12::SetTexture(bundle,fileName);
		
		UINT SizeTbl[] = { sizeof(DefaultVertex) };
		UINT OffsetTbl[] = { 0 };
		bundle->IASetVertexBuffers(0, 1, &vertexBufferView);
		bundle->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		bundle->DrawInstanced(4, 1, 0, 0);
	}
}
