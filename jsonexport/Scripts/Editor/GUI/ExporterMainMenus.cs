using UnityEditor;
using UnityEngine;
using UnityEditor.SceneManagement;
using System.Collections.Generic;
using System.Collections;
using System.Linq;

namespace SceneExport{
	public static class ExporterMainMenus{
		[MenuItem("Migrate to UE 4/Begin Export", false, 1)]
		public static void showExportConfigWindow(){
			EditorWindow.GetWindow(typeof(ExporterWindow), true, "Project Exodus Exporter");
		}

		[MenuItem("Migrate to UE 4/Disclaimer", false, 3)]
		public static void showDisclaimerWindow(){
			DisclaimerWindow.openDisclaimerWindow();
		}	
		
		[MenuItem("Migrate to UE 4/Show Export Results (debug)", false, 3)]
		public static void showExportResultsWindow(){
			var tmpLog = new Logger();
			for(int i = 0; i < 100; i++){
				tmpLog.logFormat("Test log {0}", Random.Range(0, 256));
				tmpLog.logWarningFormat("Test warning {0}", Random.Range(0, 256));
				tmpLog.logErrorFormat("Test error {0}", Random.Range(0, 256));
			}
			ExportResultWindow.openWindow(tmpLog);
		}	
	}
}
