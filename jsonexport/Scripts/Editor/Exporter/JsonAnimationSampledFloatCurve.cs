using UnityEngine;
using UnityEditor;
using System.Linq;
using System.Collections.Generic;

namespace SceneExport{
	[System.Serializable]
	public class JsonAnimationSampledFloatCurve: IFastJsonValue{
		public string objectName;
		public string objectPath;
		public List<JsonSampledFloatKey> keys = new List<JsonSampledFloatKey>();
		
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("objectName", objectName);
			writer.writeKeyVal("objectPath", objectPath);
			writer.writeKeyVal("length", (keys != null) ? keys.Count: 0);
			writer.writeKeyVal("keys", keys);
			writer.endObject();
		}
		
		public JsonAnimationSampledFloatCurve(){
		}
		
		public JsonAnimationSampledFloatCurve(string name_, string path_){
			objectName = name_;
			objectPath = path_;
		}		
	}	
}