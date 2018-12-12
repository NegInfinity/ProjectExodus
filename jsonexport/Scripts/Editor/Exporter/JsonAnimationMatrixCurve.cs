using UnityEngine;
using UnityEditor;
using System.Linq;
using System.Collections.Generic;

namespace SceneExport{
	[System.Serializable]
	public class JsonAnimationMatrixCurve: IFastJsonValue{
		public string objectName;
		public string objectPath;
		//public bool isDiscrete = false;
		//public int length = 0;
		//public string preWrapMode;
		//public string postWrapMode;
		public List<JsonTransformKey> keys = new List<JsonTransformKey>();
		
		public void addKey(float t, Transform curTransform, Transform refTransform){
			var newKey = new JsonTransformKey(t, curTransform, refTransform);
			keys.Add(newKey);
		}
		
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("objectName", objectName);
			writer.writeKeyVal("objectPath", objectPath);
			//writer.writeKeyVal("isDiscrete", isDiscrete);
			writer.writeKeyVal("length", (keys != null) ? keys.Count: 0);
			//writer.writeKeyVal("preWrapMode", preWrapMode);
			//writer.writeKeyVal("postWrapMode", postWrapMode);
			writer.writeKeyVal("keys", keys);
			writer.endObject();
		}
		
		public void simpleCompress(float matrixEpsilon = 0.00001f){
			var newKeys = keys.Where(
				(key, idx) => 
					(idx == 0) 
					|| (idx == keys.Count - 1) 
					|| (keys[idx - 1].getMaxTransformDifference(key) >= matrixEpsilon)
					|| (keys[idx + 1].getMaxTransformDifference(key) >= matrixEpsilon)
			).ToList();
			keys = newKeys;
		}
		
		/*
		public JsonAnimationMatrixCurve(AnimationCurve curve){
			length = curve.length;
			postWrapMode = curve.postWrapMode.ToString();
			preWrapMode = curve.preWrapMode.ToString();
			keys = curve.keys.Select((arg) => new JsonKeyframe(arg)).ToList();			
		}
		*/
	}	
}