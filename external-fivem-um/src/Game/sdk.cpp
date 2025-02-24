#include "sdk.h"

SDK sdk;

bool SDK::InitOffset() {
	// only for version : b2699 coming-soon features detecting version
	world = 0x26684D8;
	replay = 0x20304C8;
	viewport = 0x20D8C90;
	camera = 0x20D9868;

	m_pVehicle = 0xD30;
	m_pInfo = 0x10C8;
	m_pWeaponManager = 0x10D8;

	m_fHealthMax = 0x2A0;
	m_vecVelocity = 0x320;
	m_pBoneList = 0x430;
	m_fArmor = 0x1530;
	m_bPedTask = 0x146B;

	m_vecVehicleVelocity = 0x7F0;
	m_fVehicleEngineHealth = 0x908;

	m_fWeaponSpread = 0x84;
	return true;

}
