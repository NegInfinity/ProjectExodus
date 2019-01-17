using UnityEngine;
using UnityEditor;
using System.Linq;
using System.Collections.Generic;

namespace SceneExport{
	[System.Serializable]
	public class JsonAnimationCurve: IFastJsonValue{
		public int length = 0;
		public string preWrapMode;
		public string postWrapMode;
		public List<JsonKeyframe> keys = new List<JsonKeyframe>();
		
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("length", length);
			writer.writeKeyVal("preWrapMode", preWrapMode);
			writer.writeKeyVal("postWrapMode", postWrapMode);
			writer.writeKeyVal("keys", keys);
			writer.endObject();
		}
		
		public JsonAnimationCurve(AnimationCurve curve){
			length = curve.length;
			postWrapMode = curve.postWrapMode.ToString();
			preWrapMode = curve.preWrapMode.ToString();
			keys = curve.keys.Select((arg) => new JsonKeyframe(arg)).ToList();			
		}
	}	
}
