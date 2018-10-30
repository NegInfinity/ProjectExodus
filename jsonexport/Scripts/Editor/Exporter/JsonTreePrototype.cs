using UnityEditor;
using UnityEngine;
using System.Collections.Generic;

namespace SceneExport{
	[System.Serializable]
	public class JsonTreePrototype: IFastJsonValue{		
		public TreePrototype src;
		public int prefabId = -1;
		public int prefabObjectId = -1;
		public int meshId = -1;//trees don't exactly spawn entire prefabs.. They use meshes.
		public List<int> meshMaterials = new List<int>();
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("prefabId", prefabId);
			writer.writeKeyVal("prefabObjectId", prefabObjectId);
			writer.writeKeyVal("meshId", meshId);
			writer.writeKeyVal("meshMaterials", meshMaterials);
			writer.endObject();
		}
		
		public JsonTreePrototype(TreePrototype src_, ResourceMapper resMap){
			src = src_;
			prefabId = resMap.getRootPrefabId(src.prefab, true);
			prefabObjectId = resMap.getPrefabObjectId(src.prefab, true);
			meshId = -1;
			meshMaterials.Clear();
			
			var srcObj = src.prefab as GameObject;
			if (!srcObj)
				return;
			var filter = srcObj.GetComponent<MeshFilter>();
			if (filter && filter.sharedMesh){
				meshId = resMap.getMeshId(filter.sharedMesh);
			}
			var renderer = srcObj.GetComponent<Renderer>();
			if (renderer){
				foreach(var cur in renderer.sharedMaterials){
					meshMaterials.Add(resMap.getMaterialId(cur));
				}
			}
		}
	}	
}
