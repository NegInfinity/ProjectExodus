using UnityEditor;
using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.Linq;

namespace SceneExport{
	public class EditorCoroutine{
		Stack<IEnumerator> routineData = new Stack<IEnumerator>();
		
		public bool isFinished(){
			return (routineData == null) || routineData.Count == 0;
		}
		
		public void terminate(){
			routineData.Clear();
		}
		
		public static void unrollCoroutine(IEnumerator e){
			if (e == null)
				return;
			while(e.MoveNext()){
				object cur = e.Current;
				var child = cur as IEnumerator;
				if (child != null)
					unrollCoroutine(child);
			}
		}
		
		void update(){
			//Debug.LogFormat("Updating a coroutine");
			if (isFinished()){
				//Debug.LogFormat("Coroutine is terminated");
				return;// false;
			}
			while(routineData.Count > 0){
				//Debug.LogFormat("Couroutine cycle starting. {0} level(s) in current coroutine", routineData.Count);
				var top = routineData.Peek();
				if (!top.MoveNext()){
					//Debug.LogFormat("Current routine level is exhausted. Popping");
					routineData.Pop();
					//Debug.LogFormat("Stack levels: {0}", routineData.Count);
					continue;
				}
				else{
					var cur = top.Current;
					IEnumerator child = cur as IEnumerator;
					if (child != null){
						//Debug.LogFormat("Child subroutine has been created. Pushing.");
						routineData.Push(child);
						//Debug.LogFormat("Stack levels: {0}", routineData.Count);
						continue;
					}
				}
				//Debug.LogFormat("Current cycle is over");
				break;
			}
			/*
			var result = isFinished();//true;
			Debug.LogFormat("Finished: {0}", result);
			return result;
			*/
		}
		
		EditorCoroutine(IEnumerator e){
			routineData.Clear();
			if (e != null)
				routineData.Push(e);
		}
		
		public static EditorCoroutine start(IEnumerator e){
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
			
			void unsubscribeFromEvents(){
				EditorApplication.update -= update;
			}
			
			void subscribeToEvents(){
				EditorApplication.update += update;
			}
		
			public void Dispose(){
				unsubscribeFromEvents();
			}
		
			Manager(){
				subscribeToEvents();
			}
		
			public EditorCoroutine createCoroutine(IEnumerator e){
				var result = new EditorCoroutine(e);
				coroutines.Add(result);
				return result;
			}
		
			void update(){
				if (coroutines.Count > 0){
					//Debug.LogFormat("Updating editor coroutines. {0} coroutines total", coroutines.Count);
				}
				for(int i = 0; i < coroutines.Count;){
					var cur = coroutines[i];
					//Debug.LogFormat("Updating coroutine {0}", i);
					cur.update();
					//Debug.LogFormat("Coroutine {0} is finished: {1}", i, cur.isFinished());
					if (cur.isFinished()){
						//Debug.LogFormat("Removing coroutine {0}", i);
						coroutines.RemoveAt(i);
						continue;
					}
					i++;
				}
			}
		}		
	}
}