#include "Deferred/Common.glsl"
#include "Light.glsl"

uniform samplerCube gCubeShadowMap;
uniform PointLight_t gPointLight;

out vec4 FragColor;


vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);   

float ShadowCalculation(vec3 fragPos)
{
	// if(!gEnableShadowTest)
	// 	return 1.0f;

    // get vector between fragment position and light position
    vec3 fragToLight = (fragPos - gPointLight.Position);
    // ise the fragment to light vector to sample from the depth map    
    float depth = texture(gCubeShadowMap, fragToLight).z;
    // it is currently in linear range between [0,1], let's re-transform it back to original depth value
    float closestDepth  =  depth * 100.0f;//far_plane;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight) ;/// 100.0f;
    // test for shadows
	float bias = 0.0005;

//    float currentDistanceToLight = (currentDepth - 0.1) / (100 - 0.1);
 		// currentDistanceToLight = clamp(currentDistanceToLight, 0, 1);

    // float bias = 0.0;//5; // we use a much larger bias since depth is now in [near_plane, far_plane] range
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;        
    // display closestDepth as debug (to visualize depth cubemap)
    // FragColor = vec4(vec3(closestDepth / far_plane), 1.0);    

	// float far_plane = 100.0f;

	// float shadow = 0.0;
	// // float bias   = 0.15;
	// int samples  = 20;
	// // float viewDistance = length(viewPos - fragPos);
	// float diskRadius = 0.05;
	// for(int i = 0; i < samples; ++i)
	// {
	// 	float closestDepth = texture(ShadowMap, fragToLight + 
	// 		sampleOffsetDirections[i] * diskRadius).r;
	// 	closestDepth *= far_plane;   // Undo mapping [0;1]
	// 	if(currentDepth - bias > closestDepth)
	// 		shadow += 1.0;
	// }
	// shadow /= float(samples);  
        
    return //vec3(
		// currentDepth,
		// currentDepth / 100,
		//  depth, 
		1.0f - shadow
		// 0
	//	)
		;
	//  1.0 - 
	//  shadow;
}

void main() {
    vec2 TexCoord = CalcTexCoord(gl_FragCoord);

	vec3 worldPos = texture(gPositionMap, TexCoord).xyz;
	vec4 Color = texture(gColorMap, TexCoord);
	vec4 specularColor = texture(gSpecularMap, TexCoord);
	vec4 emissiveColor = texture(gEmissiveMap, TexCoord);
	vec4 NormalShiness = texture(gNormalMap, TexCoord);

	vec3 normal = NormalShiness.xyz;
	float shiness = NormalShiness.a;
//	Normal = normalize(Normal);

	Material_t mat;
	mat.diffuseColor = Color.xyz;
	mat.specularColor = specularColor.xyz;
	mat.emissiveColor = emissiveColor.xyz;
	mat.shinessExponent = shiness;
	mat.opacity = 1.0f;

	float shadow = ShadowCalculation(worldPos);                  
	FragColor = CalcPointLight(worldPos, normal, gPointLight, mat, shadow);
}
