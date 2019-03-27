using UnityEngine;
using System.Collections.Generic;

namespace SceneExport{
	[System.Serializable]
	public class JsonObjectReference<ObjectType>: IFastJsonValue where ObjectType: Object{
		public ObjectType obj;
		public int instId;// = -1;
		public bool isNull;// = false;

		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("instanceId", instId);
			writer.writeKeyVal("isNull", isNull);
			writer.endObject();
		}

		public JsonObjectReference(ObjectType obj_){
			obj = obj_;
			if (!obj){
				instId = 0;
				isNull = true;
			}
			else{
				instId = obj.GetInstanceID();
				isNull = false;
			}
		}
	}
}