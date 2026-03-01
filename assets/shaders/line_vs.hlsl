struct Attributes
{
    float3 pos : POSITION;
    float4 color : COLOR;
};

struct Varyings
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

cbuffer CameraBuffer : register(b0)
{
    matrix matrix_v;
    matrix matrix_p;
}

Varyings main(Attributes attribs)
{
    Varyings varyings;
    varyings.pos = mul(mul(float4(attribs.pos, 1.0), matrix_v), matrix_p);
    varyings.color = attribs.color;
    return varyings;
}