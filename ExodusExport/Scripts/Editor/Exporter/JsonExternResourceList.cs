using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

namespace SceneExport{
	[System.Serializable]
	public class ExternalAssetList{
		public List<JsonTerrainData> terrains = new List<JsonTerrainData>();
		public List<JsonTexture> textures = new List<JsonTexture>();
		public List<JsonCubemap> cubemaps = new List<JsonCubemap>();

		public void registerAsset(JsonTerrainData newObj){
			terrains.Add(newObj);
		}
		public void registerAsset(JsonTexture newObj){
			textures.Add(newObj);
		}
		public void registerAsset(JsonCubemap newObj){
			cubemaps.Add(newObj);
		}
	};

	[System.Serializable]
	public class JsonExternResourceList: JsonValueObject{
		public List<string> scenes = new List<string>();
		public List<string> materials = new List<string>();
		public List<string> skeletons = new List<string>();
		public List<string> meshes = new List<string>();
		public List<string> textures = new List<string>();
		public List<string> prefabs = new List<string>();
		public List<string> terrains = new List<string>();
		public List<string> cubemaps = new List<string>();
		public List<string> audioClips = new List<string>();
		public List<string> animatorControllers = new List<string>();
		public List<string> animationClips = new List<string>();
		public List<string> resources = new List<string>();

		public ExternalAssetList externalAssets = new ExternalAssetList();//This is NOT serialized as json

		protected bool collectExternAssets = true;

		public void registerAsset(JsonTexture asset){
			if (collectExternAssets)
				externalAssets.registerAsset(asset);
		}
		
		public void registerAsset(JsonTerrainData asset){
			if (collectExternAssets)
				externalAssets.registerAsset(asset);
		}

		public void registerAsset(JsonCubemap asset){
			if (collectExternAssets)
				externalAssets.registerAsset(asset);
		}
		
		public override void writeJsonObjectFields(FastJsonWriter writer){
			writer.writeKeyVal("scenes", scenes);
			writer.writeKeyVal("prefabs", prefabs);
			writer.writeKeyVal("terrains", terrains);
			writer.writeKeyVal("materials", materials);
			writer.writeKeyVal("skeletons", skeletons);
			writer.writeKeyVal("meshes", meshes);
			writer.writeKeyVal("textures", textures);
			writer.writeKeyVal("cubemaps", cubemaps);
			writer.writeKeyVal("audioClips", audioClips);
			writer.writeKeyVal("animationClips", animationClips);
			
			writer.writeKeyVal("animatorControllers", animatorControllers);
			
			writer.writeKeyVal("resources", resources);
		}

		public JsonExternResourceList(bool collectExternAssets_){
			collectExternAssets = collectExternAssets_;
		}
	}	
}