using UnityEngine;
using UnityEditor;
using System.Collections;
using System.Collections.Generic;
using UnityEditor.SceneManagement;


namespace SceneExport{
	public static class ExportUtility{
		public static string formatString(string fmt, params object[] args){
			return string.Format(fmt, args);
		}
		
		public static bool showCancellableProgressBar(string title, string info, int progress, int maxProgress){
			float floatProgress = 0.0f;
			if (maxProgress != 0)
				floatProgress = (float)progress/(float)maxProgress;
			return EditorUtility.DisplayCancelableProgressBar(title, info, floatProgress);
		}
		
		public static void showProgressBar(string title, string info, int progress, int maxProgress){
			float floatProgress = 0.0f;
			if (maxProgress != 0)
				floatProgress = (float)progress/(float)maxProgress;
			EditorUtility.DisplayProgressBar(title, info, floatProgress);
		}
		
		public static void hideProgressBar(){
			EditorUtility.ClearProgressBar();
		}
		
		public static JsonType[] convertComponents<CompType, JsonType>(GameObject gameObject, 
				System.Func<CompType, JsonType> converter) //thanks for not letting me specify constructor with parameters constraint, I guess?
				where CompType: Component {				
			if (!gameObject)
				throw new System.ArgumentNullException("gameObject");
			if (converter == null)
				throw new System.ArgumentNullException("converter");				
				
			var components = gameObject.GetComponents<CompType>();
			var jsonObjects = new List<JsonType>();
			var componentIndex = 0;
			foreach(var curComponent in components){
				var curIndex = componentIndex;
				componentIndex++;
				if (!curComponent){
					Debug.LogWarningFormat("Component {0} is null on object {1}. ComponentType: {2}"
						, curIndex, gameObject, typeof(CompType).Name);
					continue;
				}
				
				var jsonObject = converter(curComponent);
				jsonObjects.Add(jsonObject);
			}
			
			return jsonObjects.ToArray();
		}
		
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
