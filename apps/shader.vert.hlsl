cbuffer UBO : register(b0, space1)
{
    float4x4 model;
    float4x4 view;
    float4x4 projection;
    float4x4 model_normal;
};

struct Input
{
    float3 Position : TEXCOORD0;
    float3 Normal : TEXCOORD1;
};

struct Output
{
    float4 Position : SV_Position;
    float3 Normal : TEXCOORD0;
    float3 FragPos : TEXCOORD1;
};

Output main(Input input)
{
    Output output;
    float4 pos = float4(input.Position, 1.0f);
    output.Position = mul(projection, mul(view, mul(model, pos)));
    output.Normal = mul((float3x3)model_normal, input.Normal);
    output.FragPos = mul(model, pos).xyz;
    return output;
}