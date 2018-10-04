using UnityEngine;

namespace SceneExport{
	public static class Utility{
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
	}
	
	public partial class Exporter{
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