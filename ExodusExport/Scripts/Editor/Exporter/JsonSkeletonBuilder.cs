using UnityEngine;
using UnityEditor;
using System.Collections.Generic;
using System.Linq;

namespace SceneExport{

/*
Alright. This is fun, in dwarf fortrress definition of it.
	
Here's how it goes:
There's no concept of "skeleton" in unity, as skinned mesh objects are linked to naked transforms.
That's IF the bone transforms are exposed to the as Transform objects (they usually are).
	
SkinnedMeshRenderer components hold references to the bones that influence them, and bones are stored as list of transforms.
	
However, SkinnedMeshRenderer does not reference bones that are NOT used by the object! 
Meaning, if you have a piece of clothing that only is influenced by object's upper body, leg bone references will not be stored in it.
Obviously.
	
So. In addition to that, while SkinnedMeshRenderer can report the "root" bone, the property appears to be broken and the mesh will 
lie to you and report bones that aren't root. If Root isn't linked to anything, that is. For example, in SpaceRobotKyle model,
The component repors "chest" bone as root, EXCEPT the mesh is also influenced by hips bone, but they're both linked to the REAL root bone,
Except that real root bone does not influence anything in skeletal mesh renderer, so it doesn't know it exists. 
	
Theoretically we could try deducing root of the animation via Animator/Animation component, BUT those are not guaranteed to be present
at all. For example "Ethan" model in unity does not have them in its prefab form.
	
Additionally... SkinnedMeshRenderer holds "pose transformations" for the model being skinned, and that transformation is valid, however,
ther'es no information about relationship between bones in skin mesh renderer. Meaning, the bone hieararchy is stored as transforms 
in a prefab somewhere.
	
What's more, the orientation of the bones in that default transform is not guaranteed to match t-pose/skin parade pose either, 
if the mesh is within a prefab. That's because user can pose a mesh and stuff it into a prefab anyway. Meaning if we
attempt to build UE skeleton based on this information, we may end up with invalid transformations. If the user posed the model himself, that is.
	
So, what can be done about it?
	
I've decided to take following approach:
	
1. Assume all skeletal meshes are based on prefabs. This is usually true, as nobody sane will be dragging individual mdoels onto scene via raw mesh
references. This can be done, but chances are low and are largely restricted to stiatuions where people do something truly crazy like
procedural skinned mesh geometry.
2. From prefab object harvest all skinned meshes.
3. Gather list of all the bones being used by them.
4. From all those bones find "one root that rules them all!"
	
There IS one bone that is parent to everything. Skinned meshes do not USUALLY live within object hierarchy, and are placed as sibling nodes.
Next to the tree that influences them.
	
5. Assume that the user/creator of asset was sane and placed prefab objects in skin parade pose. 

This... s USUALLY true, but if somebody tries to make poseable statues out of skinned meshes into prefabs, it'll all break to hell due to 
discrepancy between skin transform and currently stored pose transform in a prefab.
	
To bypass that I'd need to software-transform skinned mesh into stored pose, creating new instance of it, 
AND making new bone pose transformations for it.
	
What a mess. 
	
Let's proceed with first attempt, which does not take this madness scenario into account. 
*/

	[System.Serializable]
	public class JsonSkeletonBuilder{
		[System.Serializable]
		public class ShortBoneInfo{
			public string name;
			public int id = -1;
			public int parentIndex = -1;//??
			//public ShortBoneInfo(
		}

		public delegate bool TransformFilter(Transform curTransform);
		public delegate void TransformCallback(Transform curTransform);
		
		static protected void processHierarchy(Transform rootTransform, TransformCallback callback, 
				TransformFilter childFilter = null){
			if (callback == null)
				throw new System.ArgumentNullException("callback");
			if (!rootTransform)
				return;//??
				
			var workList = new List<Transform>();
			workList.Add(rootTransform);
			for(int itemIndex = 0; itemIndex < workList.Count; itemIndex++){
				var curItem = workList[itemIndex];
				if (!curItem)
					throw new System.ArgumentException("Logic error while gathering bone ids");
					
				callback(curItem);
				
				foreach(Transform child in curItem){
					if ((childFilter == null) || (childFilter(child)))
						workList.Add(child);
				}				
			}
		}

		protected static ObjectMapper<Transform> gatherIds(Transform rootTransform, TransformFilter transformFilter = null){
			var result = new ObjectMapper<Transform>();
			if (transformFilter == null){
				processHierarchy(rootTransform, (curNode) => result.registerObject(curNode));
			}
			else{
				processHierarchy(rootTransform, 
					(curNode) => {
						if (transformFilter(curNode)) 
							result.registerObject(curNode);
					}
				);
			}
			return result;
		}
		
		protected static bool isChildOf(Transform parentCandidate, Transform childCandidate){
			if (!parentCandidate || !childCandidate)
				return false;
			var cur = childCandidate;
			while(cur){
				if (cur == parentCandidate)
					return true;
				cur = cur.parent;
			}
			return false;
		}
		
		protected static Transform skipToFirstAnimatedNode(Transform startTransform){
			if (!startTransform)
				throw new System.ArgumentNullException("startTransform");
				
			var curTransform = startTransform;
			while(curTransform){
				var curParent = curTransform.parent;
				if (!curParent)
					break;
				var animComp = curParent.GetComponent<Animation>();
				if (animComp)
					return curParent;//curTransform;
					
				var animatorComp = curParent.GetComponent<Animator>();
				//var animatorComp = curTransform.GetComponent<Animator>();//This way the skeleton will be identical to the one from prefab.
				if (animatorComp)
					return curParent;//curTransform;
				curTransform = curParent;
			}
			
			return startTransform;
		}
		
		protected static Transform findCommonParent<Container>(Container transforms) where Container: IEnumerable<Transform>{
			bool first = true;
			Transform rootTransform = null;
			foreach(var transformIterator in transforms){
				var curTransform = transformIterator;//I can't assign to iteration variable? 
				if (!curTransform)
					throw new System.ArgumentException("Null bones in container");
					
				/*
				Turns out there actually IS an animation node at the start of each prefab and it is conveniently placed below the root bone.
				
				In case it can't be found, it'll just return unchanged current transform.
				*/
				curTransform = skipToFirstAnimatedNode(curTransform);
					
				//var animatorNode = curTransform.GetComponentInParent<Animator>();
				if (first){
					rootTransform = curTransform;
					first = false;
					continue;
				}
				
				while(rootTransform){
					if (isChildOf(rootTransform, curTransform))
						break;
					rootTransform = rootTransform.parent;
				}
				if (!rootTransform)
					return null;
			}
			return rootTransform;
		}
		
		static protected void printWorkList(List<Transform> list){
			var sb = new System.Text.StringBuilder();
			bool first = true;
			foreach(var cur in list){
				sb.AppendFormat(first? "{0}": ", {0}", cur);
				if (first){
					first = false;
				}
			}
			Debug.LogFormat("{0}", sb.ToString());
		}
		
		protected void printSkeleton(ObjectMapper<Transform> bones){
			var childToParent = new Dictionary<Transform, Transform>();
			var parentToChildren = new Dictionary<Transform, List<Transform>>();
			
			foreach(var curChild in bones.objectList){
				var curParent = curChild.parent;
				if (	!curParent
					|| !bones.objectMap.ContainsKey(curChild) 
					|| !bones.objectMap.ContainsKey(curParent))
					continue;
					
				childToParent.Add(curChild, curParent);
				parentToChildren.getValueSetGenerate(curParent, () => new List<Transform>()).Add(curChild);
			}
			
			var workList = new List<Transform>();
			
			System.Func<Transform, int> getLevel = (obj) => {
				int result = 0;
				while(obj && bones.objectMap.ContainsKey(obj)){
					obj = obj.parent;
					result++;
				}
				return result;
			};
			
			workList.AddRange(bones.objectList.Where((x) => !childToParent.ContainsKey(x)));
			//Debug.LogFormat("Default work list:");
			//printWorkList(workList);
			var sb = new System.Text.StringBuilder();
			for(int i = 0; i < workList.Count; i++){
				//Debug.LogFormat("Work index {0} out of {1}", i, workList.Count);
				//printWorkList(workList);
				var curTransform = workList[i];
				int level = getLevel(curTransform);
				var str = string.Format("{0}{1}", new string('-', level), curTransform.name);
				sb.AppendLine(str);
				//Debug.LogFormat(str);
				for(int childIndex = 0; childIndex < curTransform.childCount; childIndex++){
					var curChild = curTransform.GetChild(childIndex);
					workList.Insert(i + childIndex + 1, curChild);
					//Debug.LogFormat("Adding child {0} out of {1}", childIndex, curTransform.childCount);
					//printWorkList(workList);
				}
			}
			Debug.LogFormat(sb.ToString());
		}
		
		protected static List<Transform> findSkinMeshRoots(SkinnedMeshRenderer[] skinRenderers){
			var roots = new List<Transform>();
			foreach(var curSkinRend in skinRenderers){
				var curBones = curSkinRend.bones;
				Transform curRoot = null;
				if (curBones.isNullOrEmpty()){
					Debug.LogFormat("No bones on skinned mesh object {0}, assuming gameobject to be root", curSkinRend.gameObject);
					curRoot = curSkinRend.gameObject.transform;
				}
				else{
					curRoot = findCommonParent(curBones);
					if (!curRoot){
						throw new System.ArgumentException(
							string.Format("Failed to find common parent in skin renderer {0}", curSkinRend));
					}
				}
				roots.Add(curRoot);
			}
			return roots;
		}
		
		protected static Transform findSkinMeshCommonRoot(SkinnedMeshRenderer[] meshRenders, GameObject prefabRoot = null){
			var roots = findSkinMeshRoots(meshRenders);
			var commonRoot = findCommonParent(roots);
			if (!commonRoot){
				throw new System.ArgumentException(
					string.Format("Could not find common root for prefab {0}", prefabRoot));
			}
			return commonRoot;			
		}
		
		protected static void checkPrefabRoot(GameObject prefabRoot){
			if (!prefabRoot){
				throw new System.ArgumentNullException("prefabRoot");
			}
			
			var prefabType = PrefabUtility.GetPrefabType(prefabRoot);
			if ((prefabType != PrefabType.ModelPrefab) && (prefabType != PrefabType.Prefab)){
				throw new System.ArgumentException(
					string.Format("Invalid prefab type provided: {0}", prefabType), "prefabRoot");
			}
		}
		
		protected static ObjectMapper<Transform> gatherFilteredIds(Transform prefabRoot, Transform commonRoot){
			var filteredIds = gatherIds(prefabRoot.transform, (curNode) => isChildOf(commonRoot, curNode));
			return filteredIds;
		}
		
		protected static bool hasAnimationComponents(Transform curTransform){
			return curTransform 
				&& (
					(curTransform.GetComponent<Animation>()) 
					|| (curTransform.GetComponent<Animator>())
				);
		}
				
		protected static string findSkeletonName(Transform commonRoot){
			if (!commonRoot)
				throw new System.ArgumentNullException("commonRoot");
			var parent = commonRoot.parent;
			if (hasAnimationComponents(parent))
				return parent.name;
			return commonRoot.name;
		}
		
		protected void build(JsonSkeleton result, SkinnedMeshRenderer[] meshRenders){
			var commonRoot = findSkinMeshCommonRoot(meshRenders);
			
			if (!commonRoot){
				throw new System.ArgumentException("Coudl not find common root for skin mesh renderers");
			}
			
			var finalIds = gatherIds(commonRoot);//gatherFilteredIds(prefabRoot.transform, commonRoot);
			Debug.LogFormat("Skinned mesh renderer skeleton:");
			printSkeleton(finalIds);
						
			result.clear();
			result.assignFrom(commonRoot, finalIds);
			result.name = findSkeletonName(commonRoot);
		}
				
		protected void build(JsonSkeleton result, GameObject prefabRoot){
			checkPrefabRoot(prefabRoot);
			
			var transformIds = gatherIds(prefabRoot.transform);
			Debug.LogFormat("Full skeleton:");
			printSkeleton(transformIds);
			
			var skinRenderers = prefabRoot.GetComponentsInChildren<SkinnedMeshRenderer>(true);
			var commonRoot = findSkinMeshCommonRoot(skinRenderers, prefabRoot);
			var finalIds = gatherFilteredIds(prefabRoot.transform, commonRoot);
			Debug.LogFormat("Short skeleton:");
			printSkeleton(finalIds);
			
			result.clear();
			result.assignFrom(commonRoot, finalIds); 
			result.name = findSkeletonName(commonRoot);
		}
		
		protected void buildFromTransform(JsonSkeleton result, Transform skeletonRoot){
			if (!skeletonRoot)
				throw new System.ArgumentNullException("skeletonRoot");
				
			var transformIds = gatherIds(skeletonRoot);
			Debug.LogFormat("Transofrm skeleton:");
			printSkeleton(transformIds);
			
			result.clear();
			result.assignFrom(skeletonRoot, transformIds);
			result.name = findSkeletonName(skeletonRoot);
		}
		
		/*
		This will try to find skeleton root, and it will try to find it within object prefab, if possible.
		*/
		public static Transform findSkeletonRoot(SkinnedMeshRenderer meshRend){
			if (!meshRend)
				throw new System.ArgumentNullException("meshRend");
				
			//Bah. We can end up here from prefab creator.
			var skinRend = Utility.getSrcPrefabAssetObject(meshRend, false);
			/*
			var skinRend = meshRend;
			var newSkinRend = PrefabUtility.GetCorrespondingObjectFromSource(meshRend) as SkinnedMeshRenderer;
			if (newSkinRend)
				skinRend = newSkinRend;*/
				
			var commonRoot = findSkinMeshCommonRoot(new SkinnedMeshRenderer[]{skinRend});
			
			return commonRoot;
		}		
		
		public static JsonSkeleton buildFromSkinMesh(SkinnedMeshRenderer meshRend){
			var result = new JsonSkeleton();
			var builder = new JsonSkeletonBuilder();
			builder.build(result, new SkinnedMeshRenderer[] {meshRend});
			return result;
		}
		
		public static JsonSkeleton buildFromPrefabRoot(GameObject prefabRoot){
			var result = new JsonSkeleton();
			var builder = new JsonSkeletonBuilder();
			builder.build(result, prefabRoot);
			return result;
		}
		
		public static JsonSkeleton buildFromRootTransform(Transform prefabTransform){
			var result = new JsonSkeleton();
			var builder = new JsonSkeletonBuilder();
			builder.buildFromTransform(result, prefabTransform);
			return result;
		}
		
		public static JsonSkeleton extractOriginalSkeleton(SkinnedMeshRenderer meshRend, Logger logger){
			Logger.makeValid(ref logger);
			var rootPrefab = ExportUtility.getLinkedRootPrefabAsset(meshRend.gameObject);//PrefabUtility.FindPrefabRoot(meshRend.gameObject);
			if (!rootPrefab){
				//now what?
				logger.logWarningFormat("Could not find original prefab for meshRenderer {0}({1})," 
					+ "falling back on generating unique skeleton, errors may occur",
					meshRend, meshRend.gameObject
				);
				return buildFromSkinMesh(meshRend);
			}
			return buildFromPrefabRoot(rootPrefab);
		}
	}
}