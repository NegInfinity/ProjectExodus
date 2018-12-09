using UnityEngine;
using UnityEditor;

namespace SceneExport{
	[System.Serializable]
	public class JsonAnimationEvent: IFastJsonValue{
		public AnimationEvent animEvent = null;
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			if (animEvent != null){
				writer.writeKeyVal("time", animEvent.time);
				writer.writeKeyVal("floatParameter", animEvent.floatParameter);
				writer.writeKeyVal("intParameter", animEvent.intParameter);
				writer.writeKeyVal("stringParameter", animEvent.stringParameter);
				writer.writeKeyVal("objectReferenceParameter", animEvent.objectReferenceParameter.ToString());
				writer.writeKeyVal("isFiredByAnimator", animEvent.isFiredByAnimator);
				writer.writeKeyVal("isFiredByLegacy", animEvent.isFiredByLegacy);
			}
			writer.endObject();
		}
		
		public JsonAnimationEvent(AnimationEvent animEvent_){
			animEvent = animEvent_;
		}
	}	
}
