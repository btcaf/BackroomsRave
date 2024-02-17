/*
* The pixel shader for the sample D3D12 application for JNP3 course at MIMUW.
*
* Colors the fragment according to sampled texels.
*
* Author:   Przemyslaw Rutka
* Version:  1.0
* Date:     08.01.2024
*/

struct ps_input_t
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 tex : TEXCOORD;
};

Texture2D texture_ps;
SamplerState sampler_ps;

float4 main(ps_input_t input) : SV_TARGET
{
    return texture_ps.Sample(sampler_ps, input.tex);
    //return input.color;
    //return input.color * texture_ps.Sample(sampler_ps, input.tex);
}
