using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

namespace SceneExport{
	public partial class Exporter{
		[System.Serializable]
		public class JsonScene{
			public List<JsonGameObject> objects = new List<JsonGameObject>();
			public List<JsonMaterial> materials = new List<JsonMaterial>();
			public List<JsonMesh> meshes = new List<JsonMesh>();
			public List<JsonTexture> textures = new List<JsonTexture>();
			public List<string> resources = new List<string>();
			
			public void saveToFile(string filename){
				Utility.saveStringToFile(filename, toJsonString());							
			}

			public string toJsonString(){
				FastJsonWriter writer = new FastJsonWriter();
				writer.beginDocument();
				writer.beginKeyArray("objects");
				foreach(var cur in objects){
					writer.beginObjectValue();
					writer.writeKeyVal("name", cur.name);
					writer.writeKeyVal("instanceId", cur.instanceId);
					writer.writeKeyVal("id", cur.id);
					writer.writeKeyVal("localPosition", cur.localPosition);
					writer.writeKeyVal("localRotation", cur.localRotation);
					writer.writeKeyVal("localScale", cur.localScale);
					writer.writeKeyVal("worldMatrix", cur.worldMatrix);
					writer.writeKeyVal("localMatrix", cur.localMatrix);
					writer.writeKeyVal("children", cur.children);
					writer.writeKeyVal("parent", cur.parent);
					writer.writeKeyVal("mesh", cur.mesh);
					writer.writeKeyVal("isStatic", cur.isStatic);
					writer.writeKeyVal("lightMapStatic", cur.lightMapStatic);
					writer.writeKeyVal("navigationStatic", cur.navigationStatic);
					writer.writeKeyVal("occluderStatic", cur.occluderStatic);
					writer.writeKeyVal("occludeeStatic", cur.occludeeStatic);
					writer.writeKeyVal("reflectionProbeStatic", cur.reflectionProbeStatic);
					
					writer.writeKeyVal("nameClash", cur.nameClash);
					writer.writeKeyVal("uniqueName", cur.uniqueName);
					
					writer.beginKeyArray("renderer");
					if (cur.renderer != null){
						foreach(var r in cur.renderer){
							writer.beginObjectValue();
							writer.writeKeyVal("lightmapIndex", r.lightmapIndex);
							writer.writeKeyVal("shadowCastingMode", r.shadowCastingMode);
							writer.writeKeyVal("lightmapScaleOffset", r.lightmapScaleOffset);
							writer.writeKeyVal("materials", r.materials);
							writer.writeKeyVal("receiveShadows", r.receiveShadows);
							writer.endObject();
						}
					}
					writer.endArray();
					writer.beginKeyArray("light");
					if (cur.light != null){
						foreach(var l in cur.light){
							writer.beginObjectValue();
							writer.writeKeyVal("bounceIntensity", l.bounceIntensity);
							writer.writeKeyVal("color", l.color);
							writer.writeKeyVal("intensity", l.intensity);
							writer.writeKeyVal("range", l.range);
							writer.writeKeyVal("renderMode", l.renderMode);
							writer.writeKeyVal("shadows", l.shadows);
							writer.writeKeyVal("shadowStrength", l.shadowStrength);
							writer.writeKeyVal("spotAngle", l.spotAngle);
							writer.writeKeyVal("type", l.type);
							writer.endObject();
						}
					}
					writer.endArray();
					writer.endObject();
				}
				writer.endArray();
				writer.beginKeyArray("materials");
				foreach(var cur in materials){
					writer.beginObjectValue();
					writer.writeKeyVal("id", cur.id);
					writer.writeKeyVal("renderQueue", cur.renderQueue);
					writer.writeKeyVal("name", cur.name);
					writer.writeKeyVal("path", cur.path);
					writer.writeKeyVal("shader", cur.shader);
					//writer.writeKeyVal("renderType", cur.renderType);
					writer.writeKeyVal("mainTexture", cur.mainTexture);
					writer.writeKeyVal("mainTextureOffset", cur.mainTextureOffset);
					writer.writeKeyVal("mainTextureScale", cur.mainTextureScale);
					writer.writeKeyVal("color", cur.color);
					writer.writeKeyVal("useNormalMap", cur.useNormalMap);
					writer.writeKeyVal("useAlphaTest", cur.useAlphaTest);
					writer.writeKeyVal("useAlphaBlend", cur.useAlphaBlend);
					writer.writeKeyVal("useAlphaPremultiply", cur.useAlphaPremultiply);
					writer.writeKeyVal("useEmission", cur.useEmission);
					writer.writeKeyVal("useParallax", cur.useParallax);
					writer.writeKeyVal("useDetailMap", cur.useDetailMap);
					writer.writeKeyVal("useMetallic", cur.useMetallic);
					writer.writeKeyVal("useSpecular", cur.useSpecular);
					writer.writeKeyVal("albedoTex", cur.albedoTex);
					writer.writeKeyVal("specularTex", cur.specularTex);
					writer.writeKeyVal("metallicTex", cur.metallicTex);
					writer.writeKeyVal("normalMapTex", cur.normalMapTex);
					writer.writeKeyVal("occlusionTex", cur.occlusionTex);
					writer.writeKeyVal("parallaxTex", cur.parallaxTex);
					writer.writeKeyVal("emissionTex", cur.emissionTex);
					writer.writeKeyVal("detailMaskTex", cur.detailMaskTex);
					writer.writeKeyVal("detailAlbedoTex", cur.detailAlbedoTex);
					writer.writeKeyVal("detailNormalMapTex", cur.detailNormalMapTex);
					writer.writeKeyVal("alphaCutoff", cur.alphaCutoff);
					writer.writeKeyVal("smoothness", cur.smoothness);
					writer.writeKeyVal("specularColor", cur.specularColor);
					writer.writeKeyVal("metallic", cur.metallic);
					writer.writeKeyVal("bumpScale", cur.bumpScale);
					writer.writeKeyVal("parallaxScale", cur.parallaxScale);
					writer.writeKeyVal("occlusionStrength", cur.occlusionStrength);
					writer.writeKeyVal("emissionColor", cur.emissionColor);
					writer.writeKeyVal("detailMapScale", cur.detailMapScale);
					writer.writeKeyVal("secondaryUv", cur.secondaryUv);
					writer.endObject();
				}
				writer.endArray();
				writer.beginKeyArray("meshes");
				foreach(var cur in meshes){
					writer.beginObjectValue();
					writer.writeKeyVal("name", cur.name);
					writer.writeKeyVal("id", cur.id);
					writer.writeKeyVal("path", cur.path);
					writer.writeKeyVal("materials", cur.materials);
					writer.writeKeyVal("readable", cur.readable);
					writer.writeKeyVal("vertexCount", cur.vertexCount);
					writer.writeKeyVal("colors", cur.colors);
					writer.writeKeyVal("verts", cur.verts);
					writer.writeKeyVal("normals", cur.normals);
					writer.writeKeyVal("uv0", cur.uv0);
					writer.writeKeyVal("uv1", cur.uv1);
					writer.writeKeyVal("uv2", cur.uv2);
					writer.writeKeyVal("uv3", cur.uv3);
					writer.writeKeyVal("subMeshCount", cur.subMeshCount);
					writer.beginKeyArray("subMeshes");
					foreach(var curSubMesh in cur.subMeshes){
						writer.beginObjectValue();
						writer.writeKeyVal("triangles", curSubMesh.triangles);
						writer.endObject();
					}
					writer.endArray();
					writer.endObject();
				}
				writer.endArray();
				writer.beginKeyArray("textures");
				foreach(var cur in textures){
					writer.beginObjectValue();
					writer.writeKeyVal("name", cur.name);
					writer.writeKeyVal("id", cur.id);
					writer.writeKeyVal("path", cur.path);
					writer.writeKeyVal("filterMode", cur.filterMode);
					writer.writeKeyVal("mipMapBias", cur.mipMapBias);
					writer.writeKeyVal("width", cur.width);
					writer.writeKeyVal("height", cur.height);
					writer.writeKeyVal("wrapMode", cur.wrapMode);
					writer.writeKeyVal("isTex2D", cur.isTex2D);
					writer.writeKeyVal("isRenderTarget", cur.isRenderTarget);
					writer.writeKeyVal("alphaTransparency", cur.alphaTransparency);
					writer.writeKeyVal("anisoLevel", cur.anisoLevel);
					writer.endObject();
				}
				writer.endArray();
				writer.beginKeyArray("resources");
				foreach(var cur in resources){
					writer.writeValue(cur);
				}
				writer.endArray();
				writer.endDocument();
				return writer.getString();
			}
		}
	}
}