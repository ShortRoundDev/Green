struct VS_OUTPUT
{
    float4 Position : POSITION; // vertex position
    float2 TexCoord : TEXCOORD0; // vertex interpolation value
};

struct PS_OUT
{
    float4 color : SV_Target;
    float depth : SV_Depth;
};

Texture2D inputTexture : register(t1);
SamplerState samplerState : register(s0);

PS_OUT Pixel(VS_OUTPUT input)
{
    
    PS_OUT output;
    output.depth = 0.123f;
    output.color = float4(1.0f, 1.0f, 0.0f, 1.0f);
    return output;
    
    /*float texel = 1.0f / 512.0f;
    
    float value = 0.0f;
    
    float coefficients[21] = {
        0.000272337, 0.00089296, 0.002583865, 0.00659813, 0.014869116,
	     0.029570767, 0.051898313, 0.080381679, 0.109868729, 0.132526984,
	     0.14107424,
	     0.132526984, 0.109868729, 0.080381679, 0.051898313, 0.029570767,
	     0.014869116, 0.00659813, 0.002583865, 0.00089296, 0.000272337
    };
    
    for (int i = 0; i < 21; i++)
    {
        value += inputTexture.Sample(samplerState, float2(input.TexCoord.x + (i - 10) * texel, input.TexCoord.y)) * coefficients[i];
    }
    
    output.depth = value;*/
    //return output;
}