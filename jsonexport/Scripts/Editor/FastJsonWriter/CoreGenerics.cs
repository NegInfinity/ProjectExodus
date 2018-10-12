using UnityEngine;
using UnityEditor;
using System.IO;
using System.Text;
using System.Collections;
using System.Collections.Generic;

namespace SceneExport{
	public partial class FastJsonWriter{
		public void writeRawValue<T>(T val) where T: IFastJsonValue{
			val.writeRawJsonValue(this);
		}
		
		public void writeValue<T>(T val) where T: IFastJsonValue{
			processComma();
			writeRawValue<T>(val);
		}
		
		public void writeValue<T>(T val, RawValueWriter<T> writerFunc){
			processComma();
			writerFunc(val);			
		}
		
		public void writeKeyVal<T>(string key, T val, RawValueWriter<T> writerFunc){
			writeKey(key);
			writerFunc(val);			
		}
		
		public void writeKeyVal<T>(string key, T[] val, RawValueWriter<T> writerFunc){
			writeKey(key);
			writeRawValue(val, writerFunc);
		}
		
		public void writeKeyVal<T>(string key, List<T> val, RawValueWriter<T> writerFunc){
			writeKey(key);
			writeRawValue(val, writerFunc);
		}
		
		public void writeKeyVal<T>(string key, List<T> val) where T: IFastJsonValue{
			writeKey(key);
			writeRawValue(val);
		}
		
		public void writeRawValue<T>(T[] values, RawValueWriter<T> writer){
			beginRawArray();
			foreach(var cur in values){
				writeValue<T>(cur, writer);
			}
			endArray();						
		}
		
		public void writeRawValue<T>(List<T> values, RawValueWriter<T> writer){
			beginRawArray();
			if (values != null){
				foreach(var cur in values){
					writeValue<T>(cur, writer);
				}
			}
			endArray();						
		}
		
		public void writeRawValue<T>(T[] values) where T: IFastJsonValue{
			beginRawArray();
			if (values != null){
				foreach(var cur in values){
					writeValue(cur);
				}
			}
			endArray();						
		}
		
		public void writeRawValue<T>(List<T> values) where T: IFastJsonValue{
			beginRawArray();
			if (values != null){
				foreach(var cur in values){
					writeValue(cur);
				}
			}
			endArray();						
		}
	}
}