using UnityEngine;

namespace SceneExport{
	public static class Utility{
		public static void saveStringToFile(string filename, string data){
			System.IO.File.WriteAllText(filename, data, System.Text.Encoding.UTF8);
		}

		public static void saveBytesToFile(string filename, byte[] data){
			System.IO.File.WriteAllBytes(filename, data);
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