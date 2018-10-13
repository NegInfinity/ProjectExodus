using UnityEngine;
using UnityEditor;
using System.Collections;
using System.Collections.Generic;
using UnityEditor.SceneManagement;


namespace SceneExport{
	public static class ExportUtility{
		/*
		static Scene getActiveScene(){
			return null;
		}
		*/
		
		public static List<GameObject> getSelectedGameObjects(){
			List<GameObject> result = new List<GameObject>();
			if (Selection.transforms.Length <= 0)
				return result;

			foreach(var cur in Selection.transforms){
				if (cur == null)
					continue;
				var curObj = cur.gameObject;
				if (!curObj)
					continue;
				result.Add(curObj);
			}
			return result;
		}
	}
}
