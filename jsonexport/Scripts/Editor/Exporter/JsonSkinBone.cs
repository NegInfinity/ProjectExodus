using UnityEngine;
using UnityEditor;

namespace SceneExport{
	[System.Serializable]
	public class JsonSkinBone: IFastJsonValue{
		public string name;
		public Matrix4x4 pose;
		public Matrix4x4 invPose;
		public int parentIndex = -1;
		
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("name", name);
			writer.writeKeyVal("parent", parentIndex);
			writer.writeKeyVal("pose", pose);
			writer.writeKeyVal("invPose", invPose);
			writer.endObject();
		}
		
		public JsonSkinBone(){
		}
		
		public JsonSkinBone(string name_, Matrix4x4 pose_, int parentIndex_){
			name = name_;
			pose = pose_;
			parentIndex = parentIndex_;
			invPose = pose.inverse;
		}
	}	
}
