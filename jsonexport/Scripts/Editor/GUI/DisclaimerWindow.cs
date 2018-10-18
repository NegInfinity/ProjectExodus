using UnityEngine;
using UnityEditor;

namespace SceneExport{
	public class DisclaimerWindow: EditorWindow{
		[TextArea]
		public string defaultDisclaimer = "No disclaimer has been specified. Please specify text or asset";
		public TextAsset disclaimerAsset;
		[SerializeField] Vector2 scrollPos;
		//[MenuItem("Window/Project Exodus/Disclaimer")]
		
		public static void openDisclaimerWindow(bool utility = true, bool focus = true){
			EditorWindow.GetWindow(typeof(DisclaimerWindow), utility, "Disclaimer", focus);
		}
		
		string getDisclaimerText(){
			if (disclaimerAsset)
				return disclaimerAsset.text;
			return defaultDisclaimer;
		}
		
		void OnEnable(){
			minSize = new Vector2(640.0f, 480.0f);
		}
		
		GUIStyle panelStyle = null;
		GUIStyle labelStyle = null;
		GUIStyle textAreaStyle = null;
		
		void createStyles(){
			if (panelStyle == null){
				panelStyle = new GUIStyle(EditorStyles.helpBox);
			}
			if (labelStyle == null){
				labelStyle = new GUIStyle(EditorStyles.label);
				labelStyle.wordWrap = true;
				labelStyle.richText = true;
			}
			if (textAreaStyle == null){
				textAreaStyle = new GUIStyle(EditorStyles.textArea);
				textAreaStyle.wordWrap = true;
				textAreaStyle.richText = true;
			}
		}
		
		public void OnGUI(){
			createStyles();
			GUILayout.BeginVertical();
			GUILayout.Label("Disclaimer", EditorStyles.boldLabel);
			
			scrollPos = EditorGUILayout.BeginScrollView(scrollPos);
			GUI.enabled = false;
			EditorGUILayout.TextArea(getDisclaimerText(), textAreaStyle, 
				GUILayout.ExpandHeight(true)
			);
			GUI.enabled = true;
			EditorGUILayout.EndScrollView();
			
			if (GUILayout.Button("Close")){
				Close();
			}
			GUILayout.EndVertical();
		}
	}
}