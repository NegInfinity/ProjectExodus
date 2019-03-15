using UnityEngine;
using UnityEditor;
using System.Collections.Generic;
using System.Linq;

namespace SceneExport{
	[System.Serializable]
	public class MeshUsageFlags{
		public bool triMeshCollision = false;
		public bool convexMeshCollision = false;
	}

	[System.Serializable]
	public class MeshDefaultSkeletonData{
		public Transform defaultRoot;
		public Transform meshNodeTransform;
		public List<string> defaultBoneNames;	
			
		public MeshDefaultSkeletonData(Transform defaultRoot_, Transform meshNodeTransform_, List<string> defaultBoneNames_){
			defaultRoot = defaultRoot_;
			meshNodeTransform = meshNodeTransform_;
			defaultBoneNames = defaultBoneNames_;
		}
	};

	[System.Serializable]
	public class ResourceMapper{
		protected ObjectMapper<Texture> textures = new ObjectMapper<Texture>();
		
		public ObjectMapper<Material> materials = new ObjectMapper<Material>();
		public ObjectMapper<Cubemap> cubemaps = new ObjectMapper<Cubemap>();
		public ObjectMapper<AudioClip> audioClips = new ObjectMapper<AudioClip>();

		public ObjectMapper<AnimationClipKey> animationClips = new ObjectMapper<AnimationClipKey>();
		
		public ObjectMapper<Animator> characterPrefabAnimators = new ObjectMapper<Animator>();
		
		public ObjectMapper<AnimatorControllerKey> animatorControllers = new ObjectMapper<AnimatorControllerKey>();

		/*
			Well, do to differences in handling unity's adn unreal skeletal meshes, we can no longer use naked mesh to uniquely id
			mesh being used.
			
			So we're going to pair it with prefab root and.... I guess skeleton root as well.
			This won't be needed for non-skinned meshes, of course.
		*/
		public SkeletonRegistry skelRegistry = new SkeletonRegistry();

		public ObjectMapper<MeshStorageKey> meshes = new ObjectMapper<MeshStorageKey>();
		public Dictionary<ResId, MeshUsageFlags> meshUsage = new Dictionary<ResId, MeshUsageFlags>();

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
		
		public ResId findAnimatorControllerId(UnityEditor.Animations.AnimatorController obj, Animator animator){
			return animatorControllers.getId(new AnimatorControllerKey(obj, animator), false);
		}
		
		public ResId getAnimatorControllerId(UnityEditor.Animations.AnimatorController obj, Animator animator){
			if (!animator || !obj)
				return ResId.invalid;
			return animatorControllers.getId(new AnimatorControllerKey(obj, animator), true);
		}
		
		public ResId getTerrainId(TerrainData data){
			return terrains.getId(data, true);
		}
		
		public ResId findTerrainId(TerrainData terrain){
			return terrains.getId(terrain, false);
		}

		public ResId getAudioClipId(AudioClip clip){
			return audioClips.getId(clip, true);
		}
		
		public ResId findAudioClipId(AudioClip clip){
			return audioClips.getId(clip, false);
		}
		
		public ResId getTextureId(Texture tex){
			return textures.getId(tex, true);
		}
		
		public ResId findTextureId(Texture tex){
			return textures.getId(tex, false);
		}
		
		public ResId getCubemapId(Cubemap cube){
			return cubemaps.getId(cube, true);
		}
		
		public ResId findCubemapId(Cubemap cube){
			return cubemaps.getId(cube, false);
		}
		
		public MeshIdData getMeshId(GameObject srcObj){
			return new MeshIdData(srcObj, this);
		}

		public ResId getMeshId(Mesh obj){
			var key = new MeshStorageKey(obj);
			return meshes.getId(key, true);
		}

		public bool isValidMeshId(ResId id){
			return meshes.isValidId(id);
		}

		public void flagMeshId(ResId meshId, bool flagConvexMesh, bool flagTriMesh){
			if (!meshes.isValidId(meshId))
				throw new System.ArgumentException(string.Format("invalid mesh id {0}", meshId));
			
			var flags = meshUsage.getValOrSetDefault(meshId);
			flags.convexMeshCollision |= flagConvexMesh;
			flags.triMeshCollision |= flagTriMesh;
		}

		public MeshUsageFlags GetMeshUsageFlags(ResId meshId){
			return meshUsage.getValOrDefault(meshId);
		}

		public ResId findAnimationClipId(AnimationClip animClip, Animator animator){
			return animationClips.getId(new AnimationClipKey(animClip, animator), false);
		}
		
		public ResId getAnimationClipId(AnimationClip animClip, Animator animator){
			return animationClips.getId(new AnimationClipKey(animClip, animator), true);
		}
		
		public ResId findMeshId(Mesh obj){
			var key = new MeshStorageKey(obj);
			return meshes.getId(key, false);
		}
		
		public ResId getMaterialId(Material obj){
			return materials.getId(obj, true);
		}
		
		public ResId findMaterialId(Material obj){
			return materials.getId(obj, false);
		}

		public void registerResource(string path){
			resources.Add(path);
		}
		
		public void registerTexture(Texture tex){
			//delayedTextures.getId(tex);
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

		ResId getOrRegMeshId(MeshStorageKey meshKey, GameObject obj, Mesh mesh){
			if (!mesh){
				return ResId.invalid;
			}
			ResId result = meshes.getId(meshKey, true, null);
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
			 var prefabRoot = Utility.getSrcPrefabAssetObject(
				 Utility.getPrefabInstanceRoot(meshRend.gameObject),
				 //PrefabUtility.FindPrefabRoot(meshRend.gameObject),
				false
			);
			 
			 Transform skeletonRoot = null;
			 if (includeSkeleton){
			 	skeletonRoot = Utility.getSrcPrefabAssetObject(JsonSkeletonBuilder.findSkeletonRoot(meshRend), false);
			 }
			 return new MeshStorageKey(mesh, prefabRoot, skeletonRoot);
		}
		
		
		public ResId getOrRegMeshId(SkinnedMeshRenderer meshRend, Transform skeletonRoot){
			var mesh = meshRend.sharedMesh;
			if (!mesh)
				return ResId.invalid;//ExportUtility.invalidId;
				
			var meshKey = buildMeshKey(meshRend, true);
			skelRegistry.tryRegisterMeshSkeleton(meshKey, meshRend, skeletonRoot);
			
			return getOrRegMeshId(meshKey, meshRend.gameObject, mesh);
		}
		
		public ResId getOrRegMeshId(MeshFilter meshFilter){
			var mesh = meshFilter.sharedMesh;
			if (!mesh)
				return ResId.invalid;//ExportUtility.invalidId;
				
			return getOrRegMeshId(new MeshStorageKey(mesh), meshFilter.gameObject, mesh);
		}
		
		public ResId getPrefabObjectId(GameObject obj, bool createMissing){
			if (!obj)
				return ResId.invalid;//ExportUtility.invalidId;
				
			var linkedPrefab = ExportUtility.getLinkedPrefab(obj);
			if (!linkedPrefab)
				return ResId.invalid;//ExportUtility.invalidId;
			
			var rootPrefabId = getRootPrefabId(obj, createMissing);
			if (!ExportUtility.isValidId(rootPrefabId))
				return ResId.invalid;//ExportUtility.invalidId;
				
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
					getOrRegMeshId(meshRend, null);
				}
			}
			prefabObjects.Add(rootPrefab, newMapper);
		}
		
		public ResId getRootPrefabId(GameObject obj, bool createMissing){
			if (!obj)
				return ResId.invalid;//ExportUtility.invalidId;
			var rootPrefab = ExportUtility.getLinkedRootPrefabAsset(obj);
			if (!rootPrefab)
				return ResId.invalid;//ExportUtility.invalidId;
			
			var result = prefabs.getId(rootPrefab, createMissing, onNewRootPrefab);
			
			return result;
		}
		
		public ResId gatherPrefabData(GameObject obj){
			if (!obj)
				return ResId.invalid;//ExportUtility.invalidId;
			return getRootPrefabId(obj, true);
		}
		
		List<JsonPrefabData> makePrefabList(){
			var result = new List<JsonPrefabData>();

			for(int i = 0; i < prefabs.objectList.Count; i++){
				var src = prefabs.getObjectByIndex(i);
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
		
		static string saveResourceToPath<DstClassType, SrcObjType>(string baseDir, 
				SrcObjType srcObj, int objIndex, int objCount,
				IndexedObjectConverter<SrcObjType, DstClassType> converter,
				System.Func<DstClassType, string> nameFunc, 
				string baseName, bool showGui) where DstClassType: IFastJsonValue{

				if (showGui){
					ExportUtility.showProgressBar(
						string.Format("Saving file #{0} of resource type {1}", objIndex + 1, baseName), 
						"Writing json data", objIndex, objCount);
				}
				var jsonObj = converter(srcObj, objIndex);	
				string fileName;
				if (nameFunc != null){
					fileName = makeJsonResourcePath(baseName, nameFunc(jsonObj), objIndex);	
				}
				else{
					fileName = makeJsonResourcePath(baseName, objIndex);	
				}
				var fullPath = System.IO.Path.Combine(baseDir, fileName);
				
				jsonObj.saveToJsonFile(fullPath);
				return fileName;
		}

		static bool saveResourcesToPath<ClassType, SrcObjType>(
				List<string> outObjectPaths,
				ref int lastCount,
				string baseDir, 
				List<SrcObjType> objects,
				IndexedObjectConverter<SrcObjType, ClassType> converter,
				System.Func<ClassType, string> nameFunc, string baseName, bool showGui) 
				where ClassType: IFastJsonValue{
				
			if (converter == null)
				throw new System.ArgumentNullException("converter");
				
			bool result = false;
			try{
				if (objects != null){
					//for(int i = 0; i < objects.Count; i++){
					for(int i = lastCount; i < objects.Count; i++){
						outObjectPaths.Add(
							saveResourceToPath(
								baseDir, objects[i], i, objects.Count, 
								converter, nameFunc, baseName, showGui
							)
						);
						result = true;
					}
				}
				lastCount = objects.Count;
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
		
		//Looks like I can't move this out of here for now...
		JsonMesh fixSkinMeshRootBoneTransform(MeshStorageKey meshKey, JsonMesh srcMesh){
			//return srcMesh;
			//let's check if we even NEED transformation.
			//Root			
			bool largeRootTransformFound = false;
			{
				var desired = Matrix4x4.identity;
				var current = meshKey.skeletonRoot.localToWorldMatrix;
				var maxRootDiff = SkeletalMeshTools.getMaxDifference(desired, current);
				if (maxRootDiff > SkeletalMeshTools.matrixEpsilon){
					Debug.LogFormat(
						string.Format("Large root matrix transform difference foune on mesh {0}, mesh will be transformed to accomodate." 
							+ "\ncurrent:\n{1}\ndesired:\n{2}\n",
							meshKey.mesh.name, current, desired));
					largeRootTransformFound = true;
				}
			}
			
			if (!largeRootTransformFound)
				return srcMesh;
			
			/*
				We're now bringing skleletal mesh into rootSpace. 
			*/
			var srcRootTransform = meshKey.prefab.transform.localToWorldMatrix;
			var srcRootInvTransform = meshKey.prefab.transform.worldToLocalMatrix;
			/*
				Aw, damn it. Sksleton root is not going to cut it. We need mesh node itself.
				
				So, in unity it is, by default: (Right to left notation)				
				ResultTransform = targetBoneTransform * bindPose * meshTransform. 				
			*/
			var skelData = skelRegistry.getDefaultSkeletonData(meshKey);
			if (skelData == null){
				throw new System.ArgumentException(
					string.Format("Coudl not locate default skeleton data for {0}", meshKey));
			}
				
			if (!skelData.meshNodeTransform){
				throw new System.ArgumentException(
					string.Format("mesh node transform is not set for {0}", meshKey)
				);
			}
				
			var nodeMatrix = skelData.meshNodeTransform.localToWorldMatrix;
			var nodeInvMatrix = skelData.meshNodeTransform.worldToLocalMatrix;
				
			//var relativeMatrix = Utility.getRelativeMatrix(skelData.meshNodeTransform, meshKey.prefab.transform);
			var transformMatrix = srcRootInvTransform * nodeMatrix;
				
			var newMesh = new JsonMesh(srcMesh);
			newMesh.transformMeshWith(transformMatrix);
			
			var meshMatrix = Utility.getRelativeMatrix(skelData.meshNodeTransform, meshKey.prefab.transform);
			var meshInvMatrix = Utility.getRelativeInverseMatrix(skelData.meshNodeTransform, meshKey.prefab.transform);
			
			newMesh.processBindPoses((bindPose, index) => {
				/*
				Givent that i'm getting error that's squarely factor of 10 while bones themselves have scale factor of 100
				(thanks, fbx exporter), it means I failed to accomodate for removal of mesh transform. Let's see...
				*/
				var newTransform = meshInvMatrix * bindPose;
				Debug.LogFormat("Converting bindpose {0}:\noriginal:\n{1}\nnew:\n{2}\nmesh:\n{3}\nmeshInv:\n{4}\nroot:\n{5}\nrootInv:\n{6}", 
					index, bindPose, newTransform, meshMatrix, meshInvMatrix, srcRootTransform, srcRootInvTransform);
				return newTransform;
			});
			
			return newMesh;
		}
		
		JsonMesh makeJsonMesh(MeshStorageKey meshKey, int id){
			var result = new JsonMesh(meshKey, id, this);
			if (!meshKey.skeletonRoot || !meshKey.prefab)
				return result;
			
			if (!isSkeletalMesh(meshKey.mesh)){
				return result;
			}
			
			/*
				Doing this in steps, as it is possible to have root transform issue and then somehow no bindpose issues.
				I mean, miracles can happen, right?
			*/
			result = fixSkinMeshRootBoneTransform(meshKey, result);
			
			//Currently disabled
			//result = fixSkinMeshPosedBones(meshKey, result);
			
			return result;
		}
		
		/*
		Essentually the whole unity scene is a resource graph.
		There are two resource types - "leaf" resources and "node" resources.
		Leafs are only referenced by something, while nodes can reference leafs and other nodes via Ids....

		Well, the problem is we don't know how deep the rabbit hole goes, and one referenced resource can pull in other referenced
		resources and so on. Given that a project can be huge, loading everything into memory at once might be unwise.

		As a result exporter will have to loop through the resources multiple times to acomodate for resoruces that were 
		possibly introduced. Henc the ugly "saveResourcesToPath" call with many many parameters.

		Originally I tried to avoid that by forcing specific order of initialziation which would eradicat possibility of
		missing resources, but then I got prefabs and animator contorllers...
		*/
		public JsonExternResourceList saveResourceToFolder(string baseDir, bool showGui, List<JsonScene> scenes, Logger logger){		
			Logger.makeValid(ref logger);
			var result = new JsonExternResourceList();

			int lastSceneCount = 0, lastTerrainCount = 0, lastMeshCount = 0,
				lastMaterialCount = 0, lastTextureCount = 0, lastCubemapCount = 0,
				lastAudioClipCount = 0, lastSkeletonCount = 0, lastPrefabCount = 0,
				lastAnimControllerCount = 0, lastAnimClipCount = 0;

			bool processObjects = true;
			
			processObjects = false;
			processObjects |= saveResourcesToPath(result.scenes, ref lastSceneCount, baseDir, scenes, 
				(objData, i) => objData, (obj) => obj.name, "scene", showGui);
			processObjects |= saveResourcesToPath(result.terrains, ref lastTerrainCount, baseDir, terrains.objectList, 
				(objData, i) => new JsonTerrainData(objData, this), (obj) => obj.name, "terrainData", showGui);
			processObjects |= saveResourcesToPath(result.meshes, ref lastMeshCount, baseDir, meshes.objectList, 
				(meshKey, id) => makeJsonMesh(meshKey, id), (obj) => obj.name, "mesh", showGui);
			processObjects |= saveResourcesToPath(result.materials, ref lastMaterialCount, baseDir, materials.objectList, 
				(objData, i) => {
					var mat = new JsonMaterial(objData, this);
					if (!mat.supportedShader){
						logger.logWarningFormat("Possibly unsupported shader \"{0}\" in material \"{1}\"(#{2}, path \"{3}\"). " + 
							"Correct information transfer is not guaranteed.",
							mat.shader, mat.name, mat.id, mat.path);
					}
					return mat; 
				}, (obj) => obj.name, "material", showGui);

			processObjects |= saveResourcesToPath(result.textures, ref lastTextureCount, baseDir, textures.objectList, 
				(objData, id) => new JsonTexture(objData, this), (obj) => obj.name, "texture", showGui);
			processObjects |= saveResourcesToPath(result.cubemaps, ref lastCubemapCount, baseDir, cubemaps.objectList, 
				(objData, id) => new JsonCubemap(objData, this), (obj) => obj.name, "cubemap", showGui);
			processObjects |= saveResourcesToPath(result.audioClips, ref lastAudioClipCount, baseDir, audioClips.objectList, 
				(objData, id) => new JsonAudioClip(objData, this), (obj) => obj.name, "audioClip", showGui);


			for(int i = lastSkeletonCount; i < skelRegistry.numSkeletons; i++){
								
			}	
			lastSkeletonCount = skelRegistry.numSkeletons;

			var skeletons = skelRegistry.getAllSkeletons().ToList();//skelRegistry.jsonSkeletons.Values.ToList();//<== needs a new class for htis 
			skeletons.Sort((x, y) => x.id.objectIndex.CompareTo(y.id.objectIndex));
			processObjects |= saveResourcesToPath(result.skeletons, ref lastSkeletonCount, baseDir, skeletons, 
				(objData, id) => objData, (obj) => obj.name, "skeleton", showGui);			
				
			//var prefabList = makePrefabList();//Errrrm.... TODO: This needs fixing.			
			//Why was this even used?
			processObjects |= saveResourcesToPath<JsonPrefabData, GameObject>(result.prefabs, ref lastPrefabCount, baseDir, prefabs.objectList, 
				(objData, id) => new JsonPrefabData(objData, this), 
				(obj) => obj.name, 
				"prefab", showGui);
			/*saveResourcesToPath(result.prefabs, ref lastPrefabCount, baseDir, prefabList, 
				(objData, id) => objData, (obj) => obj.name, "prefab", showGui);*/
				
			processObjects |= saveResourcesToPath(result.animatorControllers, ref lastAnimControllerCount, 
			 	baseDir, animatorControllers.objectList,
				(objData, id) => new JsonAnimatorController(objData.controller, objData.animator, id, this), 
					(obj) => obj.name, "animatorController", showGui);
				
			processObjects |= saveResourcesToPath(result.animationClips, ref lastAnimClipCount, 
				baseDir, animationClips.objectList,
				(objData, id) => new JsonAnimationClip(objData.animClip, objData.animator, id, this), 
				(obj) => obj.name, "animationClip", showGui);
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
			
			result.animatorControllers = animatorControllers.objectList
				.Select((arg, idx) => new JsonAnimatorController(arg.controller, arg.animator, idx, this)).ToList();
			result.animationClips = animationClips.objectList
				.Select((arg1, idx) => new JsonAnimationClip(arg1.animClip, arg1.animator, idx, this)).ToList();
				
			result.resources = new List<string>(resources);
			result.resources.Sort();
			
			return result;
		}
	}
}
