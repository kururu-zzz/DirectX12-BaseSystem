cbuffer mtxViewport : register(b0)
{
	matrix mtxViewport;
};

/*cbuffer viewport : register(b4)
{
	float viewportDrawable;
	float viewportNum;
};*/

struct OutputVS
{
	float4 pos			: SV_POSITION;
	float4 color		: COLOR0;
	float3 normal       : NORMAL;
	float2 texel        : TEXCOORD0;
};

struct OutputGS
{
float4 pos    : SV_POSITION;
float4 color  : COLOR0;
float3 normal : NORMAL;
float2 texel  : TEXCOORD0;
//uint   ViewportIndex : SV_ViewportArrayIndex;
};

[maxvertexcount(3)]   // ジオメトリシェーダーで出力する最大頂点数(viewport * 3でよい)
// ジオメトリシェーダー
void RenderGS(triangle OutputVS In[3],                    // トライアングル リストを構成する頂点配列の入力情報
	inout TriangleStream<OutputGS> TriStream   // 頂点情報を追加するためのストリームオブジェクト
	)
{
	OutputGS Out;

	//int drawable = viewportDrawable;//ビット演算のためint型にキャスト
	int i, j;
	//for (j = 0; j < viewportNum; j++)
	//{
	//	if (drawable & 0x01)
	//	{
			// もとの頂点を出力
			for (i = 0; i < 3; i++)
			{
				Out.pos = mul(In[i].pos, mtxViewport);

				Out.normal = float3(0.f, 0.f, 0.f);

				Out.texel = In[i].texel;

				Out.color = In[i].color;   // 元の頂点は不透明で出力

//				Out.ViewportIndex = j;

				// 頂点を追加する
				TriStream.Append(Out);
			}
			// 現在のストリップを終了し、新しいストリップを開始する。
			TriStream.RestartStrip();
	//	}
	//	drawable >>= 1;
	//}
}