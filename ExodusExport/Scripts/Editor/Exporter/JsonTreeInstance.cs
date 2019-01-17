using UnityEditor;
using UnityEngine;
using System.Collections.Generic;

namespace SceneExport{
	[System.Serializable]
	public class JsonTreeInstance: IFastJsonValue{
		public TreeInstance data;
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("color", data.color);
			writer.writeKeyVal("heightScale", data.heightScale);
			writer.writeKeyVal("lightmapColor", data.lightmapColor);
			writer.writeKeyVal("position", data.position);
			writer.writeKeyVal("rotation", data.rotation);//this is an angle and not a quaternion, surprisingly
			writer.writeKeyVal("prototypeIndex", data.prototypeIndex);
			writer.writeKeyVal("widthScale", data.widthScale);			
			writer.endObject();
		}
		
		public JsonTreeInstance(TreeInstance inst){
			data = inst;
		}
	}
}
