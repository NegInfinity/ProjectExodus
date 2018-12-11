using UnityEngine;
using UnityEditor;

namespace SceneExport{
	[System.Serializable]
	public struct JsonKeyframe: IFastJsonValue{
		Keyframe key;
		
		int weightedModeToInt(WeightedMode mode){
			switch(mode){
				case(WeightedMode.None):
					return 0;
				case(WeightedMode.In):
					return 1;
				case(WeightedMode.Out):
					return 2;
				case(WeightedMode.Both):
					return 3;
				default:
					return 0;
			}
		}
		
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject(false);
			writer.writeKeyVal("time", key.time, false);
			writer.writeKeyVal("value", key.value, false);
			writer.writeKeyVal("weightedMode", weightedModeToInt(key.weightedMode), false);//(int)key.weightedMode);
			writer.writeKeyVal("inTangent", key.inTangent, false);
			writer.writeKeyVal("inWeight", key.inWeight, false);
			writer.writeKeyVal("outTangent", key.outTangent, false);
			writer.writeKeyVal("outWeight", key.outWeight, false);
			writer.endObject(false);
		}
		
		public JsonKeyframe(Keyframe key_){
			key = key_;
		}
		/*
		public JsonKeyframe(){		
		}
		*/
	};	
}