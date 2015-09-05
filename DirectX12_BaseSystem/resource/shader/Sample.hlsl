cbuffer ConstantBuffer : register(b0)
{
	float4 offset;
}

struct InputVS
{
	float3 pos:IN_POSITION;
	float4 color:IN_COLOR;
	float2 uv : IN_TEXCOORD;
};
struct OutputVS
{
	float4 pos:SV_POSITION; // システムに渡す頂点の位置
	float4 color:IN_COLOR;
	float2 uv : TEXCOORD;
};


Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);


//! 頂点シェーダ
OutputVS RenderVS( InputVS inVert )
{
	OutputVS outVert;
	outVert.pos = float4(inVert.pos, 1);
	outVert.color = inVert.color;
	outVert.uv = inVert.uv;

	return outVert;
}

struct OutputGS
{
	float4 pos    : SV_POSITION;
	float4 color  : COLOR0;
	float2 uv : TEXCOORD;
};

[maxvertexcount(3)]   // ジオメトリシェーダーで出力する最大頂点数
void RenderGS(triangle OutputVS In[3],         // トライアングル リストを構成する頂点配列の入力情報
	inout TriangleStream<OutputGS> TriStream   // 頂点情報を追加するためのストリームオブジェクト
	)
{
	OutputGS Out;

	int i;

	// もとの頂点を出力
	for (i = 0; i < 3; i++)
	{
		Out.pos = In[i].pos + offset;

		Out.color = In[i].color;

		Out.uv = In[i].uv;

		// 頂点を追加する
		TriStream.Append(Out);
	}
	// 現在のストリップを終了し、新しいストリップを開始する。
	TriStream.RestartStrip();
}

float4 RenderPS(OutputGS inPixel) : SV_TARGET
{
	return g_texture.Sample(g_sampler, inPixel.uv);
}