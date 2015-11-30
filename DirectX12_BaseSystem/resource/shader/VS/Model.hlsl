cbuffer mtxWorld : register(b0)
{
	matrix	matWorld;
};
/*
cbuffer mtxBones : register(b4)
{
	matrix boneMatrix[90];
};
*/

struct InputVS
{
	float3 pos			: IN_POSITION;
	float4 color		: IN_COLOR;
	float3 normal		: IN_NORMAL;
	float2 texel        : IN_UV;
	//uint4  boneIndex	: IN_BONEINDEX;
	//float4 boneWeight	: IN_BONEWEIGHT;
};
struct OutputVS
{
	float4 pos			: SV_POSITION;
	float4 color		: COLOR0;
	float3 normal       : NORMAL;
	float2 texel        : TEXCOORD0;
};

OutputVS RenderVS( InputVS inVert )
{
	//matrix convertMat = (matrix)0;
	/*[unroll]
	for (int i = 0; i < 4; ++i)
	{
		uint index = inVert.boneIndex[i];
		convertMat += boneMatrix[index] * inVert.boneWeight[i];
	}*/
	float4 outPos = float4(inVert.pos, 1.0f);
	float3 outNormal = inVert.normal;
	/*if (any(convertMat))
	{
		outPos = mul(outPos, convertMat);
		outNormal = mul(outNormal, convertMat);
	}*/
	outPos = mul(outPos, matWorld);
	outNormal = mul(outNormal, matWorld);
	//outPos = mul(outPos, matVP);
	OutputVS Out = { outPos, inVert.color,outNormal, inVert.texel};
	return Out;
}