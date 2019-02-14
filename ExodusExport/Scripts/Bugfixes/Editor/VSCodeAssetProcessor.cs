using UnityEngine;
using UnityEditor;
using System.IO;
using System.Text.RegularExpressions;

namespace SceneExport{
	/*
	This file exists to handle the situation where unity generates a ProjectFile targeting version 4.5, and VSCode fails to process it,
	because 4.5 isn't a thing, while 4.5.2 is. Apparently this is fixed in 2019.1

	*sigh*

	This likely needs a check to see if unity uses relevant version.
	*/
	public class VSCodeProjectFixer: AssetPostprocessor{
		private static void OnGeneratedCSProjectFiles(){
			#if UNITY_2019
				return;
			#endif
			Debug.LogFormat("Upgrading CSProjects");

			var curDir = Directory.GetCurrentDirectory();
			var files = Directory.GetFiles(curDir, "*.csproj");
			
			foreach(var curFile in files){
				if (fixProjectFile(curFile)){
					Debug.LogFormat("*.csproj file modified: {0}", curFile);
				}
			}
		}

		static bool fixProjectFile(string filePath){
			var fileData = File.ReadAllText(filePath);

			var pattern = "<TargetFrameworkVersion>v4.5</TargetFrameworkVersion>";
			var replacePattern = "<TargetFrameworkVersion>v4.5.2</TargetFrameworkVersion>";

			if (fileData.IndexOf(pattern) != -1){				
				var content = Regex.Replace(fileData, pattern, replacePattern);
				File.WriteAllText(filePath, content);
				return true;
			}
			return false;
		}
	}
}