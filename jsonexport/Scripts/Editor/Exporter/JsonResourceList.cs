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
		public List<string> resources = new List<string>();
		
		public override void writeJsonObjectFields(FastJsonWriter writer){
			/*
			writer.beginKeyArray("objects");
			foreach(var cur in objects){
				cur.writeJsonValue(writer);
			}
			writer.endArray();
			*/
			/*
			writer.writeKeyArray("materials", materials);
			writer.writeKeyArray("meshes", meshes);
			writer.writeKeyArray("textures", textures);
			writer.writeKeyArray("resources", resources);
			*/
			writer.writeKeyVal("materials", materials);
			writer.writeKeyVal("meshes", meshes);
			writer.writeKeyVal("textures", textures);
			writer.writeKeyVal("resources", resources);
		}
	}	
}