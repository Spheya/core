struct Varyings
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

Texture2D tex : register(t0);
SamplerState point_sampler : register(s0);

float4 main(Varyings varyings) : SV_TARGET
{
    float4 col = tex.Sample(point_sampler, varyings.uv);
    col.rgb *= col.a;
    if (col.a < 0.5)
        discard;
    return col;
}