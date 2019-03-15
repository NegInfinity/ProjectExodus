using UnityEngine;
using UnityEditor;
using System.Collections.Generic;
using System.Linq;

namespace SceneExport{
	/*
	This thing exists to monitor object mappers and OTHER structures of this kind....
	It is necessary due to resource graph being possibly looping around.
	*/
	public class ResourceStorageWatcher<Storage, Resource>{
		Storage owner = default(Storage);
		int lastNumObjects = 0;
		System.Func<Storage, int> countGetter = null;
		System.Func<Storage, int, Resource> indexedResourceGetter = null;

		void sanityCheck(){
			if (owner == null)
				throw new System.ArgumentNullException("owner", "owner cannot be null");
		}

		public void reset(){
			lastNumObjects = 0;
		}

		public bool hasNewObjects{
			get{
				return numObjects != lastNumObjects;
			}
		}

		public int numObjects{
			get{
				return countGetter(owner);
			}
		}

		public IEnumerable<int> getNewIndexes(){
			if (!hasNewObjects)
				yield break;
			for(int i = lastNumObjects; i < numObjects; i++){
				yield return i;
			}
		}

		public IEnumerable<ResId> getNewIds(){
			foreach(var cur in getNewIndexes())
				yield return ResId.fromObjectIndex(cur);
		}

		public IEnumerable<Resource> getNewObjects(){
			foreach(var cur in getNewIndexes())
				yield return indexedResourceGetter(owner, cur);
		}

		public void updateNumObjects(){
			lastNumObjects = numObjects;
		}

		public ResourceStorageWatcher(Storage storage_, 
		System.Func<Storage, int> countGetter_, 
		System.Func<Storage, int, Resource> indexedResourceGetter_){
			if (storage_ == null)
				throw new System.ArgumentNullException("storage_");
			if (countGetter_ == null)
				throw new System.ArgumentNullException("countGetter_");
			if (indexedResourceGetter_ == null)
				throw new System.ArgumentNullException("indexedResourceGetter_");
			lastNumObjects = 0;
		}
	}
}