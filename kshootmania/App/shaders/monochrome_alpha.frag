// テクスチャのRGB輝度をアルファ値として使用し、描画色(input.color)で塗りつぶすシェーダー

# version 410

//
//	Textures
//
uniform sampler2D Texture0;

//
//	PSInput
//
layout(location = 0) in vec4 Color;
layout(location = 1) in vec2 UV;

//
//	PSOutput
//
layout(location = 0) out vec4 FragColor;

//
//	Constant Buffer
//
layout(std140) uniform PSConstants2D
{
	vec4 g_colorAdd;
	vec4 g_sdfParam;
	vec4 g_sdfOutlineColor;
	vec4 g_sdfShadowColor;
	vec4 g_internal;
};

//
//	Functions
//
void main()
{
	vec4 texColor = texture(Texture0, UV);

	// BT.601輝度係数でグレースケール化し、アルファとして使用
	float luminance = dot(texColor.rgb, vec3(0.299, 0.587, 0.114));

	vec4 result;
	result.rgb = Color.rgb;
	result.a = luminance * Color.a;

	FragColor = result + g_colorAdd;
}
