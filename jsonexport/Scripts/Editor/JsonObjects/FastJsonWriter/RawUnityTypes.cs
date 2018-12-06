using UnityEngine;
using UnityEditor;
using System.IO;
using System.Text;
using System.Collections;
using System.Collections.Generic;

namespace SceneExport{
	public partial class FastJsonWriter{
		public void writeRawValue(Color val){
			beginRawObject();
			writeKeyVal("r", val.r);
			writeKeyVal("g", val.g);
			writeKeyVal("b", val.b);
			writeKeyVal("a", val.a);
			endObject();
		}

		public void writeRawValue(Color32 val){
			beginRawObject();
			writeKeyVal("r", val.r);
			writeKeyVal("g", val.g);
			writeKeyVal("b", val.b);
			writeKeyVal("a", val.a);
			endObject();
		}
		
		public void writeRawValue(Vector2 val){
			beginRawObject();
			writeKeyVal("x", val.x);
			writeKeyVal("y", val.y);
			endObject();
		}

		public void writeRawValue(Vector3 val){
			beginRawObject();
			writeKeyVal("x", val.x);
			writeKeyVal("y", val.y);
			writeKeyVal("z", val.z);
			endObject();
		}

		public void writeRawValue(Vector4 val){
			beginRawObject();
			writeKeyVal("x", val.x);
			writeKeyVal("y", val.y);
			writeKeyVal("z", val.z);
			writeKeyVal("w", val.w);
			endObject();
		}

		public void writeRawValue(Matrix4x4 val){
			beginRawObject();
			writeKeyVal("e00", val.m00);
			writeKeyVal("e01", val.m01);
			writeKeyVal("e02", val.m02);
			writeKeyVal("e03", val.m03);

			writeKeyVal("e10", val.m10);
			writeKeyVal("e11", val.m11);
			writeKeyVal("e12", val.m12);
			writeKeyVal("e13", val.m13);

			writeKeyVal("e20", val.m20);
			writeKeyVal("e21", val.m21);
			writeKeyVal("e22", val.m22);
			writeKeyVal("e23", val.m23);

			writeKeyVal("e30", val.m30);
			writeKeyVal("e31", val.m31);
			writeKeyVal("e32", val.m32);
			writeKeyVal("e33", val.m33);
			endObject();
		}

		public void writeRawValue(Quaternion val){
			beginRawObject();
			writeKeyVal("x", val.x);
			writeKeyVal("y", val.y);
			writeKeyVal("z", val.z);
			writeKeyVal("w", val.w);
			endObject();
		}	
	}
}
