using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

namespace SceneExport{
	[System.Serializable]
	public class JsonMaterial: IFastJsonValue{
		public ResId id = ResId.invalid;
		public int renderQueue = 0;
		public string name;
		public string path;
		public string shader;
		
		public bool supportedShader = true;
		public int blendMode = 0; /*
									Opaque, Cutout, Fade, Transparent. 
									Risky. Judging by shader source we also need to handle names, for example...
									Name containing "/Transparent/Cutout/" or "/Transparent/"													
								*/
		
		//public string renderType;
		public ResId mainTexture = ResId.invalid;
		public Vector2 mainTextureOffset = Vector2.zero;
		public Vector2 mainTextureScale = Vector2.one;
		
		
		public Color color = Color.white;

		public bool useNormalMap = false;
		public bool useAlphaTest = false;
		public bool useAlphaBlend = false;
		public bool useAlphaPremultiply = false;
		public bool useEmission = false;
		public bool useParallax = false;
		public bool useDetailMap = false;
		public bool useMetallic = false;//this is unrealiable
			
		public bool hasMetallic = false;
		public bool hasSpecular = false;
		public bool hasEmissionColor = false;
		public bool hasEmission = false;
			
		public bool useSpecular = false;
		public ResId albedoTex = ResId.invalid;
		public ResId specularTex = ResId.invalid;
		public ResId metallicTex = ResId.invalid;
		public ResId normalMapTex = ResId.invalid;
		public ResId occlusionTex = ResId.invalid;
		public ResId parallaxTex = ResId.invalid;
		public ResId emissionTex = ResId.invalid;
		public ResId detailMaskTex = ResId.invalid;
		public ResId detailAlbedoTex = ResId.invalid;
		public ResId detailNormalMapTex = ResId.invalid;
		
		public float alphaCutoff = 1.0f;
		public float smoothness = 0.5f;
		public float smoothnessScale = 1.0f;
		public Color specularColor = Color.white;
		public float metallic = 0.5f;
		public float bumpScale = 1.0f;
		public float parallaxScale = 1.0f;
		public float occlusionStrength = 1.0f;
		public Color emissionColor = Color.black;
		public float detailMapScale = 1.0f;
		public Vector2 detailAlbedoOffset = Vector2.zero;
		public Vector2 detailAlbedoScale = Vector2.one;
		public float detailNormalMapScale = 1.0f;
		public float secondaryUv = 1.0f;
		
		public int smoothnessMapChannel = 0;
		
		public float specularHighlights = 1.0f;
		public float glossyReflections = 1.0f;
		
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("id", id);
			writer.writeKeyVal("renderQueue", renderQueue);
			writer.writeKeyVal("name", name);
			writer.writeKeyVal("path", path);
			writer.writeKeyVal("shader", shader);
			
			writer.writeKeyVal("blendMode", blendMode);
			
			writer.writeKeyVal("supportedShader", supportedShader);
			
			//writer.writeKeyVal("renderType", cur.renderType);
			writer.writeKeyVal("mainTexture", mainTexture);
			writer.writeKeyVal("mainTextureOffset", mainTextureOffset);
			writer.writeKeyVal("mainTextureScale", mainTextureScale);
			writer.writeKeyVal("color", color);
			writer.writeKeyVal("useNormalMap", useNormalMap);
			writer.writeKeyVal("useAlphaTest", useAlphaTest);
			writer.writeKeyVal("useAlphaBlend", useAlphaBlend);
			writer.writeKeyVal("useAlphaPremultiply", useAlphaPremultiply);
			writer.writeKeyVal("useEmission", useEmission);
			writer.writeKeyVal("useParallax", useParallax);
			writer.writeKeyVal("useDetailMap", useDetailMap);
			writer.writeKeyVal("useMetallic", useMetallic);
				
			writer.writeKeyVal("hasMetallic", hasMetallic);
			writer.writeKeyVal("hasSpecular", hasSpecular);
			writer.writeKeyVal("hasEmission", hasEmission);
			writer.writeKeyVal("hasEmissionColor", hasEmissionColor);
				
			writer.writeKeyVal("useSpecular", useSpecular);
			writer.writeKeyVal("albedoTex", albedoTex);
			writer.writeKeyVal("specularTex", specularTex);
			writer.writeKeyVal("metallicTex", metallicTex);
			writer.writeKeyVal("normalMapTex", normalMapTex);
			writer.writeKeyVal("occlusionTex", occlusionTex);
			writer.writeKeyVal("parallaxTex", parallaxTex);
			writer.writeKeyVal("emissionTex", emissionTex);
			writer.writeKeyVal("detailMaskTex", detailMaskTex);
			writer.writeKeyVal("detailAlbedoTex", detailAlbedoTex);
			writer.writeKeyVal("detailAlbedoOffset", detailAlbedoOffset);
			writer.writeKeyVal("detailAlbedoScale", detailAlbedoScale);
			writer.writeKeyVal("detailNormalMapScale", detailNormalMapScale);
			
			writer.writeKeyVal("detailNormalMapTex", detailNormalMapTex);
			writer.writeKeyVal("alphaCutoff", alphaCutoff);
			writer.writeKeyVal("smoothness", smoothness);
			writer.writeKeyVal("smoothnessScale", smoothnessScale);
			
			writer.writeKeyVal("specularColor", specularColor);
			writer.writeKeyVal("metallic", metallic);
			writer.writeKeyVal("bumpScale", bumpScale);
			writer.writeKeyVal("parallaxScale", parallaxScale);
			writer.writeKeyVal("occlusionStrength", occlusionStrength);
			writer.writeKeyVal("emissionColor", emissionColor);
			writer.writeKeyVal("detailMapScale", detailMapScale);
			writer.writeKeyVal("secondaryUv", secondaryUv);
			
			writer.writeKeyVal("smoothnessMapChannel", smoothnessMapChannel);
			writer.writeKeyVal("specularHighlights", specularHighlights);
			writer.writeKeyVal("glossyReflections", glossyReflections);
			
			writer.endObject();			
		}

		public static class TexParamNames{
			public static readonly string main = "_MainTex";
			public static readonly string specular = "_SpecGlossMap";
			public static readonly string metallic= "_MetallicGlossMap";
			public static readonly string normal = "_BumpMap";
			public static readonly string occlusion = "_OcclusionMap";
			public static readonly string parallax = "_ParallaxMap";
			public static readonly string emission = "_EmissionMap";
			public static readonly string detailMask = "_DetailMask";
			public static readonly string detailAlbedo = "_DetailAlbedoMap";
			public static readonly string detailNormal = "_DetailNormalMap";
		}
		
		static bool isSupportedShaderName(string name){
			return (name == "Standard") || (name == "Standard (Specular setup)");
		}
		
		static void registerLinkedTex(Material mat, string paramName, ResourceMapper resMap){
			if (!mat.HasProperty(paramName))
				return;
			var tex = mat.GetTexture(paramName);
			if (!tex)
				return;
			resMap.registerTexture(tex);
		}
		
		public static void registerLinkedData(Material mat, ResourceMapper resMap){
			if (!mat)
				return;
			registerLinkedTex(mat, TexParamNames.main, resMap);	
			registerLinkedTex(mat, TexParamNames.specular, resMap);	
			registerLinkedTex(mat, TexParamNames.metallic, resMap);	
			registerLinkedTex(mat, TexParamNames.normal, resMap);	
			registerLinkedTex(mat, TexParamNames.occlusion, resMap);	
			registerLinkedTex(mat, TexParamNames.parallax, resMap);	
			registerLinkedTex(mat, TexParamNames.emission, resMap);	
			registerLinkedTex(mat, TexParamNames.detailMask, resMap);	
			registerLinkedTex(mat, TexParamNames.detailAlbedo, resMap);	
			registerLinkedTex(mat, TexParamNames.detailNormal, resMap);	
		}		
		
		static ResId getTexId(Material mat, string texName, ResourceMapper resMap){
			if (!mat.HasProperty(texName))
				return ResId.invalid;
			return resMap.getTextureId(mat.GetTexture(texName));
		}

		static float getFloat(Material mat, string paramName, float defaultVal){
			if (!mat.HasProperty(paramName))
				return defaultVal;
			return mat.GetFloat(paramName);
		}
		
		static Color getColor(Material mat, string paramName, Color defaultVal){
			if (!mat.HasProperty(paramName))
				return defaultVal;
			return mat.GetColor(paramName);
		}
		
		static Vector2 getTextureScale(Material mat, string paramName){
			if (!mat.HasProperty(paramName))
				return Vector2.one;
			return mat.GetTextureScale(paramName);
		}
		
		static Vector2 getTextureOffset(Material mat, string paramName){
			if (!mat.HasProperty(paramName))
				return Vector2.zero;
			return mat.GetTextureOffset(paramName);
		}
		
		public static class Keywords{
			public static readonly string normalMap = "_NORMALMAP";
			public static readonly string alphaTestOn = "_ALPHATEST_ON";
			public static readonly string alphaBlendOn = "_ALPHABLEND_ON";
			public static readonly string alphaPremultiplyOn = "_ALPHAPREMULTIPLY_ON";
			public static readonly string emission = "_EMISSION";
			public static readonly string parallaxMap = "_PARALLAXMAP";
			public static readonly string detailMulX2 = "_DETAIL_MULX2";
			public static readonly string metallicGlossMap = "_METALLICGLOSSMAP";
			public static readonly string specGlossMap = "_SPECCGLOSSMAP";
		}
		
		public static class ParamNames{
			public static readonly string metallic = "_Metallic";
			public static readonly string mainColor = "_Color";
			public static readonly string specularColor = "_SpecColor";
			public static readonly string emissionColor = "_EmissionColor";
			
			public static readonly string blendMode = "_Mode";
			
			public static readonly string detailNormalMapScale = "_DetailNormalMapScale";

			public static readonly string cutoff =  "_Cutoff";
			public static readonly string glossiness = "_Glossiness";
			public static readonly string glossinessScale = "_GlossMapScale";//I'm not sure at which unity version has this one appeared.
			public static readonly string bumpScale = "_BumpScale";
			public static readonly string parallax = "_Parallax";
			public static readonly string occlusionStrength = "_OcclusionStrength";
			public static readonly string secondaryUv = "_UVSec";
			
			public static readonly string smoothnessMapChannel = "_SmoothnessTextureChannel";
			public static readonly string specularHighlights = "_SpecularHighlights";
			public static readonly string glossyReflections = "_GlossyReflections";
		}
		
		public JsonMaterial(Material mat, ResourceMapper resMap){
			name = mat.name;
			//TODO: Further investigation shows that this is likely going to return -1 for all new materials.
			id = resMap.findMaterialId(mat);//exp.materials.findId(mat);
			renderQueue = mat.renderQueue;
			var path = AssetDatabase.GetAssetPath(mat);
			this.path = path;
			shader = mat.shader.name;
			
			supportedShader = isSupportedShaderName(shader);
			
			mainTexture = resMap.getTextureId(mat.mainTexture);
			mainTextureOffset = mat.mainTextureOffset;
			mainTextureScale = mat.mainTextureScale;
			color = getColor(mat, ParamNames.mainColor, Color.white);//mat.color;

			useNormalMap = mat.IsKeywordEnabled(Keywords.normalMap);//"_NORMALMAP");
			useAlphaTest = mat.IsKeywordEnabled(Keywords.alphaTestOn);//"_ALPHATEST_ON");
			useAlphaBlend = mat.IsKeywordEnabled(Keywords.alphaBlendOn);//"_ALPHABLEND_ON");
			useAlphaPremultiply = mat.IsKeywordEnabled(Keywords.alphaPremultiplyOn);//"_ALPHAPREMULTIPLY_ON");
			useEmission = mat.IsKeywordEnabled(Keywords.emission);//"_EMISSION");
			useParallax = mat.IsKeywordEnabled(Keywords.parallaxMap);//"_PARALLAXMAP");
			useDetailMap = mat.IsKeywordEnabled(Keywords.detailMulX2);//"_DETAIL_MULX2");
			useMetallic = mat.IsKeywordEnabled(Keywords.metallicGlossMap);//"_METALLICGLOSSMAP");
			useSpecular = mat.IsKeywordEnabled(Keywords.specGlossMap);//"_SPECCGLOSSMAP");

			albedoTex = getTexId(mat, TexParamNames.main, resMap);
			specularTex = getTexId(mat, TexParamNames.specular, resMap);
			metallicTex = getTexId(mat, TexParamNames.metallic, resMap);
			normalMapTex = getTexId(mat, TexParamNames.normal, resMap);
			occlusionTex = getTexId(mat, TexParamNames.occlusion, resMap);
			parallaxTex = getTexId(mat, TexParamNames.parallax, resMap);
			emissionTex = getTexId(mat, TexParamNames.emission, resMap);
			detailMaskTex = getTexId(mat, TexParamNames.detailMask, resMap);
			detailAlbedoTex = getTexId(mat, TexParamNames.detailAlbedo, resMap);
			detailNormalMapTex = getTexId(mat, TexParamNames.detailNormal, resMap);
			
			detailAlbedoScale = getTextureScale(mat, TexParamNames.detailAlbedo);//mat.GetTextureScale(TexParamNames.detailAlbedo);
			detailAlbedoOffset = getTextureOffset(mat, TexParamNames.detailAlbedo);//mat.GetTextureOffset(TexParamNames.detailAlbedo);
			
			detailNormalMapScale = getFloat(mat, ParamNames.detailNormalMapScale, 1.0f);

			alphaCutoff = getFloat(mat, ParamNames.cutoff, 1.0f);
			smoothness = getFloat(mat, ParamNames.glossiness, 0.5f);
			smoothnessScale = getFloat(mat, ParamNames.glossinessScale, 1.0f);
			specularColor = getColor(mat, ParamNames.specularColor, Color.white);
			metallic = getFloat(mat, ParamNames.metallic, 0.5f);
			bumpScale = getFloat(mat, ParamNames.bumpScale, 1.0f);
			parallaxScale = getFloat(mat, ParamNames.parallax, 1.0f);
			occlusionStrength = getFloat(mat, ParamNames.occlusionStrength, 1.0f);
			
			emissionColor = getColor(mat, ParamNames.emissionColor, Color.black);
			detailMapScale = getFloat(mat, ParamNames.detailNormalMapScale, 1.0f);
			secondaryUv = getFloat(mat, ParamNames.secondaryUv, 1.0f);
			
			blendMode = Mathf.RoundToInt(getFloat(mat, ParamNames.blendMode, 0.0f));
			
			smoothnessMapChannel = (int)getFloat(mat, ParamNames.smoothnessMapChannel, 0.0f);
			specularHighlights = getFloat(mat, ParamNames.specularHighlights, 1.0f);
			glossyReflections = getFloat(mat, ParamNames.glossyReflections, 1.0f);
				
			hasMetallic = mat.HasProperty(ParamNames.metallic) && mat.HasProperty(TexParamNames.metallic);
			hasSpecular = mat.HasProperty(TexParamNames.specular) && mat.HasProperty(ParamNames.specularColor);
			hasEmissionColor = mat.HasProperty(ParamNames.emissionColor) && (emissionColor.maxColorComponent > 0.01f);
			hasEmission = hasEmissionColor || ExportUtility.isValidId(emissionTex);//(emissionTex >= 0);
		}
	}
}
