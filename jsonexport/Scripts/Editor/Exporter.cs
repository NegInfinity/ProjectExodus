using UnityEngine;
using UnityEditor;
using System.IO;
using System.Text;
using System.Collections;
using System.Collections.Generic;
using UnityEngine.SceneManagement;

namespace SceneExport{
	public class Exporter{
		void createResourceMapper(ref ResourceMapper resMap){
			if (resMap == null)
				resMap = new ResourceMapper();
		}
		
		public JsonScene exportScene(Scene scene, ResourceMapper resMap){
			var rootObjects = scene.GetRootGameObjects();
			return exportObjects(rootObjects, resMap);
		}
		
		public JsonScene exportObjects(GameObject[] args, ResourceMapper resMap){
			createResourceMapper(ref resMap);
			
			var result = new JsonScene();
			foreach(var cur in args){
				if (!cur)
					continue;
				resMap.getObjectId(cur);
			}
			
			for(int i = 0; i < resMap.objects.objectList.Count; i++){
				/*TODO: This is very awkward, as constructor adds more data to the exporter
				Should be split into two methods.*/
				result.objects.Add(new JsonGameObject(resMap.objects.objectList[i], resMap));
			}
			
			result.fixNameClashes();

			foreach(var cur in resMap.meshes.objectList){
				result.meshes.Add(new JsonMesh(cur, resMap));
			}

			foreach(var cur in resMap.materials.objectList){
				result.materials.Add(new JsonMaterial(cur, resMap));
			}

			foreach(var cur in resMap.textures.objectList){
				result.textures.Add(new JsonTexture(cur, resMap));
			}

			result.resources = new List<string>(resMap.resources);
			result.resources.Sort();

			return result;
		}

		public JsonScene exportOneObject(GameObject obj, ResourceMapper resCollector){
			return exportObjects(new GameObject[]{obj}, resCollector);
		}
	}
}