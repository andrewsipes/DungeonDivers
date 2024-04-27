#include "PlayerLogic.h"
#include "../Components/Identification.h"
#include "../Components/Physics.h"
#include "../Components/Visuals.h"
#include "../Components/Gameplay.h"
#include "../Entities/Prefabs.h"
#include "../Events/Playevents.h"
#include "../../gateware-main/Gateware.h"
#include "../../flecs-3.1.4/flecs.h"

using namespace ESG; // Example Space Game
using namespace GW::INPUT; // input libs
using namespace GW::AUDIO; // audio libs

// Connects logic to traverse any players and allow a controller to manipulate them
bool ESG::PlayerLogic::Init(std::shared_ptr<flecs::world> _game, std::weak_ptr<const GameConfig> _gameConfig, GW::INPUT::GInput _immediateInput, 
	GW::INPUT::GBufferedInput _bufferedInput, GW::INPUT::GController _controllerInput, GW::AUDIO::GAudio _audioEngine, GW::CORE::GEventGenerator _eventPusher)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;
	immediateInput = _immediateInput;
	bufferedInput = _bufferedInput;
	controllerInput =	_controllerInput;
	audioEngine = _audioEngine;

	// Init any helper systems required for this task
	std::shared_ptr<const GameConfig> readCfg = gameConfig.lock();
	int width = (*readCfg).at("Window").at("width").as<int>();
	float speed = (*readCfg).at("Player1").at("speed").as<float>();
	chargeTime = (*readCfg).at("Player1").at("chargeTime").as<float>();

	playerSystem = game->system<ESG::Player, ESG::World>("Player Move System")
		.iter([this, speed](flecs::iter it, ESG::Player*, ESG::World*)
		{
				for (auto i : it)
				{
					float xaxis = 0, input = 0, zaxis = 0;
					GW::INPUT::GInput t = immediateInput;

					t.GetState(G_KEY_A, input); xaxis -= input;
					t.GetState(G_KEY_D, input); xaxis += input;
					t.GetState(G_KEY_S, input); zaxis -= input;
					t.GetState(G_KEY_W, input); zaxis += input;


					GW::MATH::GVECTORF v = { xaxis * it.delta_time() * speed, 0, zaxis * it.delta_time() * speed};
					auto e = game->lookup("MegaBee");
					e.set<ESG::LastWorld>({ e.get<ESG::World>()->value });
					ESG::World* edit = game->entity(e).get_mut<ESG::World>();

					GW::MATH::GMatrix::TranslateLocalF(edit->value, v, edit->value);
				}
				//std::cout << "X: " << e.get<ESG::World>()->value.row4.x << "Y: " << e.get<ESG::World>()->value.row4.z << std::endl;
		});


	// Create an event cache for when the spacebar/'A' button is pressed
	pressEvents.Create(Max_Frame_Events); // even 32 is probably overkill for one frame
		
	// register for keyboard and controller events
	bufferedInput.Register(pressEvents);
	controllerInput.Register(pressEvents);

	// create the on explode handler
	onExplode.Create([this](const GW::GEvent& e) 
	{
		ESG::PLAY_EVENT event; ESG::PLAY_EVENT_DATA eventData;
		if (+e.Read(event, eventData)) 
		{
			// only in here if event matches
			std::cout << "Enemy Was Destroyed!\n";
		}
	});
	_eventPusher.Register(onExplode);

	return true;
}

// Free any resources used to run this system
bool ESG::PlayerLogic::Shutdown()
{
	playerSystem.destruct();
	game.reset();
	gameConfig.reset();

	return true;
}

// Toggle if a system's Logic is actively running
bool ESG::PlayerLogic::Activate(bool runSystem)
{
	if (playerSystem.is_alive()) 
	{
		(runSystem) ? 
			playerSystem.enable() 
			: playerSystem.disable();
		return true;
	}
	return false;
}

bool ESG::PlayerLogic::ProcessInputEvents(flecs::world& stage)
{
	// pull any waiting events from the event cache and process them
	GW::GEvent event;
	while (+pressEvents.Pop(event)) 
	{
		bool fire = false;
		GController::Events controller;
		GController::EVENT_DATA c_data;
		GBufferedInput::Events keyboard;
		GBufferedInput::EVENT_DATA k_data;
		// these will only happen when needed
		if (+event.Read(keyboard, k_data)) 
		{
			if (keyboard == GBufferedInput::Events::KEYPRESSED) 
			{
				if (k_data.data == G_KEY_SPACE) 
				{
					fire = true;
					chargeStart = stage.time();
				}
			}
			if (keyboard == GBufferedInput::Events::KEYRELEASED) 
			{
				if (k_data.data == G_KEY_SPACE) 
				{
					chargeEnd = stage.time();
					if (chargeEnd - chargeStart >= chargeTime) 
					{
						fire = true;
					}
				}
			}
		}
		else if (+event.Read(controller, c_data)) 
		{
			if (controller == GController::Events::CONTROLLERBUTTONVALUECHANGED) 
			{
				if (c_data.inputValue > 0 && c_data.inputCode == G_SOUTH_BTN)
					fire = true;
			}
		}

		if (fire) 
		{
			// grab player one and set them to a firing state
			stage.entity("Player One").add<Firing>();
		}
	}
	return true;
}

// play sound and launch two laser rounds
bool ESG::PlayerLogic::FireLasers(flecs::world& stage, Position origin)
{
	// Grab the prefab for a laser round
	flecs::entity bullet;
	RetreivePrefab("Lazer Bullet", bullet);

	origin.value.x -= 0.05f;
	auto laserLeft = stage.entity().is_a(bullet)
		.set<Position>(origin);
	origin.value.x += 0.1f;
	auto laserRight = stage.entity().is_a(bullet)
		.set<Position>(origin);
	// if this shot is charged \
	if (chargeEnd - chargeStart >= chargeTime) 
	{
		chargeEnd = chargeStart;
		laserLeft.set<ChargedShot>({ 2 })
			.set<Material>({1,0,0});
		laserRight.set<ChargedShot>({ 2 })
			.set<Material>({ 1,0,0 });
	}

	// play the sound of the Lazer prefab
	GW::AUDIO::GSound shoot = *bullet.get<GW::AUDIO::GSound>();
	shoot.Play();

	return true;
}