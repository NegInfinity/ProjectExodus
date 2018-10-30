* # In UE4, Albedo color is set as a param instead of const. for consistency keep it const unless there is a reason for this.
* + DonÅft connect the albedo to Specular. This causes the reflections to be killed in UE4. The default specular value should give similar results to Unity.
* + Color parameters need to be gamma corrected (pow 2.2) for UE4. This is before correction:
* # Ambient Occlusion, roughness and masks in general should not be set to linear space instead of sRGB: - Do not confirm. 
* + Normal map has an intensity slider in Unity. Add a flatten normal function in UE4 with a float to replicate this behavior.
* + Same thing with Occlusion. Add a multiply with a float
* + Texture tiling is not considered. You can use UVEdit material function from Datasmith content to do tiling, mirroring, scaling,Åc etc
* For the wood material the roughness is wrong. In Unity it is driven by the alpha channel of the metallic texture. Not connected in UE4.
* BrushedMetal material, metallic map is not connected in UE4.
* + For Transparent materials, switch the lighting mode to either ÅeSurface Translucency VolumeÅf or ÅeForward ShadingÅf to get better shading that consider reflections.
* + Emission parameters from Unity are not implemented in UE4.
* # Height Map parameter from Unity are not implemented in UE4. Use the ParallaxOcclusionMapping material function. Make sure to branch the shader if it is not set or the parameter is 0 to save performance since this function is expensive.
* # As a general note, you are currently creating a master shader for each material. Since the Standard shader in Unity is an uber shader. ...



* + Opacity should disconnect from albedo if albedo is used for smoothness
# * Metallic texture still nto connected
* + Specular node setup from homam data
* # Parallax


* Roughness values sometimes are not translated correctly. (simple metallic tex 22 in debug scene) 
	^^^^^^^<--Apparently there are TWO values now. Glossiness and GlossMapScale. Needs investigation

* + UV offset need correction as Unity and UE4 has different coordinate system.
* Regarding PNG textures (i.e. the checkerboard). UE4 is similar to photoshop in the sense that the PNG transparency is pre-multiplied by alpha