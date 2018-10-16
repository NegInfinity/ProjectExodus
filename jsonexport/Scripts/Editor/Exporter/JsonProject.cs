using UnityEngine;
using UnityEditor;
using System.Collections;
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
		
		public void clear(){
			config = new JsonProjectConfig();
			scenes.Clear();
			resourceList = new JsonResourceList();
			resourceMapper = new ResourceMapper();
		}
		
		public static JsonProject fromObject(GameObject obj, bool showGui){
			return fromObjects(new GameObject[]{obj}, showGui);
		}		
		
		public void generateResourceList(){
			resourceList = resourceMapper.makeResourceList();
		}
		
		public static JsonProject fromObjects(GameObject[] obj, bool showGui){
			var result = new JsonProject();
			var scene = JsonScene.fromObjects(obj, result.resourceMapper, showGui);
			result.scenes.Add(scene);
			result.generateResourceList();
			return result;
		}
		
		public static JsonProject fromScene(Scene scene, bool showGui){
			var rootObjects = scene.GetRootGameObjects();
			return fromObjects(rootObjects, showGui);
		}
		
		bool checkResourceFolder(string baseFilename, out string targetDir, out string projectPath){
			targetDir = System.IO.Path.GetDirectoryName(System.IO.Path.GetFullPath(baseFilename));
			if (!Application.isEditor){
				throw new System.ArgumentException("The application is not running in editor mode");
			}
				
			var dataPath = System.IO.Path.GetFullPath(Application.dataPath);
			projectPath = System.IO.Path.GetDirectoryName(dataPath);
				
			if (projectPath == targetDir){
				Debug.LogWarningFormat("You're saving into project directory, files will not be copied");
				return false;
			}
			return true;
		}
		
		/*
		IEnumerator saveResources(string baseFilename, AsyncExportTask exportTask){
			exportTask.markRunning();
			string targetDir, projectPath;
			if (!checkResourceFolder(baseFilename, out targetDir, out projectPath))
				yield break;
				
			exportTask.beginProgress(resourceList.textures.Count);
			int fileIndex = 0;
			foreach(var curTex in resourceList.textures){
				fileIndex++;
				exportTask.setStatus(string.Format("Copying files {0}/{1}", fileIndex, resourceList.textures.Count));
				TextureUtility.copyTexture(curTex, targetDir, projectPath);
				exportTask.incProgress();
				exportTask.checkRepaint();
				if (exportTask.needsPause()){
					yield return null;
				}
			}
		}
		*/
		
		void saveResources(string baseFilename, bool showGui, Logger logger = null){
			logger = Logger.getValid(logger);
			string targetDir, projectPath;
			if (!checkResourceFolder(baseFilename, out targetDir, out projectPath))
				return;
				
			var texIndex = 0;
			var numTextures = resourceList.textures.Count;
			var title = string.Format("Saving textures for {0}",
				baseFilename);
			foreach(var curTex in resourceList.textures){
				if (showGui){
					if (ExportUtility.showCancellableProgressBar(title, 
							string.Format("Saving texture {0}/{1}", texIndex, numTextures),
							texIndex, numTextures)){
						logger.logErrorFormat("Resource copying cancelled by the user.");
						break;
					}				
				}
				
				TextureUtility.copyTexture(curTex, targetDir, projectPath, logger);
				texIndex++;
			}
			if (showGui){
				ExportUtility.hideProgressBar();
			}
		}
		
		public string toJsonString(){
			FastJsonWriter writer = new FastJsonWriter();
			writeRawJsonValue(writer);
			return writer.getString();
		}
			
		/*
		public IEnumerator saveToFile(string filename, bool saveResourceFiles, AsyncExportTask exportTask){
			exportTask.startNew();
			exportTask.setStatus("Saving project to file");
			Utility.saveStringToFile(filename, toJsonString());
			if (!saveResourceFiles)
				yield break;
				
			yield return saveResources(filename, exportTask);
			exportTask.finish();
			exportTask.repaintWindow();
		}
		*/
			
		public void saveToFile(string filename, bool showGui, bool saveResourceFiles, Logger logger = null){
			if (showGui){
				ExportUtility.showProgressBar(
					string.Format("Saving file {0}", System.IO.Path.GetFileName(filename)), 
					"Writing json data", 0, 1);
			}
			Utility.saveStringToFile(filename, toJsonString());
			if (showGui){
				ExportUtility.hideProgressBar();
			}
			if (!saveResourceFiles)
				return;

			saveResources(filename, showGui, logger);
		}		
	}	
}
