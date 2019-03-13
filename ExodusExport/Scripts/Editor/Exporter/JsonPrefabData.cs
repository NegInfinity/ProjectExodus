using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

namespace SceneExport{
	[System.Serializable]
	public class JsonPrefabData: IFastJsonValue{
		public ResId id = ResId.invalid;
		public string name;
		public string path;
		public string guid;
		
		//public string prefabType;
		public string prefabAssetType;
		public string prefabInstanceStatus;
		
		public List<JsonGameObject> objects = new List<JsonGameObject>();
		
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("name", name);
			writer.writeKeyVal("path", path);
			writer.writeKeyVal("guid", guid);
			
			//writer.writeKeyVal("prefabType", prefabType);
			writer.writeKeyVal("prefabAssetType", prefabAssetType);
			writer.writeKeyVal("prefabInstanceStatus", prefabInstanceStatus);
			
			writer.writeKeyVal("objects", objects);
			writer.endObject();
		}
		
		public JsonPrefabData(GameObject prefabObject, ResourceMapper resMap){
			id = resMap.getRootPrefabId(prefabObject, true);
			
			path = AssetDatabase.GetAssetPath(prefabObject);
			guid = AssetDatabase.AssetPathToGUID(path);
			//name = System.IO.Path.GetFileName(path);
			name = prefabObject.name;//looks like name of prefab mirrors that of a file, sans extension
			//prefabType = PrefabUtility.GetPrefabType(prefabObject).ToString();
			prefabAssetType = PrefabUtility.GetPrefabAssetType(prefabObject).ToString();
			prefabInstanceStatus = PrefabUtility.GetPrefabInstanceStatus(prefabObject).ToString();
			
			var mapper = resMap.getPrefabObjectMapper(prefabObject);
			for(int i = 0; i < mapper.numObjects; i++){
				///Well, this is certainly not the best way to go about it...
				var src = mapper.getObjectByIndex(i);
				var dst = new JsonGameObject(src, mapper, resMap);
				objects.Add(dst);
			}
		}
	}	
}
