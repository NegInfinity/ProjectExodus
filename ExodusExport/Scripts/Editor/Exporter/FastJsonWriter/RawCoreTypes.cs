using UnityEngine;
using UnityEditor;
using System.IO;
using System.Text;
using System.Collections;
using System.Collections.Generic;
using System.Globalization;

namespace SceneExport{
	public partial class FastJsonWriter{
		static readonly CultureInfo writerCulture = CultureInfo.InvariantCulture;//new CultureInfo()
		public void writeRawValue(string s){
			writeString(s);
		}
		
		public void writeRawValue(int val){
			//builder.Append(val);
			builder.Append(val.ToString(writerCulture));
		}

		public void writeRawValue(ResId val){
			//builder.Append(val.rawId);
			builder.Append(val.rawId.ToString(writerCulture));
		}

		public void writeRawValue(byte val){
			//builder.Append(val);
			builder.Append(val.ToString(writerCulture));
		}

		public void writeRawValue(float val){
			//builder.Append(val);
			//This is necessary, because on some systems CurrentCulture returns culutre thath as "," as decimal separator, which breaks json code.
			builder.Append(val.ToString(writerCulture));
		}
		
		public void writeRawValue(bool b){
			if (b)
				builder.Append("true");
			else
				builder.Append("false");
		}
	}
}
