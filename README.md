# ProjectExodus

## Description

"ProjectExodus" is a conversion tool intended to convert individual scenes and whole projects from Unity to Unreal engine. The conversion focus solely on assets, ignoring all the game logic (C#) and limitations apply. 
The project is available under BSD-3 license, and constists of two plugins - one for unity and one for unreal, and also includes very basic demo scene for unity. 

## Disclaimer/Warning

This document is subject to change, and due to the project being developed, may be altered on short notice.

## Files and Folders

* SampleScene - this is a simple test for unity. Simply copy the folder somewhere within your "assets".
* "ExodusExport" folder - This is a set of scripts used for the conversion itself. Copy the folder into somepoint of your unity project to use, and wait for compilation to finish.
* "ExodusImport" folder - This is for unreal side. Within your project, create "Plugins" folder, and copy this folder there. Then you'll have to rebuild your project and enable the plugin from plugin settings.
* binaries - this folder contains several precompiled versions but will likely be removed in near future. Please do not rely on it.

## Installation - Unity side

To install exporter plugin into unity project, copy or symlink "ExodusExport" folder into any location within "Assets" folder and wait for scripts to finish compiling. Once compilation is finished the export action would be available 
in context menu within Hierarchy window, or via MainMenu.

## Installation - Unreal side.

* Within your C++ project, create "Plugins" folder if it does not already exist.
* Copy or SymLink "ExodusImport" folder there. 
* Reload the project.
* Go to "Plugins" menu in unreal editor, find "ExodusImport" under "Other" category, and enable it. The project will be restarted.
* Recompile the project if necessary.
* Once successfully installed, "Import" command will be available via button bar above scene view.

Please disregard buttons that are not marked "Import", if such buttons are present. Those are test cases.

## Usage

The exporter is accessible either through rightclick within hierarchy view in Unity, OR through "Migrate to UE4" within main menu of unity.
Following options are available:

* *Export current object* - will export current object only with minimal information about the rest of the scene.
* *Export selected objects* - will export selected objects within current scene
* *Export current scene* - will attempt to export current scene and all objects in it.
* *Export current project* - the plugin will attempt to enumerate all resources within the project and export all of them, including scenes.

Once you selected desired option, you'll be prompted to pick up an empty location for the "project" fuke and exported data. It is a good idea to select an empty folder without anything else in it.
The exported data constists of one "master" file in json format and a folder with similar name. Once project export starts, the plugin will copy and convert relevant data into the destination folder.

On unreal side simply find "Import" button in the tool bar, and select the \*.json file you exported with it. 

## What's supported and limitations

The plugin will rebuild current scene or scenes, will convert static meshes, and will *attempt* to convert terrain, landscapes, and skeletal meshes to unreal format. The plugin will also attempt to recreate materials.

Currently following limitations are in place:

* Only "Standard" and "Standard (Specular setup)" materials are currently supported. However, all parameters of those materials should be supported.
* Static meshes are supported. UV coordinates, vertex positions will be converted to unreal format.
* Light and their parameter conversion is supported.
* Reflection probes are supported.
* The plugin will attempt to transfer flags such as being static, having specific shadowcasting type, etc.
* Surface shaders and custom shaders are **not supported** and cannot be converted. The plugin will attempt to harvest their properties, but if those properties do not match properties of standard material, The material will likely appear blacko n unreal side.
* All texture formats that are not directly supported by unreal engine will be converted to png. Due to the way conversion is handled, minor data loss may occur in the process. As a result, please replace those automatically-converted textures when you can.
* Reflection cubemaps used by reflection probes will be converted by similar process and may have minor dataloss. Consider replacing them when you have opportunity.
* Prefabs are not currently converted into blueprints.
* Empty GameObject nodes that are used for "bookkeeping" purposes will be converted into unreal 4 folders within scene view.
* Due to differences of handling landscapes, 1:1 identical transfer is impossible. Maps used by terrain system will be resampled upon import, and trees will lose custom tint. The plugin will attempt to preserve grass density, but grass clump placement will differ.
* The skinned mesh/character conversion is only partially supported, and upon import character may end up being split into several objects. The plugin will attempt to convert animation clips used by the controller, but will not recreate statemachine. Artifacts are possible in converted character.
* Additional limitations may apply.

Additionally, the file format used for transferring the project is subject to change and should not be used for long term data storage or backup. 
