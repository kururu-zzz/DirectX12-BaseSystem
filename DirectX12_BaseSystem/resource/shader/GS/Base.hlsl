cbuffer mtxWorld : register(b0)
{
	matrix	mtxWorld;
};
struct Camera
{
	matrix mtxView;
	matrix mtxProjection;
	float4 eyePos;
};
cbuffer CameraBuffer : register(b1)
{
	Camera camera;
};
struct OutputVS
{
	float4 pos			: SV_POSITION;
	float4 color		: COLOR0;
	float4 eye			: COLOR1;
	float3 normal       : NORMAL;
	float2 texel        : TEXCOORD0;
};

// ジオメトリシェーダーの出力パラメータ
struct OutputGS
{
	float4 pos    : SV_POSITION;
	float4 color  : COLOR0;
	float3 eye	  : COLOR1;
	float3 normal : NORMAL;
	float2 texel  : TEXCOORD0;
};

// ジオメトリ シェーダー オブジェクト (DirectX HLSL)

[maxvertexcount(3)]
void RenderGS(
	triangle OutputVS In[3],
	inout TriangleStream<OutputGS> TriStream
	)
{
	OutputGS Out;

	int i;
	matrix matVP = mul(camera.mtxView, camera.mtxProjection);
	matrix matWVP = mul(mtxWorld, matVP);
	for (i = 0; i < 3; i++)
	{
		Out.pos = mul(In[i].pos, matWVP);

		Out.normal = In[i].normal;

		Out.texel = In[i].texel;

		Out.eye = normalize(camera.eyePos.xyz - In[i].pos.xyz);

		Out.color = In[i].color;

		TriStream.Append(Out);
	}
	TriStream.RestartStrip();
}
