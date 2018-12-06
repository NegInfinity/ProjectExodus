using UnityEngine;
using UnityEditor;
using System.Linq;

namespace SceneExport{
	[System.Serializable]
	public class JsonBlendShapeFrame: IFastJsonValue{
		public int index;
		public float weight;
		public float[] deltaVerts = null;
		//public List<float> deltaVerts = new List<float>();
		public float[] deltaTangents = null;
		public float[] deltaNormals = null;
		
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("index", index);
			writer.writeKeyVal("weight", weight);
			writer.writeKeyVal("deltaVerts", deltaVerts, 3 * JsonMesh.vertsPerLine);
			writer.writeKeyVal("deltaNormals", deltaNormals, 3 * JsonMesh.vertsPerLine);
			writer.writeKeyVal("deltaTangents", deltaTangents, 3 * JsonMesh.vertsPerLine);
			writer.endObject();
		}
		
		public JsonBlendShapeFrame(JsonBlendShapeFrame other){
			index = other.index;
			weight = other.weight;
			
			deltaVerts = null;
			deltaTangents = null;
			deltaNormals = null;
			
			if (other.deltaVerts != null)
				deltaVerts = other.deltaVerts.ToArray();
			if (other.deltaNormals != null)
				deltaNormals = other.deltaNormals.ToArray();
			if (other.deltaTangents != null)
				deltaTangents = other.deltaTangents.ToArray();
			
			//deltaVerts = 
		}
		
		public JsonBlendShapeFrame(){
		}
		
		public JsonBlendShapeFrame(Mesh mesh, int shapeIndex, int frameIndex){
			index = frameIndex;
			weight = mesh.GetBlendShapeFrameWeight(shapeIndex, frameIndex);
			var dVerts = new Vector3[mesh.vertexCount];
			var dNorms = new Vector3[mesh.vertexCount];
			var dTangents = new Vector3[mesh.vertexCount];
			
			mesh.GetBlendShapeFrameVertices(shapeIndex, frameIndex, dVerts, dNorms, dTangents);
			deltaVerts = dVerts.toFloatArray();//dVerts.toFloatArray();
			deltaNormals = dNorms.toFloatArray();
			deltaTangents = dTangents.toFloatArray();
		}
	};	
}
