
Texture2D    g_Texture : register(t0);
SamplerState g_Sampler : register(s0);

struct PS_Input
{
    float4 vertex : SV_POSITION;
    float2 uv     : TEXCOORD;
};

struct PS_Output
{
    float4 color : SV_TARGET; // pixel position in screen space
};

PS_Output main(PS_Input input)
{
    PS_Output Output;
    //Output.color = g_Texture.Sample(g_Sampler, input.uv);
    Output.color = float4(1, 0, 0, 1);

    return Output;
}
