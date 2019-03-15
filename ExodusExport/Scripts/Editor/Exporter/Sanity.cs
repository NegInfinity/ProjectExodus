using UnityEngine;

namespace SceneExport{
	/*
	We do have Debug.Assert in unity, but it can be disabled by certain flags, so it isn't really that suitable...
	Additionally, I'm tired of typing nullchecks manually every time.
	*/
	public class Sanity{
		public static void check(bool condition, string message = null){
			if (!condition){
				if (!string.IsNullOrEmpty(message))
					throw new System.ArgumentException(message);				
				throw new System.ArgumentException();
			}
		}

		public static void nullCheck(object obj, string paramName = null, string message = null){
			if (obj != null)
				return;

			if (!string.IsNullOrEmpty(paramName)){
				if (!string.IsNullOrEmpty(message))
					throw new System.ArgumentNullException(paramName, message);
				else
					throw new System.ArgumentNullException(paramName);
			}
			else{
				if (!string.IsNullOrEmpty(message)){
					throw new System.ArgumentNullException("obj", message);
				}
			}

			throw new System.ArgumentNullException();
		}
	}
}