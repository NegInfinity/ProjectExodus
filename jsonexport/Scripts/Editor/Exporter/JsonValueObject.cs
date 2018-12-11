using UnityEngine;
using UnityEditor;

namespace SceneExport{
	[System.Serializable]
	public class JsonValueObject: IFastJsonValue{
		virtual public void writeJsonObjectFields(FastJsonWriter writer){
		}
		
		public void writeRawJsonValue(FastJsonWriter writer){
			//writer.beginObjectValue();
			writer.beginRawObject();
			writeJsonObjectFields(writer);
			writer.endObject();
		}
	}
}