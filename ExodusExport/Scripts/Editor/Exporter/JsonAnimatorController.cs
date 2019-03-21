using UnityEngine;
using UnityEditor;
using UnityEditor.Animations;
using System.Collections.Generic;
using System.Linq;

namespace SceneExport{
	[System.Serializable]
	public class JsonAnimatorControllerParameter: IFastJsonValue{
		public AnimatorControllerParameter animParam;
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("name", animParam.name);
			writer.writeKeyVal("animType", animParam.type.ToString());
			writer.writeKeyVal("defaultBool", animParam.defaultBool);
			writer.writeKeyVal("defaultInt", animParam.defaultInt);
			writer.writeKeyVal("defaultFloat", animParam.defaultFloat);
			writer.endObject();
		}
		
		public JsonAnimatorControllerParameter(AnimatorControllerParameter animParam_){
			animParam = animParam_;
			if (animParam == null)
				throw new System.ArgumentNullException("animParam_");
		}
	}

	[System.Serializable]
	public class JsonAnimatorController: IFastJsonValue{
		public string name;
		public string path;
		//public int id = -1;
		public ResId id = ResId.invalid;
		public UnityEditor.Animations.AnimatorController controller;
		
		public List<JsonAnimatorControllerParameter> parameters = new List<JsonAnimatorControllerParameter>();
		public List<ResId> animationIds = new List<ResId>();
		
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("name", name);
			writer.writeKeyVal("path", path);
			writer.writeKeyVal("id", id);
			
			writer.writeKeyVal("parameters", parameters);
			//writer.writeKeyVal("animations", animations);
			writer.writeKeyVal("animationIds", animationIds);
			writer.endObject();
		}
		
		public JsonAnimatorController(){
		}
		
		public JsonAnimatorController(UnityEditor.Animations.AnimatorController controller_, Animator animator, ResId id_, ResourceMapper resMap){
			controller = controller_;
			if (!controller)
				throw new System.ArgumentNullException("controller_");
			if (!animator)
				throw new System.ArgumentNullException("animator");
			id = id_;
			name = controller.name;
			path = AssetDatabase.GetAssetPath(controller);
			
			parameters = controller.parameters.Select((arg) => new JsonAnimatorControllerParameter(arg)).ToList();
			animationIds = controller.animationClips.Select((arg) => resMap.getAnimationClipId(arg, animator)).ToList();
			//animations = controller.animationClips.Select((arg, idx) => new JsonAnimationClip(arg, idx)).ToList();
		}
	}	
}
