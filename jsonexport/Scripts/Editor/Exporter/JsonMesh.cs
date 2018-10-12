using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

namespace SceneExport{
	[System.Serializable]
	public class JsonMesh: IFastJsonValue{
		public int id = -1;
		public string name;
		public string path;
		public List<int> materials = new List<int>();
		public bool readable = false;
		public int vertexCount = 0;
		public Color[] colors = null;

		public float[] verts = null;
		public float[] normals = null;
		public float[] uv0 = null;
		public float[] uv1 = null;
		public float[] uv2 = null;
		public float[] uv3 = null;

		[System.Serializable]
		public class SubMesh: IFastJsonValue{
			public int[] triangles = null;//new int[0];
			public void writeRawJsonValue(FastJsonWriter writer){
				writer.beginRawObject();
				writer.writeKeyVal("triangles", triangles);
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
			writer.writeKeyVal("colors", colors);
			writer.writeKeyVal("verts", verts);
			writer.writeKeyVal("normals", normals);
			writer.writeKeyVal("uv0", uv0);
			writer.writeKeyVal("uv1", uv1);
			writer.writeKeyVal("uv2", uv2);
			writer.writeKeyVal("uv3", uv3);
			writer.writeKeyVal("subMeshCount", subMeshCount);
			/*
			writer.beginKeyArray("subMeshes");
			foreach(var curSubMesh in subMeshes){
				curSubMesh.writeJsonValue(writer);
			}
			writer.endArray();
			*/
			writer.writeKeyVal("subMeshes", subMeshes);
			//writer.writeKeyArray("subMeshes", subMeshes);
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

			colors = mesh.colors;
			verts = mesh.vertices.toFloatArray();
			normals = mesh.normals.toFloatArray();
			uv0 = mesh.uv.toFloatArray();
			uv1 = mesh.uv2.toFloatArray();
			uv2 = mesh.uv3.toFloatArray();
			uv3 = mesh.uv4.toFloatArray();

			subMeshCount = mesh.subMeshCount;
			for(int i = 0; i < subMeshCount; i++){
				var subMesh = new SubMesh();
				subMesh.triangles = Utility.copyArray(mesh.GetTriangles(i));
				subMeshes.Add(subMesh);
			}

			//Debug.LogFormat("Processed mesh {0}", name);
		}
	}
}
