using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

namespace SceneExport{
	[System.Serializable]
	public class JsonPrefabData: IFastJsonValue{
		public int id;
		public string name;
		public string path;
		public string guid;
		
		public string prefabType;
		
		public List<JsonGameObject> objects = new List<JsonGameObject>();
		
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("name", name);
			writer.writeKeyVal("path", path);
			writer.writeKeyVal("guid", guid);
			
			writer.writeKeyVal("prefabType", prefabType);
			
			writer.writeKeyVal("objects", objects);
			writer.endObject();
		}
		
		public JsonPrefabData(GameObject prefabObject, ResourceMapper resMap){
			id = resMap.getRootPrefabId(prefabObject, true);
			
			path = AssetDatabase.GetAssetPath(prefabObject);
			guid = AssetDatabase.AssetPathToGUID(path);
			//name = System.IO.Path.GetFileName(path);
			name = prefabObject.name;//looks like name of prefab mirrors that of a file, sans extension
			prefabType = PrefabUtility.GetPrefabType(prefabObject).ToString();
			
			var mapper = resMap.getPrefabObjectMapper(prefabObject);
			for(int i = 0; i < mapper.numObjects; i++){
				var src = mapper.getObject(i);
				var dst = new JsonGameObject(src, mapper, resMap);
				objects.Add(dst);
			}
		}
	}	
}
