using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

namespace SceneExport{
	public class ResourceMapper{
		public ObjectMapper<Texture> textures = new ObjectMapper<Texture>();
		public ObjectMapper<Mesh> meshes = new ObjectMapper<Mesh>();
		public ObjectMapper<Material> materials = new ObjectMapper<Material>();
		Dictionary<Mesh, List<Material>> meshMaterials = new Dictionary<Mesh, List<Material>>();
		public HashSet<string> resources = new HashSet<string>();
		
		public ObjectMapper<GameObject> prefabs = new ObjectMapper<GameObject>();
		Dictionary<GameObject, GameObjectMapper> prefabObjects = new Dictionary<GameObject, GameObjectMapper>();
		
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
		
		void registerNewPrefab(GameObject prefabRoot){
			if (!prefabRoot)
				throw new System.ArgumentNullException("prefabRoot");
				
			var prefType = PrefabUtility.GetPrefabType(prefabRoot);
			if ((prefType != PrefabType.ModelPrefab) && (prefType != PrefabType.ModelPrefab)){
				throw new System.ArgumentException(
					string.Format("Invalid prefab type {0} for obj {1} ({2}, {3})",
						prefType, prefabRoot, ExportUtility.getObjectPath(prefabRoot), 
						AssetDatabase.GetAssetPath(prefabRoot)
					)
				);
			}
			
			var mapper = new GameObjectMapper();
			mapper.gatherObjectIds(prefabRoot);
			prefabObjects.Add(prefabRoot, mapper);
		}
		
		[System.Serializable]
		public struct PrefabObjectId{
			public int prefabId;
			public int prefabObjectId;
			public static readonly PrefabObjectId invalid = new PrefabObjectId(-1, -1);
			public bool isValid(){
				return ExportUtility.isValidId(prefabId) && ExportUtility.isValidId(prefabObjectId);
			}
			public PrefabObjectId(int prefabId_, int prefabObjectId_){
				prefabId = prefabId_;
				prefabObjectId = prefabObjectId_;
			}
		}
		
		public PrefabObjectId getPrefabObjectId(GameObject obj, bool createIfMissing){
			Object prefab = null;
			var prefType = PrefabUtility.GetPrefabType(obj);
			
			if ((prefType == PrefabType.ModelPrefabInstance) || (prefType == PrefabType.PrefabInstance)){
				prefab = PrefabUtility.GetCorrespondingObjectFromSource(obj);
			}
			else if ((prefType == PrefabType.ModelPrefab) || (prefType == PrefabType.Prefab)){
				prefab = obj;
			}
			else{
				return PrefabObjectId.invalid;
			}
			var prefObj = prefab as GameObject;
			var prefRoot = PrefabUtility.FindPrefabRoot(prefObj);
			
			if (!prefObj || !prefRoot)
				return PrefabObjectId.invalid;
				
			var prefabRootId = prefabs.findId(prefRoot);
			if (!ExportUtility.isValidId(prefabRootId)){
				if (createIfMissing)
					registerNewPrefab(prefRoot);
				else
					return PrefabObjectId.invalid;
			}
			
			prefabRootId = prefabs.findId(prefRoot);
			if (!ExportUtility.isValidId(prefabRootId)){
				throw new System.ArgumentException(
					string.Format("Prefab failed to register for {0} ({1})", 
						prefRoot, AssetDatabase.GetAssetPath(prefRoot)
					)
				);
			}
			
			GameObjectMapper mapper = null;
			if (!prefabObjects.TryGetValue(prefRoot, out mapper)){
				throw new System.ArgumentException(
					string.Format("Failed to find mapper for {0} ({1})", 
						prefRoot, AssetDatabase.GetAssetPath(prefRoot)
					)
				);
			}
			
			var objectId = mapper.findId(obj);
			return new PrefabObjectId(prefabRootId, objectId);
		}
		
		public void gatherPrefabData(GameObject obj){
			if (!obj)
				return;
				
			getPrefabObjectId(obj, true);				
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
