using UnityEngine;
using UnityEditor;

namespace SceneExport{
	[System.Serializable]
	public class JsonSplatPrototype: IFastJsonValue{
		public SplatPrototype data;
		public ResId normalMapId = ResId.invalid;
		public ResId textureId = ResId.invalid;
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("textureId", textureId);
			writer.writeKeyVal("normalMapId", normalMapId);
			
			writer.writeKeyVal("metallic", data.metallic);
			writer.writeKeyVal("smoothness", data.smoothness);
			writer.writeKeyVal("specular", data.specular);
			writer.writeKeyVal("tileOffset", data.tileOffset);
			writer.writeKeyVal("tileSize", data.tileSize);
			writer.endObject();
		}
		
		public JsonSplatPrototype(SplatPrototype data_, ResourceMapper resMap){
			data = data_;
			textureId = resMap.getTextureId(data.texture);
			normalMapId = resMap.getTextureId(data.normalMap);
		}
	}
}