using UnityEngine;
using UnityEditor;
using System.Collections.Generic;
using System.Linq;

namespace SceneExport{
	public class SkinMeshWeightTest: MonoBehaviour{
		public List<SkinnedMeshRenderer> skinMeshes = new List<SkinnedMeshRenderer>();
		public Gradient vertGradient = new Gradient();
		public int refBoneIndex = -1;
		
		void drawCross(Vector3 pos, float halfSize){
			var dx = new Vector3(1.0f, 0.0f, 0.0f) * halfSize;
			var dy = new Vector3(0.0f, 1.0f, 0.0f) * halfSize;
			var dz = new Vector3(0.0f, 0.0f, 1.0f) * halfSize;
			Gizmos.DrawLine(pos - dx, pos + dx);
			Gizmos.DrawLine(pos - dy, pos + dy);
			Gizmos.DrawLine(pos - dz, pos + dz);
		}
		
		void drawCross(Vector4 pos, float halfSize){
			int colorIndex = Mathf.RoundToInt(pos.w);
			#if false
			var color = vertGradient.Evaluate(pos.w - 0.5f /*+ 0.02f*/);
			Gizmos.color = color;
			#endif
			
			int idx = 1 + colorIndex % 7;
			
			var dotColor = new Color( 
				((idx & 0x1) != 0) ? 1.0f: 0.0f, 
				((idx & 0x2) != 0) ? 1.0f: 0.0f, 
				((idx & 0x4) != 0) ? 1.0f: 0.0f
			);
			
			dotColor.a = 1.0f;
			
			Gizmos.color = dotColor;			
			drawCross(new Vector3(pos.x, pos.y, pos.z), halfSize);
		}
		
		Matrix4x4 getBoneTransform(int meshBoneIndex, Mesh mesh, Transform[] transforms){	
			var bindPose = mesh.bindposes[meshBoneIndex];
			var worldMatrix = transforms[meshBoneIndex].localToWorldMatrix;			
			return worldMatrix * bindPose;
		}
		
		void setupGradient(){
			if (vertGradient == null)
				vertGradient = new Gradient();
				
			var colorKeys = new List<GradientColorKey>();
			/*
			colorKeys.Add(new GradientColorKey(Color.blue, 0.499f));
			colorKeys.Add(new GradientColorKey(Color.white, 0.5f));
			colorKeys.Add(new GradientColorKey(Color.red, 0.501f));
			*/
			colorKeys.Add(new GradientColorKey(Color.red, 1.0f));
			colorKeys.Add(new GradientColorKey(Color.blue, 0.0f));
			var alphaKeys = new List<GradientAlphaKey>();
			
			vertGradient.SetKeys(
				colorKeys.ToArray(),
				alphaKeys.ToArray()
			);
		}
		
		Vector4 getSkinMeshPosition(int vertIndex, SkinnedMeshRenderer skinRend){
			if (!skinRend)
				return Vector3.zero;
			var mesh = skinRend.sharedMesh;
			if (!mesh)
				return Vector3.zero;
				
			var meshVert = mesh.vertices[vertIndex];
			var meshWeight = mesh.boneWeights[vertIndex];
			var bones = skinRend.bones;
			
			var weight = mesh.boneWeights[vertIndex];
				
			var result = Vector3.zero;
			
			float vertexWeight = 0.0f;
				
			float totalWeight =  weight.weight0 + weight.weight1 + weight.weight2 + weight.weight3;
			
			/*
			int dominantBone = weight.boneIndex0;
			float dominantWeight = weight.weight0;
			
			if (weight.weight1 > dominantWeight)
				dominantBone = weight.boneIndex1;
			if (weight.weight2 > dominantWeight)
				dominantBone = weight.boneIndex2;
			if (weight.weight3 > dominantWeight)
				dominantBone = weight.boneIndex3;
				*/
				
			int numWeights = 0;
			if (weight.weight0 > 0.0f)
				numWeights++;
			if (weight.weight1 > 0.0f)
				numWeights++;
			if (weight.weight2 > 0.0f)
				numWeights++;
			if (weight.weight3 > 0.0f)
				numWeights++;
				
			float wValue = numWeights;//1.0f;
				
			result += weight.weight0 * (getBoneTransform(weight.boneIndex0, mesh, bones).MultiplyPoint(meshVert));
			result += weight.weight1 * (getBoneTransform(weight.boneIndex1, mesh, bones).MultiplyPoint(meshVert));
			result += weight.weight2 * (getBoneTransform(weight.boneIndex2, mesh, bones).MultiplyPoint(meshVert));
			result += weight.weight3 * (getBoneTransform(weight.boneIndex3, mesh, bones).MultiplyPoint(meshVert));				
			if (weight.boneIndex0 == refBoneIndex)
				vertexWeight = weight.weight0;
			if (weight.boneIndex1 == refBoneIndex)
				vertexWeight = weight.weight1;
			if (weight.boneIndex2 == refBoneIndex)
				vertexWeight = weight.weight2;
			if (weight.boneIndex3 == refBoneIndex)
				vertexWeight = weight.weight3;
			
			return new Vector4(result.x, result.y, result.z, wValue);//.vertexWeight);//totalWeight);
		}
		
		void visualizeMesh(SkinnedMeshRenderer skinRend){
			setupGradient();
			var numTransforms = skinRend.bones.Length;
			if (numTransforms <= 0)
				return;
				
			if (!skinRend.sharedMesh)
				return;
				
			var mesh = skinRend.sharedMesh;
			var meshVerts = mesh.vertices;
			var bones = skinRend.bones;
			
			for(int i = 0; i < mesh.vertexCount; i++){
				var vertPos = getSkinMeshPosition(i, skinRend);
				drawCross(vertPos, 0.01f);
			}
		}
	
		void drawGizmos(Color c){
			var oldColor = Gizmos.color;
			var oldMatrix = Gizmos.matrix;
			Gizmos.matrix = transform.localToWorldMatrix;
			foreach(var cur in skinMeshes){
				visualizeMesh(cur);
			}
			
			Gizmos.matrix = oldMatrix;
			Gizmos.color = oldColor;
		}
	
		void OnDrawGizmos(){
			drawGizmos(Color.yellow);
		}
		
		void OnDrawGizmosSelected(){
			drawGizmos(Color.white);
		}
	}
}