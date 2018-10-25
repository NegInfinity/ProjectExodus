#pragma once
#include "JsonTypes.h"

#include "JsonObjects/JsonMaterial.h"

using MaterialFingerprintId = uint32;

class MaterialFingerprint{
public:
	enum RenderMode{
		Opaque = 0,
		Cutout = 1,
		Fade = 2,
		Transparent = 3
	};
	union{
		struct{
			//textures
			bool albedoTex: 1;//albedo provided
			bool normalmapTex: 1;//normalmap texture (bump) is present
			bool metallicTex: 1;//metallic texutre is present
			bool specularTex: 1;//specular texture is present
			bool occlusionTex: 1;//occlusion texture is present
			bool parallaxTex: 1;//parallax texture (heightmap) is present
			bool detailMaskTex: 1;//detail mask texture is present
			bool detailAlbedoTex: 1;//detail albedo texture is present
			bool detailNormalTex: 1;//detail normal map texture is present
			bool emissionTex: 1;//emissive texture is present

			//flags
			bool specularModel: 1;//uses specular model if set instead of metallic
			bool emissionEnabled: 1;//emissive if enabled
			bool mainTextureTransform: 1;//texture transformation set
			bool detailTextureTransform: 1;//texture transformation set for detail maps
			bool normalMapIntensity: 1;//normalmap slider is being used
			bool occlusionIntensity: 1;//occlusion intensity is being used
			bool altSmoothnessTexture: 1;//if true, smoothness is taken from either metallic or specular alpha.

			//Render mode
			RenderMode renderMode: 2;

			//secondary uv index:
			uint32 secondaryUv: 4;
		};
		MaterialFingerprintId id;
	};

	void clear();

	FString getMatName() const;

	MaterialFingerprint(const JsonMaterial& src);
	MaterialFingerprint();
};
