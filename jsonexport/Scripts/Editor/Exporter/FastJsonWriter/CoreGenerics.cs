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
		
		public void writeValue<T>(T val, bool indent) where T: IFastJsonValue{
			processComma(indent);
			writeRawValue<T>(val);
		}
		
		public void writeValue<T>(T val, RawValueWriter<T> writerFunc, bool indent){
			processComma(indent);
			writerFunc(val);			
		}
		
		public void writeKeyVal<T>(string key, T val, RawValueWriter<T> writerFunc){
			writeKey(key);
			writerFunc(val);			
		}
		
		public void writeKeyVal<T>(string key, T[] val, RawValueWriter<T> writerFunc){
			writeKeyVal(key, val, writerFunc, 1);
		}
		
		public void writeKeyVal<T>(string key, T[] val, RawValueWriter<T> writerFunc, int stride){
			writeKey(key);
			writeRawValue(val, writerFunc, stride);
		}
		
		public void writeKeyVal<T>(string key, List<T> val, RawValueWriter<T> writerFunc){
			writeKey(key);
			writeRawValue(val, writerFunc);
		}
		
		public void writeKeyVal<T>(string key, List<T> val, RawValueWriter<T> writerFunc, int stride){
			writeKey(key);
			writeRawValue(val, writerFunc, stride);
		}
		
		public void writeKeyVal<T>(string key, List<T> val) where T: IFastJsonValue{
			writeKeyVal(key, val, 1);
		}
		
		public void writeKeyVal<T>(string key, List<T> val, bool optional) where T: IFastJsonValue{
			if (optional && ((val == null) || (val.Count == 0)))
				return;
			writeKeyVal(key, val, 1);
		}
		
		public void writeOptionalKeyVal<T>(string key, List<T> val) where T: IFastJsonValue{
			if ((val == null) || (val.Count == 0))
				return;
			writeKeyVal(key, val, 1);
		}
		
		public void writeKeyVal<T>(string key, List<T> val, int stride) where T: IFastJsonValue{
			writeKey(key);
			writeRawValue(val, stride);
		}
		
		public void writeRawValue<T>(T[] values, RawValueWriter<T> writer){
			writeRawValue(values, writer, 1);
		}
		
		public void writeRawValue<T>(T[] values, RawValueWriter<T> writer, int stride){
			beginRawArray();
			if (values != null){
				if (stride <= 1){
					foreach(var cur in values){
						writeValue<T>(cur, writer);
					}
				}
				else{
					int index = 0;
					foreach(var cur in values){
						writeValue<T>(cur, writer, index == 0);
						index++;
						if (index >= stride)
							index = 0;
					}
				}
			}
			endArray();						
		}
		
		public void writeRawValue<T>(List<T> values, RawValueWriter<T> writer){
			writeRawValue(values, writer, 1);
		}
		
		public void writeRawValue<T>(List<T> values, RawValueWriter<T> writer, int stride){
			beginRawArray();
			if (values != null){
				if (stride <= 1){
					foreach(var cur in values){
						writeValue<T>(cur, writer);
					}
				}
				else{
					int index = 0;
					foreach(var cur in values){
						writeValue<T>(cur, writer, index == 0);
						index++;
						if (index >= stride)
							index = 0;
					}
				}
			}
			endArray();						
		}
		
		public void writeRawValue<T>(T[] values) where T: IFastJsonValue{
			writeRawValue(values, 1);
		}
		
		public void writeRawValue<T>(T[] values, int stride) where T: IFastJsonValue{
			beginRawArray();
			if (values != null){
				if (stride <= 1){
					foreach(var cur in values){
						writeValue(cur);
					}
				}
				else{
					int index = 0;
					foreach(var cur in values){
						writeValue<T>(cur, index == 0);
						index++;
						if (index >= stride)
							index = 0;
					}
				}
			}
			endArray();						
		}
		
		public void writeRawValue<T>(List<T> values) where T: IFastJsonValue{
			writeRawValue(values, 1);
		}
		
		public void writeRawValue<T>(List<T> values, int stride) where T: IFastJsonValue{
			beginRawArray();
			if (values != null){
				if (stride <= 1){
					foreach(var cur in values){
						writeValue(cur);
					}
				}
				else{
					int index = 0;
					foreach(var cur in values){
						writeValue<T>(cur, index == 0);
						index++;
						if (index >= stride)
							index = 0;
					}
				}
			}
			endArray();						
		}
	}
}