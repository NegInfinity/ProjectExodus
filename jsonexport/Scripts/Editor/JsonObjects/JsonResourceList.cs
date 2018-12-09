using UnityEngine;
using UnityEditor;
using System.Collections.Generic;
using System.Linq;

namespace SceneExport{
	[System.Serializable]
	public class JsonResourceList: JsonValueObject{
		public List<JsonMaterial> materials = new List<JsonMaterial>();
		public List<JsonMesh> meshes = new List<JsonMesh>();
		public List<JsonTexture> textures = new List<JsonTexture>();
		public List<JsonPrefabData> prefabs = new List<JsonPrefabData>();
		public List<JsonTerrainData> terrains = new List<JsonTerrainData>();
		public List<JsonCubemap> cubemaps = new List<JsonCubemap>();
		public List<JsonAudioClip> audioClips = new List<JsonAudioClip>();
		
		public List<JsonSkeleton> skeletons = new List<JsonSkeleton>();		
		public List<JsonAnimatorController> animatorControllers = new List<JsonAnimatorController>();
		public List<JsonAnimationClip> animationClips = new List<JsonAnimationClip>();
		public List<string> resources = new List<string>();
		
		public override void writeJsonObjectFields(FastJsonWriter writer){
			writer.writeKeyVal("prefabs", prefabs);
			writer.writeKeyVal("terrains", terrains);
			writer.writeKeyVal("materials", materials);
			writer.writeKeyVal("meshes", meshes);
			writer.writeKeyVal("textures", textures);
			writer.writeKeyVal("cubemaps", cubemaps);
			writer.writeKeyVal("animatorControllers", animatorControllers);
			writer.writeKeyVal("animationClips", animationClips);
			writer.writeKeyVal("audioClips", audioClips);
			writer.writeKeyVal("resources", resources);
		}
	}	
}
