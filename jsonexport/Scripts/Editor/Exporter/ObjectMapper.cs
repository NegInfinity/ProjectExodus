using UnityEngine;
using System.Collections.Generic;

namespace SceneExport{
	public class ObjectMapper<Resource>{
		public delegate void OnNewObjectAdded(Resource res);
		public int nextId = 0;
		public Dictionary<Resource, int> objectMap = new Dictionary<Resource, int>();
		public List<Resource> objectList = new List<Resource>();
			
		public int findId(Resource obj){
			int result = -1;
			if (Object.Equals(obj, null))
				return result;
			if (objectMap.TryGetValue(obj, out result))
				return result;
			return -1;
		}

		public int getId(Resource obj, OnNewObjectAdded onAddCallback = null){
			int result = -1;
			if (Object.Equals(obj, null))
				return result;
			if (objectMap.TryGetValue(obj, out result))
				return result;
			result = nextId;
			objectMap[obj] = result;
			objectList.Add(obj);
			nextId++;
			if (onAddCallback != null)
				onAddCallback(obj);
			return result;						
		}
	}
}
