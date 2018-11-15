using UnityEngine;
using System.Collections.Generic;

namespace SceneExport{
	public class ObjectMapper<Resource>{
		public delegate void OnNewObjectAdded(Resource res);
		public int nextId = 0;
		public Dictionary<Resource, int> objectMap = new Dictionary<Resource, int>();
		public List<Resource> objectList = new List<Resource>();

		public bool isValidObjectId(int id){
			return (id >= 0) && (id < objectList.Count);
		}
		
		public int numObjects{
			get{
				return objectList.Count;
			}
		}
		
		public Resource getObject(int id){
			if (!isValidObjectId(id))
				throw new System.ArgumentException(string.Format("Invalid object id {0}", id));
			return objectList[id];
		}
		
		public bool hasObject(Resource obj){
			return ExportUtility.isValidId(getId(obj, false));
		}
			
		public int findId(Resource obj){
			return getId(obj, false, null);
		}
		
		public int getId(Resource obj){
			return getId(obj, true, null);
		}
		
		//public void registerObj(Resource obj
		/*
		public int registerResource(Resource obj, OnNewObjectAdded onAddCallback = null, bool checkRegistration = true){
			if (Object.Equals(obj, null))
				throw new System.ArgumentException("At this point parameter cannot be null", "obj");
			int result = -1;
			if (objectMap.TryGetValue(obj, out result)){
				if (checkRegistration)
					throw new System.InvalidOperationException(string.Format("Multiple object registration for {0}", obj));
				return result;
			}
			result = nextId;
			objectMap[obj] = result;
			objectList.Add(obj);
			nextId++;
			if (onAddCallback != null)
				onAddCallback(obj);
			return result;						
		}
		*/
		
		public int getId(Resource obj, bool createMissing, OnNewObjectAdded onAddCallback = null, bool throwIfMissing = false){
			int result = -1;
			if (Object.Equals(obj, null))
				return result;
			if (objectMap.TryGetValue(obj, out result))
				return result;
			if (!createMissing){
				if (throwIfMissing){
					throw new System.InvalidOperationException(
						string.Format("Could not find id for resource {0}", obj)
					);
				}
				return -1;
			}
			//return registerResource(obj, onAddCallback);
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
