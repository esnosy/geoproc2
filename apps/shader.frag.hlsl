struct Input
{
	float3 Normal : TEXCOORD0;
};

struct Output
{
	float4 Color : SV_Target0;
};

Output main(Input input)
{
	Output output;
	float brightness = dot(input.Normal, float3(1.0f, 1.0f, 1.0f));
	float3 dark_color = float3(0.6f, 0.0f, 0.0f);
    float3 regular_color = float3(1.0f, 0.0f, 0.0f);
    output.Color = float4(lerp(dark_color, regular_color, brightness), 1.0f);
	return output;
}