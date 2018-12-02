using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

namespace SceneExport{
	[System.Serializable]
	public class JsonBlendShapeFrame: IFastJsonValue{
		public int index;
		public float weight;
		public float[] deltaVerts = null;
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
		
		public JsonBlendShapeFrame(Mesh mesh, int shapeIndex, int frameIndex){
			index = frameIndex;
			weight = mesh.GetBlendShapeFrameWeight(shapeIndex, frameIndex);
			var dVerts = new Vector3[mesh.vertexCount];
			var dNorms = new Vector3[mesh.vertexCount];
			var dTangents = new Vector3[mesh.vertexCount];
			
			mesh.GetBlendShapeFrameVertices(shapeIndex, frameIndex, dVerts, dNorms, dTangents);
			deltaVerts = dVerts.toFloatArray();
			deltaNormals = dNorms.toFloatArray();
			deltaTangents = dTangents.toFloatArray();
		}
	};

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

	[System.Serializable]
	public class JsonMesh: IFastJsonValue{
		public static readonly int vertsPerLine = 4;
		public static readonly int trianglesPerLine = 4;
	
		public int id = -1;
		public string name;
		public string path;
		public List<int> materials = new List<int>();
		public bool readable = false;
		public int vertexCount = 0;
		
		public byte[] colors = null;

		public float[] verts = null;
		public float[] normals = null;
		public float[] tangents = null;
		
		public float[] uv0 = null;
		public float[] uv1 = null;
		public float[] uv2 = null;
		public float[] uv3 = null;
		public float[] uv4 = null;
		public float[] uv5 = null;
		public float[] uv6 = null;
		public float[] uv7 = null;
		
		public List<float> boneWeights = new List<float>();
		public List<int> boneIndexes = new List<int>();
		
		public int defaultSkeletonId = -1;
		//public JsonSkeleton skeleton = new JsonSkeleton();			
		
		public int blendShapeCount = 0;
		public List<JsonBlendShape> blendShapes = new List<JsonBlendShape>();
		
		public List<Matrix4x4> bindPoses = new List<Matrix4x4>();
		public List<Matrix4x4> inverseBindPoses = new List<Matrix4x4>();

		[System.Serializable]
		public class SubMesh: IFastJsonValue{
			public int[] triangles = null;//new int[0];
			public void writeRawJsonValue(FastJsonWriter writer){
				writer.beginRawObject();
				writer.writeKeyVal("triangles", triangles, 3 * trianglesPerLine);
				writer.endObject();
			}
		};

		public List<SubMesh> subMeshes = new List<SubMesh>();
		public int subMeshCount = 0;
			
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("name", name);
			writer.writeKeyVal("id", id);
			writer.writeKeyVal("path", path);
			writer.writeKeyVal("materials", materials);
			writer.writeKeyVal("readable", readable);
			writer.writeKeyVal("vertexCount", vertexCount);
			writer.writeOptionalKeyVal("colors", colors, 4 * vertsPerLine);
			writer.writeOptionalKeyVal("verts", verts, 3 * vertsPerLine);
			writer.writeOptionalKeyVal("normals", normals, 3 * vertsPerLine);
			writer.writeOptionalKeyVal("tangents", tangents, 4 * vertsPerLine);
			
			writer.writeOptionalKeyVal("uv0", uv0, 2 * vertsPerLine);
			writer.writeOptionalKeyVal("uv1", uv1, 2 * vertsPerLine);
			writer.writeOptionalKeyVal("uv2", uv2, 2 * vertsPerLine);
			writer.writeOptionalKeyVal("uv3", uv3, 2 * vertsPerLine);
			writer.writeOptionalKeyVal("uv4", uv4, 2 * vertsPerLine);
			writer.writeOptionalKeyVal("uv5", uv5, 2 * vertsPerLine);
			writer.writeOptionalKeyVal("uv6", uv6, 2 * vertsPerLine);
			writer.writeOptionalKeyVal("uv7", uv7, 2 * vertsPerLine);
			
			writer.writeOptionalKeyVal("bindPoses", bindPoses);
			writer.writeOptionalKeyVal("inverseBindPoses", inverseBindPoses);
			
			writer.writeOptionalKeyVal("boneWeights", boneWeights, 4 * vertsPerLine);
			writer.writeOptionalKeyVal("boneIndexes", boneIndexes, 4 * vertsPerLine);
			writer.writeKeyVal("origSkeletonId", defaultSkeletonId);
			
			writer.writeKeyVal("blendShapeCount", blendShapeCount);			
			writer.writeOptionalKeyVal("blendShapes", blendShapes);			
			
			writer.writeKeyVal("subMeshCount", subMeshCount);			
			writer.writeKeyVal("subMeshes", subMeshes);
			writer.endObject();			
		}

		public JsonMesh(Mesh mesh, ResourceMapper exp){
			id = exp.findMeshId(mesh);//exp.meshes.findId(mesh);
			name = mesh.name;
			//Debug.LogFormat("Processing mesh {0}", name);
			var filePath = AssetDatabase.GetAssetPath(mesh);
			exp.registerResource(filePath);
			path = filePath;

			var foundMaterials = exp.findMeshMaterials(mesh);
			if (foundMaterials != null){
				foreach(var cur in foundMaterials){
					materials.Add(exp.getMaterialId(cur));
				}
			}

			#if !UNITY_EDITOR
			readable = mesh.isReadable;
			if (!readable){
				Debug.LogErrorFormat(string.Format("Mesh {0} is not marked as readable. Canot proceed", name);
				return;
			}
			#endif
			vertexCount = mesh.vertexCount;
			if (vertexCount <= 0)
				return;

			colors = mesh.colors32.toByteArray();
			verts = mesh.vertices.toFloatArray();
			normals = mesh.normals.toFloatArray();
			
			tangents = mesh.tangents.toFloatArray();
			
			uv0 = mesh.uv.toFloatArray();
			uv1 = mesh.uv2.toFloatArray();
			uv2 = mesh.uv3.toFloatArray();
			uv3 = mesh.uv4.toFloatArray();			
			uv4 = mesh.uv5.toFloatArray();			
			uv5 = mesh.uv6.toFloatArray();			
			uv6 = mesh.uv7.toFloatArray();			
			uv7 = mesh.uv8.toFloatArray();			

			subMeshCount = mesh.subMeshCount;
			for(int i = 0; i < subMeshCount; i++){
				var subMesh = new SubMesh();
				subMesh.triangles = Utility.copyArray(mesh.GetTriangles(i));
				subMeshes.Add(subMesh);
			}
			
			boneWeights.Clear();
			boneIndexes.Clear();
			defaultSkeletonId = exp.getDefaultSkeletonId(mesh);
			
			var srcWeights = mesh.boneWeights;
			if ((srcWeights != null) && (srcWeights.Length > 0)){
				foreach(var cur in srcWeights){
					boneIndexes.Add(cur.boneIndex0);
					boneIndexes.Add(cur.boneIndex1);
					boneIndexes.Add(cur.boneIndex2);
					boneIndexes.Add(cur.boneIndex3);
					
					boneWeights.Add(cur.weight0);
					boneWeights.Add(cur.weight1);
					boneWeights.Add(cur.weight2);
					boneWeights.Add(cur.weight3);
				}
			}
			
			blendShapeCount = mesh.blendShapeCount;
			blendShapes.Clear();
			for(int i = 0; i < blendShapeCount; i++){
				blendShapes.Add(new JsonBlendShape(mesh, i));
			}
			
			var srcPoses = mesh.bindposes;
			foreach(var cur in srcPoses){
				bindPoses.Add(cur);
				var inverted = cur.inverse;
				inverseBindPoses.Add(inverted);				
			}
			//blendShapeFrames = mesh.blend

			//Debug.LogFormat("Processed mesh {0}", name);
		}
	}
}
