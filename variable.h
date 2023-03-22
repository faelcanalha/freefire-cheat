#pragma once

#define LOADER_BRAND "_"
#define WIDTH  780
#define HEIGHT 520

// vector animation
float glass_opticaly = 120.0f;
float vector_speed = 0.0f;
int particle_size = 50;

// text animation
float text_animation = 0.00f;
bool text_animation_hide = true;
float loader_animation = 0.f;

bool loader_active = true;

bool particles = true;
bool authenticed;
char id[255] = "";

int tabs = 0;
int active_tab = 0;
float tab_alpha = 0.f;
float tab_add;


float text_animation0 = 0.00f;


bool dns;
bool bypassSS, hideTaskbar, bypassEmulador280, bypassEmulador, antBlacklist, norecoil, aimbotHead, aimbotNeck, aimbotTrick, aimbotScope, aimbotSniper, aimfovLegit, aimfovFull, alokFix64bits, fakedamage, increaseVision, onlyRed, fastSwitchSniper, precision, aimlock, fly, fastMedkit, magicBullets, espName, espLaser, espArrow, AntenaHandFem, AntenaHandMasc, streamMode, PauseAndResume;

namespace var {
	bool bypassSS, hideTaskbar, bypassEmulador280, bypassEmulador, antBlacklist, norecoil, aimbotHead, aimbotNeck, aimbotTrick, aimbotScope, aimbotSniper, aimfovLegit, aimfovFull, alokFix64bits, fakedamage, increaseVision, onlyRed, fastSwitchSniper, precision, aimlock, fly, fastMedkit, magicBullets, espName, espLaser, espArrow, AntenaHandFem, AntenaHandMasc, streamMode, PauseAndResume;
}