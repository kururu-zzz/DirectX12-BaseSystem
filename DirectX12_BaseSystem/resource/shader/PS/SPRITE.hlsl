/*cbuffer material : register(b2)
{
	float3 ambient;
	float3 diffuse;
	float3 emissive;
	float3 speculer;
}*/

Texture2D  texDiffuse : register(t0);
SamplerState samDiffuse  : register(s0);

struct OutputGS
{
float4 pos    : SV_POSITION;
float4 color  : COLOR0;
float3 normal : NORMAL;
float2 texel  : TEXCOORD0;
//uint   ViewportIndex : SV_ViewportArrayIndex;
};

struct OutputPS
{
	float4 out0:SV_TARGET0;
};

// ピクセルシェーダ
OutputPS RenderPS(OutputGS inPixel)
{
	OutputPS Out = (OutputPS)0;
	Out.out0 = texDiffuse.Sample(samDiffuse, inPixel.texel);
	Out.out0.a *= inPixel.color.a;
	return Out;
}