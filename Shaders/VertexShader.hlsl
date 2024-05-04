
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
}

VSOut main(VSIn input) {
    VSOut vso;
    // mul(float4(pos, 0.0f, 1.0f), transform);
    vso.pos = mul( float4( input.pos - input.instancePosition, 0.0f, 1.0f), transform );
    vso.color = input.instanceColor;

    return vso;
}
