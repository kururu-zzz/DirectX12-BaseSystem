#ifndef CAMERA___
#define CAMERA___

#include <DirectXMath.h>
#include <unordered_map>
#include <memory>

struct ID3D12GraphicsCommandList;

namespace DX12
{
	class Camera{
	public:
		/*constant buffer*/
		struct CameraBuffer;
	private:
		std::shared_ptr<CameraBuffer> camera;

		/*parameter of view matrix*/
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 focus;
		DirectX::XMFLOAT3 upDirection;

		/*parameter of projection matrix*/
		float cameraAngle = 0.f;
		float screenAspect = 0.f;

		float nearZ = 0.f;
		float farZ = 0.f;
	public:
		Camera();
		void Init(
			const DirectX::XMFLOAT3& pos, 
			const DirectX::XMFLOAT3& focus, 
			const DirectX::XMFLOAT3& upDirection = DirectX::XMFLOAT3(0.f,1.f,0.f),
			const float& cameraAngle = 45.f,
			const DirectX::XMFLOAT2& screenSize = DirectX::XMFLOAT2(4.f,3.f),
			const float& nearZ = 1.f,
			const float& farZ = 1000.f);

		/*setter function for parameter of view matrix*/
		void SetPos(const DirectX::XMFLOAT3& pos);
		void SetFocus(const DirectX::XMFLOAT3& focus);
		void SetUpDirection(const DirectX::XMFLOAT3& upDirection);

		/**setter function parameter of projection matrix*/
		void SetCameraAngle(const float& cameraAngle);
		void SetScreenAspect(const float& screenAspect);
		void SetScreenAspect(const DirectX::XMFLOAT2& screenSize);
		void SetNearZ(const float& nearZ);
		void SetFarZ(const float& farZ);

		//apply member variable to View matrix
		void ApplyLookAtLH();
		void ApplyLookAtRH();

		//apply member variable to Projection matrix
		void ApplyPerspectiveFovLH();
		void ApplyPerspectiveFovRH();

		//set constant buffer to shader
		void SetConstantBuffer(ID3D12GraphicsCommandList* commandList);
	};
}

#endif