struct Attributes
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;

    float4 matrix0 : MODEL0;
    float4 matrix1 : MODEL1;
    float4 matrix2 : MODEL2;
    float4 matrix3 : MODEL3;
    float4 st : ST;
};

struct Varyings
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

cbuffer CameraBuffer : register(b0)
{
    matrix matrix_v;
    matrix matrix_p;
}

Varyings main(Attributes attribs)
{
    float4x4 matrix_m = float4x4(attribs.matrix0, attribs.matrix1, attribs.matrix2, attribs.matrix3);

    Varyings varyings;
    varyings.pos = mul(mul(mul(float4(attribs.pos, 1.0), matrix_m), matrix_v), matrix_p);
    varyings.uv = attribs.uv * attribs.st.xy + attribs.st.zw;
    return varyings;
}