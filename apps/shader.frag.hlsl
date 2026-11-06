cbuffer UBO : register(b0, space3)
{
	float4 light_pos;
	float4 light_color;
	float4 object_color;
	float4 ambient;
};

struct Input
{
	float3 Normal : TEXCOORD0;
	float3 FragPos : TEXCOORD1;
};

struct Output
{
	float4 Color : SV_Target0;
};

Output main(Input input)
{
	Output output;
	float3 norm = normalize(input.Normal);
	float3 light_dir = normalize(light_pos.xyz - input.FragPos);
	float diff = max(dot(norm, light_dir), 0.0f);
	float3 diffuse = diff * light_color.xyz;
	float3 result = (ambient + diffuse) * object_color.xyz;
	output.Color = float4(result, 1.0f);
	return output;
}