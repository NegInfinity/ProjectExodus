using UnityEngine;
using UnityEditor;

namespace SceneExportTests{
	[CustomEditor(typeof(AnimSamplerTest))]
	public class AnimSamplerTestEditor: Editor{
		public override void OnInspectorGUI(){		
			base.OnInspectorGUI();			
			
			var obj = target as AnimSamplerTest;
			if (!obj)
				return;
			
			
			if (GUILayout.Button("Sample")){
				obj.samplePoints();
			}
			if (GUILayout.Button("Random")){
				obj.buildRandomPoints();
			}
			if (GUILayout.Button("Clear")){
				obj.clearPoints();
			}
		}
	}
}