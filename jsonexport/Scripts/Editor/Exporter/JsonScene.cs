using UnityEngine;
using UnityEditor;
using System.Collections.Generic;
using System.Linq;
using UnityEngine.SceneManagement;

namespace SceneExport{
	[System.Serializable]
	public class JsonScene: IFastJsonValue{
		public List<JsonGameObject> objects = new List<JsonGameObject>();
		public List<JsonMaterial> materials = new List<JsonMaterial>();
		public List<JsonMesh> meshes = new List<JsonMesh>();
		public List<JsonTexture> textures = new List<JsonTexture>();
		public List<string> resources = new List<string>();
			
		public static JsonScene fromScene(Scene scene, ResourceMapper resMap){
			var rootObjects = scene.GetRootGameObjects();
			return fromObjects(rootObjects, resMap);
		}
		
		public static JsonScene fromObject(GameObject arg, ResourceMapper resMap){
			return fromObjects(new GameObject[]{arg}, resMap);
		}
		
		public static JsonScene fromObjects(GameObject[] args, ResourceMapper resMap){
			var result = new JsonScene();
			
			foreach(var cur in args){
				if (!cur)
					continue;
				resMap.getObjectId(cur);
			}
			
			for(int i = 0; i < resMap.objects.objectList.Count; i++){
				/*TODO: This is very awkward, as constructor adds more data to the exporter
				Should be split into two methods.*/
				result.objects.Add(new JsonGameObject(resMap.objects.objectList[i], resMap));
			}
			
			return result;
		}
			
		void saveResources(string baseFilename){
			var targetDir = System.IO.Path.GetDirectoryName(System.IO.Path.GetFullPath(baseFilename));
			if (!Application.isEditor){
				throw new System.ArgumentException("The application is not running in editor mode");
			}
				
			var dataPath = System.IO.Path.GetFullPath(Application.dataPath);
			var projectPath = System.IO.Path.GetDirectoryName(dataPath);
			Debug.LogFormat("data Path: {0}", dataPath);
			Debug.LogFormat("projectPath: {0}", projectPath);
			Debug.LogFormat("targetDir: {0}", targetDir);
				
			if (projectPath == targetDir){
				Debug.LogWarningFormat("You're saving into project directory, files will not be copied");
				return;
			}
				
			foreach(var curTex in textures){
				TextureUtility.copyTexture(curTex, targetDir, projectPath);
			}
		}
			
		public void saveToFile(string filename, bool saveResourceFiles = false){
			Utility.saveStringToFile(filename, toJsonString());
			if (!saveResourceFiles)
				return;
			saveResources(filename);
		}
			
		public void writeJsonObjectFields(FastJsonWriter writer){
			writer.writeKeyVal("objects", objects);
			/*
			writer.beginKeyArray("objects");
			foreach(var cur in objects){
				writer.writeValue(cur);
				//cur.writeRawJsonValue(writer);
			}
			writer.endArray();
			*/
			writer.writeKeyVal("materials", materials);
			/*
			writer.beginKeyArray("materials");
			foreach(var cur in materials){
				writer.writeValue(cur);
				//cur.writeRawJsonValue(writer);
			}
			writer.endArray();
			*/
			writer.writeKeyVal("meshes", meshes);
			/*
			writer.beginKeyArray("meshes");
			foreach(var cur in meshes){
				writer.writeValue(cur);
				//cur.writeRawJsonValue(writer);
			}
			writer.endArray();
			*/
			writer.writeKeyVal("textures", textures);
			/*
			writer.beginKeyArray("textures");
			foreach(var cur in textures){
				writer.writeValue(cur);
				//cur.writeRawJsonValue(writer);
			}
			writer.endArray();
			*/
			writer.writeKeyVal("resources", resources);
			/*
			writer.beginKeyArray("resources");
			foreach(var cur in resources){
				writer.writeValue(cur);
			}
			writer.endArray();
			*/
		}
			
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writeJsonObjectFields(writer);
			writer.endObject();
		}

		public string toJsonString(){
			FastJsonWriter writer = new FastJsonWriter();
			writer.beginDocument();
			writeJsonObjectFields(writer);
			writer.endDocument();
			return writer.getString();
		}
			
		public void fixNameClashes(){
			var nameClashes = new Dictionary<NameClashKey, List<int>>();
			for(int i = 0; i < objects.Count; i++){
				var cur = objects[i];
				var key = new NameClashKey(cur.name, cur.parent);
				var idList = nameClashes.getValOrGenerate(key, (parId_) => new List<int>());
				idList.Add(cur.id);
			}
			
			foreach(var entry in nameClashes){
				var key = entry.Key;
				var list = entry.Value;
				if ((list == null) || (list.Count <= 1))
					continue;

				for(int i = 1; i < list.Count; i++){
					var curId = list[i];
					if ((curId <= 0) || (curId >= objects.Count)){
						Debug.LogErrorFormat("Invalid object id {0}, while processing name clash {1};\"{2}\"", 
							curId, key.parentId, key.name);
						continue;
					}
					var curObj = objects[curId];
					var altName = string.Format("{0}-#{1}", key.name, i);
					while(nameClashes.ContainsKey(new NameClashKey(altName, key.parentId))){
						altName = string.Format("{0}-#{1}({2})", 
							key.name, i, System.Guid.NewGuid().ToString("n"));
						//break;
					}
					curObj.nameClash = true;
					curObj.uniqueName = altName;
				}								
			}			
		}
	}
}