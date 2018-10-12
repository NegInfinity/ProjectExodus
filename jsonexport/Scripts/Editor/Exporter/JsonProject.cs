using UnityEngine;
using UnityEditor;
using System.Collections.Generic;
using UnityEngine.SceneManagement;

namespace SceneExport{
	[System.Serializable]
	public class JsonProject: JsonValueObject{
		public JsonProjectConfig config = new JsonProjectConfig();
		public List<JsonScene> scenes = new List<JsonScene>();
		public JsonResourceList resourceList = new JsonResourceList();
		public ResourceMapper resourceMapper = new ResourceMapper();
		public override void writeJsonObjectFields(FastJsonWriter writer){
			writer.writeKeyVal("config", config);
			writer.writeKeyVal("scenes", scenes);
			writer.writeKeyVal("resources", resourceList);
		}

		public static JsonProject fromObject(GameObject obj){
			return fromObjects(new GameObject[]{obj});
		}		
		
		public void generateResourceList(){
			resourceList = resourceMapper.makeResourceList();
		}
		
		public static JsonProject fromObjects(GameObject[] obj){
			var result = new JsonProject();
			var scene = JsonScene.fromObjects(obj, result.resourceMapper);
			result.scenes.Add(scene);
			result.generateResourceList();
			return result;
		}
		
		public static JsonProject fromScene(Scene scene){
			var rootObjects = scene.GetRootGameObjects();
			return fromObjects(rootObjects);
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
				
			foreach(var curTex in resourceList.textures){
				TextureUtility.copyTexture(curTex, targetDir, projectPath);
			}
		}
		
		public string toJsonString(){
			FastJsonWriter writer = new FastJsonWriter();
			writeRawJsonValue(writer);
			return writer.getString();
		}
			
		public void saveToFile(string filename, bool saveResourceFiles = false){
			Utility.saveStringToFile(filename, toJsonString());
			if (!saveResourceFiles)
				return;
			saveResources(filename);
		}		
	}	
}
