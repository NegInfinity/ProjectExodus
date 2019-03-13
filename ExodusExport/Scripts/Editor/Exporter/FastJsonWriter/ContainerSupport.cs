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
		
		public void writeRawValue(ResId[] values){
			writeRawValue(values, writeRawValue);
		}
		
		public void writeKeyVal(string key, int[] val){
			writeKeyVal<int>(key, val, writeRawValue);
		}
		
		public void writeKeyVal(string key, int[] val, int stride){
			writeKeyVal<int>(key, val, writeRawValue, stride);
		}
		
		public void writeKeyVal(string key, byte[] val){
			writeKeyVal<byte>(key, val, writeRawValue);
		}
		
		public void writeKeyVal(string key, byte[] val, int stride){
			writeKeyVal<byte>(key, val, writeRawValue, stride);
		}
		
		public void writeOptionalKeyVal(string key, byte[] val, int stride){
			if ((val == null) || (val.Length == 0))
				return;
			writeKeyVal<byte>(key, val, writeRawValue, stride);
		}
		
		public void writeKeyVal(string key, List<float> val){
			writeKeyVal<float>(key, val, writeRawValue);
		}
		
		public void writeKeyVal(string key, List<float> val, int stride){
			writeKeyVal<float>(key, val, writeRawValue, stride);
		}
		
		public void writeOptionalKeyVal(string key, List<float> val, int stride){
			if ((val == null) || (val.Count == 0))
				return;
			writeKeyVal<float>(key, val, writeRawValue, stride);
		}
		
		public void writeKeyVal(string key, List<int> val){
			writeKeyVal<int>(key, val, writeRawValue);
		}
		
		public void writeKeyVal(string key, List<int> val, int stride){
			writeKeyVal<int>(key, val, writeRawValue, stride);
		}
		
		public void writeKeyVal(string key, List<ResId> val){
			writeKeyVal<ResId>(key, val, writeRawValue);
		}
		
		public void writeKeyVal(string key, List<ResId> val, int stride){
			writeKeyVal<ResId>(key, val, writeRawValue, stride);
		}
		
		public void writeOptionalKeyVal(string key, List<int> val, int stride){
			if ((val == null) || (val.Count == 0))
				return;
			writeKeyVal<int>(key, val, writeRawValue, stride);
		}
		
		public void writeKeyVal(string key, List<Matrix4x4> val){
			writeKeyVal<Matrix4x4>(key, val, writeRawValue);
		}
		
		public void writeOptionalKeyVal(string key, List<Matrix4x4> val){
			if ((val == null) || (val.Count == 0))
				return;
			writeKeyVal<Matrix4x4>(key, val, writeRawValue);
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
		
		public void writeKeyVal(string key, float[] val, int stride){
			writeKeyVal<float>(key, val, writeRawValue, stride);
		}
		
		public void writeOptionalKeyVal(string key, float[] val, int stride){
			if ((val == null) || (val.Length == 0))
				return;
			writeKeyVal<float>(key, val, writeRawValue, stride);
		}
	}
}
