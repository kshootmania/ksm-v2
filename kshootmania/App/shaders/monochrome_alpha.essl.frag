# version 300 es

precision mediump float;

uniform sampler2D Texture0;

in vec4 Color;
in vec2 UV;

layout(location = 0) out vec4 FragColor;

layout(std140) uniform PSConstants2D
{
	vec4 g_colorAdd;
	vec4 g_sdfParam;
	vec4 g_sdfOutlineColor;
	vec4 g_sdfShadowColor;
	vec4 g_internal;
};

void main()
{
	vec4 texColor = texture(Texture0, UV);
	float luminance = dot(texColor.rgb, vec3(0.299, 0.587, 0.114));

	vec4 result;
	result.rgb = Color.rgb;
	result.a = luminance * Color.a;

	FragColor = result + g_colorAdd;
}
