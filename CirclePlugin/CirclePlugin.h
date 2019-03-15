#pragma once
#include "bakkesmod/plugin/bakkesmodplugin.h"
#pragma comment( lib, "bakkesmod.lib" )

class CirclePlugin : public BakkesMod::Plugin::BakkesModPlugin
{
private:
	std::shared_ptr<float> duration;
	std::shared_ptr<int> guiX;
	std::shared_ptr<int> guiY;
	bool gui_on;
	bool active;
	float time_remaining = 0.0f;
	float rotations = 0.0f;
	float rotations_per_minute = 0.0f;
	int prev_yaw;

	std::vector<float> speeds;
	float avg_speed = 0.0f;


public:
	CirclePlugin() = default;
	~CirclePlugin() = default;
	void onLoad();
	void onUnload();
	void Start(std::vector<string> params);
	void Cancel(std::vector<string> params);
	void Tick(string eventName);
	void ToggleGUI(std::vector<string> params);
	void OnDraw(CanvasWrapper cw);
};

struct RenderOptions
{
	Vector2 currentPosition = { 0,0 };
	Vector2 boxSize = { 0, 0 };
	float textSize = 14;
};