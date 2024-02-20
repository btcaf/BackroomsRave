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
    float4x4 matWorldView;
    float4x4 matView;
    float4 colMaterial;
    float4 colLight;
    float4 pointLight;
    float4 ambientLight;
    float4 padding;
};


struct vs_output_t
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 tex : TEXCOORD;
};

float4 clampColor(float4 color)
{
    return float4(
        min(max(color.r, 0.0f), 1.0f),
        min(max(color.g, 0.0f), 1.0f),
        min(max(color.b, 0.0f), 1.0f),
        min(max(color.a, 0.0f), 1.0f)
    );
}

vs_output_t main(
    float3 pos : POSITION, float4 col : COLOR, float2 tex : TEXCOORD, float3 normal : NORMAL
)
{
    vs_output_t result;
    float4 NW = mul(float4(normal, 0.0f), matWorldView);
    float4 dirLight = float4(pos, 0.0f) - pointLight;
    float4 LW = mul(dirLight, matView);
    result.position = mul(float4(pos, 1.0f), matWorldViewProj);
    result.color = mul(
 			max(-dot(normalize(LW), normalize(NW)), 0.0f),
 			colLight * col
    ); // lub colMaterial zamiast col*/
    //attenuate
    float dist = length(dirLight);
    result.color = result.color / (1.0f + 0.1 * dist * dist);
    result.color = clampColor(ambientLight * col + result.color);
    //result.color = col;
    result.tex = tex;
    return result;
}
