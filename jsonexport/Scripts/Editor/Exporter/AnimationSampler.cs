using UnityEngine;
using UnityEditor;
using System.Collections.Generic;
using System.Linq;

namespace SceneExport{
	[System.Serializable]
	public class AnimationSampler: System.IDisposable{	
		public Animator spawnedAnimator = null;
		//public GameObject spawnedGameObject = null;
		public Transform transformRoot = null;
		public List<Transform> targetTransforms = new List<Transform>();
		public string animControllerPath = "";
		public UnityEditor.Animations.AnimatorController samplingController = null;
		public UnityEditor.Animations.AnimatorState state = null;
		public string stateName;
			
		public void releaseResources(){
			if (spawnedAnimator){
				GameObject.DestroyImmediate(spawnedAnimator.gameObject);
			}
			spawnedAnimator = null;
			//spawnedGameObject = null;
			if (!string.IsNullOrEmpty(animControllerPath) && samplingController){
				AssetDatabase.DeleteAsset(animControllerPath);
			}
			transformRoot = null;
			targetTransforms.Clear();
			animControllerPath = "";
			samplingController = null;
			state = null;
			stateName = "";
		}
		
		public void Dispose(){
			releaseResources();
		}
			
		public AnimationSampler(Animator origAnimator, JsonSkeleton skeleton){			
			if (!origAnimator)
				throw new System.ArgumentNullException();
			spawnedAnimator = GameObject.Instantiate(origAnimator);
			if (!spawnedAnimator){		
				throw new System.Exception("Instantiating of original object failed while sampling animation");
			}
			
			transformRoot = spawnedAnimator.transform;
				
			targetTransforms = Utility.findNamedTransforms(
				skeleton.bones.Select((arg) => arg.name).ToList(),
				transformRoot);
				
			var guidName = System.Guid.NewGuid().ToString("N");
			var animControllerBaseAssetPath = string.Format("Assets/tmp-remapController-{0}.controller", guidName);
			Debug.LogFormat("guid: {0}, basePath: {1}", guidName, animControllerBaseAssetPath);
			animControllerPath = AssetDatabase.GenerateUniqueAssetPath(animControllerBaseAssetPath);
			Debug.LogFormat("generatedPath: {0}", animControllerPath);
				
			samplingController = UnityEditor.Animations.AnimatorController.CreateAnimatorControllerAtPath(animControllerPath);
			var stateMachine = samplingController.layers[0].stateMachine;
			stateName = "clip";
			state = samplingController.layers[0].stateMachine.AddState(stateName);
				
			spawnedAnimator.runtimeAnimatorController = samplingController;
		}
			
		public List<JsonAnimationMatrixCurve> sampleClip(AnimationClip animClip){
			var result = new List<JsonAnimationMatrixCurve>();
			
			state.motion = animClip;

			spawnedAnimator.Play(stateName, 0, 0.0f);
			
			var firstTime = 0.0f;
			var lastTime = animClip.length;
			var timeStep = (animClip.frameRate != 0.0f) ? 1.0f / animClip.frameRate: 1.0f;
				
			var matrixCurves = targetTransforms.Select((arg) => 
				arg ? new JsonAnimationMatrixCurve(arg.name, arg.getScenePath(transformRoot)): new JsonAnimationMatrixCurve())
				.ToList();
					
			for(float t = firstTime; t <= lastTime; t+= timeStep){
				animClip.SampleAnimation(spawnedAnimator.gameObject, t);
				targetTransforms.forEach((curTransform, index) => {
					if (curTransform)
						matrixCurves[index].addKey(t, curTransform, transformRoot);
				});
			}
					
			return matrixCurves;
		}
	}
}