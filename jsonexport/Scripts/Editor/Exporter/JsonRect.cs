using UnityEngine;
using UnityEditor;

namespace SceneExport{
	[System.Serializable]
	public class JsonRect: IFastJsonValue{
		public Vector2 min = Vector2.zero;
		public Vector2 max = Vector2.zero;	
		
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("min", min);
			writer.writeKeyVal("max", max);
			writer.endObject();
		}
		
		public JsonRect(){
		}
		
		public JsonRect(Rect rect){
			min = rect.min;
			max = rect.max;
		}
	}
}