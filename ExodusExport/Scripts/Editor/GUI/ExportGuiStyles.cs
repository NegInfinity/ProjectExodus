using UnityEngine;
using UnityEditor;

namespace SceneExport{
	[System.Serializable]
	public class ExportGuiStyles{
		public GUIStyle normalLabel = null;
		public GUIStyle errorLabel = null;
		public GUIStyle warningLabel = null;
		public GUIStyle disclaimerTextArea = null;
		public GUIStyle richTextArea = null;
		public GUIStyle logTextArea = null;
		public GUIStyle logLabel = null;
		public GUIStyle logPanel = null;
		public GUIStyle progressBar = null;
		public void init(){
			if (normalLabel == null){
				normalLabel = new GUIStyle(EditorStyles.label);
				normalLabel.wordWrap = true;
			}
			
			if (errorLabel == null){	
				errorLabel = new GUIStyle(EditorStyles.label);
				errorLabel.wordWrap = true;
				errorLabel.normal.textColor = new Color(1.0f, 0.0f, 0.0f);
				errorLabel.active.textColor = new Color(1.0f, 0.5f, 0.0f);
				errorLabel.focused.textColor = new Color(1.0f, 0.25f, 0.0f);
			}
			
			warningLabel = new GUIStyle(EditorStyles.label);
			if (warningLabel == null){
				warningLabel.normal.textColor = Color.yellow;
				warningLabel.wordWrap = true;
				warningLabel.normal.textColor = new Color(1.0f, 1.0f, 0.0f);
				warningLabel.active.textColor = new Color(1.0f, 1.0f, 0.5f);
				warningLabel.focused.textColor = new Color(1.0f, 1.0f, 0.25f);
			}

			if (disclaimerTextArea == null){
				disclaimerTextArea = new GUIStyle(EditorStyles.textArea);
				disclaimerTextArea.richText = true;
				disclaimerTextArea.wordWrap = true;
				disclaimerTextArea.normal.textColor = Color.black;
				disclaimerTextArea.active.textColor = Color.black;
				disclaimerTextArea.focused.textColor = Color.black;
			}
			
			if (richTextArea == null){
				richTextArea = new GUIStyle(EditorStyles.textArea);
				richTextArea.richText = true;
			}
			
			if (logTextArea == null){
				logTextArea = new GUIStyle(EditorStyles.textArea);
				logTextArea.wordWrap = true;
				logTextArea.richText = true;
			}
			
			if (logLabel == null){
				logLabel = new GUIStyle(EditorStyles.label);
				logLabel.wordWrap = true;
				logLabel.richText = true;
			}
			
			if (logPanel == null){
				logPanel = new GUIStyle(EditorStyles.helpBox);
			}
			
			if (progressBar == null){
				progressBar = new GUIStyle(EditorStyles.label);
				progressBar.richText = true;
			}
		}
	}	
}
