using UnityEngine;
using UnityEditor;

namespace SceneExport{
	[System.Serializable]
	public struct JsonSampledFloatKey: IFastJsonValue{
		public float time;
		public int frame;
		public float val;
									
		public float getMaxTransformDifference(JsonSampledFloatKey other){
			return Mathf.Abs(other.val - val);
		}
		
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject(false);
			writer.writeKeyVal("time", time, false);
			writer.writeKeyVal("frame", frame, false);
			writer.writeKeyVal("val", val, false);
			writer.endObject(false);
		}
		
		public JsonSampledFloatKey(float time_, int frame_, float val_){
			time = time_;
			frame = frame_;
			val = val_;
		}
	}
}
