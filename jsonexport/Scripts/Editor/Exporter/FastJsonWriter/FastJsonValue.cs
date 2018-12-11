using UnityEngine;
using UnityEditor;
using System.IO;
using System.Text;
using System.Collections;
using System.Collections.Generic;

namespace SceneExport{
	public interface IFastJsonValue{
		void writeRawJsonValue(FastJsonWriter writer);
	}	
}