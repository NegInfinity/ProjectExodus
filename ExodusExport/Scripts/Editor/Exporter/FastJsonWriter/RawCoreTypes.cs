using UnityEngine;
using UnityEditor;
using System.IO;
using System.Text;
using System.Collections;
using System.Collections.Generic;

namespace SceneExport{
	public partial class FastJsonWriter{
		public void writeRawValue(string s){
			writeString(s);
		}
		
		public void writeRawValue(int val){
			builder.Append(val);
		}

		public void writeRawValue(ResId val){
			builder.Append(val.rawId);
		}

		public void writeRawValue(byte val){
			builder.Append(val);
		}

		public void writeRawValue(float val){
			builder.Append(val);
		}
		
		public void writeRawValue(bool b){
			if (b)
				builder.Append("true");
			else
				builder.Append("false");
		}
	}
}
