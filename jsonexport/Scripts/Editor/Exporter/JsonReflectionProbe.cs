using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

namespace SceneExport{
	[System.Serializable]
	public class JsonReflectionProbe: IFastJsonValue{
		public Color backgroundColor;
		public float blendDistance;
		public bool boxProjection = false;
		public Vector3 center = Vector3.zero;
		public Vector3 size = Vector3.one;
		public string clearType = "skybox";
		public int cullingMask = 0;
		public bool hdr = false;
		public float intensity = 1.0f;
		public float farClipPlane;
		public float nearClipPlane;
		public int resolution = 512;
		public string mode = "baked";
		public string refreshMode = "awake";
		//public string timeSlicingMode;
		
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("backgroundColor", backgroundColor);
			writer.writeKeyVal("blendDistance", blendDistance);
			writer.writeKeyVal("boxProjection", boxProjection);
			writer.writeKeyVal("center", center);
			writer.writeKeyVal("size", size);
			writer.writeKeyVal("clearType", clearType);
			writer.writeKeyVal("cullingMask", cullingMask);
			writer.writeKeyVal("hdr", hdr);
			writer.writeKeyVal("intensity", intensity);
			writer.writeKeyVal("nearClipPlane", nearClipPlane);
			writer.writeKeyVal("farClipPlane", farClipPlane);
			writer.writeKeyVal("resolution", resolution);
			writer.writeKeyVal("mode", mode);
			writer.writeKeyVal("refreshMode", refreshMode);
			//writer.writeKeyVal("timeSlicingMode", timeSlicingMode);
			writer.endObject();			
		}
		
		static string getModeString(UnityEngine.Rendering.ReflectionProbeMode mode){
			if (mode == UnityEngine.Rendering.ReflectionProbeMode.Baked)
				return "baked";
			if (mode == UnityEngine.Rendering.ReflectionProbeMode.Realtime)
				return "realtime";
			return "custom";
		}
		
		static string getRefreshModeString(UnityEngine.Rendering.ReflectionProbeRefreshMode mode){
			if (mode == UnityEngine.Rendering.ReflectionProbeRefreshMode.OnAwake)
				return "awake";
			if (mode == UnityEngine.Rendering.ReflectionProbeRefreshMode.EveryFrame)
				return "frame";
			return "scripting";
		}
		/*
			Uh, do we even need temporary object with all those fields? 
			Could just grab reflection probes themselves in this case...
		*/
		public JsonReflectionProbe(ReflectionProbe obj){
			backgroundColor = obj.backgroundColor;
			blendDistance = obj.blendDistance;
			boxProjection = obj.boxProjection;
			center = obj.center;
			size = obj.size;
			/*clearType = (obj.clearFlags == UnityEngine.Rendering.ReflectionProbeClearFlags.Skybox) ? 
				"skybox": "color";*/
			clearType = obj.clearFlags.ToString();
			cullingMask = obj.cullingMask;
			hdr = obj.hdr;
			intensity = obj.intensity;
			nearClipPlane = obj.nearClipPlane;
			farClipPlane = obj.farClipPlane;
			resolution = obj.resolution;
			mode = obj.mode.ToString();//getModeString(obj.mode);
			refreshMode = obj.refreshMode.ToString();//getModeString(obj.mode);
		}
	}
}