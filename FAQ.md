Frequently Asked Questions:

* Q: "Unity side of the plugin produces a compile error: 
	```error CS1061: Type `SceneExport.MeshUsageFlags' does not contain a definition for `HasFlag' and no extension method `HasFlag' of type `SceneExport.MeshUsageFlags' could be found. Are you missing an assembly reference?```
    A: You're trying to compile the plugin with framework .NET framework version lower than 4.0. Go to `Edit-->Project Settings --> Player --> Configuration --> ScriptingBackend/API Compatibility`, and set Compatibility level to .NET 4 or .NET standard 2.0 (which corresdponds to .NET 4.6.1). This should fix the problem.