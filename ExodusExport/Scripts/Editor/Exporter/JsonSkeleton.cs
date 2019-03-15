//#define JSON_SKELETON_EXTRA_DATA

using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

namespace SceneExport{
	[System.Serializable]
	public class JsonSkeleton: JsonValueObject{
		public ResId id = ResId.invalid;
		public string name;
		
		//public List<string> defaultBoneNames = new List<string>();
		
		public bool isEmpty(){
			return (bones == null) || (bones.Count == 0);
		}
		
		[System.Serializable]
		public class Bone: JsonValueObject{
			public int id = -1;
			public ResId parentId = ResId.invalid;
			public string name = "";
			public string path = "";
			
		#if JSON_SKELETON_EXTRA_DATA
			public Vector3 localPosition = Vector3.zero;
			public Vector3 localScale = Vector3.one;
			public Quaternion localRotation = Quaternion.identity;
			
			public Vector3 position = Vector3.zero;
			public Vector3 lossyScale = Vector3.one;
			public Quaternion rotation = Quaternion.identity;
		#endif
			
			public Matrix4x4 world = Matrix4x4.identity;
			public Matrix4x4 local = Matrix4x4.identity;
			public Matrix4x4 rootRelative = Matrix4x4.identity;
			public override void writeJsonObjectFields(FastJsonWriter writer){
				writer.writeKeyVal("name", name);
				writer.writeKeyVal("path", path);
				writer.writeKeyVal("id", id);
				writer.writeKeyVal("parentId", parentId);
				
			#if JSON_SKELETON_EXTRA_DATA
				writer.writeKeyVal("localPosition", localPosition);
				writer.writeKeyVal("localScale", localScale);
				writer.writeKeyVal("localRotation", localRotation);
				writer.writeKeyVal("position", position);
				writer.writeKeyVal("lossyScale", lossyScale);
				writer.writeKeyVal("rotation", rotation);
			#endif
				
				writer.writeKeyVal("world", world);
				writer.writeKeyVal("local", local);
				writer.writeKeyVal("rootRelative", rootRelative);
			}

			public Bone(string name_, string path_, int id_, ResId parentId_, Matrix4x4 world_, Matrix4x4 local_, Matrix4x4 rootRelative_){
				name = name_;
				path = path_;
				id = id_;
				parentId = parentId_;
								
				world = world_;
				local = local_;
				rootRelative = rootRelative_;
			}	
		}
		
		public List<Bone> bones = new List<Bone>();
		
		public override void writeJsonObjectFields(FastJsonWriter writer){
			writer.writeKeyVal("id", id);
			writer.writeKeyVal("name", name);
			//writer.writeKeyVal("defaultBoneNames", defaultBoneNames);
				
			writer.writeKeyVal("bones", bones);
		}
		
		public void clear(){
			bones.Clear();
			name = "";
			id = ResId.invalid;//ExportUtility.invalidId;
		}
		
		public void assignFrom<Container>(Transform rootTransform, List<Transform> transforms) 
				where Container: IEnumerable<Transform>{
			var mapper = new ObjectMapper<Transform>();
			foreach(var cur in transforms){
				mapper.registerObject(cur);
			}
			assignFrom(rootTransform, mapper);
		}
		
		public void assignFrom(Transform rootTransform, ObjectMapper<Transform> transforms){
			if (!rootTransform)
				throw new System.ArgumentNullException("rootTransform");
			if (transforms == null)
				throw new System.ArgumentNullException("transforms");
			clear();
			
			var invRootMatrix = rootTransform.worldToLocalMatrix;
			
			for(int boneIndex = 0; boneIndex < transforms.objectList.Count; boneIndex++){
				var curBone = transforms.objectList[boneIndex];
				var parentBone = curBone.parent;
				var parentId = transforms.findId(parentBone);///why, why, WHY can't I have const methods...
				var worldMatrix = curBone.localToWorldMatrix;
				var localMatrix = worldMatrix;
				var rootRelativeMatrix = invRootMatrix * worldMatrix;
				if (parentBone){
					var invParent = parentBone.worldToLocalMatrix;
					localMatrix = invParent * worldMatrix;
				}
				
				var newJsonBone = new Bone(curBone.name, 
					curBone.getScenePath(rootTransform),
					boneIndex, parentId, worldMatrix, localMatrix, rootRelativeMatrix);
				
			#if JSON_SKELETON_EXTRA_DATA
				newJsonBone.position = curBone.position;
				newJsonBone.rotation= curBone.rotation;
				newJsonBone.lossyScale = curBone.lossyScale;
				
				newJsonBone.localPosition = curBone.localPosition;
				newJsonBone.localRotation = curBone.localRotation;
				newJsonBone.localScale = curBone.localScale;
			#endif
				
				bones.Add(newJsonBone);
			}			
		}
		
		public JsonSkeleton(string name_, ResId id_, Transform rootTransform, ObjectMapper<Transform> transforms){
			name = name_;
			id = id_;
			assignFrom(rootTransform, transforms);
		}
		
		public JsonSkeleton(){
		}		
	}
}
