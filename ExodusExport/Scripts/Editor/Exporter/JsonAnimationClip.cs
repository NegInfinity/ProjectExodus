using UnityEngine;
using UnityEditor;
using System.Collections.Generic;
using System.Linq;

namespace SceneExport{
	[System.Serializable]
	public class JsonAnimationClip: IFastJsonValue{
		public string name;
		//public int id = -1;
		public ResId id = ResId.invalid;
		public AnimationClip clip = null;
		public Animator animator = null;
		
		public List<JsonAnimationMatrixCurve> matrixCurves = new List<JsonAnimationMatrixCurve>();
		public List<JsonAnimationSampledFloatCurve> sampledFloatCurves = new List<JsonAnimationSampledFloatCurve>();
		
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("name", name);
			writer.writeKeyVal("id", id);
			
			writer.writeKeyVal("frameRate", clip.frameRate);
			writer.writeKeyVal("empty", clip.empty);
			//writer.writeKeyVal("apparentSpeed", clip.apparentSpeed);
			//writer.writeKeyVal("averageAngularSpeed", clip.averageAngularSpeed);
			//writer.writeKeyVal("averageDuration", clip.averageDuration);
			//writer.writeKeyVal("averageSpeed", clip.averageSpeed);
			writer.writeKeyVal("humanMotion", clip.humanMotion);
			writer.writeKeyVal("isLooping", clip.isLooping);
			writer.writeKeyVal("legacy", clip.legacy);
			writer.writeKeyVal("length", clip.length);
			writer.writeKeyVal("localBounds", new JsonBounds(clip.localBounds));
			writer.writeKeyVal("wrapMode", clip.wrapMode.ToString());
				
			var animEvents = AnimationUtility.GetAnimationEvents(clip)
				.Select((arg) => new JsonAnimationEvent(arg)).ToList();;
					
			writer.writeKeyVal("animEvents", animEvents);
				
			var objCurveBindings = AnimationUtility.GetObjectReferenceCurveBindings(clip)
				.Select((arg) => new JsonEditorCurveBinding(arg, clip)).ToList();;
					
			writer.writeKeyVal("objBindings", objCurveBindings);
				
			var floatCurveBindings = AnimationUtility.GetCurveBindings(clip)
				.Select((arg) => new JsonEditorCurveBinding(arg, clip)).ToList();
			writer.writeKeyVal("floatBindings", floatCurveBindings);
			
			writer.writeKeyVal("matrixCurves", matrixCurves);
			writer.writeKeyVal("sampledFloatCurves", sampledFloatCurves);
			
			writer.endObject();
		}
		
		public JsonAnimationClip(){
		}
		
		public JsonAnimationClip(AnimationClip clip_, Animator animator_, ResId id_, ResourceMapper resMap, AnimationSampler animSampler = null){
			clip = clip_;
			id = id_;
			animator = animator_;
			if (!clip)
				throw new System.ArgumentNullException("clip_");
			if (!animator)
				throw new System.ArgumentNullException("animator_");
			if (resMap == null)
				throw new System.ArgumentNullException("resMap");
				
			name = clip.name;
			sampleMatrixCurves(resMap, animSampler);
		}
		
		void sampleMatrixCurves(ResourceMapper resMap, AnimationSampler providedSampler = null){
			var skelId = resMap.skelRegistry.registerSkeleton(animator.transform, true);
			if (!ExportUtility.isValidId(skelId)){
				throw new System.ArgumentException(
					string.Format("Skeleton not found for clip \"{0}\" and animator \"{1}\"", 
						clip, animator));
			}
			var skeleton = resMap.skelRegistry.getSkeletonById(skelId);
			
			var prefabAnim = Utility.getSrcPrefabAssetObject(animator, false);

			if (providedSampler != null){
				var sampled = providedSampler.sampleClip(clip);
				matrixCurves = sampled;//.matrixCurves;
				//sampledFloatCurves = sampled.floatCurves;
			}
			else{
				using(var sampler = new AnimationSampler(prefabAnim, skeleton)){
					var sampled = sampler.sampleClip(clip);
					matrixCurves = sampled;//.matrixCurves;
					//sampledFloatCurves = sampled.floatCurves;
				}
			}
			
			matrixCurves.forEach((obj) => obj.simpleCompress());
			
			#if false
			//Black magic: on!
			//Spawning a duplicate
			
			var spawnedAnimator = GameObject.Instantiate(prefabAnim);
			if (!spawnedAnimator){
				throw new System.Exception("Instantiating of original object failed while sampling animation");
			}
			
			var transformRoot = spawnedAnimator.transform;
			
			var foundTransforms = Utility.findNamedTransforms(
				skeleton.bones.Select((arg) => arg.name).ToList(), 
				transformRoot);
			//This needs to be moved elsewhere.
			
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
			
			var firstTime = 0.0f;
			var lastTime = clip.length;
			var timeStep = 1.0f/clip.frameRate;
				
			spawnedAnimator.Play(stateName, 0, 0.0f);
			
			for(int transformIndex = 0; transformIndex < foundTransforms.Count; transformIndex++){
				var curTransform = foundTransforms[transformIndex];				
				var newCurve = new JsonAnimationMatrixCurve();
				newCurve.objectName = skeleton.bones[transformIndex].name;				
				matrixCurves.Add(newCurve);
			}
				
			for(float t = firstTime; t <= lastTime; t += timeStep){
				clip.SampleAnimation(spawnedAnimator.gameObject, t);
				for(int transformIndex = 0; transformIndex < foundTransforms.Count; transformIndex++){
					var curTransform = foundTransforms[transformIndex];
					if (!curTransform)
						continue;
					matrixCurves[transformIndex].addKey(t, curTransform, spawnedAnimator.transform);
				}
				///sampleSingleFramePoints(spawnedAnimator, boneIds, t/lastTime);
			}
			
			foreach(var cur in matrixCurves){
				cur.simpleCompress();
			}
			
			if (spawnedAnimator)
				GameObject.DestroyImmediate(spawnedAnimator.gameObject);
			if (newController)
				AssetDatabase.DeleteAsset(animControllerAssetPath);
			#endif
		}
	}
}