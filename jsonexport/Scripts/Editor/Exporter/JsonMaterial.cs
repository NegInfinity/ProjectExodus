using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

namespace SceneExport{
	public partial class Exporter{
		[System.Serializable]
		public class JsonMaterial{
			public int id = -1;
			public int renderQueue = 0;
			public string name;
			public string path;
			public string shader;
			//public string renderType;
			public int mainTexture = -1;
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
			public int albedoTex = -1;
			public int specularTex = -1;
			public int metallicTex = -1;
			public int normalMapTex = -1;
			public int occlusionTex = -1;
			public int parallaxTex = -1;
			public int emissionTex = -1;
			public int detailMaskTex = -1;
			public int detailAlbedoTex = -1;
			public int detailNormalMapTex = -1;
			public float alphaCutoff = 1.0f;
			public float smoothness = 0.5f;
			public Color specularColor = Color.white;
			public float metallic = 0.5f;
			public float bumpScale = 1.0f;
			public float parallaxScale = 1.0f;
			public float occlusionStrength = 1.0f;
			public Color emissionColor = Color.black;
			public float detailMapScale = 1.0f;
			public float secondaryUv = 1.0f;
			
			public void writeJsonValue(FastJsonWriter writer){
				writer.beginObjectValue();
				writer.writeKeyVal("id", id);
				writer.writeKeyVal("renderQueue", renderQueue);
				writer.writeKeyVal("name", name);
				writer.writeKeyVal("path", path);
				writer.writeKeyVal("shader", shader);
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
				writer.writeKeyVal("detailNormalMapTex", detailNormalMapTex);
				writer.writeKeyVal("alphaCutoff", alphaCutoff);
				writer.writeKeyVal("smoothness", smoothness);
				writer.writeKeyVal("specularColor", specularColor);
				writer.writeKeyVal("metallic", metallic);
				writer.writeKeyVal("bumpScale", bumpScale);
				writer.writeKeyVal("parallaxScale", parallaxScale);
				writer.writeKeyVal("occlusionStrength", occlusionStrength);
				writer.writeKeyVal("emissionColor", emissionColor);
				writer.writeKeyVal("detailMapScale", detailMapScale);
				writer.writeKeyVal("secondaryUv", secondaryUv);
				writer.endObject();			
			}

			public JsonMaterial(Material mat, Exporter exp){
				name = mat.name;
				id = exp.materials.findId(mat);
				renderQueue = mat.renderQueue;
				var path = AssetDatabase.GetAssetPath(mat);
				this.path = path;
				shader = mat.shader.name;
				mainTexture = exp.getTextureId(mat.mainTexture);
				mainTextureOffset = mat.mainTextureOffset;
				mainTextureScale = mat.mainTextureScale;
				color = mat.color;

				useNormalMap = mat.IsKeywordEnabled("_NORMALMAP");

				useAlphaTest = mat.IsKeywordEnabled("_ALPHATEST_ON");
				useAlphaBlend = mat.IsKeywordEnabled("_ALPHABLEND_ON");
				useAlphaPremultiply = mat.IsKeywordEnabled("_ALPHAPREMULTIPLY_ON");
				useEmission = mat.IsKeywordEnabled("_EMISSION");
				useParallax = mat.IsKeywordEnabled("_PARALLAXMAP");
				useDetailMap = mat.IsKeywordEnabled("_DETAIL_MULX2");
				useMetallic = mat.IsKeywordEnabled("_METALLICGLOSSMAP");
				useSpecular = mat.IsKeywordEnabled("_SPECCGLOSSMAP");

				System.Func<string, int> getTexId = (texName) => {
					if (!mat.HasProperty(texName))
						return -1;
					return exp.getTextureId(mat.GetTexture(texName));
				};
				System.Func<string, float, float> getFloat = (paramName, defaultVal) => {
					if (!mat.HasProperty(paramName))
						return defaultVal;
					return mat.GetFloat(paramName);
				};
				System.Func<string, Color, Color> getColor = (paramName, defaultVal) => {
					if (!mat.HasProperty(paramName))
						return defaultVal;
					return mat.GetColor(paramName);
				};
				
				var metallicParamName = "_Metallic";
				var metallicTexParamName = "_MetallicGlossMap";
				var specParamName = "_SpecColor";
				var specTexParamName = "_SpecGlossMap";
				var emissionTexParamName = "_EmissionMap";
				var emissionParamName = "_EmissionColor";

				albedoTex = getTexId("_MainTex");
				specularTex = getTexId(specTexParamName);
				metallicTex= getTexId(metallicTexParamName);
				normalMapTex = getTexId("_BumpMap");
				occlusionTex = getTexId("_OcclusionMap");
				parallaxTex = getTexId("_ParallaxMap");
				emissionTex = getTexId(emissionTexParamName);
				detailMaskTex = getTexId("_DetailMask");
				detailAlbedoTex = getTexId("_DetailAlbedoMap");
				detailNormalMapTex= getTexId("_DetailNormalMap");

				alphaCutoff = getFloat("_Cutoff", 1.0f);
				smoothness = getFloat("_Glossiness", 0.5f);
				specularColor = getColor(specParamName, Color.white);
				metallic = getFloat(metallicParamName, 0.5f);
				bumpScale = getFloat("_BumpScale", 1.0f);
				parallaxScale = getFloat("_Parallax", 1.0f);
				occlusionStrength = getFloat("_OcclusionStrength", 1.0f);
				emissionColor = getColor(emissionParamName, Color.black);
				detailMapScale = getFloat("_DetailNormalMapScale", 1.0f);
				secondaryUv = getFloat("_UVSec", 1.0f);
				
				hasMetallic = mat.HasProperty(metallicParamName) && mat.HasProperty(metallicTexParamName);
				hasSpecular = mat.HasProperty(specTexParamName) && mat.HasProperty(specParamName);
				hasEmissionColor = mat.HasProperty(emissionParamName) && (emissionColor.maxColorComponent > 0.01f);
				hasEmission = hasEmissionColor || (emissionTex >= 0);
			}
		}
	}
}
