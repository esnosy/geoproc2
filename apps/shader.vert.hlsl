cbuffer UBO : register(b0, space1)
{
    float4x4 mvp : packoffset(c0);
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
};

Output main(Input input)
{
    Output output;
    output.Position = mul(mvp, float4(input.Position, 1.0f));
    output.Normal = input.Normal;
    return output;
}