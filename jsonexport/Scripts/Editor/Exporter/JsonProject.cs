using UnityEngine;
using UnityEditor;
using System.Collections;
using System.Linq;
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
			//var resourceList = resourceMapper.makeResourceList();
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
		
		void saveResources(string baseFilename, bool showGui, Logger logger = null){
			Logger.makeValid(ref logger);
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
		
		[System.Serializable]
		public class AssetInfo{
			public string guid;
			public string path;
			public AssetInfo(string guid_, string path_){
				guid = guid_;
				path = path_;
			}
		}
		
		[System.Serializable]
		public class ProjectAssetList{
			public List<AssetInfo> textures = new List<AssetInfo>();
			public List<AssetInfo> materials = new List<AssetInfo>();
			public List<AssetInfo> gameObjects = new List<AssetInfo>();
			public List<AssetInfo> scenes = new List<AssetInfo>();
			public List<AssetInfo> defaultAssets = new List<AssetInfo>();
			public List<AssetInfo> unsupportedAssets = new List<AssetInfo>();
			
			public void addAssetFromGuid(string guid, Logger logger = null){
				Logger.makeValid(ref logger);
				var assetPath = AssetDatabase.GUIDToAssetPath(guid);
				if (string.IsNullOrEmpty(assetPath)){
					logger.logErrorFormat("Could not get path of asset \"{0}\'", guid);
					return;
				}
				
				var assetInfo = new AssetInfo(guid, assetPath);
				
				var assetType = AssetDatabase.GetMainAssetTypeAtPath(assetPath);
				//logger.logFormat("Main asset type is \"{0}\" at path \"{1}\" (guid {2}) ", assetType.Name, assetPath, guid);
			
				if (assetType == typeof(Texture2D)){
					textures.Add(assetInfo);
				}
				else if (assetType == typeof(Material)){
					materials.Add(assetInfo);
				}
				else if (assetType == typeof(SceneAsset)){
					scenes.Add(assetInfo);
				}
				else if (assetType == typeof(GameObject)){
					gameObjects.Add(assetInfo);
				}
				else if (assetType == typeof(DefaultAsset)){
					defaultAssets.Add(assetInfo);//folders
				}
				else if (assetType == typeof(Cubemap)){
					logger.logWarningFormat("Cubemaps are not currently supported: {0} ({1})", assetPath, guid);
					unsupportedAssets.Add(assetInfo);
				}
				else if (
						(assetType == typeof(Shader))||
						(assetType == typeof(ComputeShader))||
						(assetType == typeof(AnimationClip))||
						(assetType == typeof(TextAsset))||
						(assetType == typeof(AudioClip)) ||
						(assetType == typeof(MonoScript)) ||
						(assetType == typeof(LightmapParameters)) ||
						(assetType == typeof(PhysicMaterial)) ||
						(assetType == typeof(UnityEditor.Animations.AnimatorController)) ||
						(assetType == typeof(UnityEditor.LightingDataAsset)) ||
						false
				){
					unsupportedAssets.Add(assetInfo);					
				}
				else{
					logger.logWarningFormat("Unhandled asset type: {0}. path: {1}, guid: {2} ({3})", 
						assetType.Name, assetPath, guid, assetType);
				}
			}
		}
		
		string getProjectName(){
			return Application.productName;
		}
		
		public delegate bool DataProcessingDelegate<Data>(Data data, bool showGui, Logger logger);
		
		bool addTextureAsset(AssetInfo asset, bool showGui, Logger logger){
			Logger.makeValid(ref logger);
			AssetDatabase.LoadAllAssetsAtPath(asset.path);
			var mainAsset = AssetDatabase.LoadMainAssetAtPath(asset.path);
			var tex2d = mainAsset as Texture2D;
			if (!mainAsset || !tex2d){
				logger.logErrorFormat("Could not load texture at path {0}({1})", asset.path, asset.guid);
				return true;
			}
			var id = resourceMapper.getTextureId(tex2d);
			if (!ExportUtility.isValidId(id)){
				logger.logWarningFormat("Could not create id for texture {0}({1})", asset.path, asset.guid);
			}
			return true;
		}
		
		bool addMaterialAsset(AssetInfo asset, bool showGui, Logger logger){
			Logger.makeValid(ref logger);
			AssetDatabase.LoadAllAssetsAtPath(asset.path);
			var mainAsset = AssetDatabase.LoadMainAssetAtPath(asset.path);
			var mat = mainAsset as Material;
			if (!mainAsset || !mat){
				logger.logErrorFormat("Could not load material at path {0}({1})", asset.path, asset.guid);
				return true;
			}
			var id = resourceMapper.getMaterialId(mat);
			if (!ExportUtility.isValidId(id)){
				logger.logWarningFormat("Could not create id for material {0}({1})", asset.path, asset.guid);
			}
			return true;
		}
		
		bool addSceneAsset(AssetInfo asset, bool showGui, Logger logger){
			Logger.makeValid(ref logger);
			//AssetDatabase.LoadAllAssetsAtPath(asset.path);
			var mainAsset = AssetDatabase.LoadMainAssetAtPath(asset.path);
			var sceneAsset = mainAsset as SceneAsset;
			if (!mainAsset || !sceneAsset){
				logger.logErrorFormat("Could not get scene at path {0}({1})", asset.path, asset.guid);
				return true;
			}
			
			var scene = UnityEditor.SceneManagement.EditorSceneManager.OpenScene(asset.path);
			//EditorSceneManagement.
			//var scene = sceneAsset as Scene;
			logger.logFormat("Loading scene: \"{0}\"", sceneAsset.name);
			return true;
		}
		
		bool processDataList<Data>(List<Data>assets, bool showGui, string titleMsgStart, string assetMsgStart, 
				DataProcessingDelegate<Data> callback, Logger logger){
			var title = string.Format("{0} \"{1}\"", titleMsgStart, getProjectName());
			for(int i = 0; i < assets.Count; i++){
				if (showGui){
					if (	ExportUtility.showCancellableProgressBar(title, 
							string.Format("{2} {0}/{1}", i, assets.Count, assetMsgStart), i, assets.Count)){
						logger.logErrorFormat("Export was cancelled by the user");
						ExportUtility.hideProgressBar();
						return false;
					}
				}
				if (callback != null)
					callback(assets[i], showGui,  logger);
			}
			return true;
		}
		
		bool loadCurrentProject(bool showGui, Logger logger){
			var projectName = Application.productName;
			var title = string.Format("Processing proejct \"{0}\"",
				projectName);
			if (showGui){
				ExportUtility.showProgressBar(title, "Gathering all assets", 0, 1);
			}
			
			var allAssets = AssetDatabase.FindAssets("t:object").ToList();
			logger.logFormat("{0} assets found", allAssets.Count);
			
			title = string.Format("Enumerating assets for \"{0}\"", projectName);
			var numAssets = allAssets.Count;
			var assetList = new ProjectAssetList();
			
			if (!processDataList(allAssets, showGui, "Enumerating assets for", "Processing asset", 
					(data_, gui_, log_) => {assetList.addAssetFromGuid(data_, log_); return true; }, logger))
				return false;
			
			logger.logFormat("Asset information: textures: {0}; materials: {1}; gameObjects: {2}; scenes: {3}; defaultAssets: {4}; unsupported: {5};",
				assetList.textures.Count, assetList.materials.Count, assetList.gameObjects.Count, assetList.scenes.Count, 
				assetList.defaultAssets.Count, assetList.unsupportedAssets.Count);
				
			if (!processDataList(assetList.textures, showGui, "Registering textures for", "Processing texture", addTextureAsset, logger))
				return false;
			if (!processDataList(assetList.materials, showGui, "Registering materials for", "Processing material", addMaterialAsset, logger))
				return false;
				
			var sceneSetup = UnityEditor.SceneManagement.EditorSceneManager.GetSceneManagerSetup();
			var processResult = processDataList(assetList.scenes, showGui, "Registering scenes for", "Processing scene", addSceneAsset, logger);
			UnityEditor.SceneManagement.EditorSceneManager.RestoreSceneManagerSetup(sceneSetup);
			if (!processResult)
				return false;
			
			if (showGui){
				ExportUtility.hideProgressBar();
			}
			generateResourceList();
			return true;
		}
		
		public static JsonProject fromCurrentProject(bool showGui, Logger logger = null){
			Logger.makeValid(ref logger);
			var result = new JsonProject();
			if (!result.loadCurrentProject(showGui, logger)){
				//logger.logErrorFormat("Project export failed");
				return null;
			}
			return result;
		}
		
		void addAsset(string guid, Logger logger){
			logger = Logger.getValid(logger);
			
			var path = AssetDatabase.GUIDToAssetPath(guid);
			if (string.IsNullOrEmpty(path)){
				logger.logErrorFormat("Could not get path of asset \"{0}\'", guid);
				return;
			}
			
		}
	}	
}
