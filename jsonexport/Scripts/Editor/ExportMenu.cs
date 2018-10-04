using UnityEngine;
using UnityEditor;
using System.Collections.Generic;
using UnityEditor.SceneManagement;

namespace SceneExport{
	public class ExportMenu /*: MonoBehaviour*/{
		[MenuItem("GameObject/Scene Export/Export current scene", false, 0)]
		public static void  exportJsonScene(MenuCommand menuCommand){
			var scene = EditorSceneManager.GetActiveScene();
			if (!scene.IsValid()){
				Debug.LogWarningFormat("No active scene, cannot export.");
			}
			
			var path = EditorUtility.SaveFilePanel("Save category config", "", scene.name, "json");
			if (path == string.Empty)
				return;
			var exporter = new Exporter();

			var jsonObj = exporter.exportScene(scene);
			jsonObj.saveToFile(path);
		}
		
		[MenuItem("GameObject/Scene Export/Export selected objects", false, 0)]
		public static void  exportSelectedObjects(MenuCommand menuCommand){
			if (Selection.transforms.Length <= 0)
				return;

			List<GameObject> objects = new List<GameObject>();
			foreach(var cur in Selection.transforms){
				if (cur == null)
					continue;
				var curObj = cur.gameObject;
				if (!curObj)
					continue;
				objects.Add(curObj);
			}
			if (objects.Count <= 0)
				return;
			
			var path = EditorUtility.SaveFilePanel("Save category config", "", objects[0].name, "json");
			if (path == string.Empty)
				return;
			var exporter = new Exporter();

			var jsonObj = exporter.exportObjects(objects.ToArray());
			jsonObj.saveToFile(path);
		}

		[MenuItem("GameObject/Scene Export/Export current object", false, 0)]
		public static void  exportCurrentObject(MenuCommand menuCommand){
			if (Selection.activeObject == null)
				return;
			var obj = Selection.activeGameObject;
			var path = EditorUtility.SaveFilePanel("Save category config", "", obj.name, "json");
			if (path == string.Empty)
				return;
			var exporter = new Exporter();

			var jsonObj = exporter.exportOneObject(obj);
			jsonObj.saveToFile(path);
		}
	}
}

