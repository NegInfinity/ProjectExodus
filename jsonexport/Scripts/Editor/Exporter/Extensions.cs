using UnityEngine;
using System.Collections.Generic;
using System.Linq;

namespace SceneExport{
	static class Extensions{
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