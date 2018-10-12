using UnityEngine;
using System.Collections.Generic;

namespace SceneExport{
	public class ResourceIdMaper{
		ObjectMapper<GameObject> objects = new ObjectMapper<GameObject>();
		ObjectMapper<Texture> textures = new ObjectMapper<Texture>();
		ObjectMapper<Mesh> meshes = new ObjectMapper<Mesh>();
		ObjectMapper<Material> materials = new ObjectMapper<Material>();
		Dictionary<Mesh, List<Material>> meshMaterials = new Dictionary<Mesh, List<Material>>();
		HashSet<string> resources = new HashSet<string>();
		
		public List<Material> findMeshMaterials(Mesh mesh){
			List<Material> result = null;
			if (meshMaterials.TryGetValue(mesh, out result))
				return result;
			return null;
		}

		public int getObjectId(GameObject obj){
			return objects.getId(obj);
		}

		public int findObjectId(GameObject obj){
			return objects.findId(obj);
		}

		public int getTextureId(Texture tex){
			return textures.getId(tex);
		}
		
		public int findTextureId(Texture tex){
			return textures.findId(tex);
		}

		public int getMeshId(Mesh obj){
			return meshes.getId(obj);
		}
		
		public int findMeshId(Mesh obj){
			return meshes.findId(obj);
		}

		public int getMaterialId(Material obj){
			return materials.getId(obj);
		}
		
		public int findMaterialId(Material obj){
			return materials.findId(obj);
		}

		public void registerResource(string path){
			resources.Add(path);
		}

		public int getMeshId(GameObject obj){
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

		/*		
		public JsonScene exportScene(Scene scene){
			var rootObjects = scene.GetRootGameObjects();
			return exportObjects(rootObjects);
		}
		
		public JsonScene exportObjects(GameObject[] args){
			var result = new JsonScene();
			foreach(var cur in args){
				if (!cur)
					continue;
				objects.getId(cur);
			}
			
			for(int i = 0; i < objects.objectList.Count; i++){
				result.objects.Add(new JsonGameObject(objects.objectList[i], this));
			}
			
			result.fixNameClashes();

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
		*/
	}
}
