using UnityEngine;
using UnityEditor;
using System.Collections.Generic;
using System.Linq;

namespace SceneExport{
	[System.Serializable]
	public class JsonAnimationClip: IFastJsonValue{
		public string name;
		public int id = -1;
		public AnimationClip clip = null;
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			if (clip){
				writer.writeKeyVal("name", name);
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
			}
			writer.endObject();
		}
		
		public JsonAnimationClip(){
		}
		
		public JsonAnimationClip(AnimationClip clip_, int id_){
			clip = clip_;
			id = id_;
			if (!clip)
				throw new System.ArgumentNullException("clip_");
			name = clip.name;
		}
	}
}