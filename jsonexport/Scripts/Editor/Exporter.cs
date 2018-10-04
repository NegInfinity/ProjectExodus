using UnityEngine;
using UnityEditor;
using System.IO;
using System.Text;
using System.Collections;
using System.Collections.Generic;

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