using UnityEngine;
using UnityEditor;
using System.Collections.Generic;
using System.Linq;

namespace SceneExport{
	[System.Serializable]
	public class JsonBlendShape: IFastJsonValue{
		public string name = "";
		public int index = -1;
		public int numFrames = 0;
		public List<JsonBlendShapeFrame> frames = new List<JsonBlendShapeFrame>();
		
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("name", name);
			writer.writeKeyVal("index", index);
			writer.writeKeyVal("numFrames", numFrames);
			writer.writeKeyVal("frames", frames);
			writer.endObject();
		}
		
		public JsonBlendShape(JsonBlendShape other){
			name = other.name;
			index = other.index;
			numFrames = other.numFrames;
			
			frames = other.frames.Select((arg) => new JsonBlendShapeFrame(arg)).ToList();
		}
		
		public JsonBlendShape(Mesh mesh, int index_){
			if (!mesh){
				throw new System.ArgumentNullException("mesh");
			}
			index = index_;
			if ((index < 0) || (index >= mesh.blendShapeCount)){
				throw new System.ArgumentException("Invalid blendshape index", "index_");
			}
			name = mesh.GetBlendShapeName(index);
			numFrames = mesh.GetBlendShapeFrameCount(index);
			for(int frameIndex = 0; frameIndex < numFrames; frameIndex++){
				frames.Add(new JsonBlendShapeFrame(mesh, index, frameIndex));
			}
		}
	}
}
