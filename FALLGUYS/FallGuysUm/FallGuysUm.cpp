#include "includes.hpp"
#include "structs.hpp"
#include "offsets.hpp"
#include "mem.hpp"

constexpr LPCSTR procName = "FallGuys_client_game.exe";
constexpr LPCSTR OptionalDLLName = "GameAssembly.dll";
UINT_PTR MemRequests::lastReadAddress = NULL;

int main()
{
	using namespace MemRequests;
	SetConsoleTitle("Fall Guys Game Engine");
	std::cout << "Starting..." << std::endl;
	LoadLibraryA("user32.dll");

	uint32_t pid = getProcessId(procName);

	if (!pid)
		return NULL;
	std::cout << "PID: " << pid << std::endl;

	ULONG64 module = get_module_base(pid, procName);
	if (module)
	{
		module = get_module_resource(pid, procName, OptionalDLLName);

		BYTE shell_code[] = { 0xC3, 0x90, 0x90, 0x90, 0x90 };
		PatchProtected(pid, (module + UPDATE_CHECK_ROUTINE_RVA), &shell_code, sizeof(shell_code));

		//BYTE shell_code1[] = { 0x90, 0x90 };
		//PatchProtected(pid, (module + SHOULD_IGNORE_COLLISIONS), &shell_code1, sizeof(shell_code1));

		auto PlayerController = NtMultiRead<UINT_PTR>(pid, (module + GAME_MANAGER), { GAME_OFFSETS_CHAIN });
		std::cout << "-> PlayerController: ") << set_hex(PlayerController) << std::endl;

		NtWrite(pid, (PlayerController + (DWORD)normalMaxSpeed), (UINT_PTR)(float*)&hackValues::normalMaxSpeed, sizeof(UINT_PTR));
		NtWrite(pid, (PlayerController + (DWORD)rollingMaxSpeed), (UINT_PTR)(float*)&hackValues::rollingMaxSpeed, sizeof(UINT_PTR));
		//NtWrite(pid, (PlayerController + (DWORD)rollingInAirMaxSpeed), (UINT_PTR)(float*)&hackValues::rollingInAirMaxSpeed, sizeof(UINT_PTR));
		NtWrite(pid, (PlayerController + (DWORD)jumpForce), (UINT_PTR)(float*)&hackValues::jumpForce, sizeof(UINT_PTR));
		NtWrite(pid, (PlayerController + (DWORD)grabCooldown), (UINT_PTR)(float*)&hackValues::grabCooldown, sizeof(UINT_PTR));
		NtWrite(pid, (PlayerController + (DWORD)getUpDelay), (UINT_PTR)(float*)&hackValues::getUpDelay, sizeof(UINT_PTR));
		//NtWrite(pid, (PlayerController + (DWORD)getUpBlendOutRagdollDuration), (UINT_PTR)(float*)&hackValues::getUpBlendOutRagdollDuration, sizeof(UINT_PTR));
		NtWrite(pid, (PlayerController + (DWORD)dynamicCollisionStunForce), (UINT_PTR)(float*)&hackValues::dynamicCollisionStunForce, sizeof(UINT_PTR));
		NtWrite(pid, (PlayerController + (DWORD)anyCollisionStunForce), (UINT_PTR)(float*)&hackValues::anyCollisionStunForce, sizeof(UINT_PTR));
		NtWrite(pid, (PlayerController + (DWORD)collisionPlayerToPlayerUnpinMultiplier), (UINT_PTR)(float*)&hackValues::collisionPlayerToPlayerUnpinMultiplier, sizeof(UINT_PTR));
		//NtWrite((PlayerController + (DWORD)_CHEAT_STRUCT::maxGravityVelocity), (UINT_PTR)(float*)&hackValues::maxGravityVelocity, sizeof(UINT_PTR));// < --crashes
		//NtWrite((PlayerController + (DWORD)_CHEAT_STRUCT::gravityScale), (UINT_PTR)(float*)&hackValues::gravityScale, sizeof(UINT_PTR));
		std::cout << "\nSuccess!") << std::endl;
	}
	std::cout << "Closing in 5 seconds") << std::endl;
	Sleep(5000);
	return NULL;
}