
struct VSIn {
    // Vertex
    float2 pos: Position;
    float4 color: Color;

    // Instance
    float2 instancePosition: InstancePosition;
    float4 instanceColor: InstanceColor;
};

struct VSOut {
    float4 color: Color;
    float4 pos: Sv_Position;
};

cbuffer Cbuf {
    matrix transform;
    float4 colorpx;
}

VSOut main(VSIn input) {
    VSOut vso;
    vso.pos = float4(input.instancePosition, 0.0f, 0.0f);
    vso.color = input.instanceColor;

    return vso;
}
