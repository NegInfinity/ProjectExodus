using UnityEngine;
using UnityEditor;
using System.Collections.Generic;
using System.Linq;

namespace SceneExport{
	[System.Serializable]
	public class JsonMesh: IFastJsonValue{
		public static readonly int vertsPerLine = 4;
		public static readonly int trianglesPerLine = 4;
		public static readonly int bonesPerVertex = 4;//only 4 of them in unity.
	
		public ResId id = ResId.invalid;
		public MeshUsageFlags usageFlags = MeshUsageFlags.None;
		//Those two flags cannot be both set at once.
		public bool convexCollider = false;//If set, the mesh is used as convex collider... meaning plane-based collision in unreal.
		public bool triangleCollider = false;/*If set, the mesh is used as a triangle collider, meaning "complex query for simple collision" in unreal.*/
		public string name;
		public string uniqueName;
		public string path;
		public List<ResId> materials = new List<ResId>();
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
		
		public ResId defaultSkeletonId = ResId.invalid;
		public List<string> defaultBoneNames = new List<string>();
		
		public string defaultMeshNodeName;
		public string defaultMeshNodePath;
		public Matrix4x4 defaultMeshNodeMatrix;//relative to root
		//public JsonSkeleton skeleton = new JsonSkeleton();			
		
		public int blendShapeCount = 0;
		public List<JsonBlendShape> blendShapes = new List<JsonBlendShape>();
		
		public List<Matrix4x4> bindPoses = new List<Matrix4x4>();

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
		
		public delegate Matrix4x4 BindPoseProcessDelegate(Matrix4x4 pose, int index);
		
		public void processBindPoses(BindPoseProcessDelegate callback){
			if (callback == null)
				throw new System.ArgumentNullException("callback");
			bindPoses = bindPoses.Select((arg, i) => callback(arg, i)).ToList();
		}
		
		/*
		Dumb function that directly multiplies bindPoses with provided matrices.
		*/
		public void transformBindPoses(Matrix4x4 preMul, Matrix4x4 postMul){
			/*				
				inverse(worldMatrix * inverse(bindPose)) ==>
				bindPose * inverse(worldMatrix).
				
				Hmm.
				
				So, original vertex transform was: (RTL)
				targetBoneTransform * bindPoseTransfrom
				
				We introduce new multiplier into equation, and now we rotate the mesh.
				BindPoses should go along with it. 
				
				So....
				bindPose = inverse(meshTransform * inverse(bindPose));
				So....
				bindPose * inverse(meshTransform)
				Yet I somehow get incorrect scaling factors in resulting mesh. 
				What am I missing?
			*/
			//var invMatrix = matrix.inverse;
			for(int i = 0; i < bindPoses.Count; i++){
				bindPoses[i] = postMul * bindPoses[i] * preMul;
			}
		}
		
		public void transformMeshWith(Matrix4x4 matrix){
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
			
		}
		
		public bool isSkeletalMesh(){
			return (boneWeights != null)
				&& (boneIndexes != null)
				&& (boneWeights.Count > 0)
				&& (boneIndexes.Count > 0)
				&& (bindPoses != null)
				&& (bindPoses.Count > 0);
		}
		
		public Vector3 linearBlend(Vector3 arg, List<Matrix4x4> matrices, int weightIndex, bool point){
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
				var curTransform = matrices[curTransformIndex] * bindPoses[curTransformIndex];//inverseBindPoses[curTransformIndex];
				
				var vert = point ? curTransform.MultiplyPoint(arg): curTransform.MultiplyVector(arg);
				result += vert * curWeight;				
			}
			
			return result;
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
					
					//var boneTransform = rootInvMatrix * curBone.localToWorldMatrix;
					var boneInvTransform = curBone.worldToLocalMatrix * rootMatrix;
					
					bindPoses[i] = boneInvTransform;
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
			
			verts.processFloats3((vert, idx) => linearBlend(vert, matrices, idx, true));
			normals.processFloats3((norm, idx) => linearBlend(norm, matrices, idx, false));
			tangents.processFloats4((tang, idx) => linearBlend(tang.getVector3(), matrices, idx, false).toVector4(tang.w));
						
			for(int blendShapeIndex = 0; blendShapeIndex < blendShapes.Count; blendShapeIndex++){
				var curBlendShape = blendShapes[blendShapeIndex];
				for(int blendFrameIndex = 0; blendFrameIndex < curBlendShape.frames.Count; blendFrameIndex++){
					var blendFrame = curBlendShape.frames[blendFrameIndex];
					blendFrame.deltaVerts.processFloats3((arg, idx) => linearBlend(arg, matrices, idx, false));
					blendFrame.deltaNormals.processFloats3((arg, idx) => linearBlend(arg, matrices, idx, false));
					blendFrame.deltaTangents.processFloats3((arg, idx) => linearBlend(arg, matrices, idx, false));
				}
			}
		}
		
		public JsonMesh(JsonMesh other){
			if (other == null)
				throw new System.ArgumentNullException();
			id = other.id;
			name = other.name;
			uniqueName = other.uniqueName;
			usageFlags = other.usageFlags;
			convexCollider = other.convexCollider;
			triangleCollider = other.triangleCollider;

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
			
			defaultMeshNodeName = other.defaultMeshNodeName;
			defaultMeshNodePath = other.defaultMeshNodePath;
			defaultMeshNodeMatrix = other.defaultMeshNodeMatrix;
			
			blendShapeCount = other.blendShapeCount;
			blendShapes = other.blendShapes.Select((arg) => new JsonBlendShape(arg)).ToList();
			
			bindPoses = other.bindPoses.ToList();
			
			subMeshes = other.subMeshes.Select((arg) => new SubMesh(arg)).ToList();
			
			subMeshCount = other.subMeshCount;
		}
			
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("name", name);
			writer.writeKeyVal("id", id);
			writer.writeKeyVal("uniqueName", uniqueName);
			writer.writeKeyVal("path", path);
			writer.writeKeyVal("convexCollider", convexCollider);
			writer.writeKeyVal("triangleCollider", triangleCollider);
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
			writer.writeOptionalKeyVal("inverseBindPoses", 
				bindPoses.Select((arg) => arg.inverse).ToList());
			writer.writeOptionalKeyVal("bindPoseTransforms", 
				bindPoses.Select((arg) => new JsonTransform(arg, true)).ToList());
			
			writer.writeOptionalKeyVal("boneWeights", boneWeights, 4 * vertsPerLine);
			writer.writeOptionalKeyVal("boneIndexes", boneIndexes, 4 * vertsPerLine);
			writer.writeKeyVal("defaultSkeletonId", defaultSkeletonId);
			writer.writeKeyVal("defaultBoneNames", defaultBoneNames);
			
			writer.writeKeyVal("defaultMeshNodeName", defaultMeshNodeName);
			writer.writeKeyVal("defaultMeshNodePath", defaultMeshNodePath);
			writer.writeKeyVal("defaultMeshNodeMatrix", defaultMeshNodeMatrix);
			
			writer.writeKeyVal("blendShapeCount", blendShapeCount);			
			writer.writeOptionalKeyVal("blendShapes", blendShapes);			
			
			writer.writeKeyVal("subMeshCount", subMeshCount);			
			writer.writeKeyVal("subMeshes", subMeshes);
			writer.endObject();			
		}

		public JsonMesh(MeshStorageKey meshKey, ResId id_, ResourceMapper resMap){
			id = id_;
			usageFlags = meshKey.usageFlags;
			convexCollider = meshKey.usageFlags.HasFlag(MeshUsageFlags.ConvexCollider);
			triangleCollider = meshKey.usageFlags.HasFlag(MeshUsageFlags.TriangleCollider);

			var mesh = meshKey.mesh;
			name = mesh.name;
			var filePath = AssetDatabase.GetAssetPath(mesh);
			resMap.registerAssetPath(filePath);
			path = filePath;
			uniqueName = resMap.createUniqueAssetName(filePath, name, meshKey.getMeshAssetSuffix());

			var foundMaterials = resMap.findMeshMaterials(mesh);
			if (foundMaterials != null){
				foreach(var cur in foundMaterials){
					materials.Add(resMap.getMaterialId(cur));
				}
			}

			#if !UNITY_EDITOR
			readable = mesh.isReadable;
			if (!readable){
				Debug.LogErrorFormat(string.Format("Mesh {0} is not marked as readable. Cannot proceed", name);
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
			defaultSkeletonId = resMap.skelRegistry.getDefaultSkeletonId(meshKey);
			defaultBoneNames = resMap.skelRegistry.getDefaultBoneNames(meshKey);
			
			defaultMeshNodeName = resMap.skelRegistry.getDefaultMeshNodeName(meshKey);
			defaultMeshNodePath = resMap.skelRegistry.getDefaultMeshNodePath(meshKey);
			defaultMeshNodeMatrix = resMap.skelRegistry.getDefaultMeshNodeMatrix(meshKey);
			
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
			
			bindPoses = mesh.bindposes.ToList();
		}
	}
}
