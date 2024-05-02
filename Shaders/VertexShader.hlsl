
struct VSOut {
    float4 color: Color;
    float4 pos: Sv_Position;
};

cbuffer Cbuf {
    matrix transform;
    float4 colorpx;
}

VSOut main(float2 pos: Position, float4 color: Color) {
    VSOut vso;
    vso.pos = mul(float4(pos, 0.0f, 1.0f), transform);
    vso.color = colorpx;

    return vso;
}
