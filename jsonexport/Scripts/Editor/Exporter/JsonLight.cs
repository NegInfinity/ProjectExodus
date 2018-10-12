using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

namespace SceneExport{
	[System.Serializable]
	public class JsonLight{
		public Color color = Color.white;
		public float range = 0.0f;
		public float spotAngle = 0.0f;
		public string type;
		public float shadowStrength = 0.0f;
		public float intensity = 0.0f;
		public string renderMode;
		public string shadows;
		public float bounceIntensity = 0.0f;
			
		public void writeJsonValue(FastJsonWriter writer){
			writer.beginObjectValue();
			writer.writeKeyVal("bounceIntensity", bounceIntensity);
			writer.writeKeyVal("color", color);
			writer.writeKeyVal("intensity", intensity);
			writer.writeKeyVal("range", range);
			writer.writeKeyVal("renderMode", renderMode);
			writer.writeKeyVal("shadows", shadows);
			writer.writeKeyVal("shadowStrength", shadowStrength);
			writer.writeKeyVal("spotAngle", spotAngle);
			writer.writeKeyVal("type", type);
			writer.endObject();
		}
		
		public JsonLight(Light l){
			color  = l.color;
			range = l.range;
			spotAngle = l.spotAngle;
			type = l.type.ToString();
			renderMode = l.renderMode.ToString();
			shadowStrength = l.shadowStrength;
			shadows = l.shadows.ToString();
			intensity = l.intensity;
		}
	};
	
	public partial class Exporter{		
		public JsonLight[] makeLight(Light l){
			if (!l)
				return null;
			var result = new JsonLight(l);
			/*
			result.color  = l.color;
			result.range = l.range;
			result.spotAngle = l.spotAngle;
			result.type = l.type.ToString();
			result.renderMode = l.renderMode.ToString();
			result.shadowStrength = l.shadowStrength;
			result.shadows = l.shadows.ToString();
			result.intensity = l.intensity;
			*/
			return new JsonLight[]{result};
		}
	}
}
