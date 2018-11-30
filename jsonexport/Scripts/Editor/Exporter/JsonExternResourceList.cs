using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

namespace SceneExport{
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
		public List<string> resources = new List<string>();
		
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
			writer.writeKeyVal("resources", resources);
		}
	}	
}