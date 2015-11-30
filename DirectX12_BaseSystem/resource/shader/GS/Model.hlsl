struct camBuffer
{
	matrix	matView;
	matrix	matProjection;
	float4 pos;
};

cbuffer CameraBuffers : register(b1)
{
	camBuffer camera;
};

struct OutputVS
{
	float4 pos			: SV_POSITION;
	float4 color		: COLOR0;
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

[maxvertexcount(3)]
void RenderGS(triangle OutputVS In[3],
	inout TriangleStream<OutputGS> TriStream
	)
{
	OutputGS Out;
	matrix matVP = mul(camera.matView, camera.matProjection);
	int i;
	for (i = 0; i < 3; i++)
	{
		Out.pos = mul(In[i].pos, matVP);

		Out.normal = In[i].normal;

		Out.texel = In[i].texel;

		Out.eye = normalize(camera.pos.xyz - In[i].pos.xyz);

		Out.color = In[i].color;   // 元の頂点は不透明で出力

		// 頂点を追加する
		TriStream.Append(Out);
	}
	// 現在のストリップを終了し、新しいストリップを開始する。
	TriStream.RestartStrip();
}