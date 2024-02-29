cbuffer vs_const_buffer_t
{
    float4x4 matViewProj;
    float4x4 matView;
    float4 colMaterial;
    float4 colLight[7];
    float4 pointLight[7];
    float4 ambientLight;
    float4 padding;
};


struct vs_output_t
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 tex : TEXCOORD;
};

vs_output_t main(
    float3 pos : POSITION, float4 col : COLOR, float2 tex : TEXCOORD, float3 normal : NORMAL,
    float2 inst_tex : INSTANCE_TEXCOORD, float4 inst_col : INSTANCE_COLOR, row_major float4x4 mat_w : WORLD,
    uint instance_id : SV_InstanceID
)
{
    vs_output_t result;
    pos = mul(float4(pos, 1.0f), mat_w).xyz;
    result.position = mul(float4(pos, 1.0f), matViewProj);
    // if opacity nonzero we ignore lighting
    if (inst_col.a > 0.0f) {
        result.color = inst_col;
        result.tex = inst_tex + tex * 0.5f;
        return result;
    }
    
    normal = mul(float4(normal, 0.0f), mat_w).xyz;
    normal = normalize(normal);
    result.color = ambientLight * col;
    
    for (uint i = 0; i < 7; i++) {
        float4 dirLight = float4(pos, 0.0f) - pointLight[i];
        float4 NW = mul(float4(normal, 0.0f), matView);
        float4 LW = mul(dirLight, matView);
        float4 new_color = mul(
            max(-dot(normalize(LW), normalize(NW)), 0.0f),
            colLight[i] * col
        );
        float dist = length(dirLight);
        
        // attenuation
        new_color = new_color / (1.0f + 0.1 * dist * dist);
        
        // sum colors from all lights
        result.color += new_color;
    }

    result.color = saturate(result.color);
    result.tex = inst_tex + tex * 0.5f;
    return result;
}
