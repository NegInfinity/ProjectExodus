using UnityEngine;
using System.Collections.Generic;

namespace SceneExport{
	static class Extensions{
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
	
		static public float[] toFloatArray(this Matrix4x4 arg){
			float[] result = new float[16];
			for(int i = 0; i < 4; i++){
				for(int j = 0; j < 4; j++){
					result[i*4+j] = arg[i, j];
				}
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