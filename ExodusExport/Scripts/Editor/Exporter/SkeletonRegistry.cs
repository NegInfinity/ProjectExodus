using UnityEngine;
using UnityEditor;
using System.Linq;

using System.Collections.Generic;

namespace SceneExport{
	/*
	There are differences in handling skeletal meshes and animation between unity and unreal.

	Unity does not /have/ a concept of skeleton, meaning there's no specific object we can poke and query with questions like
	"how many bones", "what are default bone names", and so on.

	As such it is assumed that skeletons are /prefabs/ that store skeletal meshes.
	*/
	public class SkeletonRegistry{
		Dictionary<Transform, JsonSkeleton> jsonSkeletons = new Dictionary<Transform, JsonSkeleton>();
		//public Dictionary<Transform, ResId> jsonSkeletons = new Dictionary<Transform, ResId>();
		Dictionary<ResId, Transform> jsonSkeletonRootTransforms = new Dictionary<ResId, Transform>();
		Dictionary<MeshStorageKey, MeshDefaultSkeletonData> meshDefaultSkeletonData = new Dictionary<MeshStorageKey, MeshDefaultSkeletonData>();
		//public List<JsonSkeleton> skeletons = new List<JsonSkeleton>();		

		public int numSkeletons{
			get{
				return jsonSkeletons.Count;
			}
		}

		public IEnumerable<JsonSkeleton> getAllSkeletons(){
			foreach(var cur in jsonSkeletons.Values)
				yield return cur;
		}

		public MeshDefaultSkeletonData getDefaultSkeletonData(MeshStorageKey key){
			return meshDefaultSkeletonData.getValOrDefault(key, null);
		}
		
		public ResId getDefaultSkeletonId(MeshStorageKey key){
			var skel = getDefaultSkeleton(key);
			if (skel == null)
				return ResId.invalid;//ExportUtility.invalidId;
			return skel.id;
		}

		public string getDefaultMeshNodeName(MeshStorageKey key){
			var tmp = meshDefaultSkeletonData.getValOrDefault(
				key, null);
			if ((tmp != null) && (tmp.meshNodeTransform))
				return tmp.meshNodeTransform.name;
			
			return "";
		}

		public ResourceStorageWatcher<SkeletonRegistry, JsonSkeleton> createWatcher(){
			return new ResourceStorageWatcher<SkeletonRegistry, JsonSkeleton>(
				this, 
				(obj) => obj.numSkeletons, 
				(obj, idx) => obj.getSkeletonByIndex(idx)
			);
		}
		
		public string getDefaultMeshNodePath(MeshStorageKey key){
			var tmp = meshDefaultSkeletonData.getValOrDefault(
				key, null);
			if ((tmp != null) && (tmp.meshNodeTransform))
				return tmp.meshNodeTransform.getScenePath(tmp.defaultRoot);
			
			return "";
		}
		
		public Matrix4x4 getDefaultMeshNodeMatrix(MeshStorageKey key){
			var tmp = meshDefaultSkeletonData.getValOrDefault(
				key, null);
			if ((tmp != null) && (tmp.meshNodeTransform))
				Utility.getRelativeMatrix(tmp.meshNodeTransform, tmp.defaultRoot);
			
			return Matrix4x4.identity;
		}
		
		public List<string> getDefaultBoneNames(MeshStorageKey key){
			var tmp = meshDefaultSkeletonData.getValOrDefault(
				key, null);
			if (tmp == null)
				return new List<string>();
			return tmp.defaultBoneNames.ToList();
		}

		public JsonSkeleton getDefaultSkeleton(MeshStorageKey key){
			var defaultData = getDefaultSkeletonData(key);
			if (defaultData == null)
				return null;
				
			var skel = jsonSkeletons.getValOrDefault(defaultData.defaultRoot, null);
			return skel;
		}

		public Transform getSkeletonTransformById(ResId id){
			//if ((id < 0) || (id >= jsonSkeletons.Count))
			if (!id.isValid || (id.objectIndex >= jsonSkeletons.Count))
				throw new System.ArgumentException(string.Format("Invalid skeleton id %d", id));
			
			var skelTransform = jsonSkeletonRootTransforms.getValOrDefault(id, null);
			return skelTransform;
		}

		public JsonSkeleton getSkeletonByIndex(int index){
			return getSkeletonById(ResId.fromObjectIndex(index));
		}

		public JsonSkeleton getSkeletonById(ResId id){
			var skelTransform = getSkeletonTransformById(id);
			if (!skelTransform)
				throw new System.ArgumentException(string.Format("skeleton with id {0} not found", id));
				
			var result = jsonSkeletons.getValOrDefault(skelTransform, null);
			if (result == null)
				throw new System.ArgumentException(string.Format("skeleton with id {0} not found", id));
				
			return result;
			//return jsonSkeletons.[id];
		}

		public ResId registerSkeleton(Transform rootTransform, bool findPrefab){
			if (findPrefab)
				rootTransform = Utility.getSrcPrefabAssetObject(rootTransform, false);
				
			JsonSkeleton skel = null;
			if (jsonSkeletons.TryGetValue(rootTransform, out skel))
				return skel.id;
			var newSkel = JsonSkeletonBuilder.buildFromRootTransform(rootTransform);
			var id = ResId.fromObjectIndex(jsonSkeletons.Count);
			newSkel.id = id;
					
			jsonSkeletons.Add(rootTransform, newSkel);
			jsonSkeletonRootTransforms.Add(newSkel.id, rootTransform);
			return newSkel.id;
		}		

		//public ResId getOrRegMeshId(SkinnedMeshRenderer meshRend, Transform skeletonRoot){
		public void tryRegisterMeshSkeleton(MeshStorageKey meshKey, SkinnedMeshRenderer meshRend, Transform skeletonRoot){
			Sanity.nullCheck(meshKey.mesh, "mesh cannot be null");
				
			//var meshKey = buildMeshKey(meshRend, true);
			//TODO - this needs to be moved into a subroutine of SkeletonRepository
			if (!meshDefaultSkeletonData.ContainsKey(meshKey)){				
				var rootTransform = skeletonRoot;//JsonSkeletonBuilder.findSkeletonRoot(meshRend);
				if (!rootTransform)
					rootTransform  = JsonSkeletonBuilder.findSkeletonRoot(meshRend);
				if (!rootTransform)
					throw new System.ArgumentException(
						string.Format("Could not find skeleton root transform for {0}", meshRend));
						
				var boneNames = meshRend.bones.Select((arg) => arg.name).ToList();
				
				var meshNode = Utility.getSrcPrefabAssetObject(meshRend.gameObject.transform, false);
				var defaultData = new MeshDefaultSkeletonData(rootTransform, meshNode, boneNames);
				meshDefaultSkeletonData.Add(meshKey, defaultData);
				
				registerSkeleton(rootTransform, false);
			}
			
			//return getOrRegMeshId(meshKey, meshRend.gameObject, mesh);
		}
	}
}