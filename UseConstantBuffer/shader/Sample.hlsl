cbuffer ConstantBuffer : register(b0)
{
	float4 offset;
}

cbuffer ConstantBuffer2 : register(b1)
{
	float4 offset2;
}


struct InputVS
{
	float3 pos:IN_POSITION;
	float4 color:IN_COLOR;
};
struct OutputVS
{
	float4 pos:SV_POSITION; // システムに渡す頂点の位置
	float4 color:IN_COLOR;
};

//! 頂点シェーダ
OutputVS RenderVS( InputVS inVert )
{
	OutputVS outVert;
	outVert.pos = float4(inVert.pos, 1);
	outVert.color = inVert.color;

	return outVert;
}

struct OutputGS
{
	float4 pos    : SV_POSITION;
	float4 color  : COLOR0;
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
		Out.pos = In[i].pos + offset + offset2;

		Out.color = In[i].color;

		// 頂点を追加する
		TriStream.Append(Out);
	}
	// 現在のストリップを終了し、新しいストリップを開始する。
	TriStream.RestartStrip();
}


//! ピクセルシェーダ
float4 RenderPS(OutputGS inPixel) : SV_TARGET
{
	//return texDiffuse.Sample(samDiffuse, inPixel.uv);
	return inPixel.color;
}