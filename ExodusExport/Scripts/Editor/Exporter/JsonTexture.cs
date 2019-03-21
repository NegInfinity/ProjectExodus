using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

namespace SceneExport{
	[System.Serializable]
	public class JsonTexture: IFastJsonValue{		
		public string name;
		public ResId id = ResId.invalid;
		public string path;
		public string filterMode;
		public float mipMapBias = 0.0f;
		public int width = 0;
		public int height = 0;
		public string format = "";
		public string wrapMode;
		public bool isTex2D = false;
		public bool isRenderTarget = false;
		public bool alphaTransparency = false;
		public float anisoLevel = 0.0f;
		public string base64;
		public bool importDataFound = false;
		public bool sRGB = true;
		public string textureType = "default";
		public bool normalMapFlag = false;
		
		public JsonTextureParameters texParams = new JsonTextureParameters();
		public JsonTextureImportParameters texImportParams = new JsonTextureImportParameters();

		public Texture textureRef = null;
			
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("name", name);
			writer.writeKeyVal("id", id);
			writer.writeKeyVal("path", path);
			writer.writeKeyVal("filterMode", filterMode);
			writer.writeKeyVal("mipMapBias", mipMapBias);
			writer.writeKeyVal("width", width);
			writer.writeKeyVal("height", height);
			writer.writeKeyVal("format", format);
			writer.writeKeyVal("wrapMode", wrapMode);
			writer.writeKeyVal("isTex2D", isTex2D);
			writer.writeKeyVal("isRenderTarget", isRenderTarget);
			writer.writeKeyVal("alphaTransparency", alphaTransparency);
			writer.writeKeyVal("anisoLevel", anisoLevel);
			writer.writeKeyVal("sRGB", sRGB);
			writer.writeKeyVal("normalMapFlag", normalMapFlag);
			writer.writeKeyVal("importDataFound", importDataFound);
			writer.writeKeyVal("textureType", textureType);
			
			writer.writeKeyVal("texParams", texParams);
			writer.writeKeyVal("texImportParams", texImportParams);
			
			writer.endObject();
		}
			
		public JsonTexture(Texture tex, ResourceMapper resMap){
			name = tex.name;
			id = resMap.findTextureId(tex);//exp.textures.findId(tex);
			var assetPath = AssetDatabase.GetAssetPath(tex);
			resMap.registerAssetPath(assetPath);
			path = assetPath;
			filterMode = tex.filterMode.ToString();
			width = tex.width;
			height = tex.height;
			wrapMode = tex.wrapMode.ToString();				
			var tex2D = tex as Texture2D;
			var rendTarget = tex as RenderTexture;
			isTex2D = tex2D != null;
			isRenderTarget = rendTarget != null;
			var importer = AssetImporter.GetAtPath(assetPath);
			var texImporter = importer as TextureImporter;//(TextureImporter)importer;
			if (isTex2D){
				alphaTransparency = tex2D.alphaIsTransparency;
				format = tex2D.format.ToString();
			}
			if (isRenderTarget){
				anisoLevel = rendTarget.anisoLevel;
			}
			if (texImporter){
				importDataFound = true;
				sRGB = texImporter.sRGBTexture;
				textureType = texImporter.textureType.ToString();
				normalMapFlag = (texImporter.textureType == TextureImporterType.NormalMap);
			}
			
			texParams = new JsonTextureParameters(tex);
			texImportParams = new JsonTextureImportParameters(texImporter);
			
			textureRef = tex;
		}
	}
}