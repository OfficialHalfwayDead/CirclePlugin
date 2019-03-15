#include "CirclePlugin.h"
BAKKESMOD_PLUGIN(CirclePlugin, "Dribble in a circle plugin.", "1.0", PLUGINTYPE_FREEPLAY)
#include <numeric>
#include "utils/parser.h"

void CirclePlugin::onLoad()
{
	gui_on = false;
	active = false;
	duration = std::make_shared<float>(60.f);
	guiX = std::make_shared<int>(500);
	guiY = std::make_shared<int>(200);
	cvarManager->registerCvar("circle_duration", "60", "The duration for how long the plugin should measure your dribbles for.", true, true, 0.0f, false, 0.0f, true).bindTo(duration);
	cvarManager->registerCvar("circle_gui_x", "500", "The x distance of the gui from the right border of the screen.", true, true, 0.0f, true, 1920.f, true).bindTo(guiX);
	cvarManager->registerCvar("circle_gui_y", "200", "The y location of the gui.", true, true, 0.0f, true, 1080.0f, true).bindTo(guiY);

	cvarManager->registerNotifier("circle_start", std::bind(&CirclePlugin::Start, this, std::placeholders::_1), "Starts the circle dribble measurement.", PERMISSION_FREEPLAY);
	cvarManager->registerNotifier("circle_stop", std::bind(&CirclePlugin::Cancel, this, std::placeholders::_1), "Starts the circle dribble measurement.", PERMISSION_FREEPLAY);
	cvarManager->registerNotifier("circle_gui", std::bind(&CirclePlugin::ToggleGUI, this, std::placeholders::_1), "Starts the circle dribble measurement.", PERMISSION_FREEPLAY);


}

void CirclePlugin::onUnload()
{
}

void CirclePlugin::Start(std::vector<string> params)
{
	if (!gameWrapper->IsInFreeplay())
	{
		cvarManager->log("Measurement not started because player is not in Freeplay.");
		return;
	}
	if (active)
	{
		this->Cancel(params);
	}
	if (!gui_on)
	{
		gameWrapper->RegisterDrawable(std::bind(&CirclePlugin::OnDraw, this, std::placeholders::_1));
		gui_on = true;
	}
	gameWrapper->HookEvent("Function TAGame.Car_TA.SetVehicleInput", std::bind(&CirclePlugin::Tick, this, std::placeholders::_1));
	time_remaining = *duration;
	rotations = 0.0f;
	prev_yaw = 100000; // real yaw values are only between -32768 and 32767
	speeds.clear();
	avg_speed = 0.0f;
}

void CirclePlugin::Cancel(std::vector<string> params)
{
	cvarManager->log("Aborted measurement.");
	gameWrapper->UnhookEvent("Function TAGame.Car_TA.SetVehicleInput");
	active = false;
}

void CirclePlugin::Tick(string eventName)
{
	static int number = 0;
	++number;
	if (time_remaining <= 0.0f)
	{
		cvarManager->log("Measurement concluded.");
		gameWrapper->UnhookEvent("Function TAGame.Car_TA.SetVehicleInput");
		number = 0;
		active = false;
		return;
	}
	if (!gameWrapper->IsInFreeplay())
	{
		cvarManager->log("Measurement aborted because player is no longer in Freeplay.");
		gameWrapper->UnhookEvent("Function TAGame.Car_TA.SetVehicleInput");
		number = 0;
		active = false;
		return;
	}
	time_remaining -= 1.0f / 120.0f;
	float time_passed = *duration - time_remaining;

	auto server = gameWrapper->GetGameEventAsServer();
	auto car = server.GetCars().Get(0); // safe because inside Car_TA function
	int yaw = car.GetRotation().Yaw;
	if (prev_yaw == 100000)
	{
		prev_yaw = yaw;
		return; // initialize prev yaw on first step
	}
	int diff = yaw - prev_yaw;
	if (diff < -10000) // should definitely be an overflow
	{
		diff = 65536 + diff;
	}
	else if (diff > 10000) // should definitely be an overflow
	{
		diff = -65536 + diff;
	}
	//cvarManager->log(to_string(diff));
	// update rotations
	rotations += ((float)diff) / 65536.0f;
	rotations_per_minute = 60.0f * rotations / time_passed;

	prev_yaw = yaw;
	if (number % 30 == 0)
	{
		auto ball = server.GetBall();
		if (ball.IsNull())
			return;
		speeds.push_back(ball.GetVelocity().magnitude());
		avg_speed = std::accumulate(speeds.begin(), speeds.end(), 0.0f) / speeds.size();
	}
	
}

void CirclePlugin::ToggleGUI(std::vector<string> params)
{
	if (params.size() != 2)
	{
		cvarManager->log("Usage: circle_gui hide|show");
		return;
	}
	if (gui_on && params.at(1).compare("hide") == 0)
	{
		gameWrapper->UnregisterDrawables();
		gui_on = false;
	}
	else if (!gui_on  && params.at(1).compare("show") == 0)
	{
		gameWrapper->RegisterDrawable(std::bind(&CirclePlugin::OnDraw, this, std::placeholders::_1));
		gui_on = true;
	}
		
}

string proper_to_string_with_precision(float value, int n)
{
	float precision = 1.0f;
	for (int i = 0; i < n; ++i)
	{
		precision *= 10.0f;
	}
	return (fabs(value) < 1.0f / precision) ? "0" : to_string_with_precision(round(value * precision) / precision, n);
}

void CirclePlugin::OnDraw(CanvasWrapper cw)
{
	Vector2 canvasSize = cw.GetSize();
	RenderOptions ro;
	ro.currentPosition = { canvasSize.X - *guiX, *guiY };
	ro.boxSize = { 250, 60 };

	cw.SetPosition(ro.currentPosition);
	cw.SetColor(0, 0, 77, 150);
	cw.FillBox(ro.boxSize);
	ro.currentPosition = { ro.currentPosition.X + 8, ro.currentPosition.Y + 5 };
	cw.SetPosition(ro.currentPosition);
	cw.SetColor(255, 255, 255, 255);
	string time = "Time: " + proper_to_string_with_precision(time_remaining, 0) + " s";
	string rotation_info = "Circles: " + proper_to_string_with_precision(rotations, 1) + ", CPM: " + proper_to_string_with_precision(rotations_per_minute, 1);
	string speed_info = "Avg speed: " + proper_to_string_with_precision(avg_speed, 0) + " uu/s";

	cw.DrawString(time);
	ro.currentPosition.Y += ro.textSize;
	cw.SetPosition(ro.currentPosition);

	cw.DrawString(rotation_info);
	ro.currentPosition.Y += ro.textSize;
	cw.SetPosition(ro.currentPosition);

	cw.DrawString(speed_info);
	ro.currentPosition.Y += ro.textSize;
	cw.SetPosition(ro.currentPosition);
}
