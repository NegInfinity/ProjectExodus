# README #

This is a json import plugin for Unreal 4. It is supposed to operate in tandem with Unit 5 scene exporter.

It was designed over course of one week many months ago, and hasn't been tested since. Expect minor bugs

The code was last tested in Unreal engine 4.12.5

### Installation ###

* If you have no "Plugins" folder in your Unreal project, create it.
* Clone the repository into "Plugins/JsonImport" subdirectory of your project. Cloning can be done either via `git submodules` command or via directly cloning into tree. You should be able to download repository archive and unzip it. As long as what you did results in source code being in <YourUnrealProject>/Plugins/JsonImport, it should work.
* Recompile the project (by either pressing "Compile" button in UE4, or rebuilding project solution in visual studio).
* Restart Unreal engine.
* "JsonImport" should appear on viewport's toolbar toolbar. 

### Using the plugin ###
* Press the "Json Import" button.
* File open dialog should appear.
* Navigate to exported *.json file, select it.
* Wait for import process to complete.

### Warnings ###
* The plugin loads the whole import file into memory and constructs temporary structures in RAM. Make sure you have plenty of RAM and are running 64bit system.
* Unity projects frequently use *.tif textures which are not supported by Unreal 4. During import/export process those textures are renamed to *.png, but actual *.tif --> *.png conversion is not done. You're supposed to do it yourself, and it can be performed, for example, using batch conversion option in XNViewMp.
* Non-standard shaders are not supported.
* I tried to replicate look and configuration of unity standard shader using Unreal node setup, but it is possible that I overlooked some parameters. If something doesn't look right, try tweaking it on your own.
* Scenes like blacksmith use heavily customized non-standard shaders, which will not be converted. Only standard shaders are supported.
* Terrain is not supported. Only meshes.
* Exported *.json file does not include textures, and therefore should be placced in root of your unity project. (in the folder with "Assets" folder).
