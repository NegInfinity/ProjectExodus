using UnityEngine;
using UnityEditor;
using System.IO;
using System.Text;
using System.Collections;
using System.Collections.Generic;

namespace SceneExport{
	public partial class FastJsonWriter{
		public int indent = 0;
		public StringBuilder builder = new StringBuilder();
		public Stack<int> valCount = new Stack<int>();
		
		public delegate void RawValueWriter<Value>(Value val);
		public delegate void RawStaticValueWriter<Value>(FastJsonWriter writer, Value val);

		public string getString(){
			return builder.ToString();
		}

		public void writeIndent(){
			for (int i = 0; i < indent; i++)
				builder.Append("\t");
		}

		public void beginDocument(){
			beginRawObject();
		}

		public void endDocument(){
			endObject();
		}

		public void processComma(){
			var count = valCount.Pop();
			valCount.Push(count+1);
			if (count > 0)
				builder.AppendLine(",");
			writeIndent();
		}
		
		public void processComma(bool indent){
			var count = valCount.Pop();
			valCount.Push(count+1);
			if (count > 0){
				if (indent)
					builder.AppendLine(",");
				else
					builder.Append(", ");
			}
			if (indent)
				writeIndent();
		}

		public void beginRawObject(){
			builder.AppendLine("{");
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

		public void beginRawArray(){
			builder.AppendLine("[");
			indent++;
			valCount.Push(0);
		}

		public void endArray(){
			indent--;
			builder.AppendLine();
			writeIndent();
			builder.Append("]");
			valCount.Pop();
		}

		public void writeKey(string key){
			processComma();
			writeString(key);
			builder.Append(": ");
		}
		
		public void beginKeyArray(string key){
			writeKey(key);
			beginRawArray();
		}

		public void beginKeyObject(string key){
			writeKey(key);
			beginRawObject();
		}
	}
}