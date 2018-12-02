using UnityEngine;
using System.Collections.Generic;

namespace SceneExport{
	public static class Utility{
		/*
		public static Value getValueGenerate<Key, Value, Dict>(this Dict dict, Key key, System.Func<Value> generator)
				where Dict: IDictionary<Key, Value>{*/ //Bah
		public static Value getValueGenerate<Key, Value>(this IDictionary<Key, Value> dict, Key key, System.Func<Value> generator){
			Value result;
			if (dict.TryGetValue(key, out result))
				return result;
			return generator();
		}
		
		public static Value getValueSetGenerate<Key, Value>(this IDictionary<Key, Value> dict, Key key, System.Func<Value> generator){
			Value result;
			if (dict.TryGetValue(key, out result))
				return result;
			var newVal = generator();
			dict.Add(key, newVal);
			return newVal;
		}
	
		public static string toJsonString<T>(this T arg) where T: IFastJsonValue{
			if (arg == null)
				return string.Empty;
			var writer = new FastJsonWriter();
			arg.writeRawJsonValue(writer);
			return writer.getString();
		}
		
		public static void saveToJsonFile<T>(this T arg, string filename) where T: IFastJsonValue{
			saveStringToFile(filename, arg.toJsonString());
		}
		
		public static Matrix4x4 toMatrix(this Quaternion v){
			Matrix4x4 result = Matrix4x4.identity;
			result.SetTRS(Vector3.zero, v, Vector3.one);
			return result;
		}

		public static void saveStringToFile(string filename, string data){
			System.IO.File.WriteAllText(filename, data, System.Text.Encoding.UTF8);
		}

		public static void saveBytesToFile(string filename, byte[] data){
			System.IO.File.WriteAllBytes(filename, data);
		}
		
		public static int beginHash(){
			return 17;
		}

		public static int combineHash<T>(int prevHash, T obj){
			int nextHash = 0;
			if ((object)obj != null)
				nextHash = obj.GetHashCode();
			return prevHash * 23 + nextHash;
		}
		
		public static bool isInProjectPath(string path){
			var targetDir = System.IO.Path.GetDirectoryName(System.IO.Path.GetFullPath(path));
			var projectPath = getProjectPath();
			return projectPath == targetDir;
		}
		
		public static string getProjectPath(){
			if (!Application.isEditor){
				throw new System.ArgumentException("The application is not running in editor mode");
			}
				
			var dataPath = System.IO.Path.GetFullPath(Application.dataPath);
			var projectPath = System.IO.Path.GetDirectoryName(dataPath);
			return projectPath;
		}
		
		static public Vector3[] copyArray(Vector3[] arg){
			int size = 0;
			if (arg != null)
				size = arg.Length;
			var result = new Vector3[size];
			for(int i = 0; i < size; i++)
				result[i] = arg[i];
			return result;
		}

		static public int[] copyArray(int[] arg){
			int size = 0;
			if (arg != null)
				size = arg.Length;
			var result = new int[size];
			for(int i = 0; i < size; i++)
				result[i] = arg[i];
			return result;
		}
	}
}