#pragma once

typedef struct _READ_WRITE_REQUEST {
	BOOLEAN read;
	PVOID bufferAddy;
	UINT_PTR addy;
	ULONGLONG size;
	DWORD pid;
	PVOID output;
	BOOLEAN BaseRequest;
	const char* module_name;
	ULONG64 base_address;
	const char* optionalResourceName;
	BOOLEAN readOnly;
} READ_WRITE_REQUEST, * PREAD_WRITE_REQUEST;

struct HandleDisposer {
	using pointer = HANDLE;
	void operator()(HANDLE handle) const {
		if (handle != 0)
		{
			CloseHandle(handle);
		}
	}
};

struct hackValues {
	static constexpr float normalMaxSpeed = 50.0f;
	static constexpr float rollingMaxSpeed = 20.0f;
	static constexpr float rollingInAirMaxSpeed = 20.0f;
	static constexpr float maxGravityVelocity = 40.0f;
	static constexpr float jumpForce = 40.0f; // crash if over 50
	static constexpr float gravityScale = 70.0f;
	static constexpr float grabCooldown = 0.0f;
	static constexpr float getUpDelay = 0.0f;
	static constexpr float getUpBlendOutRagdollDuration = 0.0f;
	static constexpr float dynamicCollisionStunForce = 0.0f;
	static constexpr float anyCollisionStunForce = 0.0f;
	static constexpr float collisionPlayerToPlayerUnpinMultiplier = 0.0f;
};

//if scoped enum use keyword class!
enum _CHEAT_STRUCT {
	normalMaxSpeed = 0x18,//Float <-- crashes over 50.0f?
	rollingMaxSpeed = 0x1C,//Float
	rollingInAirMaxSpeed = 0x24,//Float
	gravityScale = 0xB8,//Float	<-- might crash if too high
	maxGravityVelocity = 0xBC,//Float
	jumpForce = 0xC8,//float
	grabCooldown = 0x17C,//Float
	dynamicCollisionStunForce = 0x318,//Float
	anyCollisionStunForce = 0x31C,//Float
	stunnedFlailRagdollBlend = 0x350,//Float
	getUpDelay = 0x360,//Float
	getUpBlendOutRagdollDuration = 0x364,//Float
	SmallImpactMinForceThreshold = 0x3D0,//Float
	MediumImpactMinForceThreshold = 0x3D4,//Float
	LargeImpactMinForceThreshold = 0x3D8,//Float
	collisionPlayerToPlayerUnpinMultiplier = 0x324
};