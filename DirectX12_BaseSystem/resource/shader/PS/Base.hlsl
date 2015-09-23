cbuffer Light : register(b3)
{
	float4 lightDirection;
};

Texture2D  texDiffuse : register(t0);
SamplerState samDiffuse  : register(s0);

struct OutputGS
{
	float4 pos    : SV_POSITION;
	float4 color  : COLOR0;
	float3 eye	  : COLOR1;
	float3 normal : NORMAL;
	float2 texel  : TEXCOORD0;
};

float4 HalfLambert( OutputGS inPixel ) : SV_TARGET
{
	float4 Out = (float4)0;
	float3 L = normalize(LightDirection.xyz);

	float p = max(dot(inPixel.normal, L), 0.f);
	p = p * 0.2f + 0.7f;
	p = p * p;
	Out = texDiffuse.Sample(samDiffuse, inPixel.texel);
	Out.rgb *= p;
	return Out;
}

float4 PhoneShader(OutputGS inPixel) : SV_TARGET
{
	float4 Out;

	float3 N = normalize(inPixel.normal);

	float3 H = normalize(lightDirection.xyz + normalize(inPixel.eye));

	float S = pow(max(0.0f, dot(N, H)), 20.0f) * 0.75f;

	Out = texDiffuse.Sample(samDiffuse, inPixel.texel);
	Out.rgb = Out.rgb * 0.5f + S;

	return Out;
}