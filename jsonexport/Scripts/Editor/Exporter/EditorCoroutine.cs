using UnityEditor;
using UnityEngine;
using System.Collections;
using System.Collections.Generic;

namespace SceneExport{
	public class EditorCoroutine{
		Stack<IEnumerator> routineData = new Stack<IEnumerator>();
		
		public bool isFinished(){
			return (routineData == null) || routineData.Count == 0;
		}
		
		bool update(){
			if (isFinished())
				return false;
			var top = routineData.Peek();
			if (!top.MoveNext()){
				routineData.Pop();
			}
			else{
				var cur = top.Current;
				IEnumerator child = cur as IEnumerator;
				if (child != null){
					routineData.Push(child);
				}
			}
			return true;
		}
		
		EditorCoroutine(IEnumerator e){
			routineData.Clear();
			if (e != null)
				routineData.Push(e);
		}
		
		public static EditorCoroutine startCoroutine(IEnumerator e){
			return Manager.instance.createCoroutine(e);
		}
		
		public static IEnumerator waitSeconds(float seconds){
			var launchTime = Time.realtimeSinceStartup;
			while(true){
				var curTime = Time.realtimeSinceStartup;
				var diff = curTime - launchTime;
				if (diff >= seconds)
					break;
				yield return null;
			}
		}

		class Manager: System.IDisposable{
			static Manager cachedInstance = null;
			static List<EditorCoroutine> coroutines = new List<EditorCoroutine>();
		
			public static Manager instance{
				get{
					if (cachedInstance == null){
						cachedInstance = new Manager();
					}
					return cachedInstance;
				}
			}
		
			public void Dispose(){
				EditorApplication.update -= update;
			}
		
			Manager(){
				EditorApplication.update += update;
			}
		
			public EditorCoroutine createCoroutine(IEnumerator e){
				var result = new EditorCoroutine(e);
				coroutines.Add(result);
				return result;
			}
		
			void update(){
				for(int i = 0; i < coroutines.Count;){
					var cur = coroutines[i];
					if (!cur.update()){
						coroutines.RemoveAt(i);
						continue;
					}
					i++;
				}
			}
		}		
	}
}