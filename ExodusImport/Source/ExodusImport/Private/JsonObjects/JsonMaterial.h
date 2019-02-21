#pragma once
#include "JsonTypes.h"

class JsonMaterial{
public:
	/*
	enum struct Queue: int{
		Background = 1000,
		Geometry = 2000,
		AlphaTest = 2450,
		Transparent = 4000
	};
	*/
	struct Queues{
		enum Queue{
			Background = 1000,
			Geometry = 2000,
			AlphaTest = 2450,
			Transparent = 3000,
			Overlay = 4000
		};
	};

	/*
	enum struct WorkflowMode{
		Specular = 0, //why is this defualt, again?
		Metallic,
		Dielectric//not supported right now.
	};
	*/
	struct WorkflowModes{
		enum WorkflowMode{
			Specular = 0, //why is this defualt, again?
			Metallic,
			Dielectric//not supported right now.
		};
	};
	/*
	As of now, the logic goes like this:
		If ther'es specular color and specular map properties, workflow is specular.
		If there's metallic value and metallic map properties, workflow is metallic
		Otherwise it is dielectric.

		Wish they stored it in a variable or something.
	*/
	
	/*
	enum struct BlendMode{
		Opaque = 0,
		Cutout,
		Fade,
		Transparent
	};
	*/
	struct BlendModes{
		enum BlendMode{
			Opaque = 0,
			Cutout,
			Fade,
			Transparent
		};
	};


	JsonMaterialId id = -1;
	int renderQueue = 0;
	FString name;
	FString path;
	FString shader;

	bool supportedShader = true;
	int blendMode = -1;

	JsonTextureId mainTexture = -1;
	FVector2D mainTextureOffset = FVector2D(0.0f, 0.0f);//Vector2.zero;
	FVector2D mainTextureScale = FVector2D(1.0f, 1.0f);//Vector2.one;
	FLinearColor color = FLinearColor::White;// = Color.white;

	bool usesMainTextureTransform() const;
	bool usesDetailTextureTransform() const;

	/*
	I think those flags are quite glitchy at best, as they're derived from shader keywords. 
	I need to rethink the way the shader reconstruction operates
	*/
	bool useNormalMap = false;
	bool useAlphaTest = false;
	bool useAlphaBlend = false;
	bool useAlphaPremultiply = false;
	bool useEmission = false;
	bool useParallax = false;
	bool useDetailMap = false;
	bool useMetallic = false;//this is unrealiable
			
	bool hasMetallic = false;
	bool hasSpecular = false;
	bool hasEmissionColor = false;
	bool hasEmission = false;
			
	bool useSpecular = false;

	JsonTextureId albedoTex = -1;
	JsonTextureId specularTex = -1;
	JsonTextureId metallicTex = -1;
	JsonTextureId normalMapTex = -1;
	JsonTextureId occlusionTex = -1;
	JsonTextureId parallaxTex = -1;
	JsonTextureId emissionTex = -1;
	JsonTextureId detailMaskTex = -1;
	JsonTextureId detailAlbedoTex = -1;

	FVector2D detailAlbedoOffset = FVector2D(0.0f, 0.0f);//Vector2.zero;
	FVector2D detailAlbedoScale = FVector2D(1.0f, 1.0f);//Vector2.one;

	float detailNormalMapScale = 1.0f;

	JsonTextureId detailNormalMapTex = -1;
	float alphaCutoff = 1.0f;
	float smoothness = 0.5f;
	float smoothnessScale = 1.0f;

	FLinearColor specularColor = FLinearColor::White;//Color.white;
	float metallic = 0.5f;
	float bumpScale = 1.0f;
	float parallaxScale = 1.0f;
	float occlusionStrength = 1.0f;
	FLinearColor emissionColor = FLinearColor::Black;//Color.black;
	float detailMapScale = 1.0f;
	float secondaryUv = 1.0f;

	FLinearColor colorGammaCorrected = FLinearColor::White;
	FLinearColor specularColorGammaCorrected = FLinearColor::White;//Color.white;
	FLinearColor emissionColorGammaCorrected = FLinearColor::Black;//Color.black;

	int smoothnessMapChannel = 0;
	float specularHighlights = 1.0f;
	float glossyReflections = 1.0f;

	bool hasValidBlendMode() const{
		return blendMode >= 0;
	}

	bool needsTransparencyFlag() const{
		if (hasValidBlendMode())
			return isTransparentMode() || isAlphaTestMode();
		return isTransparentQueue() || isAlphaTestQueue();
	}

	bool heuristicNeedsTransparentFlag() const{
		return heuristicIsTransparent() || heuristicIsCutout();
	}

	bool heuristicIsTransparent() const{
		return 
			(nameMarkedTransparent() && !nameMarkedCutout())
			|| (hasValidBlendMode() && (isTransparentMode() || isFadeMode()))
			|| (!hasValidBlendMode() && isTransparentQueue());
	}

	bool heuristicIsCutout() const{
		return 
			nameMarkedCutout()
			|| (hasValidBlendMode() && isAlphaTestMode())
			|| (!hasValidBlendMode() && isAlphaTestQueue());
	}

	bool isEmissive() const{
		/*
		This is not reliable, as it relies on both shader keywords and presence of components in material.
		*/
		return useEmission && (hasEmission/* || hasEmissionColor*/);
	}

	FString getUnrealMaterialName() const;

	bool nameMarkedTransparent() const;
	bool nameMarkedCutout() const;

	bool isTransparentMode() const;
	bool isAlphaTestMode() const;
	bool isFadeMode() const;

	bool isTransparentQueue() const;
	bool isAlphaTestQueue() const;
	bool isGeomQueue() const;

	JsonMaterial() = default;
	void load(JsonObjPtr data);
	JsonMaterial(JsonObjPtr data);
};