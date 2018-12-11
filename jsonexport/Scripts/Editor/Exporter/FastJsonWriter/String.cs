using UnityEngine;
using UnityEditor;
using System.IO;
using System.Text;
using System.Collections;
using System.Collections.Generic;

namespace SceneExport{
	public partial class FastJsonWriter{
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