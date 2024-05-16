
struct PSIn {
    float4 color: Color;
};

StructuredBuffer<uint> particals;

cbuffer Cbuf {
    matrix camera_transform;
}

float checkCube() {
    return 1.0;
}

float4 main(PSIn input): SV_Target {
    return float4(1.0);
}
