using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

namespace SceneExport{
	public partial class Exporter{
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
		};
		
		public JsonLight[] makeLight(Light l){
			if (!l)
				return null;
			var result = new JsonLight();
			result.color  = l.color;
			result.range = l.range;
			result.spotAngle = l.spotAngle;
			result.type = l.type.ToString();
			result.renderMode = l.renderMode.ToString();
			result.shadowStrength = l.shadowStrength;
			result.shadows = l.shadows.ToString();
			result.intensity = l.intensity;
			return new JsonLight[]{result};
		}
	}		
}
