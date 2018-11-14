using UnityEngine;
using UnityEditor;

namespace SceneExport{
	[System.Serializable]
	public class JsonCubemap: IFastJsonValue{
		public Cubemap cubemap = null;
		public int id = -1;
		public string name;
		public string exportPath;
		public string assetPath;
		public bool needConversion = false;
		
		public JsonTextureParameters texParams = new JsonTextureParameters();
		public JsonTextureImportParameters texImportParams = new JsonTextureImportParameters();
		
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("id", id);
			writer.writeKeyVal("name", name);
			writer.writeKeyVal("exportPath", exportPath);
			writer.writeKeyVal("assetPath", assetPath);
			writer.writeKeyVal("needConversion", needConversion);

			writer.endObject();
		}
		
		public JsonCubemap(Cubemap cubemap_, ResourceMapper resMap){
			cubemap = cubemap_;
			if (!cubemap){
				return;
			}
			id = resMap.findCubemapId(cubemap);
			assetPath = AssetDatabase.GetAssetPath(cubemap);
			
			var importer = AssetImporter.GetAtPath(assetPath);
			var texImporter = importer as TextureImporter;

			texParams = new JsonTextureParameters(cubemap);
			texImportParams = new JsonTextureImportParameters(texImporter);
		}
		
		public JsonCubemap(){
		}	
	}
}