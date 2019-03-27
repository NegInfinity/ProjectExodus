using UnityEngine;
using System.Collections.Generic;

namespace SceneExport{
	[System.Serializable]
	public class JsonObjectReference<ObjectType>: IFastJsonValue where ObjectType: Object{
		public ObjectType obj;
		public int instId;// = -1;
		public bool isNull;// = false;

		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("instanceId", instId);
			writer.writeKeyVal("isNull", isNull);
			writer.endObject();
		}

		public JsonObjectReference(ObjectType obj_){
			obj = obj_;
			if (!obj){
				instId = 0;
				isNull = true;
			}
			else{
				instId = obj.GetInstanceID();
				isNull = false;
			}
		}
	}

	public class JsonPhysicsJoint: IFastJsonValue{
		public Joint joint = null;
		public string jointType;

		public static readonly string springJointType = "spring";
		public static readonly string hingeJointType = "hinge";
		public static readonly string characterJointType = "character";
		public static readonly string fixedJointType = "fixed";
		public static readonly string configurableJointType = "configurable";

		public JsonObjectReference<Rigidbody> connectedBody = new JsonObjectReference<Rigidbody>(null);	
		public JsonObjectReference<GameObject> connectedBodyObject = new JsonObjectReference<GameObject>(null);	

		public class HingeJointData: IFastJsonValue{
			public HingeJoint joint;

			public void writeRawJsonValue(FastJsonWriter writer){
				writer.beginRawObject();
				writer.writeKeyVal("angle", joint.angle);
				writer.writeKeyVal("limits", joint.limits);
				writer.writeKeyVal("motor", joint.motor);
				writer.writeKeyVal("spring", joint.spring);
				writer.writeKeyVal("useLimits", joint.useLimits);
				writer.writeKeyVal("useMotor", joint.useMotor);
				writer.writeKeyVal("useSpring", joint.useSpring);
				writer.endObject();
			}

			public HingeJointData(HingeJoint joint_){
				if (!joint_)
					throw new System.ArgumentNullException("joint_");
				joint = joint_;
			}
		}

		public class SpringJointData: IFastJsonValue{
			public SpringJoint joint;

			public void writeRawJsonValue(FastJsonWriter writer){
				writer.beginRawObject();
				writer.writeKeyVal("damper", joint.damper);
				writer.writeKeyVal("maxDistance", joint.maxDistance);
				writer.writeKeyVal("minDistance", joint.minDistance);
				writer.writeKeyVal("spring", joint.spring);
				writer.writeKeyVal("tolerance", joint.tolerance);
				writer.endObject();
			}
			public SpringJointData(SpringJoint joint_){
				if (!joint_)
					throw new System.ArgumentNullException("joint_");
				joint = joint_;				
			}
		}

		public class CharacterJointData: IFastJsonValue{
			public CharacterJoint joint;
			public void writeRawJsonValue(FastJsonWriter writer){
				writer.beginRawObject();
				writer.writeKeyVal("enableProjection", joint.enableProjection);
				writer.writeKeyVal("highTwistLimit", joint.highTwistLimit);
				writer.writeKeyVal("lowTwistLimit", joint.lowTwistLimit);
				writer.writeKeyVal("projectionAngle", joint.projectionAngle);
				writer.writeKeyVal("projectionDistance", joint.projectionDistance);
				writer.writeKeyVal("swing1Limit", joint.swing1Limit);
				writer.writeKeyVal("swing2Limit", joint.swing2Limit);
				writer.writeKeyVal("swingAxis", joint.swingAxis);
				writer.writeKeyVal("swingLimitSpring", joint.swingLimitSpring);
				writer.writeKeyVal("twistLimitSpring", joint.twistLimitSpring);
				writer.endObject();
			}
			public CharacterJointData(CharacterJoint joint_){
				if (!joint_)
					throw new System.ArgumentNullException("joint_");
			}
		}

		public class ConfigurableJointData: IFastJsonValue{
			public ConfigurableJoint joint;
			public void writeRawJsonValue(FastJsonWriter writer){
				writer.beginRawObject();
				writer.endObject();
			}
			public ConfigurableJointData(ConfigurableJoint joint_){				
				if (!joint_)
					throw new System.ArgumentNullException("joint_");
			}
		}

		List<SpringJointData> springJointData = new List<SpringJointData>();
		List<HingeJointData> hingeJointData = new List<HingeJointData>();
		List<ConfigurableJointData> configurableJointData = new List<ConfigurableJointData>();
		List<CharacterJointData> characterJointData = new List<CharacterJointData>();

		public void writeRawJsonValue(FastJsonWriter writer){
			writer.beginRawObject();
			writer.writeKeyVal("jointType", jointType);

			writer.writeKeyVal("anchor", joint.anchor);
			writer.writeKeyVal("connectedAnchor", joint.connectedAnchor);
			writer.writeKeyVal("connectedBody", connectedBody);
			writer.writeKeyVal("connectedBodyObject", connectedBodyObject);
			writer.writeKeyVal("axis", joint.axis);
			writer.writeKeyVal("breakForce", joint.breakForce.ToString());
			writer.writeKeyVal("breakTorque", joint.breakTorque.ToString());
			writer.writeKeyVal("connectedMassScale", joint.connectedMassScale);
			writer.writeKeyVal("enableCollision", joint.enableCollision);
			writer.writeKeyVal("autoConfigureConnectedAnchor", joint.autoConfigureConnectedAnchor);

			writer.writeKeyVal("springJointData", springJointData, true);
			writer.writeKeyVal("hingeJointData", hingeJointData, true);
			writer.writeKeyVal("configurableJointData", configurableJointData, true);
			writer.writeKeyVal("characterJointData", characterJointData, true);

			writer.endObject();
		}

		public JsonPhysicsJoint(Joint joint_){
			if (!joint_)
				throw new System.ArgumentNullException("joint_");
			joint = joint_;

			connectedBody = new JsonObjectReference<Rigidbody>(joint.connectedBody);
			connectedBodyObject = joint.connectedBody ? 
				new JsonObjectReference<GameObject>(joint.connectedBody.gameObject)
				: new JsonObjectReference<GameObject>(null);

			var fixedJoint = joint as FixedJoint;
			var hingeJoint = joint as HingeJoint;
			var springJoint = joint as SpringJoint;
			var characterJoint = joint as CharacterJoint;
			var configurableJoint = joint as ConfigurableJoint;			

			if (fixedJoint){
				jointType = fixedJointType;
			}
			if (hingeJoint){
				jointType = hingeJointType;
				hingeJointData.Add(new HingeJointData(hingeJoint));
			}
			if (springJoint){
				jointType = springJointType;
				springJointData.Add(new SpringJointData(springJoint));
			}
			if (characterJoint){
				jointType = characterJointType;
				characterJointData.Add(new CharacterJointData(characterJoint));
			}
			if (configurableJoint){
				jointType = configurableJointType;
				configurableJointData.Add(new ConfigurableJointData(configurableJoint));
			}
		}
	}
}
