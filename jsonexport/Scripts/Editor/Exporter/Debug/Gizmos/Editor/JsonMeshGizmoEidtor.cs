using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

namespace SceneExport{
#if false
	[CustomEditor(typeof(JsonMeshGizmo))]
	public class JsonMeshGizmoEditor: Editor{
		public override void OnInspectorGUI(){
			base.OnInspectorGUI();
			
			var obj = target as JsonMeshGizmo;
			if (!obj)
				return;
				
			if (GUILayout.Button("Rebuild")){
				obj.rebuildMesh();
			}
		}
	}
#endif
}