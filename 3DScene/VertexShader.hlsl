/*
* The vertex shader for the sample D3D12 application for JNP3 course at MIMUW.
*
* Transforms the vertex position by a World-View-Projection matrix.
* Forwards the vertex color and texture coordinates
* in the graphics pipeline.
*
* Author:   Przemyslaw Rutka
* Version:  1.0
* Date:     08.01.2024
*/

cbuffer vs_const_buffer_t
{
    float4x4 matWorldViewProj;
    float4 padding[12];
};

struct vs_output_t
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 tex : TEXCOORD;
};

vs_output_t main(
    float3 pos : POSITION, float4 col : COLOR, float2 tex : TEXCOORD
)
{
    vs_output_t result;
    result.position = mul(float4(pos, 1.0f), matWorldViewProj);
    result.color = col;
    result.tex = tex;
    return result;
}
