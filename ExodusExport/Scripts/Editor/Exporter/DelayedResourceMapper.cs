using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

namespace SceneExport{
	public class DelayedResourceMapper<Resource> where Resource: class{
		public List<Resource> registered = new List<Resource>();//all objects that were passed in for registration
		public List<Resource> processed = new List<Resource>();//all objects that have been finalized, meaning their instances were created
		public Dictionary<Resource, ResId> resourceMap = new Dictionary<Resource, ResId>();

		public void clear(){
			registered.Clear();
			processed.Clear();
			resourceMap.Clear();
		}

		public int numProcessedItems{
			get{
				return processed.Count;
			}
		}		

		public int numRegisteredItems{
			get{
				return registered.Count;
			}
		}

		public delegate void ResourceProcessCallback(Resource res, ResId resId);

		public bool processingFinished{
			get{
				return !hasUnprocessedItems;
			}
		}

		public bool hasUnprocessedItems{
			get{
				return numProcessedItems < numRegisteredItems;
			}
		}

		public bool isValidId(ResId id){
			return (id.rawId >= 0) && (id.rawId < registered.Count);
		}

		public Resource findResource(ResId id){
			if (isValidId(id))
				return registered[id.rawId];

			return null;
		}

		public bool hasResource(ResId id){
			return isValidId(id);
		}

		public ResId findId(Resource res){
			return getId(res, false, false);
		}

		public ResId getId(Resource res, bool createNew, bool throwIfMissing = false){
			if (res == null){
				if (createNew && (res == null) && throwIfMissing){
					throw new System.ArgumentNullException("resType");
				}
				return ResId.invalid;
			}

			ResId result;
			if (resourceMap.TryGetValue(res, out result))
				return result;

			if (!createNew)
				return ResId.invalid;

			result = new ResId(numRegisteredItems);

			registered.Add(res);
			resourceMap.Add(res, result);

			return result;
		}

		/*
			Returns existing ID or creates a new one.
		*/
		public ResId getId(Resource resType){
			return getId(resType, true, false);
		}

		public bool processRemainingItems(ResourceProcessCallback callback){
			if (callback == null)
				throw new System.ArgumentNullException("callback");
			if (!hasUnprocessedItems)
				return false;

			for(int i = numProcessedItems; i < numRegisteredItems; i++){
				var id = new ResId(i);
				var obj = registered[i];
				try{
					callback(obj, id);
				}
				finally{
					processed.Add(obj);
				}
				//resourceMap.Add(obj, id);, No this should be done earlier
			}

			return true;
		}		
	}
}