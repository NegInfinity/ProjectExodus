using UnityEngine;
using System.Collections.Generic;
using System.Linq;

namespace SceneExport{
	static class Extensions{
		public static void forEach<Source>(this IEnumerable<Source> arg, System.Action<Source> callback){
			if (arg == null)
				throw new System.ArgumentNullException("arg");
			if (callback == null)
				throw new System.ArgumentNullException("callback");
			foreach(var cur in arg){
				callback(cur);
			}
		}
		
		public static void forEach<Source>(this IEnumerable<Source> arg, System.Action<Source, int> callback){
			if (arg == null)
				throw new System.ArgumentNullException("arg");
			if (callback == null)
				throw new System.ArgumentNullException("callback");
			int index = 0;
			foreach(var cur in arg){
				callback(cur, index);
				index++;
			}
		}
	
		/*
		Returns "scenePath". Scene path is object names, divided by slash. There's no root slash.
		Supposed to be compatible with unity property and bone naming scheme.
		*/
		public static string getScenePath(this Transform obj){
			if (!obj)
				return "";
			if (!obj.parent)
				return obj.name;
			return obj.parent.getScenePath() + "/" + obj.name;
		}
	
		public static string getScenePath(this GameObject gameObj){
			if (!gameObj)
				return "";
			return gameObj.transform.getScenePath();
		}
		
		public static string getScenePath(this Transform obj, Transform relativeTo = null){
			if (!obj || (obj == relativeTo))
				return "";
			if (!obj.parent || (obj.parent == relativeTo))
				return obj.name;
			return obj.parent.getScenePath(relativeTo) + "/" + obj.name;				
		}
		
		public static string getScenePath(this GameObject gameObj, GameObject relativeTo = null){
			if (!gameObj)
				return "";
			if (!relativeTo)
				return gameObj.getScenePath();
			return gameObj.transform.getScenePath(relativeTo.transform);
		}
	
		public static bool hasComponent<C>(this GameObject gameObj) where C: Component{
			if (!gameObj)
				throw new System.ArgumentNullException("gameObj");
			return gameObj.GetComponent<C>();
		}
	
		public static Vector3 abs(this Vector3 arg){
			return new Vector3(
				Mathf.Abs(arg.x), 
				Mathf.Abs(arg.y), 
				Mathf.Abs(arg.z)
			);
		}
		
		public static float maxElement(this Vector3 arg){
			return Mathf.Max(arg.x, Mathf.Max(arg.y, arg.z));
		}
		
		public static float minElement(this Vector3 arg){
			return Mathf.Min(arg.x, Mathf.Min(arg.y, arg.z));
		}
		
		public static float getMaxDifference(this Vector3 arg, Vector3 arg2){
			return (arg2 - arg).abs().maxElement();
		}
		
		public static Vector3 getVector3(this Vector4 arg){
			return new Vector3(arg.x, arg.y, arg.z);
		}
		
		public static Vector3 getXyz(this Vector4 arg){
			return new Vector3(arg.x, arg.y, arg.z);
		}
		
		public static Vector4 toVector4(this Vector3 arg, float w){
			return new Vector4(arg.x, arg.y, arg.z, w);
		}
		
		static public Val[] copyArray<Val>(this Val[] arg){
			if (arg == null)
				return null;
			return arg.ToArray();
		}
		
		static public Val getValOrGenerate<Key, Val>(this IDictionary<Key, Val> dict, Key key, System.Func<Key, Val> generator){
			if (generator == null)
				throw new System.ArgumentNullException("generator");
			Val result;
			if (dict.TryGetValue(key, out result))
				return result;
			result = generator(key);
			dict.Add(key, result);
			return result;
		}
		
		static public Val getValOrDefault<Key, Val>(this IDictionary<Key, Val> dict, Key key, Val defaultVal){
			Val result;
			if (dict.TryGetValue(key, out result))
				return result;
			return defaultVal;
		}
	
		static public Val getValOrDefault<Key, Val>(this IDictionary<Key, Val> dict, Key key) where Val: new(){
			Val result;
			if (dict.TryGetValue(key, out result))
				return result;
			return new Val();
		}

		static public Val getValOrSetDefault<Key, Val>(this IDictionary<Key, Val> dict, Key key) where Val: new(){
			Val result;
			if (dict.TryGetValue(key, out result))
				return result;

			result = new Val();
			dict.Add(key, result);
			return result;
		}

		static public float[] toFloatArray(this Matrix4x4 arg){
			float[] result = new float[16];
			for(int i = 0; i < 4; i++){
				for(int j = 0; j < 4; j++){
					result[i*4+j] = arg[i, j];
				}
			}
			return result;
		}
		
		static public byte[] toByteArray(this Color32[] arg){
			int numValues = 0;
			if (arg != null)
				numValues = arg.Length;
			
			var result = new byte[numValues * 4];
			for(int i = 0; i < numValues; i++){
				result[i*4 + 0] = arg[i].r;
				result[i*4 + 1] = arg[i].g;
				result[i*4 + 2] = arg[i].b;
				result[i*4 + 3] = arg[i].a;
			}
			return result;						
		}
		
		static public List<float> toFloatList(this Vector4[] arg){
			var result = new List<float>();
			if (arg == null)
				return result;
			foreach(var cur in arg){
				result.Add(cur.x);
				result.Add(cur.y);
				result.Add(cur.z);
				result.Add(cur.w);
			}
			return result;
		}

		static public List<float> toFloatList(this Vector3[] arg){
			var result = new List<float>();
			if (arg == null)
				return result;
			foreach(var cur in arg){
				result.Add(cur.x);
				result.Add(cur.y);
				result.Add(cur.z);
			}
			return result;
		}

		static public List<float> toFloatList(this Vector2[] arg){
			var result = new List<float>();
			if (arg == null)
				return result;
			foreach(var cur in arg){
				result.Add(cur.x);
				result.Add(cur.y);
			}
			return result;
		}

		static public float[] toFloatArray(this Vector4[] arg){
			int numValues = 0;
			if (arg != null)
				numValues = arg.Length;
			var result = new float[numValues * 4];
			for(int i = 0; i < numValues; i++){
				result[i*4 + 0] = arg[i].x;
				result[i*4 + 1] = arg[i].y;
				result[i*4 + 2] = arg[i].z;
				result[i*4 + 3] = arg[i].w;
			}
			return result;				
		}

		static public float[] toFloatArray(this Vector3[] arg){
			int numValues = 0;
			if (arg != null)
				numValues = arg.Length;
			var result = new float[numValues * 3];
			for(int i = 0; i < numValues; i++){
				result[i*3 + 0] = arg[i].x;
				result[i*3 + 1] = arg[i].y;
				result[i*3 + 2] = arg[i].z;
			}
			return result;				
		}

		static public float[] toFloatArray(this Vector2[] arg){
			int numValues = 0;
			if (arg != null)
				numValues = arg.Length;
			var result = new float[numValues * 2];
			for(int i = 0; i < numValues; i++){
				result[i*2 + 0] = arg[i].x;
				result[i*2 + 1] = arg[i].y;
			}
			return result;				
		}
		
		static public float[] toFloatArray(this Vector2 arg){
			return new float[]{arg.x, arg.y};
		}

		static public float[] toFloatArray(this Vector3 arg){
			return new float[]{arg.x, arg.y, arg.z};
		}

		static public float[] toFloatArray(this Vector4 arg){
			return new float[]{arg.x, arg.y, arg.z, arg.w};
		}				
	}
}