using UnityEngine;
using UnityEditor;
using System.Collections.Generic;
using System.Linq;
using UnityEditor.Animations;

namespace SceneExport{
	[System.Serializable]
	public class JsonHumanBone: IFastJsonValue{
		public string boneName;
		public string objectName;
		public string objectPath;
		//public int objectId = -1;
		public int objInstanceId = -1;
		public int transfInstanceId = -1;
		//public int objectInstanceId = -1;
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject(false);
			writer.writeKeyVal("boneName", boneName, false);
			writer.writeKeyVal("objectName", objectName, false);
			writer.writeKeyVal("objectPath", objectPath, false);
			//writer.writeKeyVal("objectId", objectId, false);				
			writer.writeKeyVal("objInstanceId", objInstanceId, false);
			writer.writeKeyVal("transfInstanceId", transfInstanceId, false);
			writer.endObject(false);
		}
		
		public JsonHumanBone(HumanBodyBones boneType, Animator animator){
			if (!animator)
				throw new System.ArgumentNullException("animator");
			objInstanceId = -1;
			boneName = boneType.ToString();
			//objectId = -1;
				
			var boneObjTransform = animator.GetBoneTransform(boneType);
			if (!boneObjTransform)
				return;
				
			transfInstanceId = boneObjTransform.GetInstanceID();
			objInstanceId = boneObjTransform.gameObject.GetInstanceID();
			objectName = boneObjTransform.gameObject.name;
			objectPath = boneObjTransform.getScenePath(animator.transform);
		}
	}
		
	[System.Serializable]
	public class JsonAnimator: IFastJsonValue{
		public string name = "";
		
		public ResId skeletonId = ResId.invalid;
		public List<ResId> skinMeshIds = new List<ResId>();
		public ResId animatorControllerId = ResId.invalid;
		
		public Animator animator;
		
		public List<JsonHumanBone> humanBones = new List<JsonHumanBone>();
		
		/*
		public JsonSkeleton skeleton = new JsonSkeleton();
		public List<JsonSkinRendererData> skinMeshes = new List<JsonSkinRendererData>();		
		public JsonAnimatorController animatorController = new JsonAnimatorController();
		*/
		
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("name", name);
			writer.writeKeyVal("skeletonId", skeletonId);
			//slots?
			writer.writeKeyVal("skinMeshIds", skinMeshIds);
			writer.writeKeyVal("animatorControllerId", animatorControllerId);
			
			writer.writeKeyVal("applyRootMotion", animator.applyRootMotion);
			writer.writeKeyVal("cullingMode", animator.cullingMode.ToString());
			writer.writeKeyVal("hasRootMotion", animator.hasRootMotion);
			writer.writeKeyVal("hasTransformHierarchy", animator.hasTransformHierarchy);
			writer.writeKeyVal("humanScale", animator.humanScale);
			writer.writeKeyVal("isHuman", animator.isHuman);
			writer.writeKeyVal("layerCount", animator.layerCount);
			writer.writeKeyVal("layersAffectMassCenter", animator.layersAffectMassCenter);
			//writer.writeKeyVal("linearVelocityBlending", animator.linearVelocityBlending);
			writer.writeKeyVal("speed", animator.speed);
			writer.writeKeyVal("stabilizeFeet", animator.stabilizeFeet);
			
			writer.writeKeyVal("humanBones", humanBones);
						
			//writer.writeKeyVal("skeleton", skeleton);
			//writer.writeKeyVal("animatorController", animatorController);
			writer.endObject();
		}
		
		List<JsonHumanBone> gatherHumanBones(Animator animator){
			var result = new List<JsonHumanBone>();
			if (!animator.isHuman)
				return result;
				
			foreach(HumanBodyBones cur in System.Enum.GetValues(typeof(HumanBodyBones))){
				if ((cur == HumanBodyBones.LastBone) || ((int)cur == 0))//crashes otherwise
					continue;
				result.Add(new JsonHumanBone(cur, animator)); 
			}
			
			return result;
		}
		
		public JsonAnimator(Animator animator_, ResourceMapper resMap){
			animator = animator_;
			if (!animator_)
				throw new System.ArgumentNullException("animator_");
			name = animator.name;
			skeletonId = resMap.skelRegistry.registerSkeleton(animator.transform, true);
			
			//var skel = resMap.getSkeletonById(skeletonId);
			var skelTransform = resMap.skelRegistry.getSkeletonTransformById(skeletonId);
			//var root = PrefabUtility.FindPrefabRoot(skelTransform.gameObject);
			
			//var meshKey = new MeshStorageKey(
			var skinRends = animator.GetComponentsInChildren<SkinnedMeshRenderer>();
			skinMeshIds = skinRends.Select((arg) => resMap.getOrRegMeshId(arg, skelTransform)).ToList();
			
			var runtimeAnimator = animator.runtimeAnimatorController;
			var editorAnimator = runtimeAnimator as UnityEditor.Animations.AnimatorController;
						
			animatorControllerId = resMap.getAnimatorControllerId(editorAnimator, animator);
			
			humanBones = gatherHumanBones(animator);
		}
	};
}