// テクスチャのRGB輝度をアルファ値として使用し、描画色(input.color)で塗りつぶすシェーダー

//
//	Textures
//
Texture2D		g_texture0 : register(t0);
SamplerState	g_sampler0 : register(s0);

namespace s3d
{
	//
	//	VS Output / PS Input
	//
	struct PSInput
	{
		float4 position	: SV_POSITION;
		float4 color	: COLOR0;
		float2 uv		: TEXCOORD0;
	};
}

//
//	Constant Buffer
//
cbuffer PSConstants2D : register(b0)
{
	float4 g_colorAdd;
	float4 g_sdfParam;
	float4 g_sdfOutlineColor;
	float4 g_sdfShadowColor;
	float4 g_internal;
}

float4 PS(s3d::PSInput input) : SV_TARGET
{
	float4 texColor = g_texture0.Sample(g_sampler0, input.uv);

	// BT.601輝度係数でグレースケール化し、アルファとして使用
	float luminance = dot(texColor.rgb, float3(0.299, 0.587, 0.114));

	float4 result;
	result.rgb = input.color.rgb;
	result.a = luminance * input.color.a;

	return result + g_colorAdd;
}
