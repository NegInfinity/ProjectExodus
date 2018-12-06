using UnityEngine;
using UnityEditor;
using System.Collections.Generic;
using System.Linq;

namespace SceneExport{
	public class ResourceMapper{
		public ObjectMapper<Texture> textures = new ObjectMapper<Texture>();
		
		public ObjectMapper<Material> materials = new ObjectMapper<Material>();
		public ObjectMapper<Cubemap> cubemaps = new ObjectMapper<Cubemap>();
		public ObjectMapper<AudioClip> audioClips = new ObjectMapper<AudioClip>();

		[System.Serializable]
		public struct MeshStorageKey{
			public readonly Mesh mesh;//Immutability, huh. Ugh.
			public readonly GameObject prefab;
			public readonly Transform skeletonRoot;
			
			public override string ToString(){
				return string.Format("[MeshStorageKey]{{ mesh: {0}({1}); prefab: {2}({3}); skeletonRoot: {4}({5}) }}", 
					mesh, mesh ? mesh.GetInstanceID(): 0, 
					prefab, prefab ? prefab.GetInstanceID(): 0, 
					skeletonRoot, skeletonRoot ? skeletonRoot.GetInstanceID(): 0
				);
			}
			
			public override int GetHashCode(){
				int hash = 17;
				hash = hash * 23 + (mesh ? mesh.GetHashCode(): 0);
				hash = hash * 23 + (prefab ? prefab.GetHashCode(): 0);
				hash = hash * 23 + (skeletonRoot ? skeletonRoot.GetHashCode(): 0);
				return hash;
			}
			
			public override bool Equals(object obj){
				if (obj is MeshStorageKey){
					return this.Equals((MeshStorageKey)obj);
				}
				
				return false;
			}
			
			public bool Equals(MeshStorageKey other){
				return (mesh == other.mesh) 
					&& (prefab == other.prefab)
					&& (skeletonRoot == other.skeletonRoot);					
			}
			
			public MeshStorageKey(Mesh mesh_, GameObject prefab_ = null, Transform skeletonRoot_ = null){
				mesh = mesh_;
				prefab = prefab_;
				skeletonRoot = skeletonRoot_;
			}
		};
		
		[System.Serializable]
		public class MeshDefaultSkeletonData{
			public Transform defaultRoot;
			public List<string> defaultBoneNames;			
			
			public MeshDefaultSkeletonData(Transform defaultRoot_, List<string> defaultBoneNames_){
				defaultRoot = defaultRoot_;
				defaultBoneNames = defaultBoneNames_;
			}
		};

		/*
			Well, do to differences in handling unity's adn unreal skeletal meshes, we can no longer use naked mesh to uniquely id
			mesh being used.
			
			So we're going to pair it with prefab root and.... I guess skeleton root as well.
			This won't be needed for non-skinned meshes, of course.
		*/
		Dictionary<Transform, JsonSkeleton> jsonSkeletons = new Dictionary<Transform, JsonSkeleton>();
		public ObjectMapper<MeshStorageKey> meshes = new ObjectMapper<MeshStorageKey>();
		//Dictionary<Mesh, MeshDefaultSkeletonData> meshDefaultSkeletonData = new Dictionary<Mesh, MeshDefaultSkeletonData>();
		Dictionary<MeshStorageKey, MeshDefaultSkeletonData> meshDefaultSkeletonData = new Dictionary<MeshStorageKey, MeshDefaultSkeletonData>();
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
			var key = new MeshStorageKey(obj);
			return meshes.getId(key, true);
		}
		
		public int findMeshId(Mesh obj){
			var key = new MeshStorageKey(obj);
			return meshes.getId(key, false);
		}
		
		public List<string> getDefaultBoneNames(MeshStorageKey key){
			var tmp = meshDefaultSkeletonData.getValOrDefault(
				key, null);
			if (tmp == null)
				return new List<string>();
			return tmp.defaultBoneNames.ToList();
		}
		
		/*
		public List<string> getDefaultBoneNames(Mesh mesh){
			return getDefaultBoneNames(new MeshStorageKey(mesh));
		}
		*/
		
		public JsonSkeleton getDefaultSkeleton(MeshStorageKey key){
			var defaultData = meshDefaultSkeletonData.getValOrDefault(key, null);
			if (defaultData == null)
				return null;
				
			var skel = jsonSkeletons.getValOrDefault(defaultData.defaultRoot, null);
			return skel;
		}
		
		public int getDefaultSkeletonId(MeshStorageKey key){
			var skel = getDefaultSkeleton(key);
			if (skel == null)
				return ExportUtility.invalidId;
			return skel.id;
		}
		
		/*
		public int getDefaultSkeletonId(Mesh meshObj){
			return getDefaultSkeletonId(new MeshStorageKey(meshObj));
		}
		*/
		
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

		int getOrRegMeshId(MeshStorageKey meshKey, GameObject obj, Mesh mesh){
			Debug.LogFormat("getOrRegMeshId: {0}, {1}, {2}", meshKey, obj, mesh);
			if (!mesh){
				Debug.LogFormat("Mesh is null");
				return ExportUtility.invalidId;
			}
			//var meshKey = new MeshStorageKey(mesh);
			int result = meshes.getId(meshKey, true, null);
			Debug.LogFormat("id found: {0}", result);
			if (meshMaterials.ContainsKey(mesh))
				return result;
							
			var r = obj.GetComponent<Renderer>();
			if (r){
				meshMaterials[mesh] = new List<Material>(r.sharedMaterials);
			}
			return result;
		}
		
		public MeshStorageKey buildMeshKey(SkinnedMeshRenderer meshRend, bool includeSkeleton){
			var mesh = meshRend.sharedMesh;
			 
			 /*
			 Subtle. FindPrefabRoot does not find the root of the original object, but rather a root of object currently being processed.
			 Meaning if you apply it to an instanced prefab, it'll find root of that that instanced prefab in the scene, and not the original asset
			 */
			 var prefabRoot = PrefabUtility.FindPrefabRoot(meshRend.gameObject);
			 var linkedPrefabRoot = PrefabUtility.GetCorrespondingObjectFromSource(prefabRoot) as GameObject;
			 if (linkedPrefabRoot)
			 	prefabRoot = linkedPrefabRoot;
			 
			 Transform skeletonRoot = null;
			 if (includeSkeleton){
			 	skeletonRoot = JsonSkeletonBuilder.findSkeletonRoot(meshRend);
			 	var skeletonPrefab = PrefabUtility.GetCorrespondingObjectFromSource(skeletonRoot) as Transform;
			 	if (skeletonPrefab)
			 		skeletonRoot = skeletonPrefab;
			 }
			 return new MeshStorageKey(mesh, prefabRoot, skeletonRoot);
		}
		
		public int getOrRegMeshId(SkinnedMeshRenderer meshRend){
			var mesh = meshRend.sharedMesh;
			if (!mesh)
				return ExportUtility.invalidId;
				
			var meshKey = buildMeshKey(meshRend, true);
			if (!meshDefaultSkeletonData.ContainsKey(meshKey)){				
				var rootTransform = JsonSkeletonBuilder.findSkeletonRoot(meshRend);
				if (!rootTransform)
					throw new System.ArgumentException(
						string.Format("Could not find skeleton root transform for {0}", meshRend));
						
				var boneNames = meshRend.bones.Select((arg) => arg.name).ToList();
				
				var defaultData = new MeshDefaultSkeletonData(rootTransform, boneNames);
				meshDefaultSkeletonData.Add(meshKey, defaultData);
				
				if (!jsonSkeletons.ContainsKey(rootTransform)){
					var newSkel = JsonSkeletonBuilder.buildFromRootTransform(rootTransform);
					var id = jsonSkeletons.Count;
					newSkel.id = id;
					
					//newSkel.defaultBoneNames.Clear();
					//newSkel.defaultBoneNames.AddRange(meshRend.bones.Select((arg) => arg.name));;
					jsonSkeletons.Add(rootTransform, newSkel);
				}
				//meshDefaultSkeletonRoots.Add(mesh, rootTransform);
			}
			
			return getOrRegMeshId(meshKey, meshRend.gameObject, mesh);
		}
		
		public int getOrRegMeshId(MeshFilter meshFilter){
			var mesh = meshFilter.sharedMesh;
			if (!mesh)
				return ExportUtility.invalidId;
				
			return getOrRegMeshId(new MeshStorageKey(mesh), meshFilter.gameObject, mesh);
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
			var rootPrefab = ExportUtility.getLinkedRootPrefabAsset(obj);
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
		
		public delegate ReturnType IndexedObjectConverter<SrcType, ReturnType> (SrcType src, int index);
		
		static List<string> saveResourcesToPath<ClassType, ObjType>(string baseDir, 
				List<ObjType> objects, 
				IndexedObjectConverter<ObjType, ClassType> converter,
				//System.Func<ObjType, ClassType> converter, 
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
						var jsonObj = converter(objects[i], i);	
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
		
		static bool isSkeletalMesh(Mesh mesh){
			return mesh
				&& ((mesh.bindposes.Length > 0)
				||(mesh.blendShapeCount > 0)
				||(mesh.boneWeights.Length > 0));
				/*
					Any of those indicate that this is likely a skeletal mesh, as blendhsape rendering goes through
					skinned mesh too.
					Although setting bone weighs but not bindposes woudl probably mean
					someone has a sick sense of humor.
				*/
		}
		
		static float getMaxDifference(Matrix4x4 m1, Matrix4x4 m2){
			float maxDiff = 0.0f;
			for(int i = 0; i < 16; i++){
				var diff = Mathf.Abs(m1[i] - m2[i]);
				maxDiff = Mathf.Max(maxDiff, diff);
			}
			return maxDiff;
		}
		
		JsonMesh makeJsonMesh(MeshStorageKey meshKey, int id){
			var result = new JsonMesh(meshKey, id, this);
			if (!meshKey.skeletonRoot || !meshKey.prefab)
				return result;
			
			if (!isSkeletalMesh(meshKey.mesh)){
				return result;
			}
			
			const float matrixEpsilon = 0.00001f; //I'll consider matrices equivalent if this is below this threshold
			
			var boneTransforms = Utility.findNamedTransforms(result.defaultBoneNames, meshKey.skeletonRoot);
			for(int i = 0; i < boneTransforms.Count; i++){
				if (!boneTransforms[i]){
					Debug.LogWarningFormat(
						string.Format("Could not locate bone {0}({1}) on mesh {2}", 
						result.defaultBoneNames[i], i, meshKey.mesh)
					);
				}
			}
			
			//let's check if we even NEED transformation.
			//First, bones.
			bool largeBoneTransformFound = false;
			bool largeRootTransformFound = false;
			{
				for(int i = 0; i < boneTransforms.Count; i++){
					var curBone = boneTransforms[i];
					var inverseMatrix  = curBone.worldToLocalMatrix;
					var bindPose = meshKey.mesh.bindposes[i];
					
					var diff = getMaxDifference(inverseMatrix, bindPose);
					if (diff > matrixEpsilon){
						largeBoneTransformFound = true;
						Debug.LogFormat(
							string.Format("Large transform found on mesh {0}, mesh will be transformed to accomodate",
								meshKey.mesh.name));
						break;
					}
				}
			}

			//Then, root			
			{
				var maxRootDiff = getMaxDifference(Matrix4x4.identity, meshKey.skeletonRoot.localToWorldMatrix);
				if (maxRootDiff < matrixEpsilon){
					Debug.LogFormat(
						string.Format("Root matrix nearly identical to original. Max difference {0}. Skipping transformation.", maxRootDiff));
					largeRootTransformFound = true;
				}
			}
			
			if (!largeBoneTransformFound && !largeRootTransformFound){
				return result;
			}
			
			//Oooh boy. Here it comes. The transformation.
			/*
				The fact the mesh has both prefab and skeleton root specified indicats that it is skeletal.
				
				To accomodate for unreal approach to skinning, we now need to transform it into root space of our prefab.
			*/
			
			var transformed = new JsonMesh(result);
			
			var transformMatrices = new List<Matrix4x4>();
			var rootTransform = meshKey.skeletonRoot.transform.localToWorldMatrix;
			var invRootTransform = meshKey.skeletonRoot.transform.worldToLocalMatrix;
			var oldBindPoses = meshKey.mesh.bindposes;
			var rootParentTransform = Matrix4x4.identity;
			var rootParentInvTransform = Matrix4x4.identity;
			
			if (meshKey.skeletonRoot.parent){
				var rootParent = meshKey.skeletonRoot.parent;
				rootParentTransform = rootParent.localToWorldMatrix;
				rootParentInvTransform = rootParent.worldToLocalMatrix;
			}
			
			for(int boneIndex = 0; boneIndex < transformed.bindPoses.Count; boneIndex++){
				var newTransformMatrix = Matrix4x4.identity;
				
				var curBone = boneTransforms[boneIndex];
				if (curBone){
					newTransformMatrix = 
						rootParentInvTransform * curBone.localToWorldMatrix * meshKey.mesh.bindposes[boneIndex];
						//invRootTransform * curBone.localToWorldMatrix * meshKey.mesh.bindposes[boneIndex];
				}
				
				transformMatrices.Add(newTransformMatrix);
			}
			
			transformed.transformSkeletalMesh(transformMatrices);
			transformed.setBindPosesFromTransforms(boneTransforms, meshKey.skeletonRoot);
			
			//
			return transformed;
		}
		
		public JsonExternResourceList saveResourceToFolder(string baseDir, bool showGui, List<JsonScene> scenes, Logger logger){		
			Logger.makeValid(ref logger);
			var result = new JsonExternResourceList();
			
			result.scenes = saveResourcesToPath(baseDir, scenes, 
				(objData, i) => objData, (obj) => obj.name, "scene", showGui);
			result.terrains = saveResourcesToPath(baseDir, terrains.objectList, 
				(objData, i) => new JsonTerrainData(objData, this), (obj) => obj.name, "terrainData", showGui);
			result.meshes = saveResourcesToPath(baseDir, meshes.objectList, 
				(meshKey, id) => makeJsonMesh(meshKey, id), (obj) => obj.name, "mesh", showGui);
			result.materials = saveResourcesToPath(baseDir, materials.objectList, 
				(objData, i) => {
					var mat = new JsonMaterial(objData, this);
					if (!mat.supportedShader){
						logger.logWarningFormat("Possibly unsupported shader \"{0}\" in material \"{1}\"(#{2}, path \"{3}\"). " + 
							"Correct information transfer is not guaranteed.",
							mat.shader, mat.name, mat.id, mat.path);
					}
					return mat; 
				}, (obj) => obj.name, "material", showGui);
			result.textures = saveResourcesToPath(baseDir, textures.objectList, 
				(objData, id) => new JsonTexture(objData, this), (obj) => obj.name, "texture", showGui);
			result.cubemaps = saveResourcesToPath(baseDir, cubemaps.objectList, 
				(objData, id) => new JsonCubemap(objData, this), (obj) => obj.name, "cubemap", showGui);
			result.audioClips = saveResourcesToPath(baseDir, audioClips.objectList, 
				(objData, id) => new JsonAudioClip(objData, this), (obj) => obj.name, "audioClip", showGui);
				
			var skeletons = jsonSkeletons.Values.ToList();
			skeletons.Sort((x, y) => x.id.CompareTo(y.id));
			result.skeletons = saveResourcesToPath(baseDir, skeletons, 
				(objData, id) => objData, (obj) => obj.name, "skeleton", showGui);			
				
			var prefabList = makePrefabList();
			result.prefabs = saveResourcesToPath(baseDir, prefabList, 
				(objData, id) => objData, (obj) => obj.name, "prefab", showGui);
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
			
			for(int i = 0; i < meshes.objectList.Count; i++){
				var cur = meshes.objectList[i];
				result.meshes.Add(new JsonMesh(cur, i, this));
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
