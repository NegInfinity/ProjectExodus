using UnityEditor;
using UnityEngine;
using System.Collections.Generic;
using System.Linq;

namespace SceneExportTests{
	[System.Serializable]
	public class AnimSamplerTest: MonoBehaviour{
		public Animator srcAnimator = null;
		public AnimationClip[] animClips = new AnimationClip[0];
		//public AnimationClip animClip = null;
		public HumanBodyBones humanBone = HumanBodyBones.Head;
		
		public List<Vector4> points = new List<Vector4>();
		public List<int> parentOffsets = new List<int>();
		
		public Gradient gradient = null;
		
		public void OnDrawGizmos(){
			drawGizmos(Color.yellow);
		}
		
		void checkGradient(){
			if (gradient == null)
				buildGradient();
		}
		
		void buildGradient(){
			gradient = new Gradient();
			var colors = new List<GradientColorKey>();
			colors.Add(new GradientColorKey(Color.red, 0.0f));
			colors.Add(new GradientColorKey(Color.yellow, 0.5f));
			colors.Add(new GradientColorKey(Color.white, 1.0f));
			gradient.colorKeys = colors.ToArray();
			var alphas = new List<GradientAlphaKey>();
			alphas.Add(new GradientAlphaKey(0.1f, 0.0f));
			alphas.Add(new GradientAlphaKey(0.1f, 1.0f));
			gradient.alphaKeys = alphas.ToArray();
		}
		
		void drawCross(Vector4 point, float size = 0.1f){
			checkGradient();
			
			var dx = new Vector3(1.0f, 0.0f, 0.0f);
			var dy = new Vector3(0.0f, 1.0f, 0.0f);
			var dz = new Vector3(0.0f, 0.0f, 1.0f);
			var pos = new Vector3(point.x, point.y, point.z);
			
			Gizmos.DrawLine(pos - dx * size, pos + dx * size);
			Gizmos.DrawLine(pos - dy * size, pos + dy * size);
			Gizmos.DrawLine(pos - dz * size, pos + dz * size);
		}
		
		public void OnDrawGizmosSelected(){
			drawGizmos(Color.white);
		}
		
		void drawGizmos(Color c){
			var oldColor = Gizmos.color;
			var oldMatrix = Gizmos.matrix;
			Gizmos.matrix = transform.localToWorldMatrix;
			
			var offsets = buildParentOffsets(getHumanBones());
			
			var numOffsets = offsets.Count;
			//Debug.LogFormat("Rendering");
			for(int baseOffset = 0; (baseOffset + numOffsets - 1) < points.Count; baseOffset += numOffsets){
				//Debug.LogFormat("baseOffset: {0}", baseOffset);
				for(int boneIndex = 0; (boneIndex < numOffsets); boneIndex++){
					var parentOffset = offsets[boneIndex];
					var pointIndex = baseOffset + boneIndex;
					var pointPos = points[pointIndex];
					Gizmos.color = gradient.Evaluate(pointPos.w);
					if (parentOffset != 0){
						//Debug.LogFormat("Parent offset: {0}; baseOffset: {1}; pointIndex: {2}", parentOffset, baseOffset, pointIndex);
						var parentPos = pointIndex + parentOffset;
						//Debug.LogFormat("Parent pos: {0}", parentPos);
						Gizmos.DrawLine(pointPos, points[parentPos]);
					}
					else{
						drawCross(pointPos, 0.025f);											
					}
				}
			}
			
			foreach(var cur in points)
				drawCross(cur, 0.025f);	
			
			Gizmos.matrix = oldMatrix;
			Gizmos.color = oldColor;			
		}
		
		public void buildRandomPoints(){
			points.Clear();
			for(int i = 0; i < 100; i++){
				points.Add(
					new Vector4(
						Random.Range(-1.0f, 1.0f), 
						Random.Range(-1.0f, 1.0f),
						Random.Range(-1.0f, 1.0f), 
						Random.Range(0.0f, 1.0f)
					)
				);
			}
		}
		
		void sampleSingleFramePoints(Animator anim, List<HumanBodyBones> bones, float t){
			foreach(var curId in bones){
				var bone = anim.GetBoneTransform(curId);
				if (!bone)
					continue;
					
				var pos = bone.position;
				points.Add(new Vector4(pos.x, pos.y, pos.z, t));
			}
		}
		
		List<int> buildParentOffsets(List<HumanBodyBones> boneIds){
			var childToParent = new Dictionary<HumanBodyBones, HumanBodyBones>();
			//childToParent.Add(HumanBodyBones.Jaw, HumanBodyBones.Head);
			childToParent.Add(HumanBodyBones.Head, HumanBodyBones.Neck);
			childToParent.Add(HumanBodyBones.Neck, HumanBodyBones.UpperChest);
			childToParent.Add(HumanBodyBones.UpperChest, HumanBodyBones.Chest);
			childToParent.Add(HumanBodyBones.Chest, HumanBodyBones.Spine);
			childToParent.Add(HumanBodyBones.Spine, HumanBodyBones.Hips);			
			childToParent.Add(HumanBodyBones.LeftUpperLeg, HumanBodyBones.Hips);
			childToParent.Add(HumanBodyBones.RightUpperLeg, HumanBodyBones.Hips);			
			childToParent.Add(HumanBodyBones.LeftLowerLeg, HumanBodyBones.LeftUpperLeg);
			childToParent.Add(HumanBodyBones.RightLowerLeg, HumanBodyBones.RightUpperLeg);			
			childToParent.Add(HumanBodyBones.LeftFoot, HumanBodyBones.LeftLowerLeg);
			childToParent.Add(HumanBodyBones.RightFoot, HumanBodyBones.RightLowerLeg);
			
			childToParent.Add(HumanBodyBones.LeftShoulder, HumanBodyBones.UpperChest);
			childToParent.Add(HumanBodyBones.LeftUpperArm, HumanBodyBones.LeftShoulder);
			childToParent.Add(HumanBodyBones.LeftLowerArm, HumanBodyBones.LeftUpperArm);
			childToParent.Add(HumanBodyBones.LeftHand, HumanBodyBones.LeftLowerArm);
			
			childToParent.Add(HumanBodyBones.RightShoulder, HumanBodyBones.UpperChest);
			childToParent.Add(HumanBodyBones.RightUpperArm, HumanBodyBones.RightShoulder);
			childToParent.Add(HumanBodyBones.RightLowerArm, HumanBodyBones.RightUpperArm);
			childToParent.Add(HumanBodyBones.RightHand, HumanBodyBones.RightLowerArm);
			
			var bonePositions = new Dictionary<HumanBodyBones, int>();
			int dstBoneIndex = 0;
			foreach(var curId in boneIds){
				if (!srcAnimator)
					continue;
				if (!srcAnimator.GetBoneTransform(curId))
					continue;
				bonePositions.Add(curId, dstBoneIndex);
				dstBoneIndex++;
			}
			/*
			for(int i = 0; i < boneIds.Count; i++){
				bonePositions.Add(boneIds[i], i);
			}
			*/
			
			var result = new List<int>();
			for(int boneIndex = 0; boneIndex < boneIds.Count; boneIndex++){
				var bone = boneIds[boneIndex];
				HumanBodyBones parentBone;
				if (childToParent.TryGetValue(bone, out parentBone)){
					int parentBoneIndex = -1;
					if (bonePositions.TryGetValue(parentBone, out parentBoneIndex)){
						result.Add(parentBoneIndex - boneIndex);
						continue;
					}
				}
				result.Add(0);
			}
			return result;
		}
		
		List<HumanBodyBones> getHumanBones(){
			var boneIds = new List<HumanBodyBones>();
			
			foreach(HumanBodyBones cur in System.Enum.GetValues(typeof(HumanBodyBones))){
				if ((cur == HumanBodyBones.LastBone) || ((int)cur == 0))//crashes otherwise
					continue;
				if (srcAnimator && !srcAnimator.GetBoneTransform(cur))
					continue;
				boneIds.Add(cur);
			}
			return boneIds;
		}
		
		public void samplePoints(){
			buildGradient();
			buildRandomPoints();
			
			if (!srcAnimator)
				return;
			
			points.Clear();
			var spawnedAnimator = GameObject.Instantiate(srcAnimator, transform.position, transform.rotation);			
			var animatorObj = spawnedAnimator.gameObject;			
			
			var boneIds = getHumanBones();
			
			//sampleSingleFramePoints(spawnedAnimator, boneIds, 0.0f);
			
			/*
			foreach(var curId in boneIds){
				var bone = spawnedAnimator.GetBoneTransform(curId);
				if (!bone)
					continue;
					
				points.Add(bone.position);
			}
			*/
			
			var guidName = System.Guid.NewGuid().ToString("N");
			var animControllerBaseAssetPath = string.Format("Assets/tmp-remapController-{0}.controller", guidName);
			Debug.LogFormat("guid: {0}, basePath: {1}", guidName, animControllerBaseAssetPath);
			var animControllerAssetPath = AssetDatabase.GenerateUniqueAssetPath(animControllerBaseAssetPath);
			Debug.LogFormat("generated path: {0}", animControllerAssetPath);
			var newController = UnityEditor.Animations.AnimatorController.CreateAnimatorControllerAtPath(
				animControllerAssetPath);
			
			var stateMachine = 	newController.layers[0].stateMachine;
			var stateName = "clip";
			var state = newController.layers[0].stateMachine.AddState(stateName);
			stateMachine.defaultState = state;			
			
			spawnedAnimator.runtimeAnimatorController = newController;
			foreach(var animClip in animClips){
				//var newClipPath = string.Format("Assets/tmp-{0}.anim", animClip.name);
				//var clipAssetPath = AssetDatabase.GenerateUniqueAssetPath(newClipPath);
				//var origPath = AssetDatabase.GetAssetPath(animClip);
				
				//Debug.LogFormat("origPath for the clip \"{1}\": {0}", origPath, animClip.name);
				
				state.motion = animClip;
				//AssetDatabase.CopyAsset(
				
				var firstTime = 0.0f;
				var lastTime = animClip.length;
				var timeStep = 1.0f/animClip.frameRate;
				
				//spawnedAnimator.Update(0.0f);
				spawnedAnimator.Play(stateName, 0, 0.0f);
				
				for(float t = firstTime; t <= lastTime; t += timeStep){
					//points.Clear();
					//Debug.LogFormat("Current time: {0}", t);
					animClip.SampleAnimation(spawnedAnimator.gameObject, t);
					sampleSingleFramePoints(spawnedAnimator, boneIds, t/lastTime);
					//break;
				}
			}
			
			DestroyImmediate(spawnedAnimator.gameObject);
			AssetDatabase.DeleteAsset(animControllerAssetPath);
		}
		
		public void clearPoints(){
			points.Clear();
		}
		
		void Update(){
		}
	}
}