#include "Object3d.hlsli"

struct Material
{
    float4 color;
    int enableLightig;
    float4x4 uvTransform;
    float shininess;
};

struct DirectionalLight
{
    float4 color;
    float3 direction;
    float intensity;
};

struct Camera
{
    float3 worldPosition;
};

ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);
ConstantBuffer<DirectionalLight> gDiretionalLight : register(b1);
ConstantBuffer<Camera> gCamera : register(b2);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    // 視線ベクトル
    float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
    float3 reflectLight = reflect(gDiretionalLight.direction, normalize(input.normal));
    float RdotE = dot(reflectLight, toEye);
    float specularPow = pow(saturate(RdotE), gMaterial.shininess);
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    PixelShaderOutput output;
    if (gMaterial.enableLightig != 0)
    {
        float NdotL = dot(normalize(input.normal), -gDiretionalLight.direction);
        //float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        float diffuseCos = saturate(NdotL);
        
        float3 diffuse = gMaterial.color.rgb * textureColor.rgb * gDiretionalLight.color.rgb * diffuseCos * gDiretionalLight.intensity;
        
        float3 specular = gDiretionalLight.color.rgb * gDiretionalLight.intensity * specularPow;
        
        output.color.rgb = diffuse + specular;
        
        if (textureColor.a == 0.0f)
        {
            discard;
        }
        
        output.color.a = gMaterial.color.a * textureColor.a;
        
        if (output.color.a == 0.0f)
        {
            discard;
        }
    }
    else
    {
        output.color = gMaterial.color * textureColor;
        
        if (output.color.a == 0.0f)
        {
            discard;
        }
    }
    return output;
}