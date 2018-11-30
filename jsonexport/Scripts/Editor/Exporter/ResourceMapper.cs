using UnityEngine;
using UnityEditor;
using System.Collections.Generic;
using System.Linq;

namespace SceneExport{
	public class ResourceMapper{
		public ObjectMapper<Texture> textures = new ObjectMapper<Texture>();
		public ObjectMapper<Mesh> meshes = new ObjectMapper<Mesh>();
		public ObjectMapper<Material> materials = new ObjectMapper<Material>();
		public ObjectMapper<Cubemap> cubemaps = new ObjectMapper<Cubemap>();
		public ObjectMapper<AudioClip> audioClips = new ObjectMapper<AudioClip>();
		Dictionary<Mesh, List<Material>> meshMaterials = new Dictionary<Mesh, List<Material>>();
		Dictionary<Mesh, JsonSkeleton> meshSkeletons = new Dictionary<Mesh, JsonSkeleton>();
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
		
		public int getSkeletonId(Mesh obj){
			JsonSkeleton skel = null;
			if (!meshSkeletons.TryGetValue(obj, out skel))
				return -1;
			return skel.id;
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
		
		public void registerTexture(Texture tex){
			textures.getId(tex, true, null);
		}
		
		public void registerMaterial(Material mat){
			materials.getId(mat, true, (newMat) => {
				JsonMaterial.registerLinkedData(newMat, this);
				return;
			});
		}
		
		public void registerGameObjectData(GameObject gameObj){
			if (!gameObj)
				return;
				
			JsonGameObject.registerLinkedData(gameObj, this);
			foreach(Transform child in gameObj.transform){
				if (!child)
					continue;
				if (!child.gameObject)
					registerGameObjectData(child.gameObject);
			}
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
		
		public int getOrRegMeshId(SkinnedMeshRenderer meshRend){
			var mesh = meshRend.sharedMesh;
			if (!mesh)
				return ExportUtility.invalidId;
				
			if (!meshSkeletons.ContainsKey(mesh)){
				var skel = JsonSkeleton.extractOriginalSkeleton(meshRend);
				if (skel != null){
					skel.id = meshSkeletons.Count;
					meshSkeletons.Add(mesh, skel);
				}
			}
			
			return getOrRegMeshId(meshRend.gameObject, mesh);
		}
		
		public int getOrRegMeshId(MeshFilter meshFilter){
			var mesh = meshFilter.sharedMesh;
			if (!mesh)
				return ExportUtility.invalidId;
				
			return getOrRegMeshId(meshFilter.gameObject, mesh);
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
				foreach(var meshFilter in curObj.GetComponents<MeshFilter>()){
					getOrRegMeshId(meshFilter);
				}
				foreach(var meshRend in curObj.GetComponents<SkinnedMeshRenderer>()){
					getOrRegMeshId(meshRend);
				}
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
		
		static string makeJsonResourcePath(string resourceName, int resourceIndex){
			return string.Format("{0}{1:D8}.json", resourceName, resourceIndex);
		}
		
		static string cleanUpObjectName(string src){
			var invalidChars = new HashSet<char>(System.IO.Path.GetInvalidFileNameChars());
			var sb = new System.Text.StringBuilder();
			foreach(var cur in src){
				if (cur == '%'){
					sb.Append("%%");
					continue;
				}
				if (invalidChars.Contains(cur)){
					sb.AppendFormat("%{0}", ((uint)cur).ToString("X2"));
					continue;					
				}
				sb.Append(cur);
			}
			return src.ToString();
		}
		
		static string makeJsonResourcePath(string resourceName, string objName, int resourceIndex){
			if (string.IsNullOrEmpty(objName))
				return makeJsonResourcePath(resourceName, resourceIndex);
			return string.Format("{0}{1:D8}-{2}.json", resourceName, resourceIndex, cleanUpObjectName(objName));
		}			
		
		static List<string> makeResourcePaths(string baseDir, int numResources, string baseName){
			var result = new List<string>();
			for(int i = 0; i < numResources; i++){
				var curPath = System.IO.Path.Combine(baseDir, makeJsonResourcePath(baseName, i));
				result.Add(curPath);
			}
			return result;
		}		
		
		static List<string> saveResourcesToPath<ClassType, ObjType>(string baseDir, 
				List<ObjType> objects, 
				System.Func<ObjType, ClassType> converter, 
				System.Func<ClassType, string> nameFunc, string baseName, bool showGui) 
				where ClassType: IFastJsonValue{
				
			if (converter == null)
				throw new System.ArgumentNullException("converter");
				
			try{
				var result = new List<string>();
				if (objects != null){
					for(int i = 0; i < objects.Count; i++){
						if (showGui){
							ExportUtility.showProgressBar(
								string.Format("Saving file #{0} of resource type {1}", i + 1, baseName), 
								"Writing json data", i, objects.Count);
						}
						var jsonObj = converter(objects[i]);	
						string fileName;
						if (nameFunc != null){
							fileName = makeJsonResourcePath(baseName, nameFunc(jsonObj), i);	
						}
						else{
							fileName = makeJsonResourcePath(baseName, i);	
						}
						var fullPath = System.IO.Path.Combine(baseDir, fileName);
				
						jsonObj.saveToJsonFile(fullPath);				
						result.Add(fileName);
					}
				}
				return result;		
			}
			finally{
				if (showGui){
					ExportUtility.hideProgressBar();
				}
			}
		}
		
		public JsonExternResourceList saveResourceToFolder(string baseDir, bool showGui, List<JsonScene> scenes, Logger logger){		
			Logger.makeValid(ref logger);
			var result = new JsonExternResourceList();
			
			result.scenes = saveResourcesToPath(baseDir, scenes, 
				(objData) => objData, (obj) => obj.name, "scene", showGui);
			result.terrains = saveResourcesToPath(baseDir, terrains.objectList, 
				(objData) => new JsonTerrainData(objData, this), (obj) => obj.name, "terrainData", showGui);
			result.meshes = saveResourcesToPath(baseDir, meshes.objectList, 
				(objData) => new JsonMesh(objData, this), (obj) => obj.name, "mesh", showGui);
			result.materials = saveResourcesToPath(baseDir, materials.objectList, 
				(objData) => {
					var mat = new JsonMaterial(objData, this);
					if (!mat.supportedShader){
						logger.logWarningFormat("Possibly unsupported shader \"{0}\" in material \"{1}\"(#{2}, path \"{3}\"). " + 
							"Correct information transfer is not guaranteed.",
							mat.shader, mat.name, mat.id, mat.path);
					}
					return mat; 
				}, (obj) => obj.name, "material", showGui);
			result.textures = saveResourcesToPath(baseDir, textures.objectList, 
				(objData) => new JsonTexture(objData, this), (obj) => obj.name, "texture", showGui);
			result.cubemaps = saveResourcesToPath(baseDir, cubemaps.objectList, 
				(objData) => new JsonCubemap(objData, this), (obj) => obj.name, "cubemap", showGui);
			result.audioClips = saveResourcesToPath(baseDir, audioClips.objectList, 
				(objData) => new JsonAudioClip(objData, this), (obj) => obj.name, "audioClip", showGui);
				
			var skeletons = meshSkeletons.Values.ToList();
			skeletons.Sort((x, y) => x.id.CompareTo(y.id));
			result.skeletons = saveResourcesToPath(baseDir, skeletons, 
				(objData) => objData, (obj) => obj.name, "skeleton", showGui);			
				
			var prefabList = makePrefabList();
			result.prefabs = saveResourcesToPath(baseDir, prefabList, 
				(objData) => objData, (obj) => obj.name, "prefab", showGui);
			/*result.prefabs = saveResourcesToPath(baseDir, prefabs.objectMap, 
				(objData) => new JsonPref"prefab");*/
			result.resources = new List<string>(resources);
			result.resources.Sort();
			
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
