cbuffer LightBuffer : register(b1)
{
    float4 color;
    float2 padding;
    matrix lightSpace;
};

float4 Pixel() : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}