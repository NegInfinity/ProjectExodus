using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

namespace SceneExport{
	public class ResourceMapper{
		public ObjectMapper<Texture> textures = new ObjectMapper<Texture>();
		public ObjectMapper<Mesh> meshes = new ObjectMapper<Mesh>();
		public ObjectMapper<Material> materials = new ObjectMapper<Material>();
		public ObjectMapper<Cubemap> cubemaps = new ObjectMapper<Cubemap>();
		public ObjectMapper<AudioClip> audioClips = new ObjectMapper<AudioClip>();
		Dictionary<Mesh, List<Material>> meshMaterials = new Dictionary<Mesh, List<Material>>();
		public HashSet<string> resources = new HashSet<string>();
		
		public ObjectMapper<TerrainData> terrains = new ObjectMapper<TerrainData>();
		
		public ObjectMapper<GameObject> prefabs = new ObjectMapper<GameObject>();
		Dictionary<GameObject, GameObjectMapper> prefabObjects = new Dictionary<GameObject, GameObjectMapper>();
		
		public List<Material> findMeshMaterials(Mesh mesh){
			List<Material> result = null;
			if (meshMaterials.TryGetValue(mesh, out result))
				return result;
			return null;
		}
		
		public GameObjectMapper getPrefabObjectMapper(GameObject rootPrefab){
			GameObjectMapper result = null;
			if (prefabObjects.TryGetValue(rootPrefab, out result))
				return result;
			return null;
		}
		
		public int getTerrainId(TerrainData data){
			return terrains.getId(data, true);
		}
		
		public int findTerrainId(TerrainData terrain){
			return terrains.getId(terrain, false);
		}

		public int getAudioClipId(AudioClip clip){
			return audioClips.getId(clip, true);
		}
		
		public int findAudioClipId(AudioClip clip){
			return audioClips.getId(clip, false);
		}
		
		public int getTextureId(Texture tex){
			return textures.getId(tex, true);
		}
		
		public int findTextureId(Texture tex){
			return textures.getId(tex, false);
		}
		
		public int getCubemapId(Cubemap cube){
			return cubemaps.getId(cube, true);
		}
		
		public int findCubemapId(Cubemap cube){
			return cubemaps.getId(cube, false);
		}
		
		public MeshIdData getMeshId(GameObject srcObj){
			return new MeshIdData(srcObj, this);
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
		
		int getOrRegMeshId(GameObject obj, Mesh mesh){
			if (!mesh)
				return ExportUtility.invalidId;
			int result = meshes.getId(mesh, true, null);
			if (meshMaterials.ContainsKey(mesh))
				return result;
			var r = obj.GetComponent<Renderer>();
			if (r){
				meshMaterials[mesh] = new List<Material>(r.sharedMaterials);
			}
			return result;
		}
		
		public int getOrRegMeshId(GameObject obj){
			//int result = -1;
			var meshFilter = obj.GetComponent<MeshFilter>();
			if (!meshFilter)
				return ExportUtility.invalidId;

			var mesh = meshFilter.sharedMesh;
			if (!mesh)
				return ExportUtility.invalidId;
			return getOrRegMeshId(obj, mesh);
		}
		
		public int getPrefabObjectId(GameObject obj, bool createMissing){
			if (!obj)
				return ExportUtility.invalidId;
				
			var linkedPrefab = ExportUtility.getLinkedPrefab(obj);
			if (!linkedPrefab)
				return ExportUtility.invalidId;
			
			var rootPrefabId = getRootPrefabId(obj, createMissing);
			if (!ExportUtility.isValidId(rootPrefabId))
				return ExportUtility.invalidId;
				
			var rootPrefab = prefabs.getObject(rootPrefabId);
			
			var prefabMapper = prefabObjects[rootPrefab];
			return prefabMapper.getId(linkedPrefab);
		}
		
		void onNewRootPrefab(GameObject rootPrefab){
			var newMapper = new GameObjectMapper();
			newMapper.gatherObjectIds(rootPrefab);
			foreach(var curObj in newMapper.objectList){
				getOrRegMeshId(curObj);
			}
			prefabObjects.Add(rootPrefab, newMapper);
		}
		
		public int getRootPrefabId(GameObject obj, bool createMissing){
			if (!obj)
				return ExportUtility.invalidId;
			var rootPrefab = ExportUtility.getLinkedRootPrefab(obj);
			if (!rootPrefab)
				return ExportUtility.invalidId;
			
			var result = prefabs.getId(rootPrefab, createMissing, onNewRootPrefab);
			
			return result;
		}
		
		public int gatherPrefabData(GameObject obj){
			if (!obj)
				return ExportUtility.invalidId;
			return getRootPrefabId(obj, true);
		}
		
		List<JsonPrefabData> makePrefabList(){
			var result = new List<JsonPrefabData>();

			for(int i = 0; i < prefabs.objectList.Count; i++){
				var src = prefabs.getObject(i);
				var dst = new JsonPrefabData(src, this);
				result.Add(dst);
			}
			
			return result;
		}
		
		public JsonResourceList makeResourceList(){
			var result = new JsonResourceList();
			
			foreach(var cur in terrains.objectList){
				result.terrains.Add(new JsonTerrainData(cur, this));
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
			
			foreach(var cur in cubemaps.objectList){
				result.cubemaps.Add(new JsonCubemap(cur, this));
			}
			
			foreach(var cur in audioClips.objectList){
				result.audioClips.Add(new JsonAudioClip(cur, this));
			}
			
			result.prefabs = makePrefabList();
			
			result.resources = new List<string>(resources);
			result.resources.Sort();
			
			return result;
		}
	}
}
