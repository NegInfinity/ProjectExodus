using UnityEngine;
using UnityEditor;
using System.IO;
using System.Text;
using System.Collections;
using System.Collections.Generic;

namespace SceneExport{
	public class FastJsonWriter{
		public int indent = 0;
		public StringBuilder builder = new StringBuilder();
		public Stack<int> valCount = new Stack<int>();

		public void beginDocument(){
			beginRawObject();
		}

		public void endDocument(){
			endObject();
		}

		public string getString(){
			return builder.ToString();
		}

		public void writeIndent(){
			for (int i = 0; i < indent; i++)
				builder.Append("\t");
		}

		public void processComma(){
			var count = valCount.Pop();
			valCount.Push(count+1);
			if (count > 0)
				builder.AppendLine(",");
			writeIndent();
		}

		public void beginRawObject(){
			builder.AppendLine("{");
			indent++;
			valCount.Push(0);
		}

		public void beginRawArray(){
			builder.AppendLine("[");
			indent++;
			valCount.Push(0);
		}

		public void endObject(){
			indent--;
			builder.AppendLine();
			writeIndent();
			builder.Append("}");
			valCount.Pop();
		}

		public void endArray(){
			indent--;
			builder.AppendLine();
			writeIndent();
			builder.Append("]");
			valCount.Pop();
		}

		public void beginArrayValue(){
			processComma();
			beginRawArray();
		}

		public void beginObjectValue(){
			processComma();
			beginRawObject();
		}

		public void writeKey(string key){
			processComma();
			writeString(key);
			builder.Append(": ");
		}

		public void writeKeyVal(string key, string val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeKeyVal(string key, bool val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeKeyVal(string key, int val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeKeyVal(string key, float val){
			writeKey(key);
			writeRawValue(val);
		}

		public void beginKeyArray(string key){
			writeKey(key);
			beginRawArray();
		}

		public void beginKeyObject(string key){
			writeKey(key);
			beginRawObject();
		}

		public void writeRawValue(bool b){
			if (b)
				builder.Append("true");
			else
				builder.Append("false");
		}

		public void writeRawValue(int[] val){
			beginRawArray();
			foreach(var cur in val){
				writeValue(cur);
			}
			endArray();
		}

		public void writeRawValue(IList<int> val){
			beginRawArray();
			foreach(var cur in val){
				writeValue(cur);
			}
			endArray();
		}

		public void writeRawValue(float[] val){
			beginRawArray();
			foreach(var cur in val){
				writeValue(cur);
			}
			endArray();
		}

		public void writeRawValue(Color[] val){
			beginRawArray();
			foreach(var cur in val){
				writeValue(cur);
			}
			endArray();
		}

		public void writeRawValue(IList<float> val){
			beginRawArray();
			foreach(var cur in val){
				writeValue(cur);
			}
			endArray();
		}

		public void writeRawValue(Vector3 val){
			beginRawObject();
			writeKeyVal("x", val.x);
			writeKeyVal("y", val.y);
			writeKeyVal("z", val.z);
			endObject();
		}

		public void writeRawValue(Color val){
			beginRawObject();
			writeKeyVal("r", val.r);
			writeKeyVal("g", val.g);
			writeKeyVal("b", val.b);
			writeKeyVal("a", val.a);
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

		public void writeRawValue(Vector2 val){
			beginRawObject();
			writeKeyVal("x", val.x);
			writeKeyVal("y", val.y);
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

		public void writeRawValue(Quaternion val){
			beginRawObject();
			writeKeyVal("x", val.x);
			writeKeyVal("y", val.y);
			writeKeyVal("z", val.z);
			writeKeyVal("w", val.w);
			endObject();
		}

		public void writeRawValue(int val){
			builder.Append(val);
		}

		public void writeRawValue(float val){
			builder.Append(val);
		}

		public void writeRawValue(string s){
			writeString(s);
		}

		/*
		public void writeKeyVal<Val>(string key, Val val){
			writeKey(key);
			dynamic tmp = val;
			writeRawValue(tmp);
		}

		public void writeValue<Val>(Val val){
			processComma();
			dynamic tmp = val;
			writeRawValue(tmp);
		}
		*/
		public void writeKeyVal(string key, int[]val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeValue(int[] val){
			processComma();
			writeRawValue(val);
		}

		public void writeKeyVal(string key, Color[]val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeValue(Color[] val){
			processComma();
			writeRawValue(val);
		}

		public void writeKeyVal(string key, List<int> val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeValue(List<int> val){
			processComma();
			writeRawValue(val);
		}

		public void writeKeyVal(string key, float[]val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeValue(float[] val){
			processComma();
			writeRawValue(val);
		}

		public void writeKeyVal(string key, List<float> val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeValue(List<float> val){
			processComma();
			writeRawValue(val);
		}

		public void writeValue(string val){
			processComma();
			writeRawValue(val);
		}

		public void writeValue(int val){
			processComma();
			writeRawValue(val);
		}

		public void writeValue(float val){
			processComma();
			writeRawValue(val);
		}

		public void writeValue(Vector3 val){
			processComma();
			writeRawValue(val);
		}

		public void writeValue(Vector2 val){
			processComma();
			writeRawValue(val);
		}

		public void writeValue(Vector4 val){
			processComma();
			writeRawValue(val);
		}

		public void writeValue(Quaternion val){
			processComma();
			writeRawValue(val);
		}

		public void writeValue(Color val){
			processComma();
			writeRawValue(val);
		}

		public void writeValue(Matrix4x4 val){
			processComma();
			writeRawValue(val);
		}

		public void writeKeyVal(string key, Vector3 val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeKeyVal(string key, Vector2 val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeKeyVal(string key, Vector4 val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeKeyVal(string key, Quaternion val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeKeyVal(string key, Color val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeKeyVal(string key, Matrix4x4 val){
			writeKey(key);
			writeRawValue(val);
		}

		public void writeString(string s){
			builder.Append("\"");
			if (s != null){
				foreach(var c in s){
					switch(c){
						case '"':
							builder.Append("\\\""); break;
        				case '\\': 
	        				builder.Append("\\\\"); break;
    	    			case '\b': 
        					builder.Append("\\b"); break;
        				case '\f': 
        					builder.Append("\\f"); break;
        				case '\n': 
        					builder.Append("\\n"); break;
        				case '\r': 
        					builder.Append("\\r"); break;
						case '\t': 
							builder.Append("\\t"); break;
						default:{
							if (('\x00' <= c) && (c <= '\x1f')){
								builder.Append("\\u");
								builder.AppendFormat("{0:x4}", (int)c);
							}
							else
								builder.Append(c);
							break;
						}
					}
				}
			}
			builder.Append("\"");
		}
	}

}