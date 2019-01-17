using UnityEngine;
using UnityEditor;

namespace SceneExport{
	[System.Serializable]
	public class GuiEnabledGuard: System.IDisposable{
		[SerializeField]bool oldValue = false;
		public void Dispose(){
			GUI.enabled = oldValue;
		}
		
		public GuiEnabledGuard(bool newVal){
			oldValue = GUI.enabled;
			GUI.enabled = newVal;
		}
	}
	
	public static class ExportGuiUtility{
		public static GuiEnabledGuard scopedGuiEnabled(bool newVal){
			return new GuiEnabledGuard(newVal);
		}
	}	
}
