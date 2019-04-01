#pragma once

#include "JsonTypes.h"

class JsonObjectReference{
public:
	int instanceId = -1;
	bool isNull = true;

	void load(JsonObjPtr data);
	JsonObjectReference() = default;
	JsonObjectReference(JsonObjPtr data){
		load(data);
	}
};

class JsonSoftJointLimit{
public:
	float limit;
	float spring;

	void load(JsonObjPtr data);
	JsonSoftJointLimit() = default;
	JsonSoftJointLimit(JsonObjPtr data){
		load(data);
	}
};

class JsonSoftJointLimitSpring{
public:
	float spring;
	float damper;

	void load(JsonObjPtr data);
	JsonSoftJointLimitSpring() = default;
	JsonSoftJointLimitSpring(JsonObjPtr data){
		load(data);
	}
};

class JsonJointMotor{
public:
	float targetVelocity;
	float force;
	bool freeSpin;

	void load(JsonObjPtr data);
	JsonJointMotor() = default;
	JsonJointMotor(JsonObjPtr data){
		load(data);
	}
};

class JsonJointDrive{
public:
	float positionSpring;
	float positionDamper;
	float maximumForce;

	void load(JsonObjPtr data);
	JsonJointDrive() = default;
	JsonJointDrive(JsonObjPtr data){
		load(data);
	}
};

class JsonJointLimits{
public:
	float min;
	float max;
	float bounciness;
	float bounceMinVelocity;
	float contactDistance;

	void load(JsonObjPtr data);
	JsonJointLimits() = default;
	JsonJointLimits(JsonObjPtr data){
		load(data);
	}
};

class JsonJointSpring{
public:
	float spring;
	float damper;
	float targetPosition;

	void load(JsonObjPtr data);
	JsonJointSpring() = default;
	JsonJointSpring(JsonObjPtr data){
		load(data);
	}
};

class JsonHingeJointData{
public:
	float angle;
	JsonJointLimits limits;
	JsonJointMotor motor;
	JsonJointSpring spring;
	bool useLimits;
	bool useMotor;
	bool useSpring;

	void load(JsonObjPtr data);
	JsonHingeJointData() = default;
	JsonHingeJointData(JsonObjPtr data){
		load(data);
	}
};

class JsonSpringJointData{
public:
	float damper;
	float maxDistance;
	float minDistance;
	float spring;
	float tolerance;

	void load(JsonObjPtr data);
	JsonSpringJointData() = default;
	JsonSpringJointData(JsonObjPtr data){
		load(data);
	}
};

class JsonCharacterJointData{
public:
	bool enableProjection;
	float highTwistLimit;
	float lowTwistLimit;
	float projectionAngle;
	float projectionDistance;
	JsonSoftJointLimit swing1Limit;
	JsonSoftJointLimit swing2Limit;
	FVector swingAxis;
	JsonSoftJointLimitSpring swingLimitSpring;
	JsonSoftJointLimitSpring twistLimitSpring;

	void load(JsonObjPtr data);
	JsonCharacterJointData() = default;
	JsonCharacterJointData(JsonObjPtr data){
		load(data);
	}
};

class JsonConfigurableJointData{
public:
	JsonJointDrive angularXDrive;
	JsonSoftJointLimitSpring angularXLimitSpring;
	FString angularXMotion;

	JsonSoftJointLimit angularYLimit;
	FString angularYMotion;

	JsonJointDrive angularYZDrive;
	JsonSoftJointLimitSpring angularYZLimitSpring;

	JsonSoftJointLimit angularZLimit;
	FString angularZMotion;

	bool configuredInWorldSpace;
	JsonSoftJointLimit highAngularXLimit;

	JsonSoftJointLimit linearLimit;
	JsonSoftJointLimitSpring linearLimitSpring;

	JsonSoftJointLimit lowAngularXLimit;
	float projectionAngle;
	float projectionDistance;
	FString projectionMode;

	FString rotationDriveMode;
	FVector secondaryAxis;

	JsonJointDrive slerpDrive;
	bool swapBodies;

	FVector targetAngularVelocity;

	FVector targetPosition;
	FQuat targetRotation;
	FVector targetVelocity;

	JsonJointDrive xDrive;
	FString xMotion;

	JsonJointDrive yDrive;
	FString yMotion;

	JsonJointDrive zDrive;
	FString zMotion;

	void load(JsonObjPtr data);
	JsonConfigurableJointData() = default;
	JsonConfigurableJointData(JsonObjPtr data){
		load(data);
	}
};

class JsonPhysicsJoint{
public:
	FString jointType;
	bool isSpringJointType() const{return jointType == "spring";}
	bool isHingeJointType() const{return jointType == "hinge";}
	bool isCharacterJointType() const{return jointType == "character";}
	bool isFixedJointType() const{return jointType == "fixed";}
	bool isConfigurableJointType() const{return jointType == "configurable";}

	FVector anchor;
	FVector connectedAnchor;
	JsonObjectReference connectedBody;
	JsonObjectReference connectedBodyObject;
	FVector axis;
	float breakForce;
	float breakTorque;

	float getUnrealBreakForce() const;
	float getUnrealBreakTorque() const;

	bool isLinearBreakable() const;
	bool isAngularBreakable() const;

	float connectedMassScale;
	bool enableCollision;
	bool autoConfigureConnectedAnchor;

	TArray<JsonHingeJointData> hingeJointData;
	TArray<JsonConfigurableJointData> configurableJointData;
	TArray<JsonSpringJointData> springJointData;
	TArray<JsonCharacterJointData> characterJointData;
	//

	bool isConnectedToWorld() const;
	void load(JsonObjPtr data);
	JsonPhysicsJoint() = default;
	JsonPhysicsJoint(JsonObjPtr data){
		load(data);
	}
};
