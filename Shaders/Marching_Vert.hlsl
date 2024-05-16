
struct VSIn {
    // Vertex
    float3 pos: Position;
};

struct VSOut {
    float4 color: Color;
    float4 pos: Sv_Position;
};

VSOut main(VSIn input) {
    VSOut vso;
    // mul(float4(pos, 0.0f, 1.0f), transform);

    input.pos += input.instancePosition;
    vso.pos = mul( float4(input.pos, 1.0f), transform );
    vso.color = input.instanceColor; // instanceColor

    return vso;
}
