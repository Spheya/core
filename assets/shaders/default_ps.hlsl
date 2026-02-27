struct Varyings
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

Texture2D tex : register(t0);
SamplerState point_sampler
{
    Filter = MIN_LINEAR_MAG_POINT_MIP_LINEAR;
};

float4 main(Varyings varyings) : SV_TARGET
{
    float4 col = tex.Sample(point_sampler, varyings.uv);
    col.rgb *= col.a;
    return col * 0.4;
}