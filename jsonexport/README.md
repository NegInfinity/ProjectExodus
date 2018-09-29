# README #

This is unity part of ProjectExodus.

Json Exporter is present in Editor/ExportScripts folder.
The export utility was designed to transfer unity engine scene into Unreal 4.

The contents of this folder are to be dropped into "Assets" folder of you project.

### using exporter ###

To export a scene, objects you want to export must be parented to some sort of "root" object.
To export, right click on the root object in hieararchy window, and select either "export selected objects" or 
"export current object". This will open "safe file" dialog. Resulting json file should be placed into project's root 
(i.e. into a folder that contains "Assets" subfolder).

For additional information check readme of "JsonImporter" project. There are couple of gotchas, for example, the script 
will not automatically convert *.tif textures into png, and you should do it yourself.
