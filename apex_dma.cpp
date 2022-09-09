#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <random>
#include <chrono>
#include <iostream>
#include <cfloat>
#include "Game.h"
#include <thread>
#include <fcntl.h>
#include <sys/stat.h>

Memory apex_mem;
Memory client_mem;

bool DEBUG_PRINT = false;

bool active = true;
uintptr_t aimentity = 0;
uintptr_t tmp_aimentity = 0;
uintptr_t lastaimentity = 0;
float max = 999.0f;
float max_dist = 200.0f*150.0f;	// ESP & Glow distance in meters (*40)
int localTeamId = 0;
int tmp_spec = 0, spectators = 0;
int tmp_all_spec = 0, allied_spectators = 0;
float max_fov = 2.3f;
float temp_max_fov = 4.5f;
const int toRead = 100;
int aim = 2; 					// 0 = off, 1 = on - no visibility check, 2 = on - use visibility check
int player_glow = 2;			// 0 = off, 1 = on - not visible through walls, 2 = on - visible through walls
float recoil_control = 0.50f;	// recoil reduction by this value, 1 = 100% = no recoil
Vector last_sway = Vector();	// used to determine when to reduce recoil
int last_sway_counter = 0;		// to determine if we might be shooting a semi out rifle so we need to hold to last_sway
bool item_glow = false;
bool firing_range = false;
bool target_allies = false;
int aim_no_recoil = 2;			//  0= normal recoil, 1 = use recoil control, 2 = aiming no recoil // when using recoil control , make sure the aimbot is off
int safe_level = 0;
bool aiming = true;
int smooth = 77;
int bone = 2;
bool walls = true;
bool thirdPerson = false;

bool actions_t = false;
bool aim_t = false;
bool vars_t = false;
bool item_t = false;
bool recoil_t = false;
uint64_t g_Base;
uint64_t c_Base;
bool lock = false;

//recoil control branch related
const char* printPipe = "/tmp/myfifo";	// output pipe
const char* pipeClearCmd = "\033[H\033[2J\033[3J";	// escaped 'clear' command
int shellOut = -1;

float lastvis_aim[toRead];

int aimTime = 0;
int aimTimeLimit = 7;
int value = 1;

/*Random execution*/
int generateRandom(){
	std::srand(static_cast<unsigned int>(std::time(nullptr)));
	return std::rand()%10+30;
}

bool isSingleFireWeapon(uint64_t LocalPlayer)
{	
	uint64_t entitylist = g_Base + OFFSET_ENTITYLIST;
	uint64_t wephandle = 0;
    apex_mem.Read<uint64_t>(LocalPlayer + OFFSET_WEAPON, wephandle);
	wephandle &= 0xffff;
	uint64_t wep_entity = 0;
    apex_mem.Read<uint64_t>(entitylist + (wephandle << 5), wep_entity);
	
    int name_index = 0;
    apex_mem.Read<int>(wep_entity + OFFSET_WEAPON_NAME_INDEX, name_index);
	if( name_index == 105 || name_index == 107 || name_index == 111 || name_index == 112 || name_index == 114 || name_index == 119 || name_index == 121 ){
		/*
		if(name_index == 105) printf("EVA8");
		if(name_index == 107) printf("scout");
		if(name_index == 111) printf("mastiff");
		if(name_index == 112) printf("moza");
		if(name_index == 114) printf("Peacekeeper");
		if(name_index == 119) printf("wing");
		if(name_index == 119) printf("3030");
		*/
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


void ProcessPlayer(Entity& LPlayer, Entity& target, uint64_t entitylist, int index)
{
	int entity_team = target.getTeamId();
	bool obs = target.Observing(entitylist);
	if (obs)
	{
		tmp_spec++;
		return;
	}
	Vector EntityPosition = target.getPosition();
	Vector LocalPlayerPosition = LPlayer.getPosition();
	float dist = LocalPlayerPosition.DistTo(EntityPosition);
	if (dist > max_dist)
	{
		return;
	}

	if (!target.isAlive()) return;

	if (!firing_range && (entity_team < 0 || entity_team>50)) return;
	
	if (!target_allies && (entity_team == localTeamId)) return;
	uint64_t gmode = 0;
	apex_mem.Read<uint64_t>(g_Base + 0x1E07190 + 0x58, gmode);
	if (gmode == 1953394531) {
 		if(entity_team % 2 == localTeamId % 2)
         		return;
	}

	if(aim == 2)
	{
		if((target.lastVisTime() > lastvis_aim[index]))
		{
			float fov = CalculateFov(LPlayer, target);
			if (fov < max)
			{
				max = fov;
				tmp_aimentity = target.ptr;
			}
		}
		else
		{
			if(aimentity==target.ptr)
			{
				aimentity=tmp_aimentity=lastaimentity=0;
			}
		}
	}
	else
	{
		float fov = CalculateFov(LPlayer, target);
		if (fov < max)
		{
			max = fov;
			tmp_aimentity = target.ptr;
		}
	}

	lastvis_aim[index] = target.lastVisTime();
}

void DoActions()
{
	actions_t = true;
	while (actions_t)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		bool tmp_thirdPerson = false;
		while (g_Base!=0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(30));	
			if (thirdPerson && !tmp_thirdPerson)
			{
				apex_mem.Write<int>(g_Base + OFFSET_THIRDPERSON, 1);
				tmp_thirdPerson = true;
			}
			else if (!thirdPerson && tmp_thirdPerson)
			{
				apex_mem.Write<int>(g_Base + OFFSET_THIRDPERSON, -1);
				tmp_thirdPerson = false;
			}

			uint64_t LocalPlayer = 0;
			apex_mem.Read<uint64_t>(g_Base + OFFSET_LOCAL_ENT, LocalPlayer);
			if (LocalPlayer == 0) continue;

			Entity LPlayer = getEntity(LocalPlayer);

			localTeamId = LPlayer.getTeamId();
			if (localTeamId < 0 || localTeamId > 50)
			{
				continue;
			}
			uint64_t entitylist = g_Base + OFFSET_ENTITYLIST;

			uint64_t baseent = 0;
			apex_mem.Read<uint64_t>(entitylist, baseent);
			if (baseent == 0)
			{
				continue;
			}

			max = 999.0f;
			tmp_spec = 0;
			tmp_all_spec = 0;
			tmp_aimentity = 0;
			if (firing_range)
			{
				int c=0;
				for (int i = 0; i < 10000; i++)
				{
					uint64_t centity = 0;
					apex_mem.Read<uint64_t>(entitylist + ((uint64_t)i << 5), centity);
					if (centity == 0) continue;
					if (LocalPlayer == centity) continue;

					Entity Target = getEntity(centity);
					if (!Target.isDummy() && !target_allies)
					{
						continue;
					}
					
					if(player_glow >= 1 && !Target.isGlowing())
					{
						Target.enableGlow();
					}
					else if((player_glow == 0) && Target.isGlowing())
					{
						Target.disableGlow();
					}

					ProcessPlayer(LPlayer, Target, entitylist, c);
					c++;
				}
			}
			else
			{
				for (int i = 0; i < toRead; i++)
				{
					uint64_t centity = 0;
					apex_mem.Read<uint64_t>(entitylist + ((uint64_t)i << 5), centity);
					if (centity == 0) continue;
					if (LocalPlayer == centity) continue;

					Entity Target = getEntity(centity);
					if (!Target.isPlayer())
					{
						continue;
					}

					int entity_team = Target.getTeamId();
					if (!target_allies && (entity_team == localTeamId))
					{
						continue;
					}


					if(player_glow >= 1 && !Target.isGlowing())
					{
						Target.enableGlow();
					}
					else if((player_glow == 0) && Target.isGlowing())
					{
						Target.disableGlow();
					}

					ProcessPlayer(LPlayer, Target, entitylist, i);
				}
			}
			spectators = tmp_spec;
			allied_spectators = tmp_all_spec;
			if (!lock)
				aimentity = tmp_aimentity;
			else
				aimentity = lastaimentity;
		}
	}
	actions_t = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

static void AimbotLoop()
{

	aim_t = true;
	while (aim_t)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		while (g_Base!=0)
		{
			
			if( value%85 > 83){
				std::this_thread::sleep_for(std::chrono::milliseconds(2));
				value = 0;
				bone = 1;
			} else {
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				bone = 2;
				value++;
			}
			if (aim>0)
			{
				uint64_t LocalPlayer = 0;
				apex_mem.Read<uint64_t>(g_Base + OFFSET_LOCAL_ENT, LocalPlayer);
				if (LocalPlayer == 0) continue;
				Entity LPlayer = getEntity(LocalPlayer);
				bool isSFW = isSingleFireWeapon(LocalPlayer);
				if(isSFW){
					aiming = true;
				}else{
					if(LPlayer.isZooming()){				
					aiming = true;
					} else {
						int attackState = 0;
						apex_mem.Read<int>(g_Base + OFFSET_IS_ATTACKING, attackState);
						if(attackState == 108){
							aiming = true;
						} else {
							aiming = false;
						}
					}
				}
				
				if (aimentity == 0 || !aiming)
				{
					lock=false;
					lastaimentity=0;
					continue;
				} 
				lock=true;
				lastaimentity = aimentity;
				Entity target = getEntity(aimentity);

				if (firing_range)
				{
					if (!target.isAlive())
					{
						continue;
					}
				}
				else
				{
					if (!target.isAlive() || target.isKnocked())
					{
						lock=false;
						lastaimentity=0;
						continue;
					}
				}
				temp_max_fov = max_fov;
				int temp_smooth = smooth;
				int temp_bone = bone;
				Vector EntityPosition = target.getPosition();
				Vector LocalPlayerPosition = LPlayer.getPosition();
				float dist = LocalPlayerPosition.DistTo(EntityPosition);
				if (dist < 75.0f) {
					continue;
				} else if (dist < 800.0f) {
					temp_max_fov = max_fov + 2.7f;
					temp_smooth = smooth - 5;
					if(isSFW){
						temp_max_fov = temp_max_fov + 3.7f;
						temp_smooth = temp_smooth - 8;
					}
				} else if (dist < 1350.0f) {
					temp_max_fov = max_fov +1.7f;
					temp_smooth = smooth - 3;
				}
				if(isSFW){
					temp_bone = 2;
				}
				Vector Angles = CalculateBestBoneAim(LPlayer, target, temp_max_fov, temp_bone, temp_smooth, aim_no_recoil);
				if (Angles.x == 0 && Angles.y == 0)
				{
					lock=false;
					lastaimentity=0;
					continue;
				}
				LPlayer.SetViewAngles(Angles);
			}
		}
	}
	aim_t = false;
}

static void PrintVarsToConsole() {
	printf("\n Spectators\t\t\t\t\t\t\t     Glow\n");
	printf("Enemy  Ally   Smooth\t   Aimbot\t     If Spectators\t Items  Players\n");

	// spectators
	printf(" %d\t%d\t", spectators, allied_spectators);

	// smooth
	printf("%d\t", smooth);

	// aim definition
	switch (aim)
	{
	case 0:
		printf("OFF\t\t\t");
		break;
	case 1:
		printf("ON - No Vis-check    ");
		break;
	case 2:
		printf("ON - Vis-check       ");
		break;
	default:
		printf("--\t\t\t");
		break;
	}

	// safe level definition
	switch (safe_level)
	{
	case 0:
		printf("Keep ON\t\t");
		break;
	case 1:
		printf("OFF with enemy\t");
		break;
	case 2:
		printf("OFF with any\t");
		break;
	default:
		printf("--\t\t");
		break;
	}
	
	// glow items + key
	printf((item_glow ? "  ON\t" : "  OFF\t"));

	// glow players + key
	switch (player_glow)
	{
	case 0:
		printf("  OFF\t");
		break;
	case 1:
		printf("ON - without walls\t");
		break;
	case 2:
		printf("ON - with walls\t");
		break;
	default:
		printf("  --\t");
		break;
	}

	// new string
	printf("\nFiring Range\tTarget Allies\tNo-recoil    Max Distance\n");

	// firing range + key
	printf((firing_range ? "   ON\t\t" : "   OFF\t\t"));

	// target allies + key
	printf((target_allies ? "   ON\t" : "   OFF\t"));

	// recoil + key
	switch (aim_no_recoil)
	{
	case 0:
		printf("\t  OFF\t");
		break;
	case 1:
		printf("\t  RCS\t");
		break;
	case 2:
		printf("\t  ON\t");
		break;
	default:
		printf("  --\t");
		break;
	}


	// distance
	printf("\t%d\n\n", (int)max_dist);
	printf("\t%lf\n\n", (float)max_fov);
}

static void item_glow_t()
{
	item_t = true;
	while (item_t)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		int k = 0;
		while(g_Base!=0 && c_Base!=0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			uint64_t entitylist = g_Base + OFFSET_ENTITYLIST;
			if (item_glow)
			{
				for (int i = 0; i < 10000; i++)
				{
					uint64_t centity = 0;
					apex_mem.Read<uint64_t>(entitylist + ((uint64_t)i << 5), centity);
					if (centity == 0) continue;
					Item item = getItem(centity);

					if(item.isItem() && !item.isGlowing())
					{
						item.enableGlow();
					}
				}
				k=1;
				std::this_thread::sleep_for(std::chrono::milliseconds(600));
			}
			else
			{		
				if(k==1)
				{
					for (int i = 0; i < 10000; i++)
					{
						uint64_t centity = 0;
						apex_mem.Read<uint64_t>(entitylist + ((uint64_t)i << 5), centity);
						if (centity == 0) continue;

						Item item = getItem(centity);

						if(item.isItem() && item.isGlowing())
						{
							item.disableGlow();
						}
					}
					k = 0;
				}
			}
		}
	}
	item_t = false;
}

static void RecoilLoop()
{
	recoil_t = true;
	while (recoil_t)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		while(g_Base!=0 && c_Base!=0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(12));
			if (aim_no_recoil == 1)
			{
				uint64_t LocalPlayer = 0;
				apex_mem.Read<uint64_t>(g_Base + OFFSET_LOCAL_ENT, LocalPlayer);

				if (LocalPlayer == 0) continue;
				last_sway_counter++;
				if (last_sway_counter > 10000)
					last_sway_counter = 86;

				int attackState = 0;
				apex_mem.Read<int>(g_Base + OFFSET_IS_ATTACKING, attackState);

				if (attackState != 5) {
					if (last_sway.x != 0 && last_sway_counter > 85) {	// ~ about 1 second between shot is considered semi-auto so we keep last_sway
						last_sway.x = 0;
						last_sway.y = 0;
						last_sway_counter = 0;
					}
					continue; // is not firing
				}

				Entity LPlayer = getEntity(LocalPlayer);
				Vector ViewAngles = LPlayer.GetViewAngles();
				Vector SwayAngles = LPlayer.GetSwayAngles();

				// calculate recoil angles
				Vector recoilAngles = SwayAngles - ViewAngles;
				if (recoilAngles.x == 0 || recoilAngles.y == 0 || (recoilAngles.x - last_sway.x) == 0 || (recoilAngles.y - last_sway.y) == 0)
					continue;

				// reduce recoil angles by last recoil as sway is continous
				ViewAngles.x -= ((recoilAngles.x - last_sway.x) * recoil_control);
				ViewAngles.y -= ((recoilAngles.y - last_sway.y) * recoil_control);
				LPlayer.SetViewAngles(ViewAngles);
				last_sway = recoilAngles;
				last_sway_counter = 0;
			}
		}
	}
	recoil_t = false;
}

// Requires an open pipe
static void printToPipe(std::string msg, bool clearShell = false)
{
	char buf[80];
	if (clearShell) {
		strcpy(buf, pipeClearCmd);
		write(shellOut, buf, strlen(buf)+1);
	}
	strcpy(buf, msg.c_str());
	write(shellOut, buf, strlen(buf)+1);
}

static void DebugLoop()
{
	while (DEBUG_PRINT)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		while (g_Base != 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			uint64_t LocalPlayer = 0;
			apex_mem.Read<uint64_t>(g_Base + OFFSET_LOCAL_ENT, LocalPlayer);

			if (LocalPlayer == 0) continue;

			Entity LPlayer = getEntity(LocalPlayer);

			int attackState = 0;
			apex_mem.Read<int>(g_Base + OFFSET_IS_ATTACKING, attackState);
			Vector LocalCamera = LPlayer.GetCamPos();
			Vector ViewAngles = LPlayer.GetViewAngles();
			Vector SwayAngles = LPlayer.GetSwayAngles();

			uint64_t wepHandle = 0;
			apex_mem.Read<uint64_t>(LocalPlayer + OFFSET_WEAPON, wepHandle);
			wepHandle &= 0xffff;
			uint64_t entitylist = g_Base + OFFSET_ENTITYLIST;
			uint64_t wep_entity = 0;
			apex_mem.Read<uint64_t>(entitylist + (wepHandle << 5), wep_entity);
			int ammoInClip = 0;
			apex_mem.Read<int>(wep_entity + OFFSET_AMMO_IN_CLIP, ammoInClip);

			printToPipe("Attack State:\t" + std::to_string(attackState) + "\n", true);
			printToPipe("Local Camera:\t" + std::to_string(LocalCamera.x) + "." + std::to_string(LocalCamera.y) + "." + std::to_string(LocalCamera.z) + "\n");
			printToPipe("View Angles: \t" + std::to_string(ViewAngles.x) + "." + std::to_string(ViewAngles.y) + "." + std::to_string(ViewAngles.z) + "\n");
			printToPipe("Sway Angles: \t" + std::to_string(SwayAngles.x) + "." + std::to_string(SwayAngles.y) + "." + std::to_string(SwayAngles.z) + "\n");
			printToPipe("Ammo Count:  \t" + std::to_string(ammoInClip)  + "\n");
		}
	}
}

int main(){
	const char* cl_proc = "cleaner.exe";
	const char* ap_proc = "R5Apex.exe";
	//const char* ap_proc = "EasyAntiCheat_launcher.exe";
	PrintVarsToConsole();

	//Client "add" offset
	uint64_t add_off = 0x5650;

	std::thread aimbot_thr;
	std::thread actions_thr;
	std::thread itemglow_thr;
	std::thread vars_thr;
	std::thread recoil_thr;
	std::thread debug_thr;
	
	while(active)
	{
		if(apex_mem.get_proc_status() != process_status::FOUND_READY)
		{
			if(aim_t)
			{
				aim_t = false;
				actions_t = false;
				item_t = false;
				recoil_t = false;
				g_Base = 0;

				aimbot_thr.~thread();
				actions_thr.~thread();
				//itemglow_thr.~thread();
				//recoil_thr.~thread();
				debug_thr.~thread();
				
			}

			std::this_thread::sleep_for(std::chrono::seconds(1));
			printf("Searching for apex process...\n");

			apex_mem.open_proc(ap_proc);

			if(apex_mem.get_proc_status() == process_status::FOUND_READY)
			{
				g_Base = apex_mem.get_proc_baseaddr();
				printf("\nApex process found\n");
				printf("Base: %lx\n", g_Base);

				aimbot_thr = std::thread(AimbotLoop);
				actions_thr = std::thread(DoActions);
				//itemglow_thr = std::thread(item_glow_t);
				//recoil_thr = std::thread(RecoilLoop);

				if (DEBUG_PRINT)
				{
					debug_thr = std::thread(DebugLoop);
					debug_thr.detach();
				}

				aimbot_thr.detach();
				actions_thr.detach();
				//itemglow_thr.detach();
				//recoil_thr.detach();
			}
		}
		else
		{
			apex_mem.check_proc();
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(12));
	}

	return 0;
}