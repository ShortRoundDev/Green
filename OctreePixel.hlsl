cbuffer WireframeDebugColor : register(b2)
{
    float4 color;
}

float4 Pixel() : SV_TARGET
{
	return color;
}