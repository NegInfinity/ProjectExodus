using UnityEditor;
using UnityEngine;
using System.Collections.Generic;

namespace SceneExport{
	[System.Serializable]
	public class JsonTreePrototype: IFastJsonValue{		
		public TreePrototype src;
		public int prefabId = -1;
		public int prefabObjectId = -1;
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("prefabId", prefabId);
			writer.writeKeyVal("prefabObjectId", prefabObjectId);
			writer.endObject();
		}
		
		public JsonTreePrototype(TreePrototype src_, ResourceMapper resMap){
			src = src_;
			prefabId = resMap.getRootPrefabId(src.prefab, true);
			prefabObjectId = resMap.getPrefabObjectId(src.prefab, true);
		}
	}	
}
