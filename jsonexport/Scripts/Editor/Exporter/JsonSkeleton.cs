using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

namespace SceneExport{
	[System.Serializable]
	public class JsonSkeleton: JsonValueObject{
		public int id = -1;
		public string name;
		
		public bool isEmpty(){
			return (bones == null) || (bones.Count == 0);
		}
		
		[System.Serializable]
		public class Bone: JsonValueObject{
			public int id = -1;
			public int parentId = -1;
			public string name = "";
			public Matrix4x4 world = Matrix4x4.identity;
			public Matrix4x4 local = Matrix4x4.identity;
			public Matrix4x4 rootRelative = Matrix4x4.identity;
			public override void writeJsonObjectFields(FastJsonWriter writer){
				writer.writeKeyVal("name", name);
				writer.writeKeyVal("id", id);
				writer.writeKeyVal("parentId", parentId);
				writer.writeKeyVal("world", world);
				writer.writeKeyVal("local", local);
				writer.writeKeyVal("rootRelative", rootRelative);
			}

			public Bone(string name_, int id_, int parentId_, Matrix4x4 world_, Matrix4x4 local_, Matrix4x4 rootRelative_){
				name = name_;
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
			writer.writeKeyVal("bones", bones);
		}
		
		public void clear(){
			bones.Clear();
			name = "";
			id = ExportUtility.invalidId;
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
				
				var newBone = new Bone(curBone.name, boneIndex, parentId, worldMatrix, localMatrix, rootRelativeMatrix);
				bones.Add(newBone);
			}			
		}
		
		public JsonSkeleton(string name_, int id_, Transform rootTransform, ObjectMapper<Transform> transforms){
			name = name_;
			id = id_;
			assignFrom(rootTransform, transforms);
		}
		
		public JsonSkeleton(){
		}		
	}
}
