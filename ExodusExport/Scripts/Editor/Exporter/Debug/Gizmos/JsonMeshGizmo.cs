using UnityEngine;
using UnityEditor;
using System.Collections.Generic;
using System.Linq;

namespace SceneExport{
#if false
	[ExecuteInEditMode]
	[System.Serializable]
	public class JsonMeshGizmo: MonoBehaviour{
		public SkinnedMeshRenderer skinMeshRend = null;
		public JsonMesh jsonMesh = null;
		public Transform skeletonRoot = null;
		public List<Transform> origTransforms = new List<Transform>();
		public List<Transform> debugTransforms = new List<Transform>();
		public List<Vector3> transformedVerts = new List<Vector3>();
		
		void Start(){
		}
		
		void Update(){
		}
		
		public void rebuildMesh(){
			if (!skinMeshRend)
				return;
			var resMap = new ResourceMapper();
			//jsonMesh = new JsonMesh(new Mesh)
			
			var prefabRoot = ExportUtility.getLinkedRootPrefabAsset(skinMeshRend.gameObject);
			var assetSkeletonRoot = JsonSkeletonBuilder.findSkeletonRoot(skinMeshRend);
			var mesh = skinMeshRend.sharedMesh;
			
			jsonMesh = new JsonMesh(new MeshStorageKey(mesh, prefabRoot, assetSkeletonRoot), 0, resMap);
			
			var boneNames = skinMeshRend.bones.Select((arg) => arg.name).ToList();
			origTransforms = skinMeshRend.bones.ToList();
			if (skeletonRoot)
				debugTransforms = Utility.findNamedTransforms(boneNames, skeletonRoot);
			else
				debugTransforms = null;
			//var foundTransforms = boneNames.Select((arg) => ;
			
		}
		
		void drawGizmos(Color col){
			if (jsonMesh == null)
				return;
			if ((jsonMesh.verts == null) || (jsonMesh.verts.Length == 0))
				return;
				
			var oldColor = Gizmos.color;
			var oldTransform = Gizmos.matrix;
			Gizmos.color = col;
			Gizmos.matrix = transform.localToWorldMatrix;
			
			var transforms = debugTransforms;
			if (transforms == null)
				transforms = origTransforms;

			//var matrices = transforms.Select((arg) => Matrix4x4.identity).ToList();
			var matrices = transforms.Select((arg) => arg.localToWorldMatrix).ToList();
			//var transformedVerts = new List<Vector3>();
			transformedVerts.Clear();			
			for(int i = 0; i < jsonMesh.vertexCount; i++){
				//transformedVerts.Add(jsonMesh.verts.getIdxVector3(i));//jsonMesh.linearBlend(jsonMesh.verts.getIdxVector3(i), matrices, i, true));
				transformedVerts.Add(jsonMesh.linearBlend(jsonMesh.verts.getIdxVector3(i), matrices, i, true));
			}
				
			foreach(var curSubmesh in jsonMesh.subMeshes){
				var trigs = curSubmesh.triangles;
				for(int i = 0; (i + 2) < trigs.Length; i += 3){
					var a = transformedVerts[trigs[i]];
					var b = transformedVerts[trigs[i + 1]];
					var c = transformedVerts[trigs[i + 2]];
					
					Gizmos.DrawLine(a, b);
					Gizmos.DrawLine(a, c);
					Gizmos.DrawLine(b, c);
				}
				//curSubmesh.tr
			}
			//var triangles = jsonMesh.sub
			//for(int i = 0; i < 			
			
			Gizmos.matrix = oldTransform;
			Gizmos.color = col;
		}
		
		void OnDrawGizmos(){
			drawGizmos(Color.yellow);
		}
		
		void OnDrawGizmosSelected(){
			drawGizmos(Color.white);
		}
	}
#endif
}