using UnityEngine;
using UnityEditor;
using System.Collections;
using System.Collections.Generic;
using UnityEditor.SceneManagement;


namespace SceneExport{
	public static class ExportUtility{
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
