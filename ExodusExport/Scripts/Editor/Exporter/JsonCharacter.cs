using UnityEngine;
using UnityEditor;
using System.Linq;
using System.Collections.Generic;

namespace SceneExport{
	/*
		This class is roughly equivalent to Unreal's ASkeletalMesh
		
		In context of exporter, a "Character" is something that:
		1. Has Animator component on it, that sits in a root.
		2. Has Several skeletal meshes attached to it. 
		
		Everything above animator will be considered to be character's skeleton, and 
		the exporter will harvest and resample character animation, when necessary. 
		
		This class was introduced due to discrepancies in behavior of unity and unreal engines.
		Namely ... unity does not even have a concept of a skeleton to begin with (there's "avatar"), and 
		objects are loosely attached to floating bones. Although on some occasions bones can be hidden.
		On top of that, multi-component setups where a single character is composed out of multiple different skins are extremely 
		common in unity, which calls for special conversion tactics... 
		
		So, the decision was made to consider "Animator" a characte,r and anything above "Animator" to be its skeleton.
	*/
	[System.Serializable]
	public class JsonCharacter: IFastJsonValue{
		public string name;
		public string assetPath;
		public int id = -1;
		public List<JsonGameObject> attachedObjects = new List<JsonGameObject>();
		
		public Animator animator = null;
		//public JsonSkeleton skeleton = new JsonSkeleton();		
		public JsonSkeleton skeleton = new JsonSkeleton();
		//Hmm...
		//public int skeletonId = -1;//Both controller and skeleton are supposed to be unique, which makes me wonder if I should embed them instead.
		public int animatorController = -1;
		
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("name", name); 
			writer.writeKeyVal("assetPath", assetPath);			
			writer.writeKeyVal("skeleton", skeleton);			
			writer.writeKeyVal("animatorController", animatorController);
			writer.writeKeyVal("attachedObjects", attachedObjects);
			writer.endObject();
		}
		
		static List<GameObject> harvestAttachedObjects(Animator anim){
			if (!anim)
				throw new System.ArgumentNullException("anim");
			var result = new List<GameObject>();
			
			ExportUtility.walkHierarchy(anim.gameObject, 
				(curObj) => {
					if (ExportUtility.hasSupportedComponents(curObj) && (curObj != anim.gameObject))
						result.Add(curObj);
					return true;	
				}
			);
			
			return result;
		}
		
		public JsonCharacter(Animator animator_, int id_, ResourceMapper resMap){
			id = id_;
			if (!animator_)
				throw new System.ArgumentNullException("animator_");
			animator = animator_;
			if (resMap == null)
				throw new System.ArgumentNullException("resMap");
				
			name = animator.name;
			assetPath = AssetDatabase.GetAssetPath(animator);
			
			//skeleton = new JsonSkeleton(animator.name, id, animator.transform, characterObjectMapper);
			var skeleton = JsonSkeletonBuilder.buildFromRootTransform(animator.transform);
				
			var objects = harvestAttachedObjects(animator);
			var characterObjectMapper = new GameObjectMapper();//Hmm....			
			attachedObjects = objects.Select((arg) => new JsonGameObject(arg, characterObjectMapper, resMap)).ToList();

			//Hmm. Because of retargeting, we will need to possibly create multiple copies of a single animator controller.
			
			//var JsonAnimatorController(
		}
	}
}