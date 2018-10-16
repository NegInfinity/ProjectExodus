using UnityEngine;
using UnityEditor;
using System.Collections.Generic;
using UnityEditor.SceneManagement;

namespace SceneExport{
	public class ExportMenu{	
		[MenuItem("GameObject/Scene Export/Export current scene", false, 0)]
		public static void  exportJsonSceneProj(MenuCommand menuCommand){
			var scene = EditorSceneManager.GetActiveScene();
			if (!scene.IsValid()){
				Debug.LogWarningFormat("No active scene, cannot export.");
			}
			
			var path = EditorUtility.SaveFilePanel("Save category config", "", scene.name, "json");
			if (path == string.Empty)
				return;
				
			var proj = JsonProject.fromScene(scene);
			proj.saveToFile(path, true);
		}
		
		[MenuItem("GameObject/Scene Export/Export selected objects", false, 0)]
		public static void  exportSelectedObjectsProj(MenuCommand menuCommand){
			var objects = ExportUtility.getSelectedGameObjects();
			if (objects.Count <= 0)
				return;
			
			var path = EditorUtility.SaveFilePanel("Export selected objects", "", objects[0].name, "json");
			if (path == string.Empty)
				return;
				
			var proj = JsonProject.fromObjects(objects.ToArray());
			proj.saveToFile(path, true);
		}

		[MenuItem("GameObject/Scene Export/Export current object", false, 0)]
		public static void  exportCurrentObjectProj(MenuCommand menuCommand){
			if (Selection.activeObject == null)
				return;
			var obj = Selection.activeGameObject;
			var path = EditorUtility.SaveFilePanel("Export current object", "", obj.name, "json");
			if (path == string.Empty)
				return;
				
			var proj = JsonProject.fromObject(obj);
			proj.saveToFile(path, true);
		}
	}
}

