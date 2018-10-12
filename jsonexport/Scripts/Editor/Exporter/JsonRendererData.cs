using UnityEngine;
using System.Collections.Generic;

namespace SceneExport{
	[System.Serializable]
	public class JsonRendererData{
		//public bool hasRenderer = false;
		public bool receiveShadows = false;
		public string shadowCastingMode;
		public int lightmapIndex = -1;
		public Vector4 lightmapScaleOffset = new Vector4(1.0f, 1.0f, 0.0f, 0.0f);
		public List<int> materials = new List<int>();
			
		public void writeJsonValue(FastJsonWriter writer){
			writer.beginObjectValue();
			writer.writeKeyVal("lightmapIndex", lightmapIndex);
			writer.writeKeyVal("shadowCastingMode", shadowCastingMode);
			writer.writeKeyVal("lightmapScaleOffset", lightmapScaleOffset);
			writer.writeKeyVal("materials", materials);
			writer.writeKeyVal("receiveShadows", receiveShadows);
			writer.endObject();
		}
		
		public JsonRendererData(Renderer r, Exporter exp){
			receiveShadows = r.receiveShadows;
			shadowCastingMode = r.shadowCastingMode.ToString();
			lightmapIndex = r.lightmapIndex;
			lightmapScaleOffset = r.lightmapScaleOffset;
			foreach(var cur in r.sharedMaterials){
				materials.Add(exp.getMaterialId(cur));
			}
		}
	};
	
	public partial class Exporter{		
		public JsonRendererData[] makeRenderer(Renderer r){
			if (!r)
				return null;
			var result = new JsonRendererData(r, this);
			return new JsonRendererData[]{result};
		}
	}
}
