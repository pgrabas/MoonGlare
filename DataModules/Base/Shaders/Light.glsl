//-----------LIGHT-COMMON-----------

struct BaseLight_t {
    vec3 Color;
    float AmbientIntensity;
    float DiffuseIntensity;
};

vec4 CalcLightInternal(BaseLight_t BaseLight, Material_t mat, vec3 LightDirection, vec3 WorldPos, vec3 Normal) {
    vec4 AmbientColor  = vec4(0, 0, 0, 0);
    vec4 DiffuseColor  = vec4(0, 0, 0, 0);
    vec4 SpecularColor = vec4(0, 0, 0, 0);

    AmbientColor = vec4(BaseLight.Color, 1.0) * BaseLight.AmbientIntensity;

	float gShiness = 10;
    // float gMatSpecularIntensity = 0.5;

    float DiffuseFactor = dot(Normal, -LightDirection);
    if (DiffuseFactor > 0.0) {
		DiffuseFactor *= BaseLight.DiffuseIntensity;
        DiffuseColor = vec4(BaseLight.Color, 1.0) * DiffuseFactor;
    }

	vec3 VertexToEye = normalize(CameraPos - WorldPos);
	vec3 LightReflect = normalize(reflect(LightDirection, Normal));
	float SpecularFactor = dot(VertexToEye, LightReflect);
	SpecularFactor = pow(max(SpecularFactor, 0.0),
	//  gShiness
	// // SHINESS_SCALER -
	 (1.0f - mat.shinessExponent) * SHINESS_SCALER
	);
	if (SpecularFactor > 0.0) {
		SpecularColor = vec4(BaseLight.Color, 1.0) * 
		// gMatSpecularIntensity 
		// vec4(mat.shinessExponent) *
		vec4(mat.specularColor, 1) 
		* SpecularFactor;
	}

    vec4 result = 
	( AmbientColor + DiffuseColor ) * vec4(mat.diffuseColor, 1) +
	 SpecularColor;
	result.a = 1.0;
	return result;
};

//-----------LIGHT-ATTENUATION-----------

float CalcAttenuation(vec4 att, float Distance) {
    float Attv = att[0] +			              //constant
				 att[1] * Distance +              //linerar
				 att[2] * (Distance * Distance);  //exp
	return 1.0f / 
		Attv;
		// max(Attv, att[3]);
}

//-----------LIGHT-POINT-----------

struct PointLight_t {
	BaseLight_t	Base;
    vec3 Position;
    vec4 Attenuation;
};

vec4 CalcPointLight(vec3 WorldPos, vec3 Normal, PointLight_t light, Material_t mat) {
	vec3 LightToWord = WorldPos - light.Position;
    float Distance = length(LightToWord);
	vec3 LightToPixel = normalize(LightToWord);

    vec4 Color = CalcLightInternal(light.Base, mat, LightToPixel, WorldPos, Normal);
	float factor = CalcAttenuation(light.Attenuation, Distance);
    Color.xyz *= factor;
 	Color.xyz = CalcStaticShadow(WorldPos, Color.xyz);
	return Color;
};

//-----------LIGHT-DIRECTIONAL-----------

struct DirectionalLight_t {
 	BaseLight_t	Base;
	vec3 Direction;
};
uniform DirectionalLight_t DirectionalLight;

vec4 CalcDirectionalLight(vec3 WorldPos, vec3 Normal, vec4 MaterialDiffuse) {
    vec4 Color = vec4(0);
	//  CalcLightInternal(DirectionalLight.Base, MaterialDiffuse, DirectionalLight.Direction, WorldPos, Normal);
	return Color;
};

//-----------LIGHT-SPOT-----------

struct SpotLight_t {
 	BaseLight_t	Base;
	vec3 Direction;
    vec3 Position;
    vec4 Attenuation;
	float CutOff;
};
uniform SpotLight_t SpotLight;

vec4 CalcSpotLight(vec3 WorldPos, vec3 Normal, vec4 MaterialDiffuse) {
	
	vec3 LightToWord = WorldPos - SpotLight.Position;

	vec3 LightToPixel = normalize(LightToWord);
	float SpotFactor = dot(normalize(-SpotLight.Direction), LightToPixel);

	if (SpotFactor > SpotLight.CutOff) {
		float Distance = length(LightToWord);
  	  vec4 Color = vec4(0);
		// vec4 Color = CalcLightInternal(SpotLight.Base, MaterialDiffuse, LightToPixel, WorldPos, Normal);
		float factor = CalcAttenuation(SpotLight.Attenuation, Distance);
		Color.xyz *= factor;
 		Color.xyz = CalcStaticShadow(WorldPos, Color.xyz);
		Color.xyz *= (1.0 - (1.0 - SpotFactor) * 1.0/(1.0 - SpotLight.CutOff));
		//Color.xyz = LightToPixel;

//		Color.a = 1.0f;
		//Color.g = 1.0f;
		//	Color.b = SpotFactor;
		return Color;
	}
	//else
	//	discard;

	return vec4(0.0);
}
