using UnityEngine;
using UnityEditor;

namespace SceneExport{
	[System.Serializable]
	public struct JsonTransformKey: IFastJsonValue{
		public float time;
		public JsonTransform local;
		public JsonTransform world;
		
		public float getMaxTransformDifference(JsonTransformKey other){
			return Mathf.Max(
				local.getMaxDifference(other.local),
				local.getMaxDifference(other.local)
			);
		}
		
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("time", time);
			writer.writeKeyVal("local", local);
			writer.writeKeyVal("world", world);
			writer.endObject();
		}
		
		public JsonTransformKey(float time_, Transform transform, Transform refTransform){
			time = time_;
			if (!transform)
				throw new System.ArgumentNullException("transform");
			local = new JsonTransform(transform, transform.parent ? transform.parent: refTransform);
			world = new JsonTransform(transform, refTransform);
		}
	}
}
