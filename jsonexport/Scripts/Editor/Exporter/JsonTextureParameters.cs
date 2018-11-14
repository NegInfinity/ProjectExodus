using UnityEngine;
using UnityEditor;

namespace SceneExport{
	[System.Serializable]
	public class JsonTextureParameters: IFastJsonValue{
		public string name;
		public string imageHash;
		
		public bool initialized = false;
		public int anisoLevel;
		public string dimension;
		public string filterMode;
		public int width;
		public int height;
		public float mipMapBias;
		public Vector2 texelSize;
		public string wrapMode;
		public string wrapModeU;
		public string wrapModeV;
		public string wrapModeW;
	
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("name", name);
			writer.writeKeyVal("imageHash", imageHash);
		
			writer.writeKeyVal("initialized", initialized);
			writer.writeKeyVal("anisoLevel", anisoLevel);
			writer.writeKeyVal("dimension", dimension);
			writer.writeKeyVal("filterMode", filterMode);
			writer.writeKeyVal("width", width);
			writer.writeKeyVal("height", height);
			writer.writeKeyVal("mipMapBias", mipMapBias);
			writer.writeKeyVal("texelSize", texelSize);
			writer.writeKeyVal("wrapMode", wrapMode);
			writer.writeKeyVal("wrapModeU", wrapModeU);
			writer.writeKeyVal("wrapModeV", wrapModeV);
			writer.writeKeyVal("wrapModeW", wrapModeW);
			
			writer.endObject();
		}
		
		public JsonTextureParameters(){
		}
		
		public JsonTextureParameters(Texture tex){
			if (!tex)
				return;
			anisoLevel = tex.anisoLevel;
			dimension = tex.dimension.ToString();
			filterMode = tex.filterMode.ToString();
			height = tex.height;
			width = tex.width;
			height = tex.height;
			mipMapBias = tex.mipMapBias;
			texelSize = tex.texelSize;
			name = tex.name;
			wrapMode = tex.wrapMode.ToString();
			wrapModeU = tex.wrapModeU.ToString();
			wrapModeV = tex.wrapModeV.ToString();
			wrapModeW = tex.wrapModeW.ToString();
			
			imageHash = tex.imageContentsHash.ToString();
			
			initialized = true;			
		}
	}
}
