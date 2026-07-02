# version 300 es

precision mediump float;

uniform sampler2D Texture0;
uniform sampler2D Texture1;

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
	float mask = texture(Texture1, UV).r;
	texColor.a *= mask;
	FragColor = (texColor * Color) + g_colorAdd;
}
