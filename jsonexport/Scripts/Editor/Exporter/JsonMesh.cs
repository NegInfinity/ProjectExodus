using UnityEngine;
using UnityEditor;
using System.Collections.Generic;
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

	[System.Serializable]
	public class JsonMesh: IFastJsonValue{
		public static readonly int vertsPerLine = 4;
		public static readonly int trianglesPerLine = 4;
		public static readonly int bonesPerVertex = 4;//only 4 of them in unity.
	
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
		public List<string> defaultBoneNames = new List<string>();
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
			
			public SubMesh(SubMesh other){
				triangles = other.triangles.copyArray();
			}
			
			public SubMesh(){
			}
		};

		public List<SubMesh> subMeshes = new List<SubMesh>();
		public int subMeshCount = 0;

		/*
		static void processArray<T>(T[] arg, System.Func<T, T> callback){
			if (callback == null)
				throw new System.ArgumentNullException("callback");
			if (arg == null)
				return;
			for(int i = 0; i < arg.Length; i++)
				arg[i] = callback(arg[i]);
		}
		*/
		
		static void processFloats3(float[] args, System.Func<Vector3, Vector3> callback){
			if (callback == null)
				throw new System.ArgumentNullException("callback");
				
			if (args == null)
				return;
			for(int i = 0; (i + 2) < args.Length; i += 3){
				var v = new Vector3(args[i], args[i+1], args[i+2]);
				var v1 = callback(v);
				args[i] = v1.x;
				args[i+1] = v1.y;
				args[i+2] = v1.z;
			}
		}		
		
		static void processFloats4(float[] args, System.Func<Vector4, Vector4> callback){
			if (callback == null)
				throw new System.ArgumentNullException("callback");
				
			if (args == null)
				return;
				
			for(int i = 0; (i + 3) < args.Length; i += 4){
				var v = new Vector4(args[i], args[i+1], args[i+2], args[i+3]);
				var v1 = callback(v);
				args[i] = v1.x;
				args[i+1] = v1.y;
				args[i+2] = v1.z;
				args[i+3] = v1.w;
			}
		}		
		
		static void processFloats2(float[] args, System.Func<Vector2, Vector2> callback){
			if (callback == null)
				throw new System.ArgumentNullException("callback");
				
			if (args == null)
				return;
				
			for(int i = 0; (i + 1) < args.Length; i += 2){
				var v = new Vector4(args[i], args[i+1]);
				var v1 = callback(v);
				args[i] = v1.x;
				args[i+1] = v1.y;
			}
		}
		
		static Vector4 transformTangent(Matrix4x4 matrix, Vector4 tangent){
			var v3 = new Vector3(tangent.x, tangent.y, tangent.z);
			v3 = matrix.MultiplyVector(v3);
			return new Vector4(v3.x, v3.y, v3.z, tangent.w);
		}
		
		public void transformWith(Matrix4x4 matrix){
			processFloats3(verts, (arg) => matrix.MultiplyPoint(arg));
			processFloats3(normals, (arg) => matrix.MultiplyVector(arg));
			processFloats4(tangents, (arg) => transformTangent(matrix, arg)	);
			
			for(int blendShapeIndex = 0; blendShapeIndex < blendShapes.Count; blendShapeIndex++){
				var blendShape = blendShapes[blendShapeIndex];
				for(int blendFrameIndex = 0; blendFrameIndex < blendShape.frames.Count; blendFrameIndex++){
					var blendFrame = blendShape.frames[blendFrameIndex];					
					processFloats3(blendFrame.deltaVerts, (arg) => matrix.MultiplyVector(arg));
					processFloats3(blendFrame.deltaNormals, (arg) => matrix.MultiplyVector(arg));
					processFloats3(blendFrame.deltaTangents, (arg) => matrix.MultiplyVector(arg));
				}
			}
			var invMatrix = matrix.inverse;
			for(int i = 0; i < bindPoses.Count; i++){
				bindPoses[i] = matrix * bindPoses[i];
			}
			for(int i = 0; i < inverseBindPoses.Count; i++){
				inverseBindPoses[i] = inverseBindPoses[i] * invMatrix;				
			}
		}
		
		public bool isSkeletalMesh(){
			return (boneWeights != null)
				&& (boneIndexes != null)
				&& (boneWeights.Count > 0)
				&& (boneIndexes.Count > 0)
				&& (bindPoses != null)
				&& (bindPoses.Count > 0);
		}
		
		Vector3 linearBlend(Vector3 arg, List<Matrix4x4> matrices, int weightIndex, bool point){
			Vector3 result = Vector3.zero;
			
			int baseOffset = weightIndex * bonesPerVertex;
			
			float total = 0.0f;
			for(int i = 0; i < bonesPerVertex; i++){
				total += boneWeights[baseOffset + i];				
			} 
			
			float scaleFactor = (total != 0.0f) ? 1.0f/total: 1.0f;
			for(int i = 0; i < bonesPerVertex; i++){
				var curWeight = boneWeights[baseOffset + i] * scaleFactor;				
				if (curWeight == 0.0f)
					continue;
				var curTransformIndex = boneIndexes[baseOffset + i];
				var curTransform = matrices[curTransformIndex] * inverseBindPoses[curTransformIndex];
				
				var vert = point ? curTransform.MultiplyPoint(arg): curTransform.MultiplyVector(arg);
				result += vert * curWeight;				
			}
			
			return result;
		}
		
		public static Vector4 getIdxVector4(float[] floats, int vertIndex){
			var baseOffset = vertIndex * 4;
			return new Vector4(
				floats[baseOffset], floats[baseOffset + 1], floats[baseOffset + 2], floats[baseOffset + 3]);
		}
		
		public static void setIdxVector4(float[] floats, int vertIndex, Vector4 newVal){
			var baseOffset = vertIndex * 4;
			floats[baseOffset] = newVal.x;
			floats[baseOffset + 1] = newVal.y;
			floats[baseOffset + 2] = newVal.z;
			floats[baseOffset + 3] = newVal.w;
		}
		
		public static Vector3 getIdxVector3(float[] floats, int vertIndex){
			var baseOffset = vertIndex * 3;
			return new Vector3(floats[baseOffset], floats[baseOffset + 1], floats[baseOffset + 2]);
		}
		
		public static void setIdxVector3(float[] floats, int vertIndex, Vector3 newVal){
			var baseOffset = vertIndex * 3;
			floats[baseOffset] = newVal.x;
			floats[baseOffset + 1] = newVal.y;
			floats[baseOffset + 2] = newVal.z;
		}
		
		public delegate Data IndexedDataProcessorCallback<Data>(Data arg, int argIndex);
		
		/*
		We need to fold it into one generic function, at some point.
		*/
		public static void processFloats2(float[] floats, IndexedDataProcessorCallback<Vector2> callback){
			if (floats == null)
				return;
			if (callback == null)
				throw new System.ArgumentNullException("callback");
			for(int offset = 0, index = 0; offset < (floats.Length - 1); offset += 2, index++){
				var src = new Vector2(floats[offset], floats[offset + 1]);
				var dst = callback(src, index);
				floats[offset + 0] = dst.x;
				floats[offset + 1] = dst.y;
			}
		} 
		
		public static void processFloats3(float[] floats, IndexedDataProcessorCallback<Vector3> callback){
			if (floats == null)
				return;
			if (callback == null)
				throw new System.ArgumentNullException("callback");
			for(int offset = 0, index = 0; offset < (floats.Length - 2); offset += 3, index++){
				var src = new Vector3(floats[offset], floats[offset + 1], floats[offset + 2]);
				var dst = callback(src, index);
				floats[offset + 0] = dst.x;
				floats[offset + 1] = dst.y;
				floats[offset + 2] = dst.z;				
			}
		} 
		
		public static void processFloats4(float[] floats, IndexedDataProcessorCallback<Vector4> callback){
			if (floats == null)
				return;
			if (callback == null)
				throw new System.ArgumentNullException("callback");
			for(int offset = 0, index = 0; offset < (floats.Length - 3); offset += 4, index++){
				var src = new Vector4(floats[offset], floats[offset + 1], floats[offset + 2], floats[offset + 3]);
				
				var dst = callback(src, index);
				
				floats[offset + 0] = dst.x;
				floats[offset + 1] = dst.y;
				floats[offset + 2] = dst.z;				
				floats[offset + 3] = dst.w;
			}
		} 
		
		public static Vector2 getIdxVector2(float[] floats, int vertIndex){
			var baseOffset = vertIndex * 2;
			return new Vector3(floats[baseOffset], floats[baseOffset + 1]);
		}
		
		public static void setIdxVector2(float[] floats, int vertIndex, Vector2 newVal){
			var baseOffset = vertIndex * 2;
			floats[baseOffset] = newVal.x;
			floats[baseOffset + 1] = newVal.y;
		}
		
		public void setBindPosesFromTransforms(List<Transform> bones, Transform root){
			if (bones == null)
				throw new System.ArgumentException("bones");
			if (bones.Count != bindPoses.Count){
				throw new System.ArgumentException(
					string.Format("Mismatched number of transform on mesh {2}. {0} provided vs {1} requried",
						bones.Count, bindPoses.Count, name)
				);
			}
			if (!root){
				for(int i = 0; i < bones.Count; i++){
					var curBone = bones[i];
					bindPoses[i] = curBone.worldToLocalMatrix;
					inverseBindPoses[i] = curBone.localToWorldMatrix;
				}
			}
			else{
				var rootMatrix = root.localToWorldMatrix;
				var rootInvMatrix = root.worldToLocalMatrix;
				if (root.parent){
					var parentMatrix = root.parent.localToWorldMatrix;
					var parentInvMatrix = root.parent.worldToLocalMatrix;
					
					rootMatrix = parentInvMatrix * rootMatrix;
					rootInvMatrix = rootInvMatrix * parentMatrix;
				}
				
				for(int i = 0; i < bones.Count; i++){
					var curBone = bones[i];
					
					var boneTransform = rootInvMatrix * curBone.localToWorldMatrix;
					var boneInvTransform = curBone.worldToLocalMatrix * rootMatrix;
					
					bindPoses[i] = boneInvTransform;
					inverseBindPoses[i] = boneTransform;
				}
			}
		}
		
		public void transformSkeletalMesh(List<Matrix4x4> matrices){
			if (matrices == null)
				throw new System.ArgumentNullException("matrices");
			if (matrices.Count != bindPoses.Count){
				throw new System.ArgumentException(
					string.Format("Mismatched number of matrices on mesh {2}. {0} provided vs {1} required", 
						matrices.Count, bindPoses.Count, name));
			}
			
			if (!isSkeletalMesh()){
				throw new System.ArgumentException(string.Format("Non-skeletal meshes cannot be transformed by matrix list. Mesh name {0}",
					name));
			}
			
			processFloats3(verts, (vert, idx) => linearBlend(vert, matrices, idx, true));
			processFloats3(normals, (norm, idx) => linearBlend(norm, matrices, idx, false));
			processFloats4(tangents, (tang, idx) => linearBlend(tang.getVector3(), matrices, idx, false).toVector4(tang.w));
						
			for(int blendShapeIndex = 0; blendShapeIndex < blendShapes.Count; blendShapeIndex++){
				var curBlendShape = blendShapes[blendShapeIndex];
				for(int blendFrameIndex = 0; blendFrameIndex < curBlendShape.frames.Count; blendFrameIndex++){
					var blendFrame = curBlendShape.frames[blendFrameIndex];
					processFloats3(blendFrame.deltaVerts, (arg, idx) => linearBlend(arg, matrices, idx, false));
					processFloats3(blendFrame.deltaNormals, (arg, idx) => linearBlend(arg, matrices, idx, false));
					processFloats3(blendFrame.deltaTangents, (arg, idx) => linearBlend(arg, matrices, idx, false));
				}
			}
		}
		
		public JsonMesh(JsonMesh other){
			if (other == null)
				throw new System.ArgumentNullException();
			id = other.id;
			name = other.name;
			path = other.path;
			materials = other.materials.ToList();
			readable = other.readable;
			vertexCount = other.vertexCount;
			
			colors = other.colors.copyArray();
			verts = other.verts.copyArray();
			tangents = other.tangents.copyArray();
			
			uv0 = other.uv0.copyArray();
			uv1 = other.uv1.copyArray();
			uv2 = other.uv2.copyArray();
			uv3 = other.uv3.copyArray();
			uv4 = other.uv4.copyArray();
			uv5 = other.uv5.copyArray();
			uv6 = other.uv6.copyArray();
			uv7 = other.uv7.copyArray();
			
			boneWeights = other.boneWeights.ToList();
			boneIndexes = other.boneIndexes.ToList();
			
			defaultSkeletonId = other.defaultSkeletonId;
			
			defaultBoneNames = other.defaultBoneNames.ToList();
			blendShapeCount = other.blendShapeCount;
			blendShapes = other.blendShapes.Select((arg) => new JsonBlendShape(arg)).ToList();
			
			bindPoses = other.bindPoses.ToList();
			inverseBindPoses = other.inverseBindPoses.ToList();
			
			bindPoses = other.bindPoses.ToList();
			inverseBindPoses = other.inverseBindPoses.ToList();
			
			subMeshes = other.subMeshes.Select((arg) => new SubMesh(arg)).ToList();
			
			subMeshCount = other.subMeshCount;
		}
			
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
			writer.writeKeyVal("defaultSkeletonId", defaultSkeletonId);
			writer.writeKeyVal("defaultBoneNames", defaultBoneNames);
			
			writer.writeKeyVal("blendShapeCount", blendShapeCount);			
			writer.writeOptionalKeyVal("blendShapes", blendShapes);			
			
			writer.writeKeyVal("subMeshCount", subMeshCount);			
			writer.writeKeyVal("subMeshes", subMeshes);
			writer.endObject();			
		}

		public JsonMesh(ResourceMapper.MeshStorageKey meshKey, int id_, ResourceMapper exp){
			id = id_;//exp.findMeshId(mesh);//exp.meshes.findId(mesh);
			var mesh = meshKey.mesh;
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
			defaultSkeletonId = exp.getDefaultSkeletonId(meshKey);
			defaultBoneNames = exp.getDefaultBoneNames(meshKey);
			
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
