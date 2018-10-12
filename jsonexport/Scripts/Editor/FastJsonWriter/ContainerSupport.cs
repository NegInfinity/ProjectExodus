using UnityEngine;
using UnityEditor;
using System.IO;
using System.Text;
using System.Collections;
using System.Collections.Generic;

namespace SceneExport{
	public partial class FastJsonWriter{
		public void writeRawValue(int[] values){
			writeRawValue(values, writeRawValue);
		}
		
		public void writeRawValue(string[] values){
			writeRawValue(values, writeRawValue);
		}
		
		public void writeKeyVal(string key, int[] val){
			writeKeyVal<int>(key, val, writeRawValue);
		}
		
		public void writeKeyVal(string key, List<int> val){
			writeKeyVal<int>(key, val, writeRawValue);
		}
		
		public void writeKeyVal(string key, string[] val){
			writeKeyVal<string>(key, val, writeRawValue);
		}
		
		public void writeKeyVal(string key, List<string> val){
			writeKeyVal<string>(key, val, writeRawValue);
		}
		
		public void writeKeyVal(string key, Color[] val){
			writeKeyVal<Color>(key, val, writeRawValue);
		}
		
		public void writeKeyVal(string key, float[] val){
			writeKeyVal<float>(key, val, writeRawValue);
		}
	}
}
