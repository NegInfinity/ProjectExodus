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

		public struct IndexedObjectData{
			public int index;
			public ResId id;
			public Resource data;
			public IndexedObjectData(int index_, ResId id_, Resource data_){
				index = index_;
				id = id_;
				data = data_;
			}
		}

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

		public int baseIndex{
			get{
				return lastNumObjects;
			}
		}

		Resource getObject(int index){
			return indexedResourceGetter(owner, index);
		}

		public IEnumerable<IndexedObjectData> getNewObjectsData(){
			foreach(var index in getNewIndexes()){
				var obj = getObject(index);
				var id = idFromIndex(index);
				yield return new IndexedObjectData(index, id, obj);
			}
		}

		ResId idFromIndex(int index){
			return ResId.fromObjectIndex(index);
		}

		public IEnumerable<ResId> getNewIds(){
			foreach(var index in getNewIndexes())
				yield return ResId.fromObjectIndex(index);
		}

		public IEnumerable<Resource> getNewObjects(){
			foreach(var index in getNewIndexes())
				yield return getObject(index);
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

			owner = storage_;
			countGetter = countGetter_;
			indexedResourceGetter = indexedResourceGetter_;
			lastNumObjects = 0;
		}
	}

	public static class ResourceWatcherExtensions{
		public static ResourceStorageWatcher<List<Resource>, Resource> createWatcher<Resource>(this List<Resource> storage){
			var result = new ResourceStorageWatcher<List<Resource>, Resource>(
				storage,
				(store) => store.Count,
				(store, index) => store[index]
			);
			return result;
		}
	}
}