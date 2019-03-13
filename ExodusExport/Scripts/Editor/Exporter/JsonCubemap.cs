using UnityEngine;
using UnityEditor;

namespace SceneExport{
	[System.Serializable]
	public class JsonCubemap: IFastJsonValue{
		public Cubemap cubemap = null;
		public ResId id = ResId.invalid;
		public string name;
		public string exportPath;
		public string rawPath;
		public string assetPath;
		public bool needConversion = false;
		public string format = "";
		public bool isHdr = false;
		
		public JsonTextureParameters texParams = new JsonTextureParameters();
		public JsonTextureImportParameters texImportParams = new JsonTextureImportParameters();
		
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("id", id);
			writer.writeKeyVal("name", name);
			writer.writeKeyVal("exportPath", exportPath);
			writer.writeKeyVal("rawPath", rawPath);
			writer.writeKeyVal("assetPath", assetPath);
			writer.writeKeyVal("needConversion", needConversion);
			writer.writeKeyVal("isHdr", isHdr);
			writer.writeKeyVal("format", format);
			
			writer.writeKeyVal("texParams", texParams);
			writer.writeKeyVal("texImportParams", texImportParams);

			writer.endObject();
		}
		
		public JsonCubemap(Cubemap cubemap_, ResourceMapper resMap){
			cubemap = cubemap_;
			if (!cubemap){
				return;
			}
			id = resMap.findCubemapId(cubemap);
			assetPath = AssetDatabase.GetAssetPath(cubemap);
			name = cubemap.name;
			
			var importer = AssetImporter.GetAtPath(assetPath);
			var texImporter = importer as TextureImporter;

			texParams = new JsonTextureParameters(cubemap);
			texImportParams = new JsonTextureImportParameters(texImporter);
			
			needConversion = true;
			/*
			if (TextureUtility.isSupportedTexExtension(assetPath))
				needConversion = true;
			if (texImportParams.initialized && texImportParams.importer){
				if (texImportParams.importer.generateCubemap != TextureImporterGenerateCubemap.FullCubemap){
					needConversion = true;
				}
				//huh.... I don't really have a way to get original texture dimensiosn, it looks like it. Oh well?
			}
			*/
			
			format = cubemap.format.ToString();
			isHdr = TextureUtility.isHdrFormat(cubemap.format);
			
			exportPath = assetPath;
			rawPath = "";
			if (needConversion){
				exportPath = System.IO.Path.ChangeExtension(assetPath, ".png");
				rawPath = System.IO.Path.ChangeExtension(assetPath, ".raw");
			}
		}
		
		public JsonCubemap(){
		}	
	}
}