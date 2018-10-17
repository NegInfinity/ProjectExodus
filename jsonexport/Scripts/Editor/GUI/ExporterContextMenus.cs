using UnityEditor;
using UnityEngine;
using UnityEditor.SceneManagement;
using System.Collections.Generic;
using System.Collections;
using System.Linq;

namespace SceneExport{
	public static class ExportContextMenus{	
		[MenuItem("GameObject/Migration to UE4/Export current scene", false, 0)]
		public static void  exportJsonSceneProj(MenuCommand menuCommand){
			var scene = EditorSceneManager.GetActiveScene();
			if (!scene.IsValid()){
				Debug.LogWarningFormat("No active scene, cannot export.");
			}
			
			var path = EditorUtility.SaveFilePanel("Save category config", "", scene.name, "json");
			if (path == string.Empty)
				return;
				
			var logger = new Logger();						
			var proj = JsonProject.fromScene(scene, true);
			proj.saveToFile(path, true, true, logger);
			ExportResultWindow.openWindow(logger);
		}
		
		[MenuItem("GameObject/Migration to UE4/Export selected objects", false, 0)]
		public static void  exportSelectedObjectsProj(MenuCommand menuCommand){
			var objects = ExportUtility.getSelectedGameObjects();
			if (objects.Count <= 0)
				return;
			
			var path = EditorUtility.SaveFilePanel("Export selected objects", "", objects[0].name, "json");
			if (path == string.Empty)
				return;
				
			var logger = new Logger();						
			var proj = JsonProject.fromObjects(objects.ToArray(), true);
			proj.saveToFile(path, true, true, logger);
			ExportResultWindow.openWindow(logger);
		}

		[MenuItem("GameObject/Migration to UE4/Export current object", false, 0)]
		public static void  exportCurrentObjectProj(MenuCommand menuCommand){
			if (Selection.activeObject == null)
				return;
			var obj = Selection.activeGameObject;
			var path = EditorUtility.SaveFilePanel("Export current object", "", obj.name, "json");
			if (path == string.Empty)
				return;
							
			var logger = new Logger();						
			var proj = JsonProject.fromObject(obj, true);
			proj.saveToFile(path, true, true, logger);
			ExportResultWindow.openWindow(logger);
		}
		
		[MenuItem("GameObject/Migration to UE4/Export current project", false, 0)]
		public static void  exportJsonProj(MenuCommand menuCommand){
			var path = EditorUtility.SaveFilePanel("Save category config", "", Application.productName, "json");
			if (path == string.Empty)
				return;
				
			var logger = new Logger();						
			var proj = JsonProject.fromCurrentProject(true, logger);
			Debug.LogFormat("Project: {0}", proj);
			if (proj != null){
				proj.saveToFile(path, true, true, logger);
			}
			ExportResultWindow.openWindow(logger);
		}
		
	}
}
