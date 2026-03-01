struct Varyings
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

float4 main(Varyings varyings) : SV_TARGET
{
    return varyings.color;
}