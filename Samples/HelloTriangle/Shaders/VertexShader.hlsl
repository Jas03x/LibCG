
struct VS_Input
{
    float3 vertex : POSITION;
    float3 color  : COLOR;
};

struct VS_Output
{
    float4 vertex : SV_POSITION;
    float3 color  : COLOR;
};

VS_Output main(VS_Input input)
{
    VS_Output output;
    output.vertex = float4(input.vertex, 1);
    output.color = input.color;

    return output;
}
