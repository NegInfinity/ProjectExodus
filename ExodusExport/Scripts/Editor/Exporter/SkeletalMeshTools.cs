using UnityEngine;
using UnityEditor;
using System.Collections.Generic;
using System.Linq;

namespace SceneExport{
	public static class SkeletalMeshTools{
		public static float getMaxDifference(Matrix4x4 m1, Matrix4x4 m2){
			float maxDiff = 0.0f;
			for(int i = 0; i < 16; i++){
				var diff = Mathf.Abs(m1[i] - m2[i]);
				maxDiff = Mathf.Max(maxDiff, diff);
			}
			return maxDiff;
		}
		
		public static readonly float matrixEpsilon = 0.00001f; //I'll consider matrices equivalent if this is below this threshold
		
		public static JsonMesh fixSkinMeshPosedBones(MeshStorageKey meshKey, JsonMesh srcMesh){
			var result = srcMesh;
			var boneTransforms = Utility.findNamedTransforms(result.defaultBoneNames, meshKey.skeletonRoot);
			for(int i = 0; i < boneTransforms.Count; i++){
				if (!boneTransforms[i]){
					Debug.LogWarningFormat(
						string.Format("Could not locate bone {0}({1}) on mesh {2}", 
						result.defaultBoneNames[i], i, meshKey.mesh)
					);
				}
			}
			var rootNode = meshKey.prefab.transform;
			
			bool largeBoneTransformFound = false;
			{
				var srcRootTransform = meshKey.skeletonRoot.localToWorldMatrix;
				//var srcRootInvTransform = meshKey.skeletonRoot.worldToLocalMatrix;
				for(int boneIndex = 0; boneIndex < boneTransforms.Count; boneIndex++){
					var curBone = boneTransforms[boneIndex];
					
					var curBoneMatrix = Utility.getRelativeMatrix(curBone, rootNode);
					var curBoneInvMatrix = Utility.getRelativeInverseMatrix(curBone, rootNode);
					
					var bindPose = srcMesh.bindPoses[boneIndex];
					Debug.LogFormat("curBone: {0}({1})\nmatrix:\n{2}\ninvMatrix:\n{3}\nbindPose:\n{4}\ninvBindPose:\n{5}\nroot:\n{6}\ninvRoot:\n{7}\n",
						boneIndex, curBone.name, curBoneMatrix, curBoneInvMatrix, bindPose, bindPose.inverse, 
						srcRootTransform, srcRootTransform.inverse);
					
					/*
					var curBone = Utility.getRelativeMatrix(boneTransforms[boneIndex], ;
					var inverseMatrix  = curBone.worldToLocalMatrix * srcRootTransform;
					var bindPose = srcMesh.bindPoses[boneIndex];//meshKey.mesh.bindposes[i]; ///NOPE. We're done tweaking the mesh at this point.
					var diff = getMaxDifference(inverseMatrix, bindPose);					
					Debug.LogFormat("index:{0}({1})\ninverseMatrix:\n{2}\nbindPose:\n{3}\nboneMatrix:\n{4}\nsrcRoot:\n{5}\ndiff: {6}\nepsilon: {7}",
						boneIndex, curBone.name, inverseMatrix, bindPose, curBone.worldToLocalMatrix, srcRootTransform, diff, matrixEpsilon);
					*/
					
					var curPose = bindPose;
					var desiredPose = curBoneInvMatrix;
					var diff = getMaxDifference(curPose, desiredPose);
					Debug.LogFormat("bindPose:\n{0}\ndesiredPose:\n{1}\ndiff: {2}; epsilon: {3}\n", 
						curPose, desiredPose, diff, matrixEpsilon);
					
					if (diff > matrixEpsilon){
						largeBoneTransformFound = true;
						Debug.LogFormat("Large transform found");
						//break;
					}
				}
			}
			
			if (!largeBoneTransformFound){
				return result;
			}
			else{
				Debug.LogFormat(
					string.Format("Large transform difference found on mesh {0}, mesh will be transformed to accomodate",
						meshKey.mesh.name));
			}
			
			return result;//disable it for now.
			
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
						rootParentInvTransform * curBone.localToWorldMatrix * srcMesh.bindPoses[boneIndex];//meshKey.mesh.bindposes[boneIndex];
						//invRootTransform * curBone.localToWorldMatrix * meshKey.mesh.bindposes[boneIndex];
				}
				
				transformMatrices.Add(newTransformMatrix);
			}
			
			transformed.transformSkeletalMesh(transformMatrices);
			transformed.setBindPosesFromTransforms(boneTransforms, meshKey.skeletonRoot);
			//
			return transformed;
		}
	}
}
