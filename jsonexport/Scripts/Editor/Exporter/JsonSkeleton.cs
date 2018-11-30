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
		
		public List<JsonSkinBone> bones = new List<JsonSkinBone>();
		
		public override void writeJsonObjectFields(FastJsonWriter writer){
			writer.writeKeyVal("id", id);
			writer.writeKeyVal("name", name);
			writer.writeKeyVal("bones", bones);
		}
		
		public static JsonSkeleton extractOriginalSkeleton(SkinnedMeshRenderer skinRend){
			if (!skinRend)
				throw new System.ArgumentNullException("skinRend");
				
			/*
			//Aaaaand nope. Can't do that.
			
			var prefabObj = PrefabUtility.GetPrefabObject(skinRend);
			var prefabSkinRend = prefabObj as SkinnedMeshRenderer;
			
			if (!prefabSkinRend)
				return null;
			var mesh = skinRend.sharedMesh;
			*/
			
			return new JsonSkeleton(skinRend.sharedMesh, skinRend);			
		}
		
		public JsonSkeleton(){
		}
		
		public JsonSkeleton(Mesh skinMesh, SkinnedMeshRenderer skinRend, int id_ = -1){
			id = id_;
			if (!skinMesh)
				throw new System.ArgumentNullException("skinMesh");
			if (!skinRend)
				throw new System.ArgumentException("skinRend");
			name = skinMesh.name;
			//meshId = resMap.getMeshId(skinMesh);
			
			var boneMap = new Dictionary<Transform, int>();
			var skinBones = skinRend.bones;
			for(int i = 0; i < skinBones.Length; i++){
				var bone = skinBones[i];
				boneMap.Add(bone, i);
			}
			
			bones.Clear();
			var skinPoses = skinMesh.bindposes;
			if (skinBones.Length != skinPoses.Length){
				Debug.LogErrorFormat("Skin bones and skin poses length mismatch while processing skeleton for skinMesh \"{0}\"",
					skinMesh.name);
				throw new System.ArgumentException("Bone and skin poses mismatch");
			}
			
			for(int i = 0; i < skinBones.Length; i++){
				var skinBone = skinBones[i];
				if (!skinBone){
					bones.Add(new JsonSkinBone());
					continue;
				}
				var parent = skinBone.parent;
				var parentIndex = -1;
				if (!boneMap.TryGetValue(parent, out parentIndex))
					parentIndex = -1;
				
				Matrix4x4 bindPose = Matrix4x4.identity;
				if (i < skinPoses.Length){
					bindPose = skinPoses[i];
				}
				bones.Add(new JsonSkinBone(skinBone.name, bindPose, parentIndex));
			}
		}
	}
}
