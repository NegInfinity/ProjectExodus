using UnityEngine;
using System.Collections.Generic;

namespace SceneExport{
	public class UniqueAssetNameGenerator{
		HashSet<string> uniquePaths = new HashSet<string>();

		static string removeTrailingSlash(string arg){
			var slash = "/";
			while(arg.EndsWith(slash)){
				arg = arg.Substring(arg.Length - slash.Length);
			}
			return arg;
		}

		/*
		public string generateUniqueName(string folder, string name, string suffix = ""){
			var baseName = name;
			if (!string.IsNullOrEmpty(suffix)){
				baseName += suffix;
			}
			folder = removeTrailingSlash(folder);
			Sanity.check(!folder.EndsWith("/"), "Invalid folder name");

			var fullPath = string.Format("{0}/{1}", folder, baseName);
			return baseName;
		}
		*/
		
		public string createUniqueAssetName(string folderPath, string name, string suffix = ""){
			if (!string.IsNullOrEmpty(suffix)){
				name += suffix;
			}

			folderPath = removeTrailingSlash(folderPath);

			string uniqueName = name;
			var fullPath = string.Format("{0}/{1}", folderPath, uniqueName).ToLower();
			var pathIndex = 0;
			while(uniquePaths.Contains(fullPath)){
				uniqueName = string.Format("{0}_{1}", name, pathIndex.ToString("D2"));
				fullPath = string.Format("{0}/{1}", folderPath, uniqueName).ToLower();
				pathIndex++;
			}

			uniquePaths.Add(fullPath);

			return uniqueName;
		}
	}
}