using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

namespace SceneExport{
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
		public List<AssetInfo> terrains = new List<AssetInfo>();
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
			
			if (assetType == typeof(Texture2D)){
				textures.Add(assetInfo);
			}
			else if (assetType == typeof(Material)){
				materials.Add(assetInfo);
			}
			else if (assetType == typeof(TerrainData)){
				terrains.Add(assetInfo);
			}
			else if (assetType == typeof(SceneAsset)){
				scenes.Add(assetInfo);
			}
			else if (assetType == typeof(GameObject)){
				gameObjects.Add(assetInfo);//those are prefabs
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
}
