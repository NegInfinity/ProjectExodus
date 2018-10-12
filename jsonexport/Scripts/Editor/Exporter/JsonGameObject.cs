using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

namespace SceneExport{
	[System.Serializable]
	public class JsonGameObject: IFastJsonValue{
		public string name;
		public int instanceId = -1;
		public int id = -1;
		public Vector3 localPosition = Vector3.zero;
		public Quaternion localRotation = Quaternion.identity;
		public Vector3 localScale = Vector3.one;
		public Matrix4x4 worldMatrix = Matrix4x4.identity;
		public Matrix4x4 localMatrix = Matrix4x4.identity;
		public List<int> children = new List<int>();
		public int parent = -1;
		public int mesh = -1;
		public JsonRendererData[] renderer = null;
		public JsonLight[] light = null;
		public bool isStatic = true;
		public bool lightMapStatic = true;
		public bool occluderStatic = true;
		public bool occludeeStatic = true;
		public bool navigationStatic = true;
		public bool reflectionProbeStatic = true;
			
		public bool nameClash = false;
		public string uniqueName = "";
			
		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("name", name);
			writer.writeKeyVal("instanceId", instanceId);
			writer.writeKeyVal("id", id);
			writer.writeKeyVal("localPosition", localPosition);
			writer.writeKeyVal("localRotation", localRotation);
			writer.writeKeyVal("localScale", localScale);
			writer.writeKeyVal("worldMatrix", worldMatrix);
			writer.writeKeyVal("localMatrix", localMatrix);
			writer.writeKeyVal("children", children);
			writer.writeKeyVal("parent", parent);
			writer.writeKeyVal("mesh", mesh);
			writer.writeKeyVal("isStatic", isStatic);
			writer.writeKeyVal("lightMapStatic", lightMapStatic);
			writer.writeKeyVal("navigationStatic", navigationStatic);
			writer.writeKeyVal("occluderStatic", occluderStatic);
			writer.writeKeyVal("occludeeStatic", occludeeStatic);
			writer.writeKeyVal("reflectionProbeStatic", reflectionProbeStatic);
					
			writer.writeKeyVal("nameClash", nameClash);
			writer.writeKeyVal("uniqueName", uniqueName);
				
			/*
			writer.beginKeyArray("renderer");
			if (renderer != null){
				foreach(var r in renderer){
					writer.writeValue(r);
					//r.writeRawJsonValue(writer);
				}
			}
			writer.endArray();
			*/
			writer.writeKeyVal("renderer", renderer);
			writer.writeKeyVal("light", light);
			/*
			writer.beginKeyArray("light");
			if (light != null){
				foreach(var l in light){
					writer.writeValue(l);
					//l.writeRawJsonValue(writer);
				}
			}
			writer.endArray();
			*/
			writer.endObject();
		}
			
		public JsonGameObject(GameObject obj, ResourceMapper resMap){
			name = obj.name;
			instanceId = obj.GetInstanceID();
			id = resMap.getObjectId(obj);
			localPosition = obj.transform.localPosition;
			localRotation = obj.transform.localRotation;
			localScale = obj.transform.localScale;
			worldMatrix = obj.transform.localToWorldMatrix;
			localMatrix = worldMatrix;

			isStatic = obj.isStatic;
			var flags = GameObjectUtility.GetStaticEditorFlags(obj);
			lightMapStatic = (flags & StaticEditorFlags.LightmapStatic) != 0;
			occluderStatic = (flags & StaticEditorFlags.OccluderStatic) != 0;
			occludeeStatic = (flags & StaticEditorFlags.OccludeeStatic) != 0;
			navigationStatic = (flags & StaticEditorFlags.NavigationStatic) != 0;
			reflectionProbeStatic = (flags & StaticEditorFlags.ReflectionProbeStatic) != 0;

			if (obj.transform.parent){
				localMatrix = obj.transform.parent.worldToLocalMatrix * localMatrix;
			}
			renderer = JsonRendererData.makeRendererArray(obj.GetComponent<Renderer>(), resMap);
			//light = exp.makeLight(obj.GetComponent<Light>());
			light = JsonLight.makeLightArray(obj.GetComponent<Light>());

			mesh = resMap.getMeshId(obj);

			foreach(Transform curChild in obj.transform){
				children.Add(resMap.getObjectId(curChild.gameObject));
			}
			if (obj.transform.parent)
				parent = resMap.findObjectId(obj.transform.parent.gameObject);
		}
	}
}
