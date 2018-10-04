using UnityEngine;
using UnityEditor;
using System.IO;
using System.Text;
using System.Collections;
using System.Collections.Generic;

namespace SceneExport{
	public class ExportMenu: MonoBehaviour{
		[MenuItem("GameObject/Scene Export/Export selected objects", false, 0)]
		public static void  exportJsonScene(MenuCommand menuCommand){
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
			Utility.saveStringToFile(path, jsonObj.toJsonString());
		}

		[MenuItem("GameObject/Scene Export/Export current object", false, 0)]
		public static void  exportJsonObject(MenuCommand menuCommand){
			if (Selection.activeObject == null)
				return;
			var obj = Selection.activeGameObject;
			var path = EditorUtility.SaveFilePanel("Save category config", "", obj.name, "json");
			if (path == string.Empty)
				return;
			var exporter = new Exporter();

			var jsonObj = exporter.exportOneObject(obj);
			Utility.saveStringToFile(path, jsonObj.toJsonString());
		}
	}
}

