using UnityEngine;
using UnityEditor;
using System.Collections.Generic;
using System.Linq;

namespace SceneExport{
	[System.Serializable]
	public class JsonEditorCurveBinding: IFastJsonValue{
		public EditorCurveBinding binding;
		public List<JsonAnimationCurve> curves = new List<JsonAnimationCurve>();
		
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("propertyName", binding.propertyName);
			writer.writeKeyVal("isDiscreteCurve", binding.isDiscreteCurve);
			writer.writeKeyVal("isPPtrCurve", binding.isPPtrCurve);
			writer.writeKeyVal("path", binding.path);
			writer.writeKeyVal("curves", curves);
			writer.endObject();
		}
		
		public JsonEditorCurveBinding(EditorCurveBinding editorCurveBinding_, AnimationClip clip){
			if (!clip)
				throw new System.ArgumentNullException("clip");		
			binding = editorCurveBinding_;
			var edCurve = AnimationUtility.GetEditorCurve(clip, binding);
			curves.Clear();
			if (edCurve != null){
				curves.Add(new JsonAnimationCurve(edCurve));
			}
		}
	};

}
