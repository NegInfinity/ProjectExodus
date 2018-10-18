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
		public List<JsonGameObject> prefabs = new List<JsonGameObject>();
		public List<string> resources = new List<string>();
		
		public override void writeJsonObjectFields(FastJsonWriter writer){
			writer.writeKeyVal("materials", materials);
			writer.writeKeyVal("meshes", meshes);
			writer.writeKeyVal("textures", textures);
			writer.writeKeyVal("prefabs", prefabs);
			writer.writeKeyVal("resources", resources);
		}
	}	
}
