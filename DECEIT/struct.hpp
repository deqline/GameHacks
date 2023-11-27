#pragma once


/* DEFINES */
#define EntityManager 0x2EC70A0
#define LocalPlayerManager 0x02EBE858 //or 0x2955D18
#define ResolutionManager 0x2950844
//TODO: fix view projection matrix -> try finding text over player coordinates 
#define ViewProjectionMatrix 0x28BD0D0

#define LobbySize 6

/* GLOBAL VARS */
static DWORD entbase[] = {0x508};
static DWORD playerbase[] = {0x1E0, 0x10};

/* TYPEDEFS */
typedef LONG NTSTATUS;
typedef CLIENT_ID* PCLIENT_ID;

/* USEFUL STRUCTS */
struct Vector4 {
	float x,y,z,w;
};

struct Vector3 {
	float x,y,z;
};

struct Vector2 {
	float x, y;
};

struct Mat4x4 {
	float m[4][4] = {{},{},{},{}};	 
};

//TODO: find player name by pointer scanning with base address
// Generated using ReClass 2016
class CPlayerEnt
{
public:
	void* vTable;

	Vector3 FeetPos; //0x0008 
	Vector3 HeadPos; //0x0014 
private:
	char pad_0x0020[0x2C]; //0x0020
public:
	__int16 N000000AB; //0x004C 
	__int16 N000001C4; //0x004E 
private:
	char pad_0x0050[0x3C]; //0x0050
public:
	__int16 N000000BB; //0x008C 
private:
	char pad_0x008E[0x22]; //0x008E
public:
	Vector3 Position; //0x00B0 
private:
	char pad_0x00BC[0x8]; //0x00BC
public:
	Vector3 CameraLookAt; //0x00C4 
private:
	char pad_0x00D0[0x18]; //0x00D0
public:
	__int32 N00000027; //0x00E8 
private:
	char pad_0x00EC[0x4]; //0x00EC
public:
	__int32 N00000028; //0x00F0 
private:
	char pad_0x00F4[0x14]; //0x00F4
public:
	__int32 DeathFlag2; //0x0108  if down 19456 else 16384 -> good flag 
private:
	char pad_0x010C[0x24]; //0x010C
public:
	__int32 TeamFlag1; //0x0130 
	__int32 TeamFlag2; //0x0134 
	__int32 HealthMaybe; //0x0138 
private:
	char pad_0x013C[0x3C]; //0x013C
public:
	Vector3 MoveAmplitude; //0x0178 
	Vector3 MoveAmplitude2; //0x0184 
private:
	char pad_0x0190[0x14]; //0x0190
public:
	Vector3 N00000102; //0x01A4 
private:
	char pad_0x01B0[0x4]; //0x01B0
public:
	Vector3 N00000106; //0x01B4 
private:
	char pad_0x01C0[0x10]; //0x01C0
public:
	float CrouchSlide; //0x01D0 
	float PlayerDistanceFromGround; //0x01D4 
private:
	char pad_0x01D8[0x20]; //0x01D8
public:
	__int32 JumpFlag1; //0x01F8 
	__int32 JumpFlag2; //0x01FC when -1 jump else 1
	__int32 TeamFlag3; //0x0200 
private:
	char pad_0x0204[0xD0]; //0x0204
public:
	Vector2 RelativePosition; //0x02D4 
private:
	char pad_0x02DC[0x164]; //0x02DC

}; //Size=0x0440


/* FUNCTION DECLARATIONS */
extern "C" NTSTATUS NTAPI NtOpenProcess	(	OUT PHANDLE 	ProcessHandle,
IN ACCESS_MASK 	DesiredAccess,
IN POBJECT_ATTRIBUTES 	ObjectAttributes,
IN PCLIENT_ID 	ClientId 
);	

extern "C" NTSTATUS NTAPI NtReadVirtualMemory	(	IN HANDLE 	ProcessHandle,
IN PVOID 	BaseAddress,
OUT PVOID 	Buffer,
IN SIZE_T 	NumberOfBytesToRead,
OUT PSIZE_T NumberOfBytesRead 	OPTIONAL 
);	
