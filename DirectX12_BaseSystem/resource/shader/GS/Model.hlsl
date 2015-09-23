// サンプル描画

//! コンスタントバッファ
/*!
コンスタントバッファを使用することでいくつかのコンスタントレジスタを1まとめにすることができる
レジスタの変化のタイミングによって分けておく方がよい
*/
cbuffer WorldBuffers : register(b0)
{
	matrix	matWorld;
};
struct camBuffer
{
	matrix	matView;
	matrix	matProjection;
	matrix  matViewPort;
	matrix  matLVP;
	float4 eyePos;
};

cbuffer CameraBuffers : register(b1)
{
	camBuffer cam[5];
};

cbuffer MaterialBuffers : register(b2)
{
	float4 ambient;
	float4 diffuse;
	float4 emissive;
	float4 speculer;
}

cbuffer LightBuffers : register(b3)
{
	float4 LightDirection;
};
cbuffer ViewportBuffers : register(b4)
{
	float viewportDrawable;
	float viewportNum;
};
cbuffer BoneBuffers : register(b5)
{
	matrix boneMatrix[90];
};

Texture2D  texDiffuse : register(t0);
SamplerState samDiffuse  : register(s0);
Texture2D  texDiffuse1 : register(t1);
SamplerState samDiffuse1  : register(s1);

//! 頂点属性
/*!
基本はD3D9と変わらない
ただし、いくつかのセマンティクスが変わっている
システム側に渡すセマンティクスはSV_の接頭辞がついている
*/
struct InputVS
{
	float3 pos			: IN_POSITION;
	float4 color		: IN_COLOR;
	float3 normal		: IN_NORMAL;
	float2 texel        : IN_TEXCOORD;   // テクセル
	uint4  boneIndex	: IN_BONEINDEX;
	float4 boneWeight	: IN_BONEWEIGHT;
};
struct OutputVS
{
	float4 pos			: SV_POSITION;
	float4 color		: COLOR0;
	float4 eye			: COLOR1;
	float3 normal       : NORMAL;
	float2 texel        : TEXCOORD0;
};


/*-----3Dモデル描画用ハーフランバート-----*/


//! 頂点シェーダ
OutputVS RenderVS( InputVS inVert )
{
	matrix convertMat = (matrix)0;
	[unroll]
	for (int i = 0; i < 4; ++i)
	{
		uint index = inVert.boneIndex[i];
		convertMat += boneMatrix[index] * inVert.boneWeight[i];
	}
	float4 outPos = float4(inVert.pos, 1.0f);
	float3 outNormal = inVert.normal;
	if (any(convertMat))
	{
		outPos = mul(outPos, convertMat);
		outNormal = mul(outNormal, convertMat);
	}
	outPos = mul(outPos, matWorld);
	outPos.w = 1.f;
	OutputVS Out = { outPos, inVert.color, float4(0.f,0.f,0.f,0.f),outNormal, inVert.texel};
	return Out;
}


// ジオメトリシェーダーの入力パラメータ
//typedef OutputVS InputGS;

// ジオメトリシェーダーの出力パラメータ
struct OutputGS
{
float4 pos    : SV_POSITION;
float4 color  : COLOR0;          // 頂点カラーはジオメトリシェーダー内で追加する
float3 eye	  : COLOR1;
float3 normal : NORMAL;
float2 texel  : TEXCOORD0;
uint   viewportIndex : SV_ViewportArrayIndex;
};

// ジオメトリ シェーダー オブジェクト (DirectX HLSL)

[maxvertexcount(15)]   // ジオメトリシェーダーで出力する最大頂点数
// ジオメトリシェーダー
void RenderGS3(triangle OutputVS In[3],                    // トライアングル リストを構成する頂点配列の入力情報
	inout TriangleStream<OutputGS> TriStream   // 頂点情報を追加するためのストリームオブジェクト
	)
{
	OutputGS Out;

	int drawable = viewportDrawable;//ビット演算のためint型にキャスト
	int i,j;

	for (j = 0; j < viewportNum; j++)
	{
		if (drawable & 0x01)
		{
			// もとの頂点を
			[unroll]
			for (i = 0; i < 3; i++)
			{
				matrix matVP = mul(cam[j].matView, cam[j].matProjection);

				Out.pos = mul(In[i].pos, matVP);

				Out.normal = In[i].normal;

				Out.texel = In[i].texel;

				Out.eye = normalize(cam[j].eyePos.xyz - In[i].pos.xyz);

				Out.color = In[i].color;   // 元の頂点は不透明で出力

				Out.viewportIndex = j;
				// 頂点を追加する
				TriStream.Append(Out);
			}
			// 現在のストリップを終了し、新しいストリップを開始する。
			TriStream.RestartStrip();
		}
		drawable >>= 1;
	}
}