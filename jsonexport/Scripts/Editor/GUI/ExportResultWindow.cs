using UnityEditor;
using UnityEngine;
using UnityEditor.SceneManagement;
using System.Collections.Generic;
using System.Collections;
using System.Linq;

namespace SceneExport{
	[System.Serializable]
	public class ExportResultWindow: EditorWindow{
		[SerializeField] Logger logger = new Logger();
		[SerializeField] public TextAsset instructionsTextAsset;
		
		//FIXME well specifying this one in inspector doesn't work. Need a scriptable asset or something.
		[SerializeField] public static readonly string instructionsUrl = "http://www.example.com/";
		
		public static void openWindow(Logger logs = null, bool utility = true, bool focus = true){
			var wnd = EditorWindow.GetWindow(typeof(ExportResultWindow), utility, "Export Complete", focus) as ExportResultWindow;
			if (logs == null)
				return;
			if (!wnd)
				return;
			wnd.setMessages(logs);
		}
		
		//[System.Serializable] List<string> logs = new List<string>();
		public void clearMessages(){
			logger.clear();
		}
		
		public void setMessages(Logger messages){
			if (messages == null)
				throw new System.ArgumentNullException("messages");
			if (logger.Equals(messages))
				return;
			logger = new Logger(messages);
			Repaint();
		}
		
		[SerializeField] Vector2 messageScrollPos;
		[SerializeField] bool messagesOpen = true;		
		[SerializeField] bool messageFiltersOpen = false;		
		[SerializeField] bool instructionsOpen = true;		
		[SerializeField] bool showLogs = true;
		[SerializeField] bool showWarnings = true;
		[SerializeField] bool showErrors = true;
		
		ExportGuiStyles guiStyles = new ExportGuiStyles();

		void OnEnable(){
			minSize = new Vector2(640.0f, 400.0f);
		}
		
		void OnDisable(){
		}
		
		public static class GuiTexts{
			public static readonly string defaultInstructions = @"This is a placeholder instruction set. Replace or specify a text asset file via inspector";
		}

		string getInstructionsText(){
			if (instructionsTextAsset)
				return instructionsTextAsset.text;
			return GuiTexts.defaultInstructions;
		}

		void initGuiStyles(){
			if (guiStyles == null)
				guiStyles = new ExportGuiStyles();
			guiStyles.init();
		}
		
		bool canShowMessage(Logger.MessageType type_){
			return (showLogs && (type_ == Logger.MessageType.Log)) ||
				(showWarnings && (type_ == Logger.MessageType.Warning)) ||
				(showErrors && (type_ == Logger.MessageType.Error));
		}
				
		string makeDisplayString(Logger.Message msg){
			if (msg.msgType == Logger.MessageType.Error)
				return "<color=red>Error:</color> " + msg.message;
				
			if (msg.msgType == Logger.MessageType.Warning)
				return "<color=yellow>Warning:</color> " + msg.message;
				
			return msg.message;			
		}
		
		void processMessageArea(){			
			messagesOpen = EditorGUILayout.Foldout(messagesOpen, "Export log messages:");
			if (!messagesOpen)
				return;
				
			EditorGUILayout.LabelField("Messages:", EditorStyles.boldLabel);
			
			messageScrollPos = EditorGUILayout.BeginScrollView(messageScrollPos);
			EditorGUILayout.BeginVertical(guiStyles.logPanel);
			
			foreach(var cur in logger.messages){
				if (!canShowMessage(cur.msgType))
					continue;
				EditorGUILayout.SelectableLabel(makeDisplayString(cur), guiStyles.logLabel);
			}
			
			EditorGUILayout.EndVertical();
			EditorGUILayout.EndScrollView();
			
			messageFiltersOpen = EditorGUILayout.Foldout(messageFiltersOpen, "Message filters:");
			if (messageFiltersOpen){
				showLogs= EditorGUILayout.Toggle("Show log messages", showLogs);
				showWarnings= EditorGUILayout.Toggle("Show warnings", showWarnings);
				showErrors= EditorGUILayout.Toggle("Show errors", showErrors);
			}			
		}	
		
		void processFurtherInstructions(){
			instructionsOpen = EditorGUILayout.Foldout(instructionsOpen, "Further information:");
			if (!instructionsOpen)
				return;
				
			using(var tmp = ExportGuiUtility.scopedGuiEnabled(false)){
				EditorGUILayout.TextArea(getInstructionsText(), guiStyles.disclaimerTextArea);
			}
			
			if (!string.IsNullOrEmpty(instructionsUrl)){
				if (GUILayout.Button("See online information")){
					Application.OpenURL(instructionsUrl);
				}
			}
		}
		
		void processCloseButton(){
			GUILayout.BeginHorizontal();
			if (GUILayout.Button("Close"))
				Close();
			GUILayout.EndHorizontal();
		}
		
		void OnGUI(){
			initGuiStyles();
			processFurtherInstructions();
			processMessageArea();
			processCloseButton();
		}
	}
}
