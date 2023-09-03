
struct VS_Input
{
    float3 vertex : POSITION;
    float2 uv     : TEXCOORD;
};

struct VS_Output
{
    float4 vertex : SV_POSITION;
    float2 uv     : TEXCOORD;
};

VS_Output main(VS_Input input)
{
    VS_Output output;
    output.vertex = float4(input.vertex, 1);
    output.uv = input.uv;

    return output;
}
