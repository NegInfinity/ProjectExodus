using UnityEditor;
using UnityEngine;

namespace SceneExport{
	[System.Serializable]
	public class JsonBounds: IFastJsonValue{
		public Vector3 center;
		public Vector3 size;
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("center", center);
			writer.writeKeyVal("size", size);
			writer.endObject();
		}
		
		public JsonBounds(){
			center = Vector3.zero;
			size = Vector3.zero;
		}
		
		public JsonBounds(Bounds bounds){
			center = bounds.center;
			size = bounds.size;
		}
	}
}