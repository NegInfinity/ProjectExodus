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
		public JsonExternResourceList externResourceList = new JsonExternResourceList(true);
		public ResourceMapper resourceMapper = new ResourceMapper();
		public override void writeJsonObjectFields(FastJsonWriter writer){
			writer.writeKeyVal("config", config);
			writer.writeKeyVal("externResources", externResourceList);
		}
		
		public void clear(){
			config = new JsonProjectConfig();
			scenes.Clear();
			resourceMapper = new ResourceMapper();
		}
		
		public static JsonProject fromObject(GameObject obj, bool showGui){
			return fromObjects(new GameObject[]{obj}, showGui);
		}		
		
		public static JsonProject fromObjects(GameObject[] obj, bool showGui){
			var result = new JsonProject();
			var scene = JsonScene.fromObjects(obj, result.resourceMapper, showGui);
			result.scenes.Add(scene);
			return result;
		}
		
		public static JsonProject fromScene(Scene scene, bool showGui){
			var rootObjects = scene.GetRootGameObjects();
			return fromObjects(rootObjects, showGui);
		}
		
		bool checkResourceFolder(string baseFilename, out string targetDir, out string projectPath, bool forbidProjectDir){
			//var subDirName = System.IO.Path.GetFileNameWithoutExtension(baseFilename);
			targetDir = System.IO.Path.GetDirectoryName(System.IO.Path.GetFullPath(baseFilename));
			if (!Application.isEditor){
				throw new System.ArgumentException("The application is not running in editor mode");
			}
				
			var dataPath = System.IO.Path.GetFullPath(Application.dataPath);
			projectPath = System.IO.Path.GetDirectoryName(dataPath);
				
			if (projectPath == targetDir){
				if (forbidProjectDir)
					Debug.LogWarningFormat("You're to save files project directory root, this is no longer allowed");
				else
					Debug.LogWarningFormat("You're saving into project directory, files will not be copied");
				return false;
			}
			//targetDir = System.IO.Path.Combine(targetDir, subDirName);
			//System.IO.Directory.CreateDirectory(targetDir);
			return true;
		}
		
		bool saveResourceType<T>(List<T> resourceList, string baseFilename, string targetDir, string projectPath, bool showGui, Logger logger,
				System.Action<T, bool, Logger> callback, string singular, string plural){
			Logger.makeValid(ref logger);
			logger.logFormat("Saving {2} to {0}, {1}", targetDir, projectPath, plural);
			
			var resourceIndex = 0;
			var numResources = resourceList.Count;
			var title = string.Format("Saving {1} for {0}",
				baseFilename, plural);
			foreach(var curResource in resourceList){
				if (showGui){
					if (ExportUtility.showCancellableProgressBar(title, 
							string.Format("Saving {2} {0}/{1}", resourceIndex, numResources, singular),
							resourceIndex, numResources)){
						logger.logErrorFormat("Resource copying cancelled by the user.");
						return false;
						//break;
					}				
				}
				
				if (callback != null)
					callback(curResource, true, logger);
				resourceIndex++;
			}
			return true;			
		}
		
		bool saveCubemaps(List<JsonCubemap> cubemaps, string baseFilename, string targetDir, string projectPath, bool showGui, Logger logger = null){
			return saveResourceType(cubemaps, baseFilename, targetDir, projectPath, showGui, logger,
				(curTex, showGui_, logger_) => TextureUtility.copyCubemap(curTex, targetDir, projectPath, logger), 
				"cubemap", "cubemaps"
			);
		}
		
		bool saveTextures(List<JsonTexture> textures, string baseFilename, string targetDir, string projectPath, bool showGui, Logger logger = null){
			return saveResourceType(textures, baseFilename, targetDir, projectPath, showGui, logger,
				(curTex, showGui_, logger_) => TextureUtility.copyTexture(curTex, targetDir, projectPath, logger), 
				"texture", "textures"
			);
		}
		
		bool saveTerrains(List<JsonTerrainData> terrains, string baseFilename, string targetDir, string projectPath, bool showGui, Logger logger = null){
			return saveResourceType(terrains, baseFilename, targetDir, projectPath, showGui, logger,
				(curTerrain, showGui_, logger_) => TerrainUtility.saveTerrain(curTerrain, targetDir, projectPath, showGui_, logger_), 
				"terrain", "terrains"
			);;
		}
		
		void saveAssetFiles(ExternalAssetList externAssets, string baseFilename, bool showGui, Logger logger = null){
			try{
				Logger.makeValid(ref logger);
				logger.logFormat("Save resources entered");
				string targetDir, projectPath;
				if (!checkResourceFolder(baseFilename, out targetDir, out projectPath, false))
					return;

				var baseProjectName = System.IO.Path.GetFileNameWithoutExtension(baseFilename);
				var targetAssetPath = System.IO.Path.Combine(targetDir, baseProjectName);
			
				bool cancelled = false;
				if (!cancelled){
					if (!saveTerrains(externAssets.terrains, baseFilename, targetAssetPath, projectPath, showGui, logger))
						cancelled = true;
				}
				if (!cancelled){
					if (!saveCubemaps(externAssets.cubemaps, baseFilename, targetAssetPath, projectPath, showGui, logger))
						cancelled = true;
				}
				if (!cancelled){
					if (!saveTextures(externAssets.textures, baseFilename, targetAssetPath, projectPath, showGui, logger))
						cancelled = true;
				}
			}
			finally{
				if (showGui){
					ExportUtility.hideProgressBar();
				}
			}
		}
		
		public string toJsonString(){
			FastJsonWriter writer = new FastJsonWriter();
			writeRawJsonValue(writer);
			return writer.getString();
		}

		void confirmAndEraseExistingDirectory(string filesDir){
			if (!System.IO.Directory.Exists(filesDir))
				return;
			if (!EditorUtility.DisplayDialog("WARNING! Erase existing directory?",
				string.Format("Directory \"{0}\" already exists.\nDo you want to erase existing directory?\n"
				+ "This directory will hold additional data. Erasing it is recommended.\n\n"
				+"Make SURE that the directory has no sensitive or important data, as all its contents will be removed", 
				filesDir),
				"Erase", "Keep"))
				return;

			System.IO.Directory.Delete(filesDir, true);
		}
		
		public void saveToFile(string filename, bool showGui, bool saveResourceFiles, Logger logger = null){
			try{
				if (showGui){
					ExportUtility.showProgressBar(
						string.Format("Saving file {0}", System.IO.Path.GetFileName(filename)), 
						"Writing json data", 0, 1);
				}
				string targetDir;
				string projectPath;
			
				checkResourceFolder(filename, out targetDir, out projectPath, true);
			
				string baseName = System.IO.Path.GetFileNameWithoutExtension(filename);
				string filesDir = System.IO.Path.Combine(targetDir, baseName);// + "_data");
				if (showGui){
					confirmAndEraseExistingDirectory(filesDir);
				}
				System.IO.Directory.CreateDirectory(filesDir);
						
				externResourceList = resourceMapper.saveResourceToFolder(filesDir, showGui, scenes, logger, saveResourceFiles);
			
				Utility.saveStringToFile(filename, toJsonString());

				if (!saveResourceFiles)
					return;
					
				saveAssetFiles(externResourceList.externalAssets, filename, showGui, logger);
			}
			finally{//Otherwise progress bar can get stuck, blocking entirety of editor
				if (showGui){
					ExportUtility.hideProgressBar();
				}
			}
		}
		
		string getProjectName(){
			return Application.productName;
		}
		
		public delegate bool DataProcessingDelegate<Data>(Data data, bool showGui, Logger logger);
		
		//TODO this needs to be optmized and collapsed into single method
		bool addTerrainAsset(AssetInfo asset, bool showGui, Logger logger){
			Logger.makeValid(ref logger);
			AssetDatabase.LoadAllAssetsAtPath(asset.path);
			var mainAsset = AssetDatabase.LoadMainAssetAtPath(asset.path);
			var terrainData = mainAsset as TerrainData;
			if (!mainAsset || !terrainData){
				logger.logErrorFormat("Could not load terrain data at path {0}({1})", asset.path, asset.guid);
				return true;
			}
			var id = resourceMapper.getTerrainId(terrainData);
			if (!ExportUtility.isValidId(id)){
				logger.logWarningFormat("Could not create id for terrain {0}({1})", asset.path, asset.guid);
			}
			return true;
		}
		
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
		
		bool addGameObjectAsset(AssetInfo asset, bool showGui, Logger logger){
			Logger.makeValid(ref logger);
			AssetDatabase.LoadAllAssetsAtPath(asset.path);
			var mainAsset = AssetDatabase.LoadMainAssetAtPath(asset.path);
			var obj = mainAsset as GameObject;
			if (!mainAsset || !obj){
				logger.logErrorFormat("Could not load GameObject at path {0}({1})", asset.path, asset.guid);
				return true;
			}
			
			var id = resourceMapper.gatherPrefabData(obj);
			
			if (!ExportUtility.isValidId(id)){
				logger.logWarningFormat("Could not create id for GameObject {0}({1})", asset.path, asset.guid);
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
			
			var scene = UnityEditor.SceneManagement.EditorSceneManager.OpenScene(
				asset.path, UnityEditor.SceneManagement.OpenSceneMode.Single);
				
			var jsonScene = JsonScene.fromScene(scene, resourceMapper, false);
			scenes.Add(jsonScene);
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
		
		bool collectProjectResources(bool showGui, Logger logger){
			try{
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
			
				logger.logFormat("Asset information: textures: {0}; materials: {1}; gameObjects: {2}; "
					+ "terrains: {3}; scenes: {4}; defaultAssets: {5}; unsupported: {6};",
					assetList.textures.Count, assetList.materials.Count, assetList.gameObjects.Count, 
					assetList.terrains.Count, assetList.scenes.Count, 
					assetList.defaultAssets.Count, assetList.unsupportedAssets.Count);
				
				if (!processDataList(assetList.terrains, showGui, "Registering terrains for", "Processing terrain", addTerrainAsset, logger))
					return false;
				if (!processDataList(assetList.textures, showGui, "Registering textures for", "Processing texture", addTextureAsset, logger))
					return false;
				if (!processDataList(assetList.materials, showGui, "Registering materials for", "Processing material", addMaterialAsset, logger))
					return false;
				if (!processDataList(assetList.gameObjects, showGui, "Registering prefabs for", "Processing prefab", addGameObjectAsset, logger))
					return false;
				
				var sceneSetup = UnityEditor.SceneManagement.EditorSceneManager.GetSceneManagerSetup();
				var processResult = processDataList(assetList.scenes, showGui, "Registering scenes for", "Processing scene", addSceneAsset, logger);
				UnityEditor.SceneManagement.EditorSceneManager.RestoreSceneManagerSetup(sceneSetup);
				if (!processResult)
					return false;
			}
			finally{
				if (showGui){
					ExportUtility.hideProgressBar();
				}
			}
			
			return true;
		}
		
		public static JsonProject fromCurrentProject(bool showGui, Logger logger = null){
			Logger.makeValid(ref logger);
			var result = new JsonProject();
			if (!result.collectProjectResources(showGui, logger)){
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
