using UnityEngine;
using UnityEditor;

namespace SceneExport{
	[System.Serializable]
	public class JsonSpriteMetaData: IFastJsonValue{
		public string name;
		public Vector2 pivot;
		public Vector4 border;
		public int alignment;
		
		public JsonRect rect = new JsonRect();
	
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("name", name);
			writer.writeKeyVal("pivot", pivot);
			writer.writeKeyVal("border", border);
			writer.writeKeyVal("alignment", alignment);
			writer.writeKeyVal("rect", rect);
			writer.endObject();
		}
		
		public JsonSpriteMetaData(SpriteMetaData spriteData){
			name = spriteData.name;
			pivot = spriteData.pivot;
			border = spriteData.border;
			alignment = spriteData.alignment;
			rect = new JsonRect(spriteData.rect);
		}
	}
}