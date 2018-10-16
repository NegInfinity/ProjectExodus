using UnityEngine;
using System.Collections.Generic;

namespace SceneExport{
	public class ResourceMapper{
		public ObjectMapper<Texture> textures = new ObjectMapper<Texture>();
		public ObjectMapper<Mesh> meshes = new ObjectMapper<Mesh>();
		public ObjectMapper<Material> materials = new ObjectMapper<Material>();
		Dictionary<Mesh, List<Material>> meshMaterials = new Dictionary<Mesh, List<Material>>();
		public HashSet<string> resources = new HashSet<string>();
		
		public List<Material> findMeshMaterials(Mesh mesh){
			List<Material> result = null;
			if (meshMaterials.TryGetValue(mesh, out result))
				return result;
			return null;
		}

		public int getTextureId(Texture tex){
			return textures.getId(tex, true);
		}
		
		public int findTextureId(Texture tex){
			return textures.getId(tex, false);
		}

		public int getMeshId(Mesh obj){
			return meshes.getId(obj, true);
		}
		
		public int findMeshId(Mesh obj){
			return meshes.getId(obj, false);
		}

		public int getMaterialId(Material obj){
			return materials.getId(obj, true);
		}
		
		public int findMaterialId(Material obj){
			return materials.getId(obj, false);
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
			result = meshes.getId(mesh, true, null);

			if (meshMaterials.ContainsKey(mesh))
				return result;
			var r = obj.GetComponent<Renderer>();
			if (r){
				meshMaterials[mesh] = new List<Material>(r.sharedMaterials);
			}
			return result;
		}
		
		public JsonResourceList makeResourceList(){
			var result = new JsonResourceList();
			
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
	}
}
