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
			public bool useMetallic = false;
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

				albedoTex = getTexId("_MainTex");
				specularTex = getTexId("_SpecGlossMap");
				metallicTex= getTexId("_MetallicGlossMap");
				normalMapTex = getTexId("_BumpMap");
				occlusionTex = getTexId("_OcclusionMap");
				parallaxTex = getTexId("_ParallaxMap");
				emissionTex = getTexId("_EmissionMap");
				detailMaskTex = getTexId("_DetailMask");
				detailAlbedoTex = getTexId("_DetailAlbedoMap");
				detailNormalMapTex= getTexId("_DetailNormalMap");

				alphaCutoff = getFloat("_Cutoff", 1.0f);
				smoothness = getFloat("_Glossiness", 0.5f);
				specularColor = getColor("_SpecColor", Color.white);
				metallic = getFloat("_Metallic", 0.5f);
				bumpScale = getFloat("_BumpScale", 1.0f);
				parallaxScale = getFloat("_Parallax", 1.0f);
				occlusionStrength = getFloat("_OcclusionStrength", 1.0f);
				emissionColor = getColor("_EmissionColor", Color.black);
				detailMapScale = getFloat("_DetailNormalMapScale", 1.0f);
				secondaryUv = getFloat("_UVSec", 1.0f);
			}
		}
	}
}
