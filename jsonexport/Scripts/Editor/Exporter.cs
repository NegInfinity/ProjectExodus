using UnityEngine;
using UnityEditor;
using System.IO;
using System.Text;
using System.Collections;
using System.Collections.Generic;
using UnityEngine.SceneManagement;

namespace SceneExport{
	public partial class Exporter{
		ObjectMapper<GameObject> objects = new ObjectMapper<GameObject>();
		ObjectMapper<Texture> textures = new ObjectMapper<Texture>();
		ObjectMapper<Mesh> meshes = new ObjectMapper<Mesh>();
		ObjectMapper<Material> materials = new ObjectMapper<Material>();
		Dictionary<Mesh, List<Material>> meshMaterials = new Dictionary<Mesh, List<Material>>();
		HashSet<string> resources = new HashSet<string>();

		int getObjectId(GameObject obj){
			return objects.getId(obj);
		}

		int getTextureId(Texture tex){
			return textures.getId(tex);
		}

		int getMeshId(Mesh obj){
			return meshes.getId(obj);
		}

		int getMaterialId(Material obj){
			return materials.getId(obj);
		}

		void registerResource(string path){
			resources.Add(path);
		}

		int getMeshId(GameObject obj){
			int result = -1;
			var meshFilter = obj.GetComponent<MeshFilter>();
			if (!meshFilter)
				return result;

			var mesh = meshFilter.sharedMesh;
			result = meshes.getId(mesh);

			if (meshMaterials.ContainsKey(mesh))
				return result;
			var r = obj.GetComponent<Renderer>();
			if (r){
				meshMaterials[mesh] = new List<Material>(r.sharedMaterials);
			}
			return result;
		}
		
		public JsonScene exportScene(Scene scene){
			var rootObjects = scene.GetRootGameObjects();
			return exportObjects(rootObjects);
		}
		
		[System.Serializable]
		struct NameClashKey{
			public string name;
			public int parentId;
			
			public override int GetHashCode(){
				int result = Utility.beginHash();
				result = Utility.combineHash(result, name);
				result = Utility.combineHash(result, parentId);
				return result;
			}
			
			public override bool Equals(object obj){
				if (!(obj is NameClashKey))
					return false;
				NameClashKey other = (NameClashKey)obj;
				return (name == other.name) && (parentId == other.parentId);					
			}

			
			public NameClashKey(string name_, int parentId_){
				name = name_;
				parentId = parentId_;
			}
		}
		
		void fixNameClashes(JsonScene scene){
			var nameClashes = new Dictionary<NameClashKey, List<int>>();
			for(int i = 0; i < scene.objects.Count; i++){
				var cur = scene.objects[i];
				var key = new NameClashKey(cur.name, cur.parent);
				var idList = nameClashes.getValOrGenerate(key, (parId_) => new List<int>());
				idList.Add(cur.id);
			}
			
			foreach(var entry in nameClashes){
				var key = entry.Key;
				var list = entry.Value;
				if ((list == null) || (list.Count <= 1))
					continue;

				for(int i = 1; i < list.Count; i++){
					var curId = list[i];
					if ((curId <= 0) || (curId >= scene.objects.Count)){
						Debug.LogErrorFormat("Invalid object id {0}, while processing name clash {1};\"{2}\"", 
							curId, key.parentId, key.name);
						continue;
					}
					var curObj = scene.objects[curId];
					var altName = string.Format("{0}-#{1}", key.name, i);
					while(nameClashes.ContainsKey(new NameClashKey(altName, key.parentId))){
						altName = string.Format("{0}-#{1}({2})", 
							key.name, i, System.Guid.NewGuid().ToString("n"));
						//break;
					}
					curObj.nameClash = true;
					curObj.uniqueName = altName;
				}								
			}			
		}

		public JsonScene exportObjects(GameObject[] args){
			var result = new JsonScene();
			foreach(var cur in args){
				if (!cur)
					continue;
				objects.getId(cur);
			}
			
			for(int i = 0; i < objects.objectList.Count; i++){
				/*TODO: This is very awkward, as constructor adds more data to the exporter
				Should be split into two methods.*/
				result.objects.Add(new JsonGameObject(objects.objectList[i], this));
			}
			
			fixNameClashes(result);

			foreach(var cur in meshes.objectList){
				result.meshes.Add(new JsonMesh(cur, this));
			}

			foreach(var cur in materials.objectList){
				result.materials.Add(new JsonMaterial(cur, this));
			}

			foreach(var cur in textures.objectList){
				result.textures.Add(new JsonTexture(cur, this));
			}

			result.resources = new List<string>(resources);
			result.resources.Sort();

			return result;
		}

		public JsonScene exportOneObject(GameObject obj){
			return exportObjects(new GameObject[]{obj});
		}
	}
}