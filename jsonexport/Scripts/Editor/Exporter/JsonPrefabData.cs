using UnityEngine;
using UnityEditor;

namespace SceneExport{
	[System.Serializable]
	public class JsonPrefabData: IFastJsonValue{
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.endObject();
		}
		
		public JsonPrefabData(GameObject prefabObject){
		}
	}	
}
