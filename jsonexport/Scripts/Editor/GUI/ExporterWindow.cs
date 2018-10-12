using UnityEditor;
using UnityEngine;
using UnityEditor.SceneManagement;
using System.Collections.Generic;
using System.Collections;
using System.Linq;

namespace SceneExport{

[System.Serializable]
public class ExporterWindow: EditorWindow{
	//[MenuItem("Window/Project Exodus/Exporter Window")]
	[MenuItem("Window/Migrate to Unreal 4/Migrate")]
	public static void showWindow(){
		EditorWindow.GetWindow(typeof(ExporterWindow), true, "Project Exodus Exporter");
	}
	
	[SerializeField]bool disclaimerVisible = false;
	[SerializeField]Vector2 windowScrollPos;
	[SerializeField]Vector2 disclaimerScrollPos;
	[SerializeField]Vector2 messagesScrollPos;
	[SerializeField]string targetPath = "";
	//[SerializeField]bool targetPathInProject = false;
	
	
	
	/*
	public enum MigrationPhase{
		Start
	}
	*/
	
	public enum ExportType{
		CurrentObject,
		SelectedObjects,
		CurrentScene,
		WholeProject
	}
	
	[System.Serializable]
	public class GuiStyles{
		public GUIStyle normalLabel = null;
		public GUIStyle errorLabel = null;
		public GUIStyle warningLabel = null;
		public GUIStyle disclaimerTextArea = null;
		public GUIStyle richTextArea = null;
		public GUIStyle logTextArea = null;
		public GUIStyle logLabel = null;
		public GUIStyle logPanel = null;
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
		}
	}
	
	void onEditorUpdate(){
	}
	
	void OnEnable(){
		minSize = new Vector2(320.0f, 240.0f);
		
		EditorApplication.update += onEditorUpdate;
	}
	
	void OnDisable(){
		EditorApplication.update -= onEditorUpdate;
	}
	
	/*[SerializeField]*/GuiStyles guiStyles = new GuiStyles();
	[SerializeField]ExportType exportType = ExportType.CurrentObject;

	string getDefaultJsonFileName(){
		switch(exportType){
			case(ExportType.CurrentObject):{
				var obj = Selection.activeGameObject;
				if (obj)
					return obj.name;
				break;
			}
			case(ExportType.SelectedObjects):{
				foreach(var cur in Selection.transforms){
					if (cur == null)
						continue;
					var curObj = cur.gameObject;
					if (!curObj)
						continue;
					return curObj.name;
				}
				break;
			}
			case(ExportType.CurrentScene):{
				var scene = EditorSceneManager.GetActiveScene();
				if (scene.IsValid())
					return scene.name;
				break;
			}
			case(ExportType.WholeProject):{
				return Application.productName;
			}
		}
		return Application.productName;
	}	
	
	[SerializeField] List<string> messages = new List<string>();
	void logMsg(string message){
		messages.Add(message);
	}
	
	static readonly string warningEntryPrefix = "<color=yellow>Warning: </color>";
	static readonly string errorEntryPrefix = "<color=red>Error: </color>";
	static readonly string infoEntryPrefix = "<color=blue>Info: </color>";
	//static readonly string logEntryPrefix = "<color=yellow>Log: </color>";
	
	void logMsgFormat(string message, params object[] args){
		var s = string.Format(message, args);
		messages.Add(s);
	}
	
	void logInfo(string message){
		messages.Add(infoEntryPrefix + message);
	}
	void logInfoFormat(string message, params object[] args){
		var s = string.Format(infoEntryPrefix + message, args);
		messages.Add(s);
	}
	
	void logWarning(string message){
		messages.Add(warningEntryPrefix + message);
	}	
	void logWarningFormat(string message, params object[] args){
		var s = string.Format(warningEntryPrefix + message, args);
		messages.Add(s);
	}
	
	void logError(string message){
		messages.Add(errorEntryPrefix + message);
	}	
	void logErrorFormat(string message, params object[] args){
		var s = string.Format(errorEntryPrefix + message, args);
		messages.Add(s);
	}
	
	void clearMessages(){
		messages.Clear();
	}
	
	bool isValidExportState(){
		string tmp;
		return isValidExportState(out tmp);
	}
	
	bool isValidExportState(out string msg){
		msg = "";
		switch(exportType){
			case(ExportType.CurrentObject):{
				if (!Selection.activeGameObject){
					msg = "No active object! Select an object for this export type";
					return false;
				}
				return true;
			}
			case(ExportType.SelectedObjects):{
				if ((Selection.gameObjects == null) || (Selection.gameObjects.Length == 0)){
					msg = "No objects selected. Selected objects for this export type.";
					return false;
				}
				return true;
			}
		}
		return true;
	}

	[SerializeField] public TextAsset disclaimerTextAsset;//this needs to be set in inspector
	public static class GuiTexts{
		public static readonly string disclaimer = @"This is a placeholder disclaimer. Replace or specify a text asset file";
	}
	
	public static class GuiContents{
		public static readonly GUIContent disclaimerSection = 
			new GUIContent(
				"Disclaimer", "Click to open the section and read the disclaimer for the plugin"
			);
			
		public static readonly GUIContent exportType =			
			new GUIContent("Export Type", 
@"Select export type here: 
	* single object (currently selected object will be exported), 
	* selected object (selected objects will be exported),
	* current scene (currently selected scene will be exported)
	* whole project (entire project will be processed)"
			);
	}
	
	string getDisclaimerText(){
		if (disclaimerTextAsset)
			return disclaimerTextAsset.text;
		return GuiTexts.disclaimer;
	}
	
	void initGuiStyles(){
		//minSize = new Vector2(320.0f, 240.0f);
		if (guiStyles == null){
			guiStyles = new GuiStyles();
		}
		guiStyles.init();
	}
	
	[System.Serializable]
	class GuiEnabledGuard: System.IDisposable{
		[SerializeField]bool oldValue = false;
		public void Dispose(){
			GUI.enabled = oldValue;
		}
		
		public GuiEnabledGuard(bool newVal){
			oldValue = GUI.enabled;
			GUI.enabled = newVal;
		}
	}
	
	GuiEnabledGuard scopedGuiEnabled(bool newVal){
		return new GuiEnabledGuard(newVal);
	}
	
	void processDisclaimerArea(){
		disclaimerVisible = 
		EditorGUILayout.Foldout(disclaimerVisible, GuiContents.disclaimerSection);
		if (!disclaimerVisible)
			return;
		using(var tmp = scopedGuiEnabled(false)){
			EditorGUILayout.TextArea(getDisclaimerText(), guiStyles.disclaimerTextArea);
			if (GUILayout.Button("View full Disclaimer")){
				DisclaimerWindow.openDisclaimerWindow();
			}
		}
	}
	
	void processTargetPathLabel(){
		if (string.IsNullOrEmpty(targetPath)){
			GUILayout.Label("Target path not selected. Press \"select target\" button to select desintation file", guiStyles.errorLabel);
		}
		else{
			GUILayout.Label(string.Format("Target path: \"{0}\"", targetPath), guiStyles.normalLabel);
			if (Utility.isInProjectPath(targetPath)){
				GUILayout.Label("Target path is within project folder. *.tif textures will not be automatically converted to png.", guiStyles.warningLabel);
			}
			else{
				GUILayout.Label("Target path is outside of project folder. Textures will be copied and converted to png when needed. " +
					"Please note that previous data will be silently overwritten.", guiStyles.normalLabel);
			}
		}
	}
	
	void processSelectionWarningControls(){
		string msg;
		var result = isValidExportState(out msg);
		if (!result){
			EditorGUILayout.LabelField(msg, guiStyles.errorLabel);
		}
	}
	
	void processMessageArea(){
		if (messages.Count <= 0)
			return;
			
		EditorGUILayout.LabelField("Messages:", EditorStyles.boldLabel);
		messagesScrollPos = EditorGUILayout.BeginScrollView(messagesScrollPos);
			
		EditorGUILayout.BeginVertical(guiStyles.logPanel);
		foreach(var cur in messages){
			EditorGUILayout.SelectableLabel(cur, guiStyles.logLabel);
		}
		EditorGUILayout.EndVertical();
		EditorGUILayout.EndScrollView();
	}
	
	void processExportButtons(){
		GUILayout.BeginHorizontal();
		if (GUILayout.Button("Select Target Path")){
			clearMessages();
			var defaultName = getDefaultJsonFileName();//"name";
			var filePath = EditorUtility.SaveFilePanel("Export selected objects", "", defaultName, "json");
			if (!string.IsNullOrEmpty(filePath)){
				targetPath = filePath;
				var pathInProject = Utility.isInProjectPath(filePath);
				if (pathInProject)
					logWarningFormat("File \'{0}\' is in project directory.\nTexture conversion willl not be performed", filePath);
			}
		}

		using(var tmp = scopedGuiEnabled(canExport())){
			if (GUILayout.Button("Begin export")){
				processExport();
			}
		}
		
		GUILayout.EndHorizontal();
	}
	
	void processCloseButton(){
		GUILayout.BeginHorizontal();
		if (GUILayout.Button("Close")){
			Close();
		}
		GUILayout.EndHorizontal();
	}
	
	void processExportProgress(){
	}
	
	IEnumerator waitCoroutine(float seconds){
		var launchTime = Time.realtimeSinceStartup;//Time.time;
		while(true){
			var curTime = Time.realtimeSinceStartup;
			//Debug.LogFormat("Cur time: {0}", curTime);
			var diff = curTime - launchTime;
			if (diff >= seconds)
				break;
			yield return null;
		}
	}
	
	IEnumerator testCoroutine(){
		for(int i = 0; i < 10; i++){
			Debug.LogFormat("Value from coroutine: {0}", i);
			Debug.LogFormat("Waiting");
			yield return EditorCoroutine.waitSeconds(1.0f);//waitCoroutine(1.0f);
			Debug.LogFormat("Waiting complete");
			yield return null;
		}
	}
	
	void OnGUI(){
		initGuiStyles();
		
		windowScrollPos = EditorGUILayout.BeginScrollView(windowScrollPos);
		
		GUILayout.BeginVertical();//GUILayout.MinWidth(640), GUILayout.MinHeight(400));
		GUILayout.Label("Export Settings", EditorStyles.boldLabel);
		
		processDisclaimerArea();
		
		exportType = (ExportType)EditorGUILayout.EnumPopup(GuiContents.exportType, exportType);

		processSelectionWarningControls();		
		processTargetPathLabel();
		processMessageArea();
		
		processExportButtons();
		processExportProgress();
		processCloseButton();	
		
		if (GUILayout.Button("Launch coroutine")){
			EditorCoroutine.startCoroutine(testCoroutine());
		}
		
		GUILayout.EndVertical();
		EditorGUILayout.EndScrollView();
	}
	
	bool canExport(){
		return isValidExportState() && !string.IsNullOrEmpty(targetPath);
	}
	
	void processExport(){		
	}
}

}//namespace SceneExport