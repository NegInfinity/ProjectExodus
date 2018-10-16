using UnityEditor;
using UnityEngine;
using UnityEditor.SceneManagement;
using System.Collections.Generic;
using System.Collections;
using System.Linq;

namespace SceneExport{

	public enum ExportType{
		CurrentObject,
		SelectedObjects,
		CurrentScene,
		WholeProject
	}
	
[System.Serializable]
public class ExporterWindow: EditorWindow{
	//[MenuItem("Window/Project Exodus/Exporter Window")]
	[SerializeField]bool disclaimerVisible = true;
	[SerializeField]Vector2 windowScrollPos;
	[SerializeField]Vector2 disclaimerScrollPos;
	[SerializeField]Vector2 messagesScrollPos;
	[SerializeField]string targetPath = "";
	
	[SerializeField]AsyncExportTask exportTask = new AsyncExportTask();
	[SerializeField]EditorCoroutine exportCoroutine = null;
	
	void OnEnable(){
		minSize = new Vector2(640.0f, 480.0f);
		if (exportTask != null)
			exportTask.exporterWindow = this;
	}
	
	void OnDisable(){
	}
	
	[SerializeField]ExportGuiStyles guiStyles = new ExportGuiStyles();
	[SerializeField]ExportType exportType = ExportType.CurrentObject;

	static string getDefaultJsonFileName(ExportType exportType){
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
			guiStyles = new ExportGuiStyles();
		}
		guiStyles.init();
	}
	
	void processDisclaimerArea(){
		disclaimerVisible = 
		EditorGUILayout.Foldout(disclaimerVisible, GuiContents.disclaimerSection);
		if (!disclaimerVisible)
			return;
		using(var tmp = ExportGuiUtility.scopedGuiEnabled(false)){
			EditorGUILayout.TextArea(getDisclaimerText(), guiStyles.disclaimerTextArea);
		}
		if (GUILayout.Button("View full Disclaimer")){
			DisclaimerWindow.openDisclaimerWindow();
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
			var defaultName = getDefaultJsonFileName(exportType);//"name";
			var filePath = EditorUtility.SaveFilePanel("Export selected objects", "", defaultName, "json");
			if (!string.IsNullOrEmpty(filePath)){
				targetPath = filePath;
				var pathInProject = Utility.isInProjectPath(filePath);
				if (pathInProject)
					logWarningFormat("File \'{0}\' is in project directory.\nTexture conversion willl not be performed", filePath);
			}
		}

		using(var tmp = ExportGuiUtility.scopedGuiEnabled(canExport() && !hasRunningTask())){
			if (GUILayout.Button("Begin export")){
				processExport();
			}
		}
		
		GUILayout.EndHorizontal();
	}
	
	void processExportProgress(){
		if (!hasRunningTask())
			return;
		GUILayout.BeginHorizontal();
		var rect = GUILayoutUtility.GetRect(new GUIContent(exportTask.currentStatus), guiStyles.richTextArea);
		EditorGUI.ProgressBar(rect, exportTask.getFloatProgress(), exportTask.currentStatus);
		GUILayout.EndHorizontal();
	}
	
	void processCancelButton(){
		if (!hasRunningTask())
			return;
		GUILayout.BeginHorizontal();
		if (GUILayout.Button("Cancel")){
			terminateBackgroundTasks();
		}
		GUILayout.EndHorizontal();
	}
	
	void processCloseButton(){
		if (hasRunningTask())
			return;
		GUILayout.BeginHorizontal();
		if (GUILayout.Button("Close")){
			Close();
		}
		GUILayout.EndHorizontal();
	}
	
	void terminateBackgroundTasks(){
		if (exportCoroutine != null){
			exportCoroutine.terminate();
			exportCoroutine = null;
		}
		if (exportTask != null){
			exportTask.markRunning(false);
			exportTask.markFinished();
		}
	}
	
	bool hasRunningTask(){
		return (exportTask != null) && (exportTask.running) && !exportTask.finished;
	}
	
	bool exportTaskFinished(){
		return (exportTask != null) && (exportTask.finished);
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
		processCancelButton();
		
		//processTestCoroutine();
		
		GUILayout.EndVertical();
		EditorGUILayout.EndScrollView();
	}
	
	bool canExport(){
		return isValidExportState() && !string.IsNullOrEmpty(targetPath);
	}
	
	bool checkTargetPath(string targetPath){
		if (string.IsNullOrEmpty(targetPath)){
			Debug.LogErrorFormat("No target path, cannot proceed");
			return false;
		}
		return true;
	}
	
	void beginSingleObjectExport(){
		if (Selection.activeObject == null){
			Debug.LogErrorFormat("No active object, cannot proceed with export");
			return;
		}
		var obj = Selection.activeGameObject;
		if (!checkTargetPath(targetPath))
			return;
				
		var proj = JsonProject.fromObject(obj, true);
		exportCoroutine = EditorCoroutine.start(proj.saveToFile(targetPath, true, exportTask));
		//proj.saveToFile(targetPath, true);
	}
	
	void beginSelectedObjectsExport(){
		var objects = ExportUtility.getSelectedGameObjects();
		if (objects.Count <= 0)
			return;

		if (!checkTargetPath(targetPath))
			return;			
				
		var proj = JsonProject.fromObjects(objects.ToArray(), true);
		exportCoroutine = EditorCoroutine.start(proj.saveToFile(targetPath, true, exportTask));
		//proj.saveToFile(targetPath, true);
	}
	
	void beginCurrentSceneExport(){
		var scene = EditorSceneManager.GetActiveScene();
		if (!scene.IsValid()){
			Debug.LogWarningFormat("No active scene, cannot export.");
		}
		
		if (!checkTargetPath(targetPath))
			return;
					
		var proj = JsonProject.fromScene(scene, true);
		exportCoroutine = EditorCoroutine.start(proj.saveToFile(targetPath, true, exportTask));
		//proj.saveToFile(targetPath, true);
	}
	
	void beginFullProjectExport(){
		Debug.LogErrorFormat("Full project export is not currently supported.");
	}
	
	void processExport(){
		terminateBackgroundTasks();
		switch(exportType){
			case(ExportType.CurrentObject):{
				beginSingleObjectExport();
				break;
			}
			case(ExportType.SelectedObjects):{
				beginSelectedObjectsExport();
				break;
			}
			case(ExportType.CurrentScene):{
				beginCurrentSceneExport();
				break;
			}
			case(ExportType.WholeProject):{
				beginFullProjectExport();
				break;
			}
		}		
	}
}

}//namespace SceneExport